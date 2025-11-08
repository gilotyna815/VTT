// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshBaseActor.h"

#include "ReplicatedSDMCActor.generated.h"

class UReplicatedDynamicMeshComponent;
class URuntimeMeshSceneObject;

/**
 * A replicated reimplementation of ADynamicSDMCActor using UReplicatedDynamicMeshComponent.
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API AReplicatedSDMCActor : public ADynamicMeshBaseActor
{
	GENERATED_BODY()
	
public:
	AReplicatedSDMCActor();

	UPROPERTY(Replicated, VisibleAnywhere)
	UReplicatedDynamicMeshComponent* MeshComponent = nullptr;

	UPROPERTY(Replicated)
	URuntimeMeshSceneObject* SceneObject = nullptr;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

protected:
	/**
	 * ADynamicBaseActorAPI
	 */
	virtual void OnMeshEditedInternal() override;

protected:
	virtual void UpdateSDMCMesh();
};
