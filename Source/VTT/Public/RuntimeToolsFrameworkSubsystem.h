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

#include "ToolsContextRenderComponent.h"
//#include "MeshScene/SceneHistoryManager.h"
//#include "Interaction/SceneObjectSelectionInteraction.h"
//#include "Interaction/SceneObjectTransformInteraction.h"

#include "RuntimeToolsFrameworkSubsystem.generated.h"

/**
 * Code and comments based on Ryan Schmidt's UE5RuntimeToolsFrameworkDemo
 * https://github.com/gradientspace/UE5RuntimeToolsFrameworkDemo
 */
UCLASS()
class VTT_API URuntimeToolsFrameworkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	//
	// Small hack to workaround the fact that you generally need rthe UGameInstance pointer to look up a GameInstance subsystem. We store the pointer abd then allow ::Get() to return it (ie actually a singleton)
	//
public:
	static void InitializeSingleton(URuntimeToolsFrameworkSubsystem* Subsystem);
	static URuntimeToolsFrameworkSubsystem* Get();
protected:
	static URuntimeToolsFrameworkSubsystem* InstanceSingleton;

	//
	// Functions to setup/shutdown/operate the RuntimeToolsFramework
	//
public:
	void InitializeToolsContext(UWorld* TargetWorld);
	virtual void Tick(float DeltaTime);


public:
	UPROPERTY()
	UWorld* TargetWorld;

	//UPROPERTY()
	//UInteractiveToolsContext* ToolsContext;
	
};
