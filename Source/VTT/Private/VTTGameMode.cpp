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

#include "VTTGameMode.h"

#include "RuntimeToolsSystem/Public/RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.h"
#include "RuntimeToolsSystem/Public/MeshScene/RuntimeMeshSceneSubsystem.h"
//
//#include "AddPrimitiveTool.h"
//#include "DrawAndRevolveTool.h"
//#include "MeshVertexSculptTool.h"
//#include "PlaneCutTool.h"
//
//#include "Tools/RuntimeDrawPolygonTool.h"
//#include "Tools/RuntimeDynamicMeshSculptTool.h"
//#include "Tools/RuntimeRemeshMeshTool.h"
//#include "Tools/RuntimeMeshBooleanTool.h"
//#include "Tools/RuntimePolyEditTool.h"

AVTTGameMode::AVTTGameMode()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AVTTGameMode::StartPlay()
{
	Super::StartPlay();
	InitializeToolsSystem();
}

void AVTTGameMode::InitializeToolsSystem()
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = GetGameInstance();
	check(World && GameInstance);

	// create Scene subsystem
	SceneSystem = UGameInstance::GetSubsystem<URuntimeMeshSceneSubsystem>(GameInstance);
	URuntimeMeshSceneSubsystem::InitializeSingleton(SceneSystem);

	// create Tools subsystem
	ToolsSystem = UGameInstance::GetSubsystem<URuntimeToolsFrameworkSubsystem>(GameInstance); // <==
	URuntimeToolsFrameworkSubsystem::InitializeSingleton(ToolsSystem);

	check(SceneSystem && ToolsSystem);

	// initialize Tools and Scene systems
	ToolsSystem->InitializeToolsContext(World);
	//SceneSystem->SetCurrentTransactionsAPI();
}

void AVTTGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ToolsSystem)
	{
		ToolsSystem->Tick(DeltaTime);
	}
}

void AVTTGameMode::ShutdownToolsSystem()
{
}

void AVTTGameMode::RegisterTools()
{
}

