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

#include "RuntimeToolsFramework/ToolsContextActor.h"

#include "RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.h"
#include "GameFramework/PlayerInput.h"
#include "MeshScene/RuntimeMeshSceneSubsystem.h"

// Sets default values
AToolsContextActor::AToolsContextActor()
{
	// Set this pawn to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AToolsContextActor::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance* GameInstance = GetGameInstance();
	ToolsSystem = UGameInstance::GetSubsystem<URuntimeToolsFrameworkSubsystem>(GameInstance);
	ToolsSystem->SetContextActor(this);
}

void AToolsContextActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AToolsContextActor::PossessedBy(AController* ControllerIn)
{
	this->PlayerController = Cast<APlayerController>(ControllerIn);
	//if (ensure(this->PlayerController))
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Got Player Controller!"));
	//}
}

void AToolsContextActor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// do not want default player input behavior
	//Super::SetupPlayerInputComponent(PlayerInputComponent

	// TODO: Redo in Enhanced Input, Axis and Action mappings are deprecated
	// TODO: Allow to remap keys in in-game settings menu

	// Player Camera Movement
	UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("TFPawn_MoveForward", EKeys::W, 1.f));
	UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("TFPawn_MoveForward", EKeys::S, -1.f));
	UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("TFPawn_MoveRight", EKeys::A, -1.f));
	UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("TFPawn_MoveRight", EKeys::D, 1.f));
	UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("TFPawn_MoveUp", EKeys::E, 1.f));
	UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("TFPawn_MoveUp", EKeys::Q, -1.f));
	PlayerInputComponent->BindAxis("TFPawn_MoveForward", this, &AToolsContextActor::OnMoveForwardKeyAxis);
	PlayerInputComponent->BindAxis("TFPawn_MoveRight", this, &AToolsContextActor::OnMoveRightKeyAxis);
	PlayerInputComponent->BindAxis("TFPawn_MoveUp", this, &AToolsContextActor::OnMoveUpKeyAxis);

	UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("StandardAltButton", EKeys::LeftAlt));
	UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("StandardAltButton", EKeys::RightAlt));
	PlayerInputComponent->BindAction("StandardAltButton", IE_Pressed, this, &AToolsContextActor::OnAltKeyDown);
	PlayerInputComponent->BindAction("StandardAltButton", IE_Released, this, &AToolsContextActor::OnAltKeyUp);

	UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("StandardMiddleMouseButton", EKeys::MiddleMouseButton));
	PlayerInputComponent->BindAction("StandardMiddleMouseButton", IE_Pressed, this, &AToolsContextActor::OnMiddleMouseDown);
	PlayerInputComponent->BindAction("StandardMiddleMouseButton", IE_Released, this, &AToolsContextActor::OnMiddleMouseUp);

	UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("StandardRightMouseButton", EKeys::RightMouseButton));
	PlayerInputComponent->BindAction("StandardRightMouseButton", IE_Pressed, this, &AToolsContextActor::OnRightMouseDown);
	PlayerInputComponent->BindAction("StandardRightMouseButton", IE_Released, this, &AToolsContextActor::OnRightMouseUp);

	//
	// buttons defined in Project Settings
	//
	
	// [RMS] AddEngineDefinedActionMapping() does not work with EKeys::LeftMouseButton (can't figure out why...)
	// As a workaround I have added a "LeftMouseButtonAction" mapping in the Input section of Project Settings
	//UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("StandardLeftMouseButton", EKeys::LeftMouseButton));
	//PlayerInputComponent->BindAction("StandardLeftMouseButton", IE_Pressed, this, &AToolsContextActor::OnLeftMouseDown);
	//PlayerInputComponent->BindAction("StandardLeftMouseButton", IE_Released, this, &AToolsContextActor::OnLeftMouseUp);
	InputComponent->BindAction("LeftMouseButtonAction", IE_Pressed, this, &AToolsContextActor::OnLeftMouseDown);
	InputComponent->BindAction("LeftMouseButtonAction", IE_Released, this, &AToolsContextActor::OnLeftMouseUp);

	InputComponent->BindAxis("MouseMovementX", this, &AToolsContextActor::OnMouseMoveX);
	//PlayerInputComponent->BindAxis("MouseMovementX", this, &AToolsContextActor::OnMouseMoveX);
	InputComponent->BindAxis("MouseMovementY", this, &AToolsContextActor::OnMouseMoveY);

	// bound to enter in Project Settings
	InputComponent->BindAction("ActiveToolAccept", IE_Released, this, &AToolsContextActor::OnToolAccept);
	// bound to escape in Project Settings
	InputComponent->BindAction("ActiveToolExit", IE_Released, this, &AToolsContextActor::OnToolExit);

	// bound to ctrl+z in Project Settings
	InputComponent->BindAction("UndoAction", IE_Released, this, &AToolsContextActor::OnUndo);
	// bound to ctrl+y in the project settings
	InputComponent->BindAction("RedoAction", IE_Released, this, &AToolsContextActor::OnRedo);

	// bound to delete in the project settings
	InputComponent->BindAction("DeleteAction", IE_Released, this, &AToolsContextActor::OnDelete);
}

void AToolsContextActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AToolsContextActor::OnMoveForwardKeyAxis(float MoveDelta)
{
	if (CurrentInteractionMode == EToolActorInteractionMode::RightMouseCameraControl)
	{
		ADefaultPawn::MoveForward(MoveDelta);
	}
}

void AToolsContextActor::OnMoveRightKeyAxis(float MoveDelta)
{
	if (CurrentInteractionMode == EToolActorInteractionMode::RightMouseCameraControl)
	{
		ADefaultPawn::MoveRight(MoveDelta);
	}
}

void AToolsContextActor::OnMoveUpKeyAxis(float MoveDelta)
{
	if (CurrentInteractionMode == EToolActorInteractionMode::RightMouseCameraControl)
	{
		ADefaultPawn::MoveUp_World(MoveDelta);
	}
}

void AToolsContextActor::OnAltKeyDown()
{
	bIsAltKeyDown = true;

	// if we are in right-mouse camera mode and press alt, ignore it
	if (CurrentInteractionMode == EToolActorInteractionMode::RightMouseCameraControl)
	{
		ensure(ToolsSystem->IsCapturingMouse() == false);
	}
	else if (ToolsSystem->IsCapturingMouse())
	{
		// if tool system is capturing, ignore alt
	}
	else
	{
		// switch to alt camera control
		CurrentInteractionMode = EToolActorInteractionMode::AltCameraControl;
	}
}

void AToolsContextActor::OnAltKeyUp()
{
	bIsAltKeyDown = false;

	// if we are in right-mouse camera mode and release alt, ignore it
	if (CurrentInteractionMode == EToolActorInteractionMode::RightMouseCameraControl)
	{
		ensure(ToolsSystem->IsCapturingMouse() == false);
	}
	else if (ToolsSystem->IsCapturingMouse())
	{
		// if tool system is capturing, ignore alt
	}
	else
	{
		// switch out of alt camera control
		CurrentInteractionMode = EToolActorInteractionMode::NoInteraction;
	}
}

void AToolsContextActor::OnLeftMouseDown()
{
	/// debugging
	UE_LOG(LogTemp, Warning, TEXT("Left Down"));
	ADefaultPawn::MoveUp_World(-10000000.0);

	bIsLeftMouseDown = true;

	// if we are in right-mouse cam mode and press left, ignore it
	if (CurrentInteractionMode == EToolActorInteractionMode::RightMouseCameraControl)
	{
		ensure(ToolsSystem->IsCapturingMouse() == false);
	}
	else if (CurrentInteractionMode == EToolActorInteractionMode::AltCameraControl)
	{
		// do orbit
		ensure(ToolsSystem->IsCapturingMouse() == false);
		return;
	}
	else
	{
		ToolsSystem->OnLeftMouseDown();
	}
}

