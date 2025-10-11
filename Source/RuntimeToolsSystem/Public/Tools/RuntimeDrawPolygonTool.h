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

#include "DrawPolygonTool.h"

#include "RuntimeDrawPolygonTool.generated.h"

/**
 * Tool builder for URuntimeDrawPolygonTool instances
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API URuntimeDrawPolygonToolBuilder : public UDrawPolygonToolBuilder
{
	GENERATED_BODY()

public:
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};

UENUM(BlueprintType)
enum class ERuntimeDrawPolygonType : uint8
{
	Freehand = 0,
	Circle = 1,
	Square = 2,
	Rectangle = 3,
	RoundedRectangle = 4,
	HoleyCircle = 5
};

UCLASS(BlueprintType)
class RUNTIMETOOLSSYSTEM_API URuntimeDrawPolygonToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	int SelectedPolygonType;
};

/**
 * Extension of UDrawPolygonTool that overrides EmitCurrentPolygon() to work at Runtime,
 * because the base implementation calls checkNoEntry() in non-Editor builds.
 */
UCLASS(BlueprintType)
class RUNTIMETOOLSSYSTEM_API URuntimeDrawPolygonTool : public UDrawPolygonTool
{
	GENERATED_BODY()
	
public:
	virtual void Setup() override;

	UPROPERTY(BlueprintReadOnly)
	URuntimeDrawPolygonToolProperties* RuntimeProperties;
};
