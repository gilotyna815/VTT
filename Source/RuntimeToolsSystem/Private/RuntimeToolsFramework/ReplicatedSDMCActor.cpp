// Fill out your copyright notice in the Description page of Project Settings.


#include "RuntimeToolsFramework/ReplicatedSDMCActor.h"

#include "RuntimeToolsFramework/ReplicatedDynamicMeshComponent.h"
#include "MeshScene/RuntimeMeshSceneObject.h"

// Replication
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

// SDMCActor reimplemantation
#include "Operations/MeshConvexHull.h"
#include "ShapeApproximation/SimpleShapeSet3.h"
#include "Physics/ComponentCollisionUtil.h"
#include "MaterialDomain.h"

AReplicatedSDMCActor::AReplicatedSDMCActor()
{
	bReplicates = true;

	MeshComponent = CreateDefaultSubobject<UReplicatedDynamicMeshComponent>(TEXT("MeshComponent"), false);
	SetRootComponent(MeshComponent);

	SceneObject = NewObject<URuntimeMeshSceneObject>();
}

void AReplicatedSDMCActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AReplicatedSDMCActor, SceneObject);
}

void AReplicatedSDMCActor::BeginPlay()
{
	Super::BeginPlay();
}

void AReplicatedSDMCActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AReplicatedSDMCActor::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (IsValid(MeshComponent))
	{
		bWroteSomething |= Channel->ReplicateSubobject(MeshComponent, *Bunch, *RepFlags);
	}

	if (IsValid(SceneObject))
	{
		bWroteSomething |= Channel->ReplicateSubobject(MeshComponent, *Bunch, *RepFlags);
	}

	return bWroteSomething;
}

void AReplicatedSDMCActor::OnMeshEditedInternal()
{
	UpdateSDMCMesh();
	Super::OnMeshEditedInternal();
}

void AReplicatedSDMCActor::UpdateSDMCMesh()
{
	if (MeshComponent)
	{
		*(MeshComponent->GetMesh()) = SourceMesh;

		if (this->CollisionMode == EDynamicMeshActorCollisionMode::ComplexAsSimple
			|| this->CollisionMode == EDynamicMeshActorCollisionMode::ComplexAsSimpleAsync)
		{
			MeshComponent->SetComplexAsSimpleCollisionEnabled(true);
		}
		else if (this->CollisionMode == EDynamicMeshActorCollisionMode::SimpleConvexHull)
		{
			UE::Geometry::FMeshConvexHull HullCompute(&SourceMesh);
			int32 NumTris = FMath::Clamp(this->MaxHullTriangles, 0, 1000);
			if (NumTris != 0)
			{
				HullCompute.bPostSimplify = true;
				HullCompute.MaxTargetFaceCount = NumTris;
			}
			if (HullCompute.Compute())
			{
				UE::Geometry::FSimpleShapeSet3d ShapeSet;
				UE::Geometry::FConvexShape3d& Convex = ShapeSet.Convexes.Emplace_GetRef();
				Convex.Mesh = MoveTemp(HullCompute.ConvexHull);

				MeshComponent->SetComplexAsSimpleCollisionEnabled(false);
				UE::Geometry::SetSimpleCollision(MeshComponent, &ShapeSet);
			}
		}

		MeshComponent->NotifyMeshUpdated();

		// update material
		UMaterialInterface* UseMaterial = (this->Material != nullptr) ? this->Material : UMaterial::GetDefaultMaterial(MD_Surface);
		MeshComponent->SetMaterial(0, UseMaterial);
	}
}
