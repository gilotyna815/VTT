

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/DynamicMeshComponent.h"

#include "TestDynamicMeshActor.generated.h"

UCLASS()
class VTT_API ATestDynamicMeshActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestDynamicMeshActor();

	UPROPERTY()
	UDynamicMeshComponent* DynamicMeshComponent;

	void SetDynamicMesh(UDynamicMesh* DynamicMesh);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
