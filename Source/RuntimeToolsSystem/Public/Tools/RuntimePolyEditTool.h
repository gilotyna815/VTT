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

#include "EditMeshPolygonsTool.h"

#include "RuntimePolyEditTool.generated.h"

UCLASS()
class RUNTIMETOOLSSYSTEM_API URuntimePolyEditToolBuilder : public UEditMeshPolygonsToolBuilder
{
	GENERATED_BODY()

public:
	virtual USingleTargetWithSelectionTool* CreateNewTool(const FToolBuilderState& SceneState) const override;
};

UCLASS(BlueprintType)
class RUNTIMETOOLSSYSTEM_API URuntimePolyEditToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()
};

UCLASS(BlueprintType)
class RUNTIMETOOLSSYSTEM_API URuntimePolyEditTool : public UEditMeshPolygonsTool
{
	GENERATED_BODY()
	
public:
	virtual void Setup() override;

	UPROPERTY(BlueprintReadOnly)
	URuntimePolyEditToolProperties* RuntimeProperties;

	UFUNCTION(BlueprintCallable)
	void BeginExtrudeAction();

	UFUNCTION(BlueprintCallable)
	void BeginInsetAction();

	UFUNCTION(BlueprintCallable)
	void BeginOutsetAction();

	UFUNCTION(BlueprintCallable)
	void BeginCutFaceAction();

	// Multiple base classes have a UWorld* TargetWorld, causes scoping problems...so we add a third that
	// we can rely on...
	UPROPERTY()
	TObjectPtr<UWorld> LocalTargetWorld = nullptr;

	// Override this to work around a bug in PolyEditActivityUtil::CreatePolyEditPreviewMesh() that calls
	// GetWorld instead of GetTargetWorld()
	virtual UWorld* GetWorld() const;
};
