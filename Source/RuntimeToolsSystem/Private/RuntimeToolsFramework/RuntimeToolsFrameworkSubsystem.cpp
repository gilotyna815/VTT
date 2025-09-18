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

#include "ToolTargetManager.h"
#include "RuntimeDynamicMeshComponentToolTarget.h"

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

class FRuntimeToolsContextTransactionImpl : public IToolsContextTransactionsAPI
{
public:
	bool bInTransaction = false;

	virtual void DisplayMessage(const FText& Message, EToolMessageLevel Level) override
	{
		UE_LOG(LogTemp, Warning, TEXT("[ToolMessage] %s"), *Message.ToString());
	}

	virtual void PostInvalidation() override
	{
		// not necessary in runtime context
	}

	virtual void BeginUndoTransaction(const FText& Description) override
	{
		URuntimeToolsFrameworkSubsystem::Get()->SceneHistory->BeginTransaction(Description);
		bInTransaction = true;
	}

	virtual void EndUndoTransaction() override
	{
		URuntimeToolsFrameworkSubsystem::Get()->SceneHistory->EndTransaction();
		bInTransaction = false;
	}

	virtual void AppendChange(UObject* TargetObject, TUniquePtr<FToolCommandChange> Change, const FText& Description) override
	{
		bool bCloseTransaction = false;
		if (!bInTransaction)
		{
			BeginUndoTransaction(Description);
			bCloseTransaction = true;
		}

		URuntimeToolsFrameworkSubsystem::Get()->SceneHistory->AppendChange(TargetObject, MoveTemp(Change), Description);

		if (bCloseTransaction)
		{
			EndUndoTransaction();
		}
	}

	virtual bool RequestSelectionChange(const FSelectedObjectsChangeList& SelectionChange) override
	{
		// not supported. would need to map elements of SelectionChange to MeshSceneObjects.
		return false;
	}
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

	ContextTransactionsAPI = MakeShared<FRuntimeToolsContextTransactionImpl>();

	ToolsContext->Initialize(ContextQueriesAPI.Get(), ContextTransactionsAPI.Get());

	// register event handlers
	ToolsContext->ToolManager->OnToolStarted.AddUObject(this, &URuntimeToolsFrameworkSubsystem::OnToolStarted);
	ToolsContext->ToolManager->OnToolEnded.AddUObject(this, &URuntimeToolsFrameworkSubsystem::OnToolEnded);

	// create scene history
	SceneHistory = NewObject<USceneHistoryManager>(this);
	SceneHistory->OnHistoryStateChange.AddUObject(this, &URuntimeToolsFrameworkSubsystem::OnSceneHistoryStateChange);

	//register selection interaction
	SelectionInteraction = NewObject<USceneObjectSelectionInteraction>();
	SelectionInteraction->Initialize([this]()
	{
		return HaveActiveTool() == false;
	});
	ToolsContext->InputRouter->RegisterSource(SelectionInteraction);

	//create transform interaction
	TransformInteraction = NewObject<USceneObjectTransformInteraction>();
	TransformInteraction->Initialize([this]()
	{
		return false;
	});

	// create PDI rendering bridge Component
	FActorSpawnParameters SpawnInfo;
	PDIRenderActor = TargetWorld->SpawnActor<AActor>(FVector::ZeroVector, FRotator(0, 0, 0), SpawnInfo);
	PDIRenderComponent = NewObject<UToolsContextRenderComponent>(PDIRenderActor);
	PDIRenderActor->SetRootComponent(PDIRenderComponent);
	PDIRenderComponent->RegisterComponent();

	// have to disable this for current tools framework handling of property defaults
	GShouldVerifyGCAssumptions = false;

	//// make sure we have registered FPrimitiveComponentTarget factories
	//FSimpleDynamicMeshComponentTargetFactory::RegisterFactory();

	// register target factory for dynamic mesh components
	//ToolsContext->TargetManager->AddTargetFactory(NewObject<URuntimeDynamicMeshComponentToolTargetFactory>(ToolsContext->ToolManager));
	ToolsContext->TargetManager->AddTargetFactory(NewObject<URuntimeDynamicMeshComponentToolTargetFactory>());
	
	// register transform gizmo util helper
	UE::TransformGizmoUtil::RegisterTransformGizmoContextObject(ToolsContext);

}

void URuntimeToolsFrameworkSubsystem::ShutdownToolsContext()
{
	bIsShuttingDown = true;

	if (ToolsContext != nullptr)
	{
		CancelOrCompleteActiveTool();

		// TransformInteraction->Shutdown(); // <==

void URuntimeToolsFrameworkSubsystem::OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	AddAllPropertySetKeepAlives(Tool);

	TransformInteraction->ForceUpdateGizmoState();
}

void URuntimeToolsFrameworkSubsystem::OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	if (!bIsShuttingDown)
	{
		TransformInteraction->ForceUpdateGizmoState();
	}
}

void URuntimeToolsFrameworkSubsystem::OnSceneHistoryStateChange()
{
	if (!bIsShuttingDown)
	{
		TransformInteraction->ForceUpdateGizmoState();
	}
}

void URuntimeToolsFrameworkSubsystem::AddAllPropertySetKeepAlives(UInteractiveTool* Tool)
{
	TArray<UObject*> PropertySets = Tool->GetToolProperties(false);
	for (UObject* PropertySetObject : PropertySets)
	{
		if (UInteractiveToolPropertySet* PropertySet = Cast<UInteractiveToolPropertySet>(PropertySetObject))
		{
			AddPropertySetKeepalives(PropertySet);
		}
	}
}

void URuntimeToolsFrameworkSubsystem::AddPropertySetKeepalives(UInteractiveToolPropertySet* PropertySet)
{
	if (ensure(PropertySet != nullptr))
	{
		bool bCached = false;

		UInteractiveToolPropertySet* CDO = GetMutableDefault<UInteractiveToolPropertySet>(PropertySet->GetClass());

		for (TFieldIterator<FProperty> It(PropertySet->GetClass(), EFieldIterationFlags::IncludeSuper); It; ++It)
		{
			FString Name = It->GetName();
			if (Name == TEXT("CachedProperties"))
			{
				const FProperty* Property = *It;
				if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
				{
					UObject* TargetObject = ObjectProperty->GetObjectPropertyValue(Property->ContainerPtrToValuePtr<UObject*>(CDO));
					PropertySetKeepAlives.AddUnique(TargetObject);
					bCached = true;
				}
			}
		}

		if (bCached == true)
		{
			FString PropSetName;
			PropertySet->GetClass()->GetName(PropSetName);
			UE_LOG(LogTemp, Warning, TEXT("Failed to find PropertySet Keepalive for %s!"), *PropSetName);
		}
	}
}

void URuntimeToolsFrameworkSubsystem::SetContextActor(AToolsContextActor* ActorIn)
{
	ContextActor = ActorIn;
	if (ContextQueriesAPI)
	{
		ContextQueriesAPI->SetContextActor(ContextActor);
	}
}

bool URuntimeToolsFrameworkSubsystem::HaveActiveTool()
{
	return (ToolsContext != nullptr)
		&& (ToolsContext->ToolManager != nullptr)
		&& ToolsContext->ToolManager->HasActiveTool(EToolSide::Mouse);
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
