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

#include "RuntimeToolsFramework\RuntimeModelingObjectsCreationAPI.h"

#include "InteractiveToolsContext.h"
#include "ContextObjectStore.h"
#include "MeshScene/RuntimeMeshSceneSubsystem.h"

FCreateMeshObjectResult URuntimeModelingObjectsCreationAPI::CreateMeshObject(const FCreateMeshObjectParams& CreateMeshParams)
{
	// create new SceneObject
	URuntimeMeshSceneObject* SceneObject = URuntimeMeshSceneSubsystem::Get()->CreateNewSceneObject();

	// initialize the mesh, depending on whether we were passed a FMeshDescription or a FDynamicMesh3
	if (CreateMeshParams.MeshType == ECreateMeshObjectSourceMeshType::MeshDescription)
	{
		SceneObject->Initialize(CreateMeshParams.TargetWorld, & CreateMeshParams.MeshDescription.GetValue());
	}
	else
	{
		SceneObject->Initialize(CreateMeshParams.TargetWorld, & CreateMeshParams.DynamicMesh.GetValue());
	}

	SceneObject->SetTransform(CreateMeshParams.Transform);

	// return the created Actor and Component
	FCreateMeshObjectResult Result;
	Result.ResultCode = ECreateModelingObjectResult::Ok;
	Result.NewActor = SceneObject->GetActor();
	Result.NewComponent = SceneObject->GetMeshComponent();

	return Result;
}

URuntimeModelingObjectsCreationAPI* URuntimeModelingObjectsCreationAPI::Register(UInteractiveToolsContext* ToolsContext)
{
	check(ToolsContext);

	if (URuntimeModelingObjectsCreationAPI* CreationAPI = ToolsContext->ContextObjectStore->FindContext<URuntimeModelingObjectsCreationAPI>())
	{
		return CreationAPI;
	}
	auto NewCreationAPI = NewObject<URuntimeModelingObjectsCreationAPI>(ToolsContext);
	ToolsContext->ContextObjectStore->AddContextObject(NewCreationAPI);
	return NewCreationAPI;
}

bool URuntimeModelingObjectsCreationAPI::Deregister(UInteractiveToolsContext* ToolsContext)
{
	check(ToolsContext);

	if (URuntimeModelingObjectsCreationAPI* FoundAPI =
		ToolsContext->ContextObjectStore->FindContext<URuntimeModelingObjectsCreationAPI>())
	{
		ToolsContext->ContextObjectStore->RemoveContextObject(FoundAPI);
		return true;
	}
	return false;
}
