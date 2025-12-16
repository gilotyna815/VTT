// Fill out your copyright notice in the Description page of Project Settings.


#include "VTTPlayerController.h"

#include "TransferComponent/Public/TransferComponent.h"
#include "GeneratedMesh.h"

#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "UDynamicMesh.h"
#include "TestDynamicMeshActor.h"

AVTTPlayerController::AVTTPlayerController()
{
	TransferComponent = CreateDefaultSubobject<UTransferComponent>(TEXT("TransferComponent"));
	TransferComponent->SetVerboseLoggingEnabled(true);
}

void AVTTPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

struct FMySaveGameArchive : public FObjectAndNameAsStringProxyArchive
{
	FMySaveGameArchive(FArchive& InInnerArchive) : FObjectAndNameAsStringProxyArchive(InInnerArchive, false)
	{
		ArIsSaveGame = false;
	}
};

void AVTTPlayerController::TestTransferComponent()
{
	if (GetNetMode() == ENetMode::NM_Client)
	{
		// import bunny model
		FString UsePath = TEXT("RuntimeToolsSystem/Bunny.obj");
		if (FPaths::FileExists(UsePath) == false && FPaths::IsRelative(UsePath))
		{
			UsePath = FPaths::ProjectContentDir() + UsePath;
		}

		UGeneratedMesh* ImportMesh = NewObject<UGeneratedMesh>();
		if (ImportMesh->ReadMeshFromFile(UsePath, true) == false)
		{
			ImportMesh->AppendSphere(200, 8, 8);
		}

		UDynamicMesh* ImportDynamicMesh = NewObject<UDynamicMesh>();
		ImportDynamicMesh->ResetToCube();
		ImportDynamicMesh->SetMesh(*ImportMesh->GetMesh().Get());

		// spawn
		ATestDynamicMeshActor* SpawnedActor = (ATestDynamicMeshActor*) GetWorld()->SpawnActor(ATestDynamicMeshActor::StaticClass());
		SpawnedActor->SetDynamicMesh(ImportDynamicMesh);

		// serialize bunny model
		TArray<uint8> BunnyData;
		FMemoryWriter MemoryWriter(BunnyData, true);
		FMySaveGameArchive MyArchive(MemoryWriter);
		ImportDynamicMesh->Serialize(MyArchive);

		// create ///test buffer
		int bufferLength = BunnyData.Num(); /// 1000000;
		uint8* buffer = new uint8[bufferLength];
		for (int i = 0; i < bufferLength; i++)
		{
			buffer[i] = BunnyData[i]; /// i % 16;
		}

		uint64 transferId = TransferComponent->SendBufferToServer(
			buffer,
			bufferLength,
			[this](uint64 completedTransferId)
				{
					UE_LOG(LogTemp, Warning, TEXT("We sent a buffer with id %llu"), completedTransferId);
					ClientNotifiesServerTransferCompleted(completedTransferId);
				}
			);

		UE_LOG(LogTemp, Warning, TEXT("Started sending a buffer with id %llu"), transferId);
	}
}

void AVTTPlayerController::ClientNotifiesServerTransferCompleted_Implementation(uint8 transferId)
{
	UE_LOG(LogTemp, Warning, TEXT("Received buffer with id %llu"), transferId);

	Transfer* Transfer = TransferComponent->ServerGetsCompletedTransfer(transferId);
	uint8* buffer = Transfer->Buffer;
	int length = Transfer->Length;
	UE_LOG(LogTemp, Warning, TEXT("Received buffer of length %d srtarting with %u"), length, &buffer);

	TArray<uint8> BunnyData;
	for (int i = 0; i < length; i++)
	{
		////if (buffer[i] == i % 16)
		////{
		////	//UE_LOG(LogTemp, Warning, TEXT("YAY!"));
		////}
		////else
		////{
		////	UE_LOG(LogTemp, Warning, TEXT("THE BUFFERS ARE NOT THE SAME."));
		////	return;
		////}
		BunnyData.Emplace(buffer[i]);
	}
	///UE_LOG(LogTemp, Warning, TEXT("THE BUFFERS ARE THE SAME."));

	FMemoryReader MemoryReader(BunnyData, true);
	FMySaveGameArchive MyArchive(MemoryReader);
	UDynamicMesh* ImportedBunny = NewObject<UDynamicMesh>();
	ImportedBunny->Serialize(MyArchive);

	// spawn
	ATestDynamicMeshActor* SpawnedActor = (ATestDynamicMeshActor*)GetWorld()->SpawnActor(ATestDynamicMeshActor::StaticClass());
	SpawnedActor->SetDynamicMesh(ImportedBunny);

	// after using buffer, clear it by hand or it will stick around until PlayerController is destroyed
	TransferComponent->ServerFreesCompletedTransfer(transferId);
}

// NEVERMIND, IT NEEDS TO BE CALLED AS AN RPC
void AVTTPlayerController::TransferCompleted(uint64 transferId)
{
	UE_LOG(LogTemp, Warning, TEXT("Received buffer with id %llu"), transferId);

	Transfer* Transfer = nullptr;
	uint8* buffer = nullptr;
	int length;

	switch (GetNetMode())
	{
	case ENetMode::NM_Client:
		break;
	case ENetMode::NM_DedicatedServer:
	case ENetMode::NM_ListenServer:
		Transfer = TransferComponent->ServerGetsCompletedTransfer(transferId);
		buffer = Transfer->Buffer;
		length = Transfer->Length;
		break;
	default:
		return;
		break;
	}
}

