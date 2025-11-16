/*
 * Copyright © 2023 kirby561
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "TransferComponent.h"
#include "Engine/ActorChannel.h"

DEFINE_LOG_CATEGORY_STATIC(TransferComponentSub, Log, All);

UTransferComponent::UTransferComponent() {
    if (GetNetMode() == ENetMode::NM_Client) {
        _nextTransferId = 1;
    }
    else {
        _nextTransferId = 2;
    }

    PrimaryComponentTick.bCanEverTick = true;
}

UTransferComponent::~UTransferComponent() {
    // Free any pending client transfers
    while (!_pendingOutgoingTransfers.IsEmpty()) {
        Transfer* transfer = nullptr;
        _pendingOutgoingTransfers.Dequeue(transfer);
        delete[] transfer->Buffer;
        delete transfer;
    }

    // Free any completed transfers still being held
    TArray<uint64> completedTransferIds;
    _completedIncomingTransfers.GenerateKeyArray(completedTransferIds);
    for (uint64 transferId : completedTransferIds) {
        Transfer* transfer = _completedIncomingTransfers[transferId];
        delete[] transfer->Buffer;
        delete transfer;
        _completedIncomingTransfers.Remove(transferId);
    }
}

uint64 UTransferComponent::SendBufferToServer(uint8* buffer, int length, std::function<void(uint64)> onTransferCompleted /* = nullptr */) {
    return SendBuffer(ETransferDirection::ClientToServer, buffer, length, onTransferCompleted);
}

uint64 UTransferComponent::SendBufferToClient(uint8* buffer, int length, std::function<void(uint64)> onTransferCompleted /* = nullptr */) {
    return SendBuffer(ETransferDirection::ServerToClient, buffer, length, onTransferCompleted);
}

Transfer* UTransferComponent::ServerGetsCompletedTransfer(uint64 transferId) {
    if (GetNetMode() == ENetMode::NM_Client) {
        UE_LOG(TransferComponentSub, Error, TEXT("ServerGetsCompletedTransfer not called by the server! This is an error."));
        return nullptr;
    }
    return GetCompletedTransfer(transferId);
}

Transfer* UTransferComponent::ClientGetsCompletedTransfer(uint64 transferId) {
    if (GetNetMode() != ENetMode::NM_Client) {
        UE_LOG(TransferComponentSub, Error, TEXT("ClientGetsCompletedTransfer not called by the client! This is an error."));
        return nullptr;
    }
    return GetCompletedTransfer(transferId);
}

void UTransferComponent::ServerFreesCompletedTransfer(uint64 transferId) {
    if (GetNetMode() == ENetMode::NM_Client) {
        UE_LOG(TransferComponentSub, Error, TEXT("ServerFreesCompletedTransfer not called by the server! This is an error."));
        return;
    }
    FreeCompletedTransfer(transferId);
}

void UTransferComponent::ClientFreesCompletedTransfer(uint64 transferId) {
    if (GetNetMode() != ENetMode::NM_Client) {
        UE_LOG(TransferComponentSub, Error, TEXT("ClientFreesCompletedTransfer not called by the client! This is an error."));
        return;
    }
    FreeCompletedTransfer(transferId);
}

void UTransferComponent::ServerSetsRateLimit(float rateLimitMbps) {
    if (GetNetMode() == ENetMode::NM_Client) {
        UE_LOG(TransferComponentSub, Error, TEXT("ServerSetsRateLimit not called by the server! This is an error."));
        return;
    }

    _rateLimitMbps = rateLimitMbps;
    ServerSendsRateLimitToClient(rateLimitMbps);
}

void UTransferComponent::TickComponent(float deltaSeconds, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) {
    Super::TickComponent(deltaSeconds, tickType, thisTickFunction);

    TickTransfers(deltaSeconds);
}

void UTransferComponent::ClientSendsChunkToServer_Implementation(uint64 transferId, const TArray<uint8>& chunk, int totalBytes) {
    ReceiveChunkOnReceiver(ETransferDirection::ClientToServer, transferId, chunk, totalBytes);
}

void UTransferComponent::ServerSendsChunkToClient_Implementation(uint64 transferId, const TArray<uint8>& chunk, int totalBytes) {
    ReceiveChunkOnReceiver(ETransferDirection::ServerToClient, transferId, chunk, totalBytes);
}

void UTransferComponent::ServerSendsRateLimitToClient_Implementation(float newRateLimitMbps) {
    _rateLimitMbps = newRateLimitMbps;
}

void UTransferComponent::TickTransfers(float deltaSeconds) {
    AActor* owner = GetOwner();
    if (owner == nullptr) return; // No owner yet

    UNetConnection* netConnection = owner->GetNetConnection();
    if (netConnection == nullptr) return; // No net connection yet

    // Do we have pending transfers and no current transfer?
    if (_outgoingTransfer == nullptr && !_pendingOutgoingTransfers.IsEmpty()) {
        _pendingOutgoingTransfers.Dequeue(_outgoingTransfer);
    }

    if (_outgoingTransfer != nullptr) {
        // Calculate the limits we need based on the UActorChannel being used for network communication.
        //     The way this works is the actor channel has a queue of "bunches"
        //     that will be sent that are stored in its "reliable buffer". This
        //     reliable buffer can hold RELIABLE_BUFFER (256) bunches. Each bunch
        //     can be a maximum of NetMaxConstructedPartialBunchSizeBytes (64k)
        //     So we limit our packets to 1/2 of the max bunch size (to leave some
        //     room for any overhead of making the bunch) and make sure we're not
        //     using more than half the reliable buffer bunch limit.
        UActorChannel* networkChannel = netConnection->FindActorChannelRef(owner);
        int outputChunkSizeLimit = NetMaxConstructedPartialBunchSizeBytes / 2;
        int numberOfReliableOutputBunchesLimit = RELIABLE_BUFFER / 2;

        // If we are on the client we need to use a different
        // send function than if we are on the server.
        std::function<void(uint64, const TArray<uint8>&, int)> sendChunkMethod = nullptr;
        if (GetNetMode() == ENetMode::NM_Client) {
            sendChunkMethod = [this](uint64 transferId, const TArray<uint8>& chunk, int totalBytes) {
                ClientSendsChunkToServer(transferId, chunk, totalBytes);
            };
        }
        else {
            sendChunkMethod = [this](uint64 transferId, const TArray<uint8>& chunk, int totalBytes) {
                ServerSendsChunkToClient(transferId, chunk, totalBytes);
            };
        }

        // Define the maximum number of bytes we can send this tick
        // to stay under our set rate limit if there is one set.
        int maxBytesPerRateLimit = _outgoingTransfer->Length;
        int bytesTransferredThisTick = 0;
        float totalTimeSinceLastTransfer = deltaSeconds + _accumulatedTickTime;
        if (_rateLimitMbps > 0) {
            float mbpsToBps = 1024 * 1024 / 8.0f;
            maxBytesPerRateLimit = Max(1, (int)(totalTimeSinceLastTransfer * (_rateLimitMbps * mbpsToBps)));
        }

        // Continue this transfer
        while (_outgoingTransfer->BytesTransferred < _outgoingTransfer->Length
            && networkChannel->NumOutRec < numberOfReliableOutputBunchesLimit
            && bytesTransferredThisTick < maxBytesPerRateLimit) {
            // Get the chunk size and copy it to the outgoing transfer buffer
            int chunkSize = Min(_outgoingTransfer->GetBytesRemaining(), outputChunkSizeLimit);

            // If the chunk size goes over our rate limit reduce it so that it doesn't
            int rateLimitBytesRemaining = maxBytesPerRateLimit - bytesTransferredThisTick;
            chunkSize = Min(chunkSize, rateLimitBytesRemaining);

            // If we're rate limiting, only transmit this chunk if it's bigger than our minimum allowed
            // size or if it's the rest of the transfer.
            bool isRateLimiting = _rateLimitMbps > 0;
            bool isLastChunk = chunkSize == _outgoingTransfer->GetBytesRemaining();
            if (!isRateLimiting || chunkSize >= _minimumBytesPerChunk || isLastChunk) {
                _outgoingTransferBuffer.Reset();
                _outgoingTransferBuffer.Append(_outgoingTransfer->Buffer + _outgoingTransfer->BytesTransferred, chunkSize);

                // Send the chunk
                sendChunkMethod(
                    _outgoingTransfer->TransferId,
                    _outgoingTransferBuffer,
                    _outgoingTransfer->Length);

                if (_verboseLoggingEnabled) {
                    UE_LOG(TransferComponentSub, Display, TEXT("Sent chunk of size %d (%d out of %d in total), NumOutRec=%d"), chunkSize, _outgoingTransfer->BytesTransferred, _outgoingTransfer->Length, networkChannel->NumOutRec);
                }

                // Book keeping
                _outgoingTransfer->BytesTransferred += chunkSize;
                bytesTransferredThisTick += chunkSize;
                _accumulatedTickTime = 0; // Reset the accumulated time since last transfer
            }
            else {
                _accumulatedTickTime += deltaSeconds;
                break;
            }
        }

        // Is the transfer complete?
        if (_outgoingTransfer->BytesTransferred >= _outgoingTransfer->Length) {
            if (_outgoingTransfer->BytesTransferred > _outgoingTransfer->Length) {
                // We have a math error somewhere.
                UE_LOG(
                    TransferComponentSub,
                    Warning,
                    TEXT("Too many bytes accounted for when transferring a buffer. Transferred = %d, length = %d, TransferID = %llu"),
                    _outgoingTransfer->BytesTransferred, _outgoingTransfer->Length, _outgoingTransfer->TransferId);
                // Continue with the warning
            }

            // We're done, call the callback
            if (_outgoingTransfer->OnTransferCompleted) {
                _outgoingTransfer->OnTransferCompleted(_outgoingTransfer->TransferId);
            }

            // Cleanup
            delete[] _outgoingTransfer->Buffer;
            delete _outgoingTransfer;
            _outgoingTransfer = nullptr;
        }
    }
}

