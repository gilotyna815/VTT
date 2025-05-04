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

#include "RuntimeMeshSceneObject.generated.h"

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
class VTT_API URuntimeMeshSceneObject : public UObject
{

	GENERATED_BODY()

public:
	URuntimeMeshSceneObject();

	void Initialize(UWorld* TargetWorld, const FMeshDescription* InitialMeshDescription);

	// get the mesh component that represents this scene object
	UMeshComponent* GetMeshComponent();

protected:
	// URuntimeMeshSceneObject's representation in UE Level is ADynamic SDMCActor
	//UPROPERTY()
	//ADynamicSDMCActor <==

	TUniquePtr<UE::Geometry::FDynamicMesh3> SourceMesh;
	TUniquePtr<UE::Geometry::FDynamicMeshAABBTree3> MeshAABBTree;

	TArray<UMaterialInterface*> Materials;
};
