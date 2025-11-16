// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "TransferComponent/Public/TransferComponent.h" //class UTransferComponent;

#include "VTTPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class VTT_API AVTTPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AVTTPlayerController();

	UTransferComponent* TransferComponent = nullptr;

	virtual void BeginPlay() override;
	virtual void TestTransferComponent();

protected:
	UFUNCTION(Server, Reliable)
	void ClientNotifiesServerTransferCompleted(uint8 transferId);
	void ClientNotifiesServerTransferCompleted_Implementation(uint8 transferId);
	void TransferCompleted(uint64 transferId);
};
