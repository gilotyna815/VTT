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
#include "GeneratedMeshPool.h"

#include "DynamicMeshBaseActor.generated.h"

/**
 * Source of mesh used to initialize ADynamicMeshBaseActor
 */
UENUM(BlueprintType)
enum class EDynamicMeshActorSourceType : uint8
{
	// Initialize the mesh with a generated 3D primitive shape (box, sphere, etc)
	Primitive,
	// Initialize the mesh by importing an external mesh file (OBJ format)
	ImportedMesh,
	// Do not initialize the mesh, allow an external source (eg a Constructor Script) to initialize it
	ExternallyGenerated
};

UENUM(BlueprintType)
enum class EDynamicMeshActorCollisionMode : uint8
{
	// No auto-generated collision
	NoCollision,
	// Complex Collision generated directly from the triangle mesh
	ComplexAsSimple,
	// Complex Collision generated directly from the triangle mesh, but computed asynchronously (so not immediately available)
	ComplexAsSimpleAsync,
	// Simple Collision initialized by a single Convex Hull fit to the entire triangle mesh
	SimpleConvexHull
};

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

	// Type of mesh used to initialize this Actor - either a generated mesh Primitive, or an Imported OBJ file
	UPROPERTY(EditAnywhere, Category = "DynamicMeshActor")
	EDynamicMeshActorSourceType SourceType = EDynamicMeshActorSourceType::Primitive;

	//
	// ADynamicMeshBaseActor API
	//
public:
	// Call EditMesh() to safely modify the SourceMesh owned by this Actor. Your EditFunc will be called with the CurrentSourceMesh as argument, and you are expected to pass back the new.modified version.
	// If you are generating an entirely new mesh, MoveTemp can be used to do this without a copy.
	virtual void EditMesh(TFunctionRef<void(FDynamicMesh3&)> EditFunc);

	//This deletage is broadcast whenever the internal SourceMesh is updated.
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnMeshModified, ADynamicMeshBaseActor*);
	FOnMeshModified OnMeshModified;

protected:
	/** The SourceMesh used to initialize the mesh components in the various subclasses */
	FDynamicMesh3 SourceMesh;

	/** Accumulated time since Actor was created, this is used for the animated primitives when bRegenerateOnTick = true */
	double AccumulatedTime = 0;

	//
	// Support for AABBTree / Spatial Queries
	//
public:
	UPROPERTY(EditAnywhere, Category = "DynamicMeshActor|SpatialQueries")
	bool bEnableSpatialQueries = false;

	UPROPERTY(EditAnywhere, Category = "DynamicMeshActor|SpatialQueries")
	bool bEnableInsideQueries = false;

protected:
	// This AABBTree is updated each time SourceMesh is modified if bEnableSpatialQueries = true or bEnableInsideQueries = true
	FDynamicMeshAABBTree3 MeshAABBTree;
	// This FastWindingTree is updated each time SourceMesh is modified if bEnableInsideQueries = true
	TUniquePtr<UE::Geometry::TFastWindingTree<FDynamicMesh3>> FastWinding;

	//
	// Support for Runtime-Generated Collision
	//
public:
	// Auto-Generated Collision Mode for this Actor (currently only works with DynamicPMCActor subclass
	UPROPERTY(EditAnywhere, Category = "DynamicMeshActor|RuntimeCollision")
	EDynamicMeshActorCollisionMode CollisionMode = EDynamicMeshActorCollisionMode::NoCollision;

	//
	// ADynamicMeshBaseActor API that subclasses must implement.
	//
protected:
	//Called when the SourceMesh has been modified. Subclasses override this function to update their respective Component with the new SourceMesh.
	virtual void OnMeshEditedInternal();

	//
	// Standard UE Actor Callbacks. If you need to override these functions, make sure to call (eg) Super::Tick() or you will break the mesh updating functionality!
	//
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(Transient)
	UGeneratedMeshPool* MeshPool = nullptr;

};