void AToolsContextActor::OnLeftMouseUp()
{
	/// debugging
	UE_LOG(LogTemp, Warning, TEXT("Left Up"));
	ADefaultPawn::MoveUp_World(10000000.0);

	bIsLeftMouseDown = false;

	// if we are in right-mouse cam mode and press left, ignore it
	if (CurrentInteractionMode == EToolActorInteractionMode::RightMouseCameraControl)
	{
		ensure(ToolsSystem->IsCapturingMouse() == false);
	}
	else if (CurrentInteractionMode == EToolActorInteractionMode::AltCameraControl)
	{
		// end orbit
		ensure(ToolsSystem->IsCapturingMouse() == false);
		return;
	}
	else
	{
		ToolsSystem->OnLeftMouseDown();
	}
}

void AToolsContextActor::OnMiddleMouseDown()
{
	bIsMiddleMouseDown = true;

	if (CurrentInteractionMode == EToolActorInteractionMode::AltCameraControl)
	{
		// want to start pan...
	}
}

void AToolsContextActor::OnMiddleMouseUp()
{
	bIsMiddleMouseDown = false;

	if (CurrentInteractionMode == EToolActorInteractionMode::AltCameraControl)
	{
		// want to end pan...
	}
}

void AToolsContextActor::OnRightMouseDown()
{
	bIsRightMouseDown = true;

	// ignore if tool system is capturing
	if (ToolsSystem && ToolsSystem->IsCapturingMouse())
	{
		ensure(CurrentInteractionMode == EToolActorInteractionMode::NoInteraction);
		return;
	}

	if (CurrentInteractionMode == EToolActorInteractionMode::NoInteraction)
	{
		CurrentInteractionMode = EToolActorInteractionMode::RightMouseCameraControl;
	}
	else if (CurrentInteractionMode == EToolActorInteractionMode::AltCameraControl)
	{
		// want to start dolly...
	}
}

void AToolsContextActor::OnRightMouseUp()
{
	bIsRightMouseDown = false;

	if (CurrentInteractionMode == EToolActorInteractionMode::RightMouseCameraControl)
	{
		CurrentInteractionMode = EToolActorInteractionMode::NoInteraction;
	}
	else if (CurrentInteractionMode == EToolActorInteractionMode::AltCameraControl)
	{
		// end dolly
	}
}

void AToolsContextActor::OnMouseMoveX(float MoveX)
{
	if (CurrentInteractionMode == EToolActorInteractionMode::RightMouseCameraControl)
	{
		ADefaultPawn::AddControllerYawInput(MoveX);
	}
	else if (CurrentInteractionMode == EToolActorInteractionMode::AltCameraControl)
	{
		if (bIsLeftMouseDown)
		{
			ADefaultPawn::AddControllerYawInput(MoveX);
		}
		else if (bIsMiddleMouseDown)
		{
			ADefaultPawn::MoveRight(100.0 * MoveX);
		}
		else if (bIsRightMouseDown)
		{
			ADefaultPawn::MoveForward(MoveX);
		}
	}
	else
	{
	}
}

void AToolsContextActor::OnMouseMoveY(float MoveY)
{
	if (CurrentInteractionMode == EToolActorInteractionMode::RightMouseCameraControl)
	{
		ADefaultPawn::AddControllerPitchInput(-MoveY);
	}
	else if (CurrentInteractionMode == EToolActorInteractionMode::AltCameraControl)
	{
		if (bIsLeftMouseDown)
		{
			ADefaultPawn::AddControllerPitchInput(-MoveY);
		}
		else if (bIsMiddleMouseDown)
		{
			ADefaultPawn::MoveUp_World(100.0 * MoveY);
		}
		else if (bIsRightMouseDown)
		{
			ADefaultPawn::MoveForward(-100.0 * MoveY);
		}
		// else RMB
	}
	else
	{
	}
}

void AToolsContextActor::OnToolAccept()
{
	ToolsSystem->AcceptActiveTool();
}

void AToolsContextActor::OnToolExit()
{
	ToolsSystem->CancelOrCompleteActiveTool();
}

void AToolsContextActor::OnUndo()
{
	ToolsSystem->GetSceneHistory()->Undo();
}

void AToolsContextActor::OnRedo()
{
	ToolsSystem->GetSceneHistory()->Redo();
}

void AToolsContextActor::OnDelete()
{
	if (ToolsSystem->HaveActiveTool() == false)
	{
		URuntimeMeshSceneSubsystem::Get()->DeleteSelectedSceneObjects();
	}
}
