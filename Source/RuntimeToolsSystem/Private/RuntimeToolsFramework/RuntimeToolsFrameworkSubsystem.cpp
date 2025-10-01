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

#include "Slate/SGameLayerManager.h"
#include "ContextObjectStore.h"
#include "BaseGizmos/GizmoViewContext.h"
#include "Slate/SceneViewport.h"

#include "ToolTargetManager.h"
#include "RuntimeToolsFramework/RuntimeDynamicMeshComponentToolTarget.h"

#include "RuntimeToolsFramework/RuntimeModelingObjectsCreationAPI.h"

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

IToolsContextTransactionsAPI* URuntimeToolsFrameworkSubsystem::GetTransactionsAPI()
{
	return ContextTransactionsAPI.Get();
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

	// register object creation API
	URuntimeModelingObjectsCreationAPI* ModelCreationAPI = URuntimeModelingObjectsCreationAPI::Register(ToolsContext);
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

class FRuntimeToolsFrameworkRenderImpl : public IToolsContextRenderAPI
{
public:
	UToolsContextRenderComponent* RenderComponent;
	TSharedPtr<FPrimitiveDrawInterface> PDI;
	const FSceneView* SceneView;
	FViewCameraState ViewCameraState;

	FRuntimeToolsFrameworkRenderImpl(UToolsContextRenderComponent* RenderComponentIn,
		const FSceneView* ViewIn,
		FViewCameraState CameraState)
		: RenderComponent(RenderComponentIn), SceneView(ViewIn), ViewCameraState(CameraState)
	{
		PDI = RenderComponentIn->GetPDIForView(ViewIn);
	}

	virtual FPrimitiveDrawInterface* GetPrimitiveDrawInterface() override
	{
		return PDI.Get();
	}

	virtual const FSceneView* GetSceneView() override
	{
		return SceneView;
	}

	virtual FViewCameraState GetCameraState() override
	{
		return ViewCameraState;
	}

	virtual EViewInteractionState GetViewInteractionState() override
	{
		return EViewInteractionState::Focused;
	}
};

//UE_DISABLE_OPTIMIZATION
void URuntimeToolsFrameworkSubsystem::Tick(float DeltaTime)
{
	if (ensure(ContextActor) == false) return;

	// no longer exists...
	//GizmoRenderingUtil::SetGlobalFocusedEditorSceneVew(nullptr);

	FInputDeviceState InputState = CurrentMouseState;
	InputState.InputDevice = EInputDevices::Mouse;

	FVector2D MousePosition = FSlateApplication::Get().GetCursorPos();
	FVector2D LastMousePosition = FSlateApplication::Get().GetLastCursorPos();
	FModifierKeysState ModifierState = FSlateApplication::Get().GetModifierKeys();

	UGameViewportClient* ViewportClient = TargetWorld->GetGameViewport();
	TSharedPtr<IGameLayerManager> LayerManager = ViewportClient->GetGameLayerManager();
	FGeometry ViewportGeometry;
	if (ensure(LayerManager.IsValid()))
	{
		ViewportGeometry = LayerManager->GetViewportWidgetHostGeometry();
	}
	// why do we need this scale here? what is it for?
	FVector2D ViewportMousePos = ViewportGeometry.Scale * ViewportGeometry.AbsoluteToLocal(MousePosition);

	// update modifier keys
	InputState.SetModifierKeyStates(
		ModifierState.IsLeftShiftDown(),
		ModifierState.IsAltDown(),
		ModifierState.IsControlDown(),
		ModifierState.IsCommandDown());

	if (ViewportClient)
	{
		FSceneViewport* Viewport = ViewportClient->GetGameViewport();

		FEngineShowFlags* ShowFlags = ViewportClient->GetEngineShowFlags();
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			ViewportClient->Viewport,
			TargetWorld->Scene,
			*ShowFlags)
			.SetRealtimeUpdate(true));

		ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(ContextActor->PlayerController->Player);
		FVector ViewLocation;
		FRotator ViewRotation;
		FSceneView* SceneView = LocalPlayer->CalcSceneView(&ViewFamily,
			/*out*/ ViewLocation,
			/*out*/ ViewRotation,
			LocalPlayer->ViewportClient->Viewport);
		if (SceneView == nullptr)
		{
			return; // abort
		}

		UGizmoViewContext* GizmoViewContext = ToolsContext->ContextObjectStore->FindContext<UGizmoViewContext>();
		if (GizmoViewContext)
		{
			GizmoViewContext->ResetFromSceneView(*SceneView);
		}

		ContextQueriesAPI->UpdateActiveViewport(Viewport);

		CurrentViewCameraState.Position = ViewLocation;
		CurrentViewCameraState.Orientation = ViewRotation.Quaternion();
		CurrentViewCameraState.HorizontalFOVDegrees = SceneView->FOV;
		CurrentViewCameraState.AspectRatio = Viewport->GetDesiredAspectRatio(); //ViewportClient->AspectRatio;
		CurrentViewCameraState.bIsOrthographic = false;
		CurrentViewCameraState.bIsVR = false;
		CurrentViewCameraState.OrthoWorldCoordinateWidth = 1;

		FVector4 ScreenPos = SceneView->PixelToScreen(ViewportMousePos.X, ViewportMousePos.Y, 0);

		const FMatrix InvViewMatrix = SceneView->ViewMatrices.GetInvViewMatrix();
		const FMatrix InvProjMatrix = SceneView->ViewMatrices.GetInvProjectionMatrix();

		const float ScreenX = ScreenPos.X;
		const float ScreenY = ScreenPos.Y;

		FVector Origin;
		FVector Direction;
		if (! ViewportClient->IsOrtho())
		{
			Origin = SceneView->ViewMatrices.GetViewOrigin();
			
			FVector Temp = InvViewMatrix.TransformVector(
				FVector(
					InvProjMatrix.TransformVector(
						FVector4(
							ScreenX * GNearClippingPlane,
							ScreenY * GNearClippingPlane,
							0.0f,
							GNearClippingPlane))));
			Direction = Temp.GetSafeNormal();
		}
		else
		{
			FVector4 Temp4 = InvViewMatrix.TransformFVector4(
				InvProjMatrix.TransformFVector4(
					FVector4(
						ScreenX * GNearClippingPlane,
						ScreenY * GNearClippingPlane,
						0.0f,
						GNearClippingPlane)));
			Origin = Temp4.GetSafeNormal();

			Direction = InvViewMatrix.TransformVector(FVector(0, 0, 1)).GetSafeNormal();
		}

		// fudge factor so we don't hit actor...
		Origin += 1.0 * Direction;

		InputState.Mouse.Position2D = ViewportMousePos;
		InputState.Mouse.Delta2D = CurrentMouseState.Mouse.Position2D - PrevMousePosition;
		PrevMousePosition = InputState.Mouse.Position2D;
		InputState.Mouse.WorldRay = FRay(Origin, Direction);

		// if we are in camera control we don't send any events
		bool bInCameraControl = (ContextActor->GetCurrentInteractionMode() != EToolActorInteractionMode::NoInteraction);
		if (bInCameraControl)
		{
			ensure(bPendingMouseStateChange == false);
			ensure(ToolsContext->InputRouter->HasActiveMouseCapture() == false);
			//ToolsContext->InputRouter->PostHoverInputEvent(InputState);
		}
		else if (bPendingMouseStateChange || ToolsContext->InputRouter->HasActiveMouseCapture())
		{
			ToolsContext->InputRouter->PostInputEvent(InputState);
		}
		else
		{
			ToolsContext->InputRouter->PostHoverInputEvent(InputState);
		}

		// clear down or up flags now that we have sent event
		if (bPendingMouseStateChange)
		{
			if (CurrentMouseState.Mouse.Left.bDown)
			{
				CurrentMouseState.Mouse.Left.SetStates(false, true, false);
			}
			else
			{
				CurrentMouseState.Mouse.Left.SetStates(false, false, false);
			}
			bPendingMouseStateChange = false;
		}

		// tick things
		ToolsContext->ToolManager->Tick(DeltaTime);
		ToolsContext->GizmoManager->Tick(DeltaTime);

		// render things
		FRuntimeToolsFrameworkRenderImpl RenderAPI(PDIRenderComponent, SceneView, CurrentViewCameraState);
		ToolsContext->ToolManager->Render(&RenderAPI);
		ToolsContext->GizmoManager->Render(&RenderAPI);

		// force rendering flush so that PDI lines get drawn
		FlushRenderingCommands();
	}
}
//UE_ENABLE_OPTIMIZATION

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

