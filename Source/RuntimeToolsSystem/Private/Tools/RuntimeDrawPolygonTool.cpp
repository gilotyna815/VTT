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

#include "Tools/RuntimeDrawPolygonTool.h"

#include "RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.h"
#include "MeshScene/RuntimeMeshSceneSubsystem.h"

#define LOCTEXT_NAMESPACE "URuntimeDrawPolygonTool"

UInteractiveTool* URuntimeDrawPolygonToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
    URuntimeDrawPolygonTool* NewTool = NewObject<URuntimeDrawPolygonTool>(SceneState.ToolManager);
    NewTool->SetWorld(SceneState.World);
    return NewTool;
}

void URuntimeDrawPolygonTool::Setup()
{
    UDrawPolygonTool::Setup();

    // initialize to drawing material
    this->MaterialProperties->Material = URuntimeMeshSceneSubsystem::Get()->StandardMaterial;

    // mirror properties we want to explose at runtime
    RuntimeProperties = NewObject<URuntimeDrawPolygonToolProperties>(this);

    RuntimeProperties->SelectedPolygonType = (int)PolygonProperties->PolygonDrawMode;
    RuntimeProperties->WatchProperty(RuntimeProperties->SelectedPolygonType,
        [this](int NewType) { PolygonProperties->PolygonDrawMode = (EDrawPolygonDrawMode)NewType; });

    AddToolPropertySource(RuntimeProperties);
}

#undef LOCTEXT_NAMESPACE