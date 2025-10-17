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

#include "Tools/RuntimePolyEditTool.h"

#include "RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.h"

#define LOCTEXT_NAMESPACE "URuntimeEditMeshPolygonsTool"

USingleTargetWithSelectionTool* URuntimePolyEditToolBuilder::CreateNewTool(const FToolBuilderState& SceneState) const
{
	URuntimePolyEditTool* PolyEditTool = NewObject<URuntimePolyEditTool>(SceneState.ToolManager);
	PolyEditTool->LocalTargetWorld = SceneState.World;
	return PolyEditTool;
}

void URuntimePolyEditTool::Setup()
{
	UEditMeshPolygonsTool::Setup();

	// mirror properties we want to expose at runtime
	RuntimeProperties = NewObject<URuntimePolyEditToolProperties>(this);

	AddToolPropertySource(RuntimeProperties);

	check(GEngine->WireframeMaterial != nullptr);
}

void URuntimePolyEditTool::BeginExtrudeAction()
{
	RequestAction(EEditMeshPolygonsToolActions::Extrude);
}

void URuntimePolyEditTool::BeginInsetAction()
{
	RequestAction(EEditMeshPolygonsToolActions::Inset); // <== crashes?
}

void URuntimePolyEditTool::BeginOutsetAction()
{
	RequestAction(EEditMeshPolygonsToolActions::Outset);
}

void URuntimePolyEditTool::BeginCutFaceAction()
{
	RequestAction(EEditMeshPolygonsToolActions::CutFaces);
}

UWorld* URuntimePolyEditTool::GetWorld() const
{
	return this->LocalTargetWorld;
}

#undef LOCTEXT_NAMESPACE