uint64 UTransferComponent::GetNextTransferId() {
    uint64 nextId = _nextTransferId;
    _nextTransferId += 2;
    return nextId;
}

Transfer* UTransferComponent::GetCompletedTransfer(uint64 transferId) {
    if (_completedIncomingTransfers.Contains(transferId)) {
        Transfer* transfer = _completedIncomingTransfers[transferId];
        return transfer;
    }
    return nullptr; // Not found
}

void UTransferComponent::FreeCompletedTransfer(uint64 transferId) {
    if (_completedIncomingTransfers.Contains(transferId)) {
        Transfer* transfer = _completedIncomingTransfers[transferId];
        _completedIncomingTransfers.Remove(transferId);
        delete[] transfer->Buffer;
        delete transfer;
    }
    else {
        UE_LOG(TransferComponentSub, Warning, TEXT("Tried to free transferId %llu but this transfer does not exist."), transferId);
    }
}

uint64 UTransferComponent::SendBuffer(ETransferDirection transferDirection, uint8* buffer, int length, std::function<void(uint64)> onTransferCompleted /* = nullptr */) {
    // Make sure we are on the client if it's Client->Server and the Server if it's Server->Client
    if (GetNetMode() != ENetMode::NM_Client && transferDirection == ETransferDirection::ClientToServer) {
        UE_LOG(TransferComponentSub, Error, TEXT("SendBufferToServer not called by the client! This is an error."));
        return 0;
    }

    if (GetNetMode() == ENetMode::NM_Client && transferDirection == ETransferDirection::ServerToClient) {
        UE_LOG(TransferComponentSub, Error, TEXT("SendBufferToClient not called by the server! This is an error."));
        return 0;
    }

    // Make a transfer for this
    Transfer* transfer = new Transfer();
    transfer->Buffer = buffer;
    transfer->Length = length;
    transfer->Direction = transferDirection;
    transfer->OnTransferCompleted = onTransferCompleted;
    transfer->TransferId = GetNextTransferId();

    // Add it to the queue
    _pendingOutgoingTransfers.Enqueue(transfer);

    return transfer->TransferId;
}

void UTransferComponent::ReceiveChunkOnReceiver(ETransferDirection direction, uint64 transferId, const TArray<uint8>& chunk, int totalBytes) {
    // Is this transfer in the map yet?
    if (!_completedIncomingTransfers.Contains(transferId)) {
        Transfer* newTransfer = new Transfer();
        newTransfer->Buffer = new uint8[totalBytes];
        newTransfer->Length = totalBytes;
        newTransfer->Direction = direction;
        newTransfer->TransferId = transferId;
        _completedIncomingTransfers.Add(transferId, newTransfer);
    }

    // Copy the chunk into the overall array
    Transfer* transfer = _completedIncomingTransfers[transferId];
    memcpy(transfer->Buffer + transfer->BytesTransferred, chunk.GetData(), chunk.Num());
    transfer->BytesTransferred += chunk.Num();

    if (_verboseLoggingEnabled) {
        UE_LOG(TransferComponentSub, Display, TEXT("Received chunk of size %d (%d out of %d in total)"), chunk.Num(), transfer->BytesTransferred, totalBytes);
    }
}
