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

#include "Tools/RuntimeDynamicMeshSculptTool.h"

#include "RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.h"

#define LOCTEXT_NAMESPACE "URuntimeDynamicMeshSculptTool"

UMeshSurfacePointTool* URuntimeDynamicMeshSculptToolBuilder::CreateNewTool(const FToolBuilderState& SceneState) const
{
    URuntimeDynamicMeshSculptTool* SculptTool = NewObject<URuntimeDynamicMeshSculptTool>(SceneState.ToolManager);
    SculptTool->SetEnableRemeshing(this->bEnableRemeshing);
    SculptTool->SetWorld(SceneState.World);
    return SculptTool;
}

static ERuntimeDynamicMeshSculptBrushType Convert(EDynamicMeshSculptBrushType BrushType)
{
    switch (BrushType)
    {
    case EDynamicMeshSculptBrushType::Move:
        return ERuntimeDynamicMeshSculptBrushType::Move;
    case EDynamicMeshSculptBrushType::Smooth:
        return ERuntimeDynamicMeshSculptBrushType::Smooth;
    case EDynamicMeshSculptBrushType::Inflate:
        return ERuntimeDynamicMeshSculptBrushType::Inflate;
    case EDynamicMeshSculptBrushType::PlaneViewAligned:
        return ERuntimeDynamicMeshSculptBrushType::Flatten;
    default:
        return ERuntimeDynamicMeshSculptBrushType::Sculpt;
    }
}
static EDynamicMeshSculptBrushType Convert(ERuntimeDynamicMeshSculptBrushType BrushType)
{
    switch (BrushType)
    {
    case ERuntimeDynamicMeshSculptBrushType::Move:
        return EDynamicMeshSculptBrushType::Move;
    case ERuntimeDynamicMeshSculptBrushType::Smooth:
        return EDynamicMeshSculptBrushType::Smooth;
    case ERuntimeDynamicMeshSculptBrushType::Inflate:
        return EDynamicMeshSculptBrushType::Inflate;
    case ERuntimeDynamicMeshSculptBrushType::Flatten:
        return EDynamicMeshSculptBrushType::PlaneViewAligned;
    default:
        return EDynamicMeshSculptBrushType::Offset;
    }
}

void URuntimeDynamicMeshSculptTool::Setup()
{
    UDynamicMeshSculptTool::Setup();

    // mirror properties we want exposed at runtime
    RuntimeProperties = NewObject<URuntimeDynamicMeshSculptToolProperties>(this);

    RuntimeProperties->BrushSize = BrushProperties->BrushSize.AdaptiveSize;
    RuntimeProperties->WatchProperty(RuntimeProperties->BrushSize,
        [this](float NewValue)
        {
            BrushProperties->BrushSize.AdaptiveSize = NewValue;
            OnPropertyModified(nullptr, nullptr); // hack to get CalculateBrushRadius() to be called, because it is private (why?)
        });

    RuntimeProperties->BrushStrength = SculptProperties->PrimaryBrushSpeed;
    RuntimeProperties->WatchProperty(RuntimeProperties->BrushStrength,
        [this](float NewValue)
        {
            SculptProperties->PrimaryBrushSpeed = NewValue;
        });

    RuntimeProperties->BrushFalloff = BrushProperties->BrushFalloffAmount;
    RuntimeProperties->WatchProperty(RuntimeProperties->BrushFalloff,
        [this](float NewValue)
        {
            BrushProperties->BrushFalloffAmount = NewValue;
        });

    RuntimeProperties->SelectedBrushType = (int)Convert(SculptProperties->PrimaryBrushType);
    RuntimeProperties->WatchProperty(RuntimeProperties->SelectedBrushType,
        [this](int NewType)
        {
            SculptProperties->PrimaryBrushType = Convert((ERuntimeDynamicMeshSculptBrushType)NewType);
        });

    AddToolPropertySource(RuntimeProperties);
}

#undef LOCTEXT_NAMESPACE