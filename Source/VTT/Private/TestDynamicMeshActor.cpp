


#include "TestDynamicMeshActor.h"


// Sets default values
ATestDynamicMeshActor::ATestDynamicMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;

	DynamicMeshComponent = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("DynamicMeshComponent"), false);
}

// Called when the game starts or when spawned
void ATestDynamicMeshActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATestDynamicMeshActor::SetDynamicMesh(UDynamicMesh* DynamicMesh)
{
	DynamicMeshComponent->SetDynamicMesh(DynamicMesh);
}

// Called every frame
void ATestDynamicMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


