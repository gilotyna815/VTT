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
#include "GameFramework/Actor.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/DynamicMeshAABBTree3.h"
#include "Spatial/FastWinding.h"

#include "DynamicMeshBaseActor.generated.h"

/**
 * ADynamicMeshBaseActor is a base class for Actors that support being
 * rebuilt in-game after mesh editing operations. The base Actor itself
 * does not have any Components, it should be used via one of the
 * DynamicPMCActor, DynamicSMCActor, or DynamicSDMCActor subclasses.
 *
 * ADynamicMeshBaseActor provides a FDynamicMesh3 "Source Mesh", which can
 * be modified via lambdas passed to the EditMesh() function, which will
 * then cause necessary updates to happen to the implementing Components.
 * An AABBTree and FastWindingTree can optionally be enabled with the
 * bEnableSpatialQueries and bEnableInsideQueries flags.
 *
 * When Spatial queries are enabled, a set of UFunctions DistanceToPoint(),
 * NearestPoint(), ContainsPoint(), and IntersectRay() are available via Blueprints
 * on the relevant Actor. These functions *do not* depend on the UE4 Physics
 * system to work.
 *
 * A small set of mesh modification UFunctions are also available via Blueprints,
 * including BooleanWithMesh(), SolidifyMesh(), SimplifyMeshToTriCount(), and
 * CopyFromMesh().
 *
 * Meshes can be read from OBJ files either using the ImportedMesh type for
 * the SourceType property, or by calling the ImportMesh() UFunction from a Blueprint.
 * Note that calling this in a Construction Script will be problematic in the Editor
 * as the OBJ will be re-read any time the Actor is modified (including translated/rotated).
 *
 * Any Material set on the subclass Components will be overriden by the Material property.
 *
 *
 */
UCLASS(Abstract)
class VTT_API ADynamicMeshBaseActor : public AActor
{
	using FDynamicMesh3 = UE::Geometry::FDynamicMesh3;
	using FDynamicMeshAABBTree3 = UE::Geometry::FDynamicMeshAABBTree3;

	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADynamicMeshBaseActor();

protected:
	/** The SourceMesh used to initialize the mesh components in the various subclasses */
	FDynamicMesh3 SourceMesh;

	/** Accumulated time since Actor was created, this is used for the animated primitives when bRegenerateOnTick = true */
	double AccumulatedTime = 0;

protected:
	// This AABBTree is updated each time SourceMesh is modified if bEnableSpatialQueries = true or bEnableInsideQueries = true
	FDynamicMeshAABBTree3 MeshAABBTree;
	// This FastWindingTree is updated each time SourceMesh is modified if bEnableInsideQueries = true
	TUniquePtr<UE::Geometry::TFastWindingTree<FDynamicMesh3>> FastWinding;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	//UPROPERTY(Transient)
	//UGeneratedMeshPool* <==

};
