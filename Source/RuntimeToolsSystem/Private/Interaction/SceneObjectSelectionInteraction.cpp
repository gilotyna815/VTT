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

#include "Interaction/SceneObjectSelectionInteraction.h"

#include "MeshScene/RuntimeMeshSceneSubsystem.h"

void USceneObjectSelectionInteraction::Initialize(TUniqueFunction<bool()> CanChangeSelectionCallbackIn)
{
	CanChangeSelectionCallback = MoveTemp(CanChangeSelectionCallbackIn);

	//create click behavior and set ourselves as click target
	ClickBehavior = NewObject<USingleClickInputBehavior>();
	ClickBehavior->Modifiers.RegisterModifier(AddToSelectionModifier, FInputDeviceState::IsShiftKeyDown); // HARDCODED INPUT
	ClickBehavior->Modifiers.RegisterModifier(ToggleSelectionModifier, FInputDeviceState::IsCtrlKeyDown); // HARDCODED INPUT
	ClickBehavior->Initialize(this);

	BehaviorSet = NewObject<UInputBehaviorSet>();
	BehaviorSet->Add(ClickBehavior, this);
}

FInputRayHit USceneObjectSelectionInteraction::IsHitByClick(const FInputDeviceRay& ClickPos)
{
	FInputRayHit RayHit;

	if (CanChangeSelectionCallback() == false)
	{
		return RayHit;
	}

	FVector HitPoint, BaryCoordinates;
	float HitDistance;
	int32 HitTriangle;
	URuntimeMeshSceneObject* HitObject = URuntimeMeshSceneSubsystem::Get()->FindNearestHitObject(
		ClickPos.WorldRay.Origin, ClickPos.WorldRay.Direction, HitPoint, HitDistance, HitTriangle, BaryCoordinates);

	if (HitObject != nullptr)
	{
		RayHit.bHit = true;
		RayHit.HitDepth = HitDistance;
		//RayHit.HitNormal = ;			// todo - can compute from bary coords
		//RayHit.bHasHitNormal = ;		// todo - can compute from bary coords
		RayHit.HitIdentifier = HitTriangle;
		RayHit.HitOwner = HitObject;
	}
	else
	{
		RayHit.bHit = true;
		RayHit.HitDepth = TNumericLimits<float>::Max();
		RayHit.HitIdentifier = 0;
		RayHit.HitOwner = this;
	}

	return RayHit;
}

void USceneObjectSelectionInteraction::OnClicked(const FInputDeviceRay& ClickPos)
{
	FVector HitPoint, BaryCoordinates;
	float HitDistance;
	int32 HitTriangle;
	URuntimeMeshSceneObject* HitObject = URuntimeMeshSceneSubsystem::Get()->FindNearestHitObject(
		ClickPos.WorldRay.Origin, ClickPos.WorldRay.Direction, HitPoint, HitDistance, HitTriangle, BaryCoordinates);

	if (HitObject != nullptr)
	{
		if (bAddToSelectionEnabled)
		{
			URuntimeMeshSceneSubsystem::Get()->SetSelected(HitObject, false, false);
		}
		else if (bToggleSelectionEnabled)
		{
			URuntimeMeshSceneSubsystem::Get()->ToggleSelected(HitObject);
		}
		else
		{
			URuntimeMeshSceneSubsystem::Get()->SetSelected(HitObject, false, true);
		}
	}
	else
	{
		URuntimeMeshSceneSubsystem::Get()->ClearSelection();
	}

}

void USceneObjectSelectionInteraction::OnUpdateModifierState(int ModifierID, bool bIsOn)
{
	// update modifier state flags
	if (ModifierID == AddToSelectionModifier)
	{
		bAddToSelectionEnabled = bIsOn;
	}
	else if (ModifierID == ToggleSelectionModifier)
	{
		bToggleSelectionEnabled = bIsOn;
	}
}

