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

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/DefaultPawn.h"
#include "ToolsContextActor.generated.h"

class URuntimeToolsFrameworkSubsystem;

/**
 * Camera-control modes that AToolsContextActor has implemented
 */
UENUM()
enum class EToolActorInteractionMode : uint8
{
	NoInteraction,
	RightMouseCameraControl,
	AltCameraControl
};

/**
 * AToolsContextActor is the Pawn used in the AToolsFrameworkDemoGameModeBase.
 * This Game Mode initializes the URuntimeMeshSceneSubsystem and
 * URuntimeToolsFrameworkSubsystem.
 * Essentially, this Actor has two jobs:
 *
 * 1) to forward Input events (from the PlayerInputComponent) to these subsystems,
 * both mouse events and Actions configured in the project settings.
 *
 * 2) to implement "camera control", as the rendering is done from this actors position (ie
 * as a first-person view). The base ADefaultPawn implements standard right-mouse-fly
 * controls, and so if right mouse is held down, we just call those functions. In addition,
 * Hold-alt camera controls are also done here, rather than using the ITF. (*However*, the
 * alt-controls are not fully implemented, ie it is not Maya-style alt+3-mouse-button
 * controls)
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API AToolsContextActor : public ADefaultPawn
{
	GENERATED_BODY()
	

public:
	// Sets default values for this pawn's properties
	AToolsContextActor();

	// set in PossessedBy(), we keep track of this so that the ToolsFramework can access it
	// to figure out cursor rays
	UPROPERTY()
	APlayerController* PlayerController;

	// return current interaction mode, ie are we in a camera interaction or not
	EToolActorInteractionMode GetCurrentInteractionMode() { return CurrentInteractionMode; }

protected:
	// Called when the game starts or spawned
	virtual void BeginPlay() override;

	// Called when despawned
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// called on startup
	virtual void PossessedBy(AController* ControllerIn) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	URuntimeToolsFrameworkSubsystem* ToolsSystem;

	EToolActorInteractionMode CurrentInteractionMode = EToolActorInteractionMode::NoInteraction;
	bool bIsAltKeyDown = false;
	bool bIsLeftMouseDown = false;
	bool bIsMiddleMouseDown = false;
	bool bIsRightMouseDown = false;

	virtual void OnMoveForwardKeyAxis(float MoveDelta);
	virtual void OnMoveRightKeyAxis(float MoveDelta);
	virtual void OnMoveUpKeyAxis(float MoveDelta);

	virtual void OnAltKeyDown();
	virtual void OnAltKeyUp();

	virtual void OnLeftMouseDown();
	virtual void OnLeftMouseUp();

	virtual void OnMiddleMouseDown();
	virtual void OnMiddleMouseUp();

	virtual void OnRightMouseDown();
	virtual void OnRightMouseUp();

	virtual void OnMouseMoveX(float MoveX);
	virtual void OnMouseMoveY(float MoveY);

	virtual void OnToolAccept();
	virtual void OnToolExit();

	virtual void OnUndo();
	virtual void OnRedo();

	virtual void OnDelete();
};
