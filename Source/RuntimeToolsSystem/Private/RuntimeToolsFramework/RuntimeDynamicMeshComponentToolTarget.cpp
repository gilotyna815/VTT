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

#include "RuntimeToolsFramework/RuntimeDynamicMeshComponentToolTarget.h"

#include "Components/DynamicMeshComponent.h"
#include "ModelingToolTargetUtil.h"
#include "RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.h" // to emit change transaction

#define LOCTEXT_NAMESPACE "URuntimeDynamicMeshComponentToolTarget"

bool URuntimeDynamicMeshComponentToolTarget::IsValid() const
{
    if (!UPrimitiveComponentToolTarget::IsValid())
    {
        return false;
    }
    if (UDynamicMeshComponent* DynamicMeshComponent = Cast<UDynamicMeshComponent>(Component))
    {
        if (UDynamicMesh* Dynamic = DynamicMeshComponent->GetDynamicMesh())
        {
            return true;
        }
    }
    return false;
}

int32 URuntimeDynamicMeshComponentToolTarget::GetNumMaterials() const
{
    check(IsValid());
    return Component->GetNumMaterials();
}

UMaterialInterface* URuntimeDynamicMeshComponentToolTarget::GetMaterial(int32 MaterialIndex) const
{
    check(IsValid());
    return Component->GetMaterial(MaterialIndex);
}

void URuntimeDynamicMeshComponentToolTarget::GetMaterialSet(FComponentMaterialSet& MaterialSetOut, bool bPreferAssetMaterials) const
{
    check(IsValid());

    int32 NumMaterials = Component->GetNumMaterials();
    MaterialSetOut.Materials.SetNum(NumMaterials);
    for (int32 k = 0; k < NumMaterials; ++k)
    {
        MaterialSetOut.Materials[k] = Component->GetMaterial(k);
    }
}

bool URuntimeDynamicMeshComponentToolTarget::CommitMaterialSetUpdate(const FComponentMaterialSet& MaterialSet, bool bApplyToAsset)
{
    check(IsValid());

    UDynamicMeshComponent* DynamicMeshComponent = Cast<UDynamicMeshComponent>(Component);

    // TODO: is it necessary to filter the material set like UDynamicMeshComponentToolTarget does?

    int32 NumMaterialsNeeded = Component->GetNumMaterials();
    int32 NumMaterialsGiven = MaterialSet.Materials.Num();
    for (int32 k = 0; k < NumMaterialsGiven; ++k)
    {
        DynamicMeshComponent->SetMaterial(k, MaterialSet.Materials[k]);
    }

    return true;
}

void URuntimeDynamicMeshComponentToolTarget::CommitDynamicMesh(const UE::Geometry::FDynamicMesh3& Mesh, const FDynamicMeshCommitInfo& CommitInfo)
{
    UE::ToolTarget::Internal::
        CommitDynamicMeshViaIPersistentDynamicMeshSource(*this, Mesh, CommitInfo.bTopologyChanged);
}

UE::Geometry::FDynamicMesh3 URuntimeDynamicMeshComponentToolTarget::GetDynamicMesh()
{
    UDynamicMesh* DynamicMesh = GetDynamicMeshContainer();
    FDynamicMesh3 Mesh;
    DynamicMesh->ProcessMesh([&](const FDynamicMesh3& ReadMesh) { Mesh = ReadMesh; });
    return Mesh;
}

UDynamicMesh* URuntimeDynamicMeshComponentToolTarget::GetDynamicMeshContainer()
{
    return Cast<UDynamicMeshComponent>(Component)->GetDynamicMesh();
}

void URuntimeDynamicMeshComponentToolTarget::CommitDynamicMeshChange(TUniquePtr<FToolCommandChange> Change, const FText& ChangeMessage)
{
    URuntimeToolsFrameworkSubsystem::Get()
        ->GetTransactionsAPI()
            ->AppendChange(GetDynamicMeshComponent(), MoveTemp(Change), LOCTEXT("UpdateMeshChange", "Update Mesh"));
    
    InvalidateCachedMeshDescription();
}

UDynamicMeshComponent* URuntimeDynamicMeshComponentToolTarget::GetDynamicMeshComponent()
{
    return Cast<UDynamicMeshComponent>(Component);
}

void URuntimeDynamicMeshComponentToolTarget::InvalidateCachedMeshDescription()
{
    if (bHaveCachedMeshDescription)
    {
        CachedMeshDescription = nullptr;
        bHaveCachedMeshDescription = false;
    }
}

bool URuntimeDynamicMeshComponentToolTargetFactory::CanBuildTarget(UObject* SourceObject, const FToolTargetTypeRequirements& Requirements) const
{
    UDynamicMeshComponent* Component = Cast<UDynamicMeshComponent>(SourceObject);
    return Component
        && IsValidChecked(Component)
        && (!Component->IsUnreachable())
        && Component->IsValidLowLevel()
        && Component->GetDynamicMesh()
        && Requirements.AreSatisfiedBy(URuntimeDynamicMeshComponentToolTarget::StaticClass());
}

UToolTarget* URuntimeDynamicMeshComponentToolTargetFactory::BuildTarget(UObject* SourceObject, const FToolTargetTypeRequirements& TargetTypeInfo)
{
    URuntimeDynamicMeshComponentToolTarget* Target = NewObject<URuntimeDynamicMeshComponentToolTarget>(this);
    Target->Component = Cast<UDynamicMeshComponent>(SourceObject);
    return Target;
}

#undef LOCTEXT_NAMESPACE

