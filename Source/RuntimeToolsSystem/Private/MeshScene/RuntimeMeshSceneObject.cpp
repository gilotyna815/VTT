/**
 * Code and comments based on Ryan Schmidt's UE5RuntimeToolsFrameworkDemo obtained on MIT license
 * https://github.com/gradientspace/UE5RuntimeToolsFrameworkDemo
 *
 * MIT License
 *
 * Copyright (c) 2021 gradientspace
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include "MeshScene/RuntimeMeshSceneObject.h"

#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/DynamicMeshAABBTree3.h"

#include "MaterialDomain.h"
#include <Runtime/MeshConversion/Public/MeshDescriptionToDynamicMesh.h>

URuntimeMeshSceneObject::URuntimeMeshSceneObject()
{
	if (!SourceMesh)
	{
		SourceMesh = MakeUnique<UE::Geometry::FDynamicMesh3>();
	}
	if (!MeshAABBTree)
	{
		MeshAABBTree = MakeUnique<UE::Geometry::FDynamicMeshAABBTree3>();
	}

	UMaterialInterface* DefaultMaterial = UMaterial::GetDefaultMaterial(MD_Surface);
	Materials.Add(DefaultMaterial);
}

void URuntimeMeshSceneObject::Initialize(UWorld* TargetWorld, const FMeshDescription* InitialMeshDescription)
{
	FActorSpawnParameters SpawnInfo;
	SimpleDynamicMeshActor = TargetWorld->SpawnActorAbsolute<ADynamicSDMCActor>(FVector::ZeroVector, FRotator(0, 0, 0), SpawnInfo);

	// listen for changes
	SimpleDynamicMeshActor->MeshComponent->OnMeshChanged.AddLambda([this]()
	{
		OnExternalDynamicMeshComponentUpdate();
	});

	GetActor()->SourceType = EDynamicMeshActorSourceType::ExternallyGenerated;
	GetActor()->CollisionMode = EDynamicMeshActorCollisionMode::ComplexAsSimpleAsync;

	UpdateSourceMesh(InitialMeshDescription);

	GetActor()->EditMesh([&](FDynamicMesh3& MeshToEdit)
	{
		MeshToEdit = *SourceMesh;
	});

	UpdateComponentMaterials(false);
}

void URuntimeMeshSceneObject::Initialize(UWorld* TargetWorld, const FDynamicMesh3* InitialMesh)
{
	FActorSpawnParameters SpawnInfo;
	SimpleDynamicMeshActor = TargetWorld->SpawnActor<ADynamicSDMCActor>(FVector::ZeroVector, FRotator(0, 0, 0), SpawnInfo);

	// listen for changes
	SimpleDynamicMeshActor->MeshComponent->OnMeshChanged.AddLambda([this]()
	{
		OnExternalDynamicMeshComponentUpdate();
	});
	
	GetActor()->SourceType = EDynamicMeshActorSourceType::ExternallyGenerated;
	GetActor()->CollisionMode = EDynamicMeshActorCollisionMode::ComplexAsSimpleAsync;
	
	*SourceMesh = *InitialMesh;
	MeshAABBTree->SetMesh(SourceMesh.Get(), true);

	GetActor()->EditMesh([&](FDynamicMesh3& MeshToEdit)
	{
		MeshToEdit = *SourceMesh;
	});

	UpdateComponentMaterials(false);
}

ADynamicMeshBaseActor* URuntimeMeshSceneObject::GetActor()
{
	return SimpleDynamicMeshActor;
}

UMeshComponent* URuntimeMeshSceneObject::GetMeshComponent()
{
	return (SimpleDynamicMeshActor) ? SimpleDynamicMeshActor->MeshComponent : nullptr;
}

void URuntimeMeshSceneObject::UpdateSourceMesh(const FMeshDescription* MeshDescriptionIn)
{
	FMeshDescriptionToDynamicMesh Converter;
	FDynamicMesh3 TmpMesh;
	Converter.Convert(MeshDescriptionIn, TmpMesh);
	*SourceMesh = MoveTemp(TmpMesh);

	MeshAABBTree->SetMesh(SourceMesh.Get(), true);
}

void URuntimeMeshSceneObject::OnExternalDynamicMeshComponentUpdate()
{
	const FDynamicMesh3* Mesh = SimpleDynamicMeshActor->MeshComponent->GetMesh();
	*SourceMesh = *Mesh;
	MeshAABBTree->SetMesh(SourceMesh.Get(), true);
}

void URuntimeMeshSceneObject::UpdateComponentMaterials(bool bForceRefresh)
{
	UMaterialInterface* DefaultMaterial = UMaterial::GetDefaultMaterial(MD_Surface);

	UMeshComponent* Component = GetMeshComponent();
	if (!Component) return;

	int32 NumMaterials = FMath::Max(1, Component->GetNumMaterials());
	for (int32 k = 0; k < NumMaterials; ++k)
	{
		UMaterialInterface* SetMaterial = (k < Materials.Num()) ? Materials[k] : DefaultMaterial;
		Component->SetMaterial(k, SetMaterial);
	}

	// HACK TO FORCE MATERIAL UPDATE IN SDMC
	if (bForceRefresh)
	{
		SimpleDynamicMeshActor->MeshComponent->NotifyMeshUpdated();
	}
}

void URuntimeMeshSceneObject::SetAllMaterials(UMaterialInterface* SetToMaterial)
{
	int32 NumMaterials = Materials.Num();
	for (int32 k = 0; k < NumMaterials; ++k)
	{
		Materials[k] = SetToMaterial;
	}
	UpdateComponentMaterials(true);
}

void URuntimeMeshSceneObject::SetToHighlightMaterial(UMaterialInterface* SetToMaterial)
{
	UMeshComponent* Component = GetMeshComponent();
	int32 NumMaterials = FMath::Max(1, Component->GetNumMaterials());
	for (int32 k = 0; k < NumMaterials; ++k)
	{
		Component->SetMaterial(k, SetToMaterial);
	}

	// HACK TO FORCE MATERIAL UPDATE IN SDMC
	SimpleDynamicMeshActor->MeshComponent->NotifyMeshUpdated();
}

void URuntimeMeshSceneObject::ClearHighlightMaterial()
{
	UpdateComponentMaterials(true);
}

bool URuntimeMeshSceneObject::IntersectRay(FVector RayOrigin, FVector RayDirection, FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle, FVector& TriBaryCoordinates, float MaxDistance)
{
	if (!ensure(SourceMesh)) return false;
	if (!GetActor()) return false; // this can happen if the Actor gets deleted, but the SceneObject does not

	UE::Geometry::FTransformSRT3d ActorToWorld(GetActor()->GetActorTransform());
	FVector3d WorldDirection(RayDirection); WorldDirection.Normalize();
	FRay3d LocalRay(ActorToWorld.InverseTransformPosition((FVector3d)RayOrigin), ActorToWorld.InverseTransformNormal(WorldDirection));
	UE::Geometry::IMeshSpatial::FQueryOptions QueryOptions;
	if (MaxDistance > 0)
	{
		QueryOptions.MaxDistance = MaxDistance;
	}
	NearestTriangle = MeshAABBTree->FindNearestHitTriangle(LocalRay, QueryOptions);
	if (SourceMesh->IsTriangle(NearestTriangle))
	{
		UE::Geometry::FIntrRay3Triangle3d IntrQuery = UE::Geometry::TMeshQueries<FDynamicMesh3>::TriangleIntersection(*SourceMesh, NearestTriangle, LocalRay);
		if (IntrQuery.IntersectionType == EIntersectionType::Point)
		{
			HitDistance = IntrQuery.RayParameter;
			WorldHitPoint = (FVector)ActorToWorld.TransformPosition(LocalRay.PointAt(IntrQuery.RayParameter));
			TriBaryCoordinates = (FVector)IntrQuery.TriangleBaryCoords;
			return true;
		}
	}
	return false;
}