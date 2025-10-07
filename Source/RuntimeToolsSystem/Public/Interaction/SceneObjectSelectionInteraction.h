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

#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "InputBehaviorSet.h"

#include "SceneObjectSelectionInteraction.generated.h"

/**
 * USceneObjectSelectionInteraction implements standard mouse-click selection
 * interaction of "Scene Objects", ie the URuntimeMeshSceneObjects in the current
 * URuntimeMeshSceneSubsystem.
 * 
 * - Left-Click on object changes active selection to that object
 * - Left-Click on 'background' clears active selection
 * - Shift+Click modifier adds to selection
 * - Ctrl+Click modifier toggles selected/deselected
 * 
 * Currently Hovering is not supported, but this would be relatively easy to add
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API USceneObjectSelectionInteraction
	: public UObject, public IInputBehaviorSource, public IClickBehaviorTarget
{
	GENERATED_BODY()

public:

	/**
	 * Set up the Interaction, create and register Behaviors/etc.
	 * @param CanChangeSelectionCallbackIn this function will be called to determine if
	 * the current Selection is allowed to be modified (for example, when a Tool is
	 * active, we may wish to lock selection)
	 */
	void Initialize(TUniqueFunction<bool()> CanChangeSelectionCallbackIn);

	// click-to-select behavior
	UPROPERTY()
	USingleClickInputBehavior* ClickBehavior;

	// set of all behaviors, will be passed up to UInputRouter
	UPROPERTY()
	UInputBehaviorSet* BehaviorSet;

	//
	// Inherited via IInputBehaviorSource
	//
	virtual const UInputBehaviorSet* GetInputBehaviors() const
	{
		return BehaviorSet;
	}

	//
	// Inherited via IClickBehaviorTarget
	//
	virtual FInputRayHit IsHitByClick(const FInputDeviceRay& ClickPos) override;
	virtual void OnClicked(const FInputDeviceRay& ClickPos) override;

	//
	// IModifierToggleBehaviorTarget implementation
	//
	virtual void OnUpdateModifierState(int ModifierID, bool bIsOn) override;

protected:

	//default change-selection callback alwaysn allows selection change
	TUniqueFunction<bool()> CanChangeSelectionCallback = []() { return true; };

	// flags used to identify behavior modifier keys/buttons
	static const int AddToSelectionModifier = 1;
	bool bAddToSelectionEnabled = false;

	static const int ToggleSelectionModifier = 2;
	bool bToggleSelectionEnabled = false;

};
