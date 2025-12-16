// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "TransferComponent/Public/TransferComponent.h"
#include "UDynamicMesh.h"

#include "VTTPlayerController.generated.h"

/**
 * Player controller with Transfer Component for sending arbitrary serialized data between players.
 */
UCLASS()
class VTT_API AVTTPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	// For sending arbitrary serialized data between players.
	UTransferComponent* TransferComponent = nullptr;

public:
	AVTTPlayerController();

	virtual void BeginPlay() override;
	virtual void TestTransferComponent();

protected:
	UFUNCTION(Server, Reliable)
	void ClientNotifiesServerTransferCompleted(uint8 transferId);
	void ClientNotifiesServerTransferCompleted_Implementation(uint8 transferId);
	
	void TransferCompleted(uint64 transferId);
};
