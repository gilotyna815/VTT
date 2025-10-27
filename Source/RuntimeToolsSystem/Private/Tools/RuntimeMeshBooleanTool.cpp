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

#include "Tools/RuntimeMeshBooleanTool.h"

//#include "ToolBuilderUtil.h"
#include "ModelingToolTargetUtil.h"
#include "MeshScene/RuntimeMeshSceneSubsystem.h"

#define LOCTEXT_NAMESPACE "URuntimeMeshBooleanTool"

UMultiSelectionMeshEditingTool* URuntimeMeshBooleanToolBuilder::CreateNewTool(const FToolBuilderState& SceneState) const
{
    return NewObject<URuntimeMeshBooleanTool>(SceneState.ToolManager);
}

void URuntimeMeshBooleanTool::Setup()
{
    UCSGMeshesTool::Setup();

    this->CSGProperties->bTryFixHoles = true;
    // write to first input asset
    this->HandleSourcesProperties->OutputWriteTo = EBaseCreateFromSelectedTargetType::FirstInputObject;
    // Set to keep sources because we will handle deleting SceneObjects ourselves. We cannot allow the
    // Tool to do it because it will just call Actor.Destroy(), but we actually want to keep the Actor
    // around for Undo/Redo.
    this->HandleSourcesProperties->HandleInputs = EHandleSourcesMethod::KeepSources;

    //mirror properties we want to expose at runtime
    RuntimeProperties = NewObject<URuntimeMeshBooleanToolProperties>(this);

    RuntimeProperties->OperationType = (int)CSGProperties->Operation;
    RuntimeProperties->WatchProperty(RuntimeProperties->OperationType,
        [this](int NewType)
        {
            CSGProperties->Operation = (ECSGOperation)NewType;
            Preview->InvalidateResult();
        });

    AddToolPropertySource(RuntimeProperties);
}

void URuntimeMeshBooleanTool::Shutdown(EToolShutdownType ShutdownType)
{
    if (ShutdownType == EToolShutdownType::Accept)
    {
        GetToolManager()->BeginUndoTransaction(GetActionName());

        // base UCSGMeshesTool will delete the Actor, but we need to also delete the SceneObject
        AActor* KeepActor = UE::ToolTarget::GetTargetActor(Targets[0]);
        URuntimeMeshSceneObject* KeepSceneObject = URuntimeMeshSceneSubsystem::Get()->FindSceneObjectByActor(KeepActor);
        URuntimeMeshSceneSubsystem::Get()->SetSelected(KeepSceneObject, true, false);
        URuntimeMeshSceneSubsystem::Get()->DeleteSelectedSceneObjects(KeepActor);
    }

    UCSGMeshesTool::Shutdown(ShutdownType);

    if (ShutdownType == EToolShutdownType::Accept)
    {
        GetToolManager()->EndUndoTransaction();
    }
}

#undef LOCTEXT_NAMESPACE