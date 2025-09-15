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
#include "UObject/NoExportTypes.h"
#include "SceneObjectTransformInteraction.generated.h"

class UTransformProxy;
class UCombinedTransformGizmo;
class URuntimeMeshSceneObject;

/**
 * USceneObjectTransformInteraction manages a 3D Translate/Rotate/Scale (TRS) Gizmo for the
 * current URuntimeMeshSceneObject selection set (stored in URuntimeMeshSceneSubsystem).
 *
 * Gizmo local/global frame is not controlled here, the Gizmo looks this information up itself
 * based on the EToolContextCoordinateSystem provided by the IToolsContextQueriesAPI implementation
 * in URuntimeToolsFrameworkSubsystem. You can configure the Gizmo to ignore this, in UpdateGizmoTargets()
 *
 * Behavior of the TRS Gizmo (ie pivot position, etc) is controlled by a standard UTransformProxy.
 * See UTransformMeshesTool for sample code for doing things like modifying pivot dynamically/etc.
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API USceneObjectTransformInteraction : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Set up the transform interaction.
	 * @param GizmoEnabledCallbackIn callback that determines if Gizmo should be created and visible. For example during a Tool we generally want to hide the TRS Gizmo.
	 */
	void Initialize(TUniqueFunction<bool()> GizmoEnabledCallbackIn);

	void Shutdown();

	// Recreate Gizmo. Call when external state changes, like set of selected objects.
	UFUNCTION(BlueprintCallable)
	void ForceUpdateGizmoState();

protected:

	FDelegateHandle SelectionChangedEventHandle;

	UPROPERTY()
	UTransformProxy* TransformProxy;

	UPROPERTY()
	UCombinedTransformGizmo* TransformGizmo;

	void UpdateGizmoTargets(const TArray<URuntimeMeshSceneObject*>& Selection);
	
	bool bEnableScaling = true;
	bool bEnableNonUniformScaling = true;

	TUniqueFunction<bool()> GizmoEnabledCallback = [&]() { return true; };
};
