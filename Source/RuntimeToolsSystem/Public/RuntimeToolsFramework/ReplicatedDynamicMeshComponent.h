// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"
#include "ReplicatedDynamicMeshComponent.generated.h"

/**
 * Allows for DynamicMeshComponent to be replicated by AReplicatedSDMCActor.
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API UReplicatedDynamicMeshComponent : public UDynamicMeshComponent
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
