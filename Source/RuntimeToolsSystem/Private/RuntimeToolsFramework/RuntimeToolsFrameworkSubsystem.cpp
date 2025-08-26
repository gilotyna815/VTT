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

#include "RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.h"

#include "ToolContextInterfaces.h"
#include "RuntimeToolsFramework/ToolsContextActor.h"
#include "MeshScene/RuntimeMeshSceneSubsystem.h"

#include "MaterialDomain.h"

#include "BaseGizmos/TransformGizmoUtil.h"

class FRuntimeToolsContextQueriesImpl : public IToolsContextQueriesAPI
{
public:
	FRuntimeToolsContextQueriesImpl(UInteractiveToolsContext* InContext, UWorld* InWorld)
	{
		ToolsContext = InContext;
		TargetWorld = InWorld;
	}

	virtual void SetContextActor(AToolsContextActor* ContextActorIn)
	{
		ContextActor = ContextActorIn;
	}

	void UpdateActiveViewport(FViewport* ViewportIn)
	{
		ActiveViewport = ViewportIn;
	}

	virtual UWorld* GetCurrentEditingWorld() const override
	{
		return TargetWorld;
	}

	virtual void GetCurrentSelectionState(FToolBuilderState& StateOut) const override
	{
		StateOut.ToolManager = ToolsContext->ToolManager;
		StateOut.TargetManager = ToolsContext->TargetManager;
		StateOut.GizmoManager = ToolsContext->GizmoManager;
		StateOut.World = TargetWorld;

		const TArray<URuntimeMeshSceneObject*>& Selection = URuntimeMeshSceneSubsystem::Get()->GetSelection();
		for (URuntimeMeshSceneObject* SelectedObject : Selection)
		{
			StateOut.SelectedActors.Add(SelectedObject->GetActor());
			StateOut.SelectedComponents.Add(SelectedObject->GetMeshComponent());
		}
	}

	virtual void GetCurrentViewState(FViewCameraState& StateOut) const override
	{
		if (!ContextActor)
		{
			return;
		}

		bool bHasCamera = ContextActor->HasActiveCameraComponent();

		FVector Location;
		FRotator Rotation;
		ContextActor->GetActorEyesViewPoint(Location, Rotation);

		StateOut.Position = Location;
		StateOut.Orientation = Rotation.Quaternion();
		StateOut.HorizontalFOVDegrees = 90;
		StateOut.OrthoWorldCoordinateWidth = 1;
		StateOut.AspectRatio = 1.0;
		StateOut.bIsOrthographic = false;
		StateOut.bIsVR = false;
	}

	virtual EToolContextCoordinateSystem GetCurrentCoordinateSystem() const override
	{
		return URuntimeToolsFrameworkSubsystem::Get()->GetCurrentCoordinateSystem();
	}

	virtual FToolContextSnappingConfiguration GetCurrentSnappingSettings() const override
	{
		return FToolContextSnappingConfiguration();
	}

	virtual UMaterialInterface* GetStandardMaterial(EStandardToolContextMaterials MaterialType) const override
	{
		return UMaterial::GetDefaultMaterial(MD_Surface);
	}

	virtual FViewport* GetHoveredViewport() const override
	{
		return ActiveViewport;
	}

	virtual FViewport* GetFocusedViewport() const override
	{
		return ActiveViewport;
	}

protected:
	UInteractiveToolsContext* ToolsContext;
	UWorld* TargetWorld;
	AToolsContextActor* ContextActor = nullptr;
	FViewport* ActiveViewport = nullptr;
};



URuntimeToolsFrameworkSubsystem* URuntimeToolsFrameworkSubsystem::InstanceSingleton = nullptr;

void URuntimeToolsFrameworkSubsystem::InitializeSingleton(URuntimeToolsFrameworkSubsystem* Subsystem)
{
	check(InstanceSingleton == nullptr);
	InstanceSingleton = Subsystem;
}

URuntimeToolsFrameworkSubsystem* URuntimeToolsFrameworkSubsystem::Get()
{
	check(InstanceSingleton);
	return InstanceSingleton;
}

void URuntimeToolsFrameworkSubsystem::Deinitialize()
{
	ShutdownToolsContext();

	InstanceSingleton = nullptr;
}

void URuntimeToolsFrameworkSubsystem::InitializeToolsContext(UWorld* TargetWorldIn)
{
	TargetWorld = TargetWorldIn;

	ToolsContext = NewObject<UInteractiveToolsContext>();

	ContextQueriesAPI = MakeShared<FRuntimeToolsContextQueriesImpl>(ToolsContext, TargetWorld);
	if (ContextActor)
	{
		ContextQueriesAPI->SetContextActor(ContextActor);
	}

	// ==>
}

void URuntimeToolsFrameworkSubsystem::ShutdownToolsContext()
{
	bIsShuttingDown = true;

	if (ToolsContext != nullptr)
	{
		CancelOrCompleteActiveTool();

		// TransformInteraction->Shutdown(); // <==

void URuntimeToolsFrameworkSubsystem::SetContextActor(AToolsContextActor* ActorIn)
{
	ContextActor = ActorIn;
	if (ContextQueriesAPI)
	{
		ContextQueriesAPI->SetContextActor(ContextActor);
	}
}

bool URuntimeToolsFrameworkSubsystem::CancelOrCompleteActiveTool()
{
	if (ToolsContext && ToolsContext->ToolManager)
	{
		bool bActive = ToolsContext->ToolManager->HasActiveTool(EToolSide::Mouse);
		if (bActive)
		{
			EToolShutdownType ShutdownType = ToolsContext->ToolManager->CanCancelActiveTool(EToolSide::Mouse) ? EToolShutdownType::Cancel : EToolShutdownType::Completed;
			ToolsContext->ToolManager->DeactivateTool(EToolSide::Mouse, ShutdownType);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("URuntimeToolsFrameworkSubsystem::CancelOrCompleteActiveTool - No Active Tool!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("URuntimeToolsFrameworkSubsystem::CancelOrCompleteActiveTool - Tool Context is not initialized!"));
	}

	InternalConsistencyChecks();
	return false;
}

void URuntimeToolsFrameworkSubsystem::InternalConsistencyChecks()
{
	if (GetSceneHistory())
	{
		if (!ensure(GetSceneHistory()->IsBuildingTransaction() == false))
		{
			UE_LOG(LogTemp, Warning, TEXT("[URuntimeToolsFrameworkSubsystem::InternalConsistencyChecks] Still building Transaction! Likely forgot to EndTransaction() somewhere!!"));
		}
	}
}
