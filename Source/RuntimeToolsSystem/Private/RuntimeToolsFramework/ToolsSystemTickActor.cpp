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

#include "RuntimeToolsSystem/Public/RuntimeToolsFramework/ToolsSystemTickActor.h"

#include "RuntimeToolsSystem/Public/RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.h"
#include "RuntimeToolsSystem/Public/MeshScene/RuntimeMeshSceneSubsystem.h"

// Tools
#include "AddPrimitiveTool.h"
#include "RuntimeToolsSystem/Public/Tools/RuntimeDrawPolygonTool.h"
#include "DrawAndRevolveTool.h"
#include "RuntimeToolsSystem/Public/Tools/RuntimePolyEditTool.h"
#include "PlaneCutTool.h"
#include "RuntimeToolsSystem/Public/Tools/RuntimeRemeshMeshTool.h"
#include "MeshVertexSculptTool.h"
#include "RuntimeToolsSystem/Public/Tools/RuntimeDynamicMeshSculptTool.h"
#include "RuntimeToolsSystem/Public/Tools/RuntimeMeshBooleanTool.h"

// Sets default values
AToolsSystemTickActor::AToolsSystemTickActor()
{
 	// Set this actor to call Tick() every frame. Needs to Tick, as it also Ticks the Tools system.
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AToolsSystemTickActor::BeginPlay()
{
	Super::BeginPlay();

	InitializeToolsSystem();
}

void AToolsSystemTickActor::InitializeToolsSystem()
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = GetGameInstance();
	check(World && GameInstance);

	// create Scene subsystem
	SceneSystem = UGameInstance::GetSubsystem<URuntimeMeshSceneSubsystem>(GameInstance);
	URuntimeMeshSceneSubsystem::InitializeSingleton(SceneSystem);

	// create Tools subsystem
	ToolsSystem = UGameInstance::GetSubsystem<URuntimeToolsFrameworkSubsystem>(GameInstance);
	URuntimeToolsFrameworkSubsystem::InitializeSingleton(ToolsSystem);

	check(SceneSystem && ToolsSystem);

	// initialize Tools and Scene systems
	ToolsSystem->InitializeToolsContext(World);
	SceneSystem->SetCurrentTransactionsAPI(ToolsSystem->GetTransactionsAPI());

	RegisterTools();
}

void AToolsSystemTickActor::RegisterTools()
{
	UInteractiveToolManager* ToolManager = ToolsSystem->ToolsContext->ToolManager;

	auto AddPrimitiveToolBuilder = NewObject<UAddPrimitiveToolBuilder>();
	AddPrimitiveToolBuilder->ShapeType = UAddPrimitiveToolBuilder::EMakeMeshShapeType::Box;
	ToolManager->RegisterToolType("AddPrimitiveBox", AddPrimitiveToolBuilder);

	auto DrawPolygonToolBuilder = NewObject<URuntimeDrawPolygonToolBuilder>();
	ToolManager->RegisterToolType("DrawPolygon", DrawPolygonToolBuilder);

	auto PolyRevolveToolBuilder = NewObject<UDrawAndRevolveToolBuilder>();
	ToolManager->RegisterToolType("PolyRevolve", PolyRevolveToolBuilder);

	auto PolyEditToolBuilder = NewObject<URuntimePolyEditToolBuilder>();
	ToolManager->RegisterToolType("EditPolygons", PolyEditToolBuilder);

	auto MeshPlaneCutToolBuilder = NewObject<UPlaneCutToolBuilder>();
	ToolManager->RegisterToolType("PlaneCut", MeshPlaneCutToolBuilder);

	auto RemeshMeshToolBuilder = NewObject<URuntimeRemeshMeshToolBuilder>();
	ToolManager->RegisterToolType("RemeshMesh", RemeshMeshToolBuilder);

	auto VertexSculptToolBuilder = NewObject<UMeshVertexSculptToolBuilder>();
	ToolManager->RegisterToolType("VertexSculpt", VertexSculptToolBuilder);

	auto DynamicSculptToolBuilder = NewObject<URuntimeDynamicMeshSculptToolBuilder>();
	DynamicSculptToolBuilder->bEnableRemeshing = true;
	ToolManager->RegisterToolType("DynamicSculpt", DynamicSculptToolBuilder);

	auto MeshBooleanToolBuilder = NewObject<URuntimeMeshBooleanToolBuilder>();
	ToolManager->RegisterToolType("MeshBoolean", MeshBooleanToolBuilder);
}

// Called every frame
void AToolsSystemTickActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ToolsSystem)
	{
		ToolsSystem->Tick(DeltaTime);
	}
}
