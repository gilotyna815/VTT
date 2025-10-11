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

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/DynamicMeshAABBTree3.h"
#include "DynamicSDMCActor.h"

#include "RuntimeMeshSceneObject.generated.h"

struct FMeshDescription;

/**
 * URuntimeMeshSceneObject is a "Scene Object" in the "Scene". Do not create these yourself.
 * Use the functions in URuntimeMeshSceneSubsystem to create and manage SceneObjects.
 *
 * Conceptually, URuntimeMeshSceneObject is a triangle mesh object that can be selected,
 * transformed, and edited using mesh editing tools.
 *
 * Under the hood, URuntimeMeshSceneObject will spawn a ADynamicSDMCActor to actually implement
 * most of that functionality. But, the premise is that the higher level Scene is not aware
 * of those details.
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API URuntimeMeshSceneObject : public UObject
{

	GENERATED_BODY()

public:
	URuntimeMeshSceneObject();

	void Initialize(UWorld* TargetWorld, const FMeshDescription* InitialMeshDescription);
	void Initialize(UWorld* TargetWorld, const FDynamicMesh3* InitialMesh);

	// set the 3D transform of this SceneObject
	void SetTransform(FTransform Transform);

	// get the Actor that represents this SceneObject
	ADynamicMeshBaseActor* GetActor();

	// get the mesh component that represents this scene object
	UMeshComponent* GetMeshComponent();

	//
	// Material functions
	//

	UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
	void SetAllMaterials(UMaterialInterface* SetToMaterial);

	UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
	void SetToHighlightMaterial(UMaterialInterface* SetToMaterial);

	UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
	void ClearHighlightMaterial();

	//
	// Spatial Query functions
	//
	UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
	bool IntersectRay(FVector RayOrigin, FVector RayDirection, FVector& WorrldHitPoint, float& HitDistance, int& NearestTriangle, FVector& TriBaryCoordinates, float MaxDistance = 0);

protected:
	// URuntimeMeshSceneObject's representation in UE Level is ADynamicSDMCActor
	UPROPERTY()
	ADynamicSDMCActor* SimpleDynamicMeshActor = nullptr;

	TUniquePtr<UE::Geometry::FDynamicMesh3> SourceMesh;
	TUniquePtr<UE::Geometry::FDynamicMeshAABBTree3> MeshAABBTree;

	void UpdateSourceMesh(const FMeshDescription* MeshDescription);

	void OnExternalDynamicMeshComponentUpdate();

	TArray<UMaterialInterface*> Materials;

	void UpdateComponentMaterials(bool bForceRefresh);
};
