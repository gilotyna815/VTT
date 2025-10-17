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

#include "TargetInterfaces/DynamicMeshCommitter.h"
#include "TargetInterfaces/DynamicMeshProvider.h"
#include "TargetInterfaces/DynamicMeshSource.h"
#include "TargetInterfaces/MaterialProvider.h"
#include "ToolTargets/ToolTarget.h"
#include "ToolTargets/PrimitiveComponentToolTarget.h"

#include "RuntimeDynamicMeshComponentToolTarget.generated.h"

class UDynamicMesh;

/**
 * URuntimeDynamicMeshComponentToolTarget is a UToolTarget implementation suitable for UDynamicMeshComponent,
 * which is the component type that ultimately backs a URuntimeMeshSceneObject (via ADynamicSDMCActor).
 * 
 * The Engine provides UPrimitiveComponentToolTarget, whuich we subclass to re-use various common
 * UPrimitiveComponent functionality. The Engine actually also provides UDynamicMeshComponentToolTarget, but
 * it is Editor-Only and cannot be used at Runtime. This class mirrors that implementation heavily, although
 * some things are specific to the Runtime system (ie no transactions, changes are emitted via
 * RuntimeToolsFrameworkSubsystem).
 */
UCLASS(Transient)
class RUNTIMETOOLSSYSTEM_API URuntimeDynamicMeshComponentToolTarget : public UPrimitiveComponentToolTarget,
	public IMaterialProvider,
	public IDynamicMeshCommitter,
	public IDynamicMeshProvider,
	public IPersistentDynamicMeshSource
{
	GENERATED_BODY()

public:
	// override UPrimitiveComponentToolTarget to also check if underlying UDynamicMesh is valid
	virtual bool IsValid() const override;

	// Inherited via IMaterialProvider
	int32 GetNumMaterials() const override;
	UMaterialInterface* GetMaterial(int32 MaterialIndex) const override;
	void GetMaterialSet(FComponentMaterialSet& MaterialSetOut, bool bPreferAssetMaterials) const override;
	bool CommitMaterialSetUpdate(const FComponentMaterialSet& MaterialSet, bool bApplyToAsset) override;

	// Inherited via IDynamicMeshCommitter
	virtual void CommitDynamicMesh(const UE::Geometry::FDynamicMesh3& Mesh, const FDynamicMeshCommitInfo& CommitInfo) override;
	using IDynamicMeshCommitter::CommitDynamicMesh;

	// Inherited via IDynamicMeshProvider
	UE::Geometry::FDynamicMesh3 GetDynamicMesh() override;
	
	// IPersistentDynamicMeshSource
	virtual UDynamicMesh* GetDynamicMeshContainer() override;
	virtual void CommitDynamicMeshChange(TUniquePtr<FToolCommandChange> Change, const FText& ChangeMessage) override;
	virtual UDynamicMeshComponent* GetDynamicMeshComponent() override;

protected:
	/**
	 * In many cases it is necessary to convert the DynamicMeshComponent's UDynamicMesh/FDynamicMesh3 to a
	 * FMeshDescription. We cache this cionversion in case it can be re-used.
	 */
	TUniquePtr<FMeshDescription> CachedMeshDescription;
	bool bHaveCachedMeshDescription = false;
	void InvalidateCachedMeshDescription();

protected:
	friend class URuntimeDynamicMeshComponentToolTargetFactory;
};

UCLASS(Transient)
class RUNTIMETOOLSSYSTEM_API URuntimeDynamicMeshComponentToolTargetFactory : public UToolTargetFactory
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTarget(UObject* SourceObject, const FToolTargetTypeRequirements& TargetTypeInfo) const override;
	virtual UToolTarget* BuildTarget(UObject* SourceObject, const FToolTargetTypeRequirements& TargetTypeInfo) override;
};