bool URuntimeToolsFrameworkSubsystem::AcceptActiveTool()
{
	bool bSuccess = false;

	if (ToolsContext && ToolsContext->ToolManager)
	{
		bool bActive = ToolsContext->ToolManager->HasActiveTool(EToolSide::Mouse);
		if (bActive)
		{
			bool bCanAccept = ToolsContext->ToolManager->CanAcceptActiveTool(EToolSide::Mouse);
			if (bCanAccept)
			{
				ToolsContext->ToolManager->DeactivateTool(EToolSide::Mouse, EToolShutdownType::Accept);
				bSuccess = true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("URuntimeToolsFrameworkSubsystem::AcceptActiveTool - Cannot Accept Active Tool!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("URuntimeToolsFrameworkSubsystem::AcceptActiveTool - No Active Tool!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("URuntimeToolsFrameworkSubsystem::AcceptActiveTool - ToolsContext is not initialized!"));
	}

	InternalConsistencyChecks();
	return bSuccess;
}

bool URuntimeToolsFrameworkSubsystem::IsCapturingMouse() const
{
	return ToolsContext && ToolsContext->InputRouter->HasActiveMouseCapture();
}

void URuntimeToolsFrameworkSubsystem::OnLeftMouseDown()
{
	CurrentMouseState.Mouse.Left.SetStates(true, false, false);
	bPendingMouseStateChange = true;
}

void URuntimeToolsFrameworkSubsystem::OnLeftMouseUp()
{
	CurrentMouseState.Mouse.Left.SetStates(false, false, true);
	bPendingMouseStateChange = true;
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
