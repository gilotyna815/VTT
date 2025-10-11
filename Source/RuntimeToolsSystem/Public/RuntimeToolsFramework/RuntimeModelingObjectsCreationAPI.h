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

#include "ModelingObjectsCreationAPI.h"

#include "RuntimeModelingObjectsCreationAPI.generated.h"

class UInteractiveToolsContext;

/**
 * Implementation of UModelingObjectsCreationAPI, which UE Modeling Tools use to
 * emit new "Mesh Objects (eg StaticMeshAsset+Actor, Dynamic MeshActor, AVolume).
 * Currently this API can only emit URuntimeMeshSceneObjects.
 * 
 * The UE Modeling Tools find an available UModelingObjectsCreationAPI by searching
 * in the ContextStore of their ToolsContext. So use the
 * URuntimeModelingObjectsCreationAPI::Register(ToolsContext) function to create
 * a new instance and set it in the ContextStore (and Deregister to remove it).
 * 
 * This is similiar to UEditorModelingObjectsCreationAPI, which is what Modeling Mode
 * in the UE Editor makes available to the Editor Tools.
 * 
 * CreateTexttureObject currently not supported.
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API URuntimeModelingObjectsCreationAPI : public UModelingObjectsCreationAPI
{
	GENERATED_BODY()

public:

	virtual FCreateMeshObjectResult CreateMeshObject(const FCreateMeshObjectParams& CreateMeshParams) override;

	// Call this to provide and instance of URuntimeModelingObjectsCreationAPI to the Tools
	static URuntimeModelingObjectsCreationAPI* Register(UInteractiveToolsContext* ToolsContext);
	// Call this to clean up the Register'd instance
	static bool Deregister(UInteractiveToolsContext* ToolsContext);
};
