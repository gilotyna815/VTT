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

#pragma once

#include "CoreMinimal.h"
#include <functional>
#include "TransferComponent.generated.h"

enum class ETransferDirection {
    ClientToServer,
    ServerToClient
};

/**
 * Simple struct for keeping track of in flight and completed transfers.
 */
struct Transfer {
public:
    // Input/Output
    uint8* Buffer = nullptr;
    int Length = 0;
    ETransferDirection Direction = ETransferDirection::ClientToServer;
    std::function<void(uint64)> OnTransferCompleted = nullptr;
    uint64 TransferId = 0;

    // Transfer progress
    int BytesTransferred = 0; // Number of bytes transferred so far

    int GetBytesRemaining() {
        return Length - BytesTransferred;
    }
};

/**
 * Facilitates transfering large buffers of data between the client and
 * the server using the built in RPC framework and thus not requiring a
 * separate socket connection to be established. This allows users to reliably send
 * data larger than the usual limit by splitting the transfer up into chunks
 * and adding the chunks to the reliable buffer across multiple ticks as needed.
 *
 * The main interface is SendBufferToServer/SendBufferToClient. After the transfer
 * completes, the caller's callback function is called on the sending side. This makes
 * it easy to have continuation logic inline with the transfer for what to do with the
 * buffer once it gets there. The buffer can be retrieved on the receiving side with
 * ClientGetsCompletedTransfer(...)/ServerGetsCompletedTransfer(...) depending on which
 * side was the receiver. The buffers can be freed when no longer needed using
 * ClientFreesCompletedTransfer(...)/ServerFreesCompletedTransfer. The receiver MUST call
 * the corresponding FreesCompletedTransfer method or they will not get cleaned up until
 * the actor this component is attached to is destroyed.
 *
 * The public interface to this class's methods all have a Client and Server
 * version to be very explicit which side of the connection has the buffer and which side
 * is sending it and each point. Internally they do the same thing plus error checking.
 */
UCLASS()
class TRANSFERCOMPONENT_API UTransferComponent : public UActorComponent {
    GENERATED_BODY()

public:
    UTransferComponent();
    virtual ~UTransferComponent();

    /**
     * Sends the given buffer of the given length to the server and splits it
     * up into chunks if necessary. This method takes an optional function to
     * be called when the transfer is complete and providing the ID of the transfer
     * for context to the caller. This callback function is called on the client.
     * @param buffer The buffer to send from the client to the server. This component owns this buffer and will delete it if the transfer is completed or cancelled.
     * @param length The number of bytes in the buffer.
     * @param onTransferCompleted An optional function to call when this method completes that provides the same transfer ID as an argument that this function returns.
     *                            Note: This is only called on the client.
     * @returns Returns a number that uniquely identifies this transfer and will be provided on onTransferCompleted as well.
     *
     * @remarks There is no error handling on this method because this is a reliable transfer. If anything goes wrong it's because the client was
     *              disconnected from the server and presumably this transfer is no longer needed. In that event, the buffer passed in is still
     *              deleted since it cleans up any in progress transfers when it goes away.
     */
    uint64 SendBufferToServer(uint8* buffer, int length, std::function<void(uint64)> onTransferCompleted = nullptr);

    /**
     * Sends the given buffer of the given length to the client and splits it
     * up into chunks if necessary. This method takes an optional function to
     * be called when the transfer is complete and providing the ID of the transfer
     * for context to the caller. This callback function is called only on the Server.
     * @param buffer The buffer to send from the client to the server. This component owns this buffer and will delete it if the transfer is completed or cancelled.
     * @param length The number of bytes in the buffer.
     * @param onTransferCompleted An optional function to call when this method completes that provides the same transfer ID as an argument that this function returns.
     *                            Note: This is only called on the Server.
     * @returns Returns a number that uniquely identifies this transfer and will be provided on onTransferCompleted as well.
     *
     * @remarks There is no error handling on this method because this is a reliable transfer. If anything goes wrong it's because the client was
     *              disconnected from the server and presumably this transfer is no longer needed. In that event, the buffer passed in is still
     *              deleted since it cleans up any in progress transfers when it goes away.
     */
    uint64 SendBufferToClient(uint8* buffer, int length, std::function<void(uint64)> onTransferCompleted = nullptr);

    //
    // Interface from the server to get/free transfers sent by the client
    //

    /**
     * Called on the server to get a transfer that was sent by a client
     * with the given transferId.
     * @param transferId - The unique ID of the transfer to retrieve.
     * @returns Returns the transfer or nullptr if it is not found.
     */
    Transfer* ServerGetsCompletedTransfer(uint64 transferId);

    /**
     * Called on the server to get a transfer that was sent by a client
     * with the given transferId.
     * @param transferId - The unique ID of the transfer to retrieve.
     * @returns Returns the transfer or nullptr if it is not found.
     */
    Transfer* ClientGetsCompletedTransfer(uint64 transferId);

    /**
     * Called on the server to delete a transfer that is no longer needed.
     * @param transferId - The unique ID of the transfer to delete.
     */
    void ServerFreesCompletedTransfer(uint64 transferId);

    /**
     * Called on the client to delete a transfer that is no longer needed.
     * @param transferId - The unique ID of the transfer to delete.
     */
    void ClientFreesCompletedTransfer(uint64 transferId);

