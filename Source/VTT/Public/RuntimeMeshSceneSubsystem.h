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
#include "Subsystems/GameInstanceSubsystem.h"
#include "InteractiveToolsContext.h"

#include "RuntimeMeshSceneSubsystem.generated.h"

/**
 * URuntimeMeshSceneSubsystem manages a "Scene" of "SceneObjects", currently only URuntimeMeshSceneObject (SO).
 *
 * Use CreateNewSceneObject() to create a new SO, and the various Delete functions to remove them.
 * These changes will be undo-able, ie they will send Change events to the USceneHistoryManager instance.
 *
 * An active Selection Set is tracked, and there are API functions for modifying this Selection set, also undo-able.
 *
 * Cast rays into the scene using FindNearestHitObject()
 */
UCLASS()
class VTT_API URuntimeMeshSceneSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	static void InitializeSingleton(URuntimeMeshSceneSubsystem* Subsystem);
	static URuntimeMeshSceneSubsystem* Get();

protected:
	static URuntimeMeshSceneSubsystem* InstanceSingleton;
};