    /**
     * Sets whether verbose logging should be enabled or not.
     * @param isEnabled - True to enable it, false to disable.
     */
    void SetVerboseLoggingEnabled(bool isEnabled) { _verboseLoggingEnabled = isEnabled; }

    /**
     * Called by the server to set the rate limit on both the server and
     * the client. Must be called on the server.
     * @param rateLimitMbps The rate limit for sending buffers in megabits per second. Set to -1 to disable the rate limit.
     */
    void ServerSetsRateLimit(float rateLimitMbps);

    /**
     * Sets the rate limit directly on the client or the server.
     * @param rateLimitMbps The rate limit for sending buffers in megabits per second. Set to -1 to disable the rate limit.
     */
    void SetRateLimit(float rateLimitMbps) { _rateLimitMbps = rateLimitMbps; }

    /**
     * Sets the minimum number of bytes in each chunk that we send (unless the total remaining number of bytes in the transfer is less than this).
     * This helps prevent flooding the reliable buffer with FOutBunch's with 1 byte in them if a low Rate Limit is set and we have a high frame rate,
     * which can slow the transfer speed down to far lower than the set rate limit waiting for acks on all those tiny bunches. The tradeoff here is
     * generally the rate limit is used to prevent jitter when sending large transfers during gameplay because it crowds out other network activity.
     * If we have a large minimum chunk size it could cause jitter again because it will cause bursts of high bandwidth even though the average is
     * still less than the set rate limit.
     * @param minNumBytesPerChunk The minimum number of bytes to allow per transfer (unless there are less than this remaining in the transfer).
     */
    void SetMinimumBytesPerChunk(int32 minNumBytesPerChunk) { _minimumBytesPerChunk = minNumBytesPerChunk; }

public: // UActorComponent interface
    virtual void TickComponent(float deltaSeconds, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;

private: // Network methods
    UFUNCTION(Server, Reliable) // Client->Server
        void ClientSendsChunkToServer(uint64 transferId, const TArray<uint8>& chunk, int totalBytes);

    UFUNCTION(Client, Reliable) // Server->Client
        void ServerSendsChunkToClient(uint64 transferId, const TArray<uint8>& chunk, int totalBytes);

    UFUNCTION(Client, Reliable)
    void ServerSendsRateLimitToClient(float newRateLimitMbps);

private: // Private methods
    /**
     * Checks if any work needs to be done on outgoing transfers
     * and does it.
     */
    void TickTransfers(float deltaSeconds);

    // Network transfer methods used by the public interface above.
    uint64 GetNextTransferId();
    Transfer* GetCompletedTransfer(uint64 transferId);
    void FreeCompletedTransfer(uint64 transferId);
    uint64 SendBuffer(ETransferDirection transferDirection, uint8* buffer, int length, std::function<void(uint64)> onTransferCompleted = nullptr);
    void ReceiveChunkOnReceiver(ETransferDirection direction, uint64 transferId, const TArray<uint8>& chunk, int totalBytes);

    // Helper methods
    int Min(int a, int b) { if (a < b) return a; else return b; }
    int Max(int a, int b) { if (a > b) return a; else return b; }

private: // Transfer state. Note that there are 2 versions of this actor, one on the server and one on the client. So each one has its own outgoing and incoming transfers.
    // Outgoing state
    TQueue<Transfer*> _pendingOutgoingTransfers; // Transfers we haven't sent yet
    Transfer* _outgoingTransfer = nullptr; // Currently outgoing transfer (if any)
    TArray<uint8> _outgoingTransferBuffer; // Keep a buffer around so we don't need to reallocate constantly
    float _accumulatedTickTime = 0; // If we have data to send in TickTransfers but are limited due to minimum packet requirements, still account for that time in the rate limitting by remembering how much tick time didn't send any data for the next tick

    // Incoming state
    TMap<uint64, Transfer*> _completedIncomingTransfers; // When a transfer completes, hang on to it by ID until the owner tells us to delete it.

    // Transfer ID:
    //    "0" is an error, this should be initialized in the constructor to either 1 or 2 depending on if
    //    we're on the client or the server. The client's IDs will always be odd and the server's will always
    //    be even. This guarantees all transfer IDs are unique between the client/server (although for multicast
    //    they will not be unique between clients but that's ok - we can add the client ID to it or something later
    //    if we need to do multicast).
    uint64 _nextTransferId = 0;

    // The current rate limit in megabits per second.
    // Set to <0 if you want to disable it.
    float _rateLimitMbps = -1.0f;

    // Sets the minimum number of bytes to send in each transfer when a rate limit is set
    // (unless there are less than this total remaining in the transfer). This can be used
    // to prevent stacking too many FOutBunch's in the reliable buffer when limiting the
    // transfer rate in high framerate scenarios.
    int32 _minimumBytesPerChunk = 256;

    // Whether you want logs when transfers come in or not
    bool _verboseLoggingEnabled = false;

private: // Constants
    // This is defined in DataChannel.cpp in the engine
    // but is not exposed so it is duplicated here.
    static const int32 NetMaxConstructedPartialBunchSizeBytes = 1024 * 64; // 64KB
};
