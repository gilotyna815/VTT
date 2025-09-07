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

#include "MeshScene/RuntimeMeshSceneSubsystem.h"

#define LOCTEXT_NAMESPACE "URuntimeMeshSceneSubsystem"

URuntimeMeshSceneSubsystem* URuntimeMeshSceneSubsystem::InstanceSingleton = nullptr;

void URuntimeMeshSceneSubsystem::InitializeSingleton(URuntimeMeshSceneSubsystem* Subsystem)
{
	check(InstanceSingleton == nullptr);
	InstanceSingleton = Subsystem;
}

URuntimeMeshSceneSubsystem* URuntimeMeshSceneSubsystem::Get()
{
	return InstanceSingleton;
}

void URuntimeMeshSceneSubsystem::Deinitialize()
{
	InstanceSingleton = nullptr;
}

void URuntimeMeshSceneSubsystem::SetCurrentTransactionsAPI(IToolsContextTransactionsAPI* TransactionsAPIIn)
{
	TransactionsAPI = TransactionsAPIIn;
}

void URuntimeMeshSceneSubsystem::SetSelected(URuntimeMeshSceneObject* SceneObject, bool bDeselect, bool bDeselectOthers)
{
	if (bDeselect)
	{
		if (SelectedSceneObjects.Contains(SceneObject))
		{
			BeginSelectionChange();
			SelectedSceneObjects.Remove(SceneObject);
			SceneObject->ClearHighlightMaterial();
			EndSelectionChange();
			OnSelectionModified.Broadcast(this);
		}
	}
	else
	{
		BeginSelectionChange();

		bool bIsSelected = SelectedSceneObjects.Contains(SceneObject);
		if (bDeselectOthers)
		{
			for (URuntimeMeshSceneObject* SelectedSceneObject : SelectedSceneObjects)
			{
				if (SelectedSceneObject != SceneObject)
				{
					SelectedSceneObject->ClearHighlightMaterial();
				}
			}
			SelectedSceneObjects.Reset();
		}
		if (bIsSelected == false)
		{
			SceneObject->SetToHighlightMaterial(this->SelectedMaterial);
		}
		SelectedSceneObjects.Add(SceneObject);

		EndSelectionChange();
		OnSelectionModified.Broadcast(this);
	}
}

void URuntimeMeshSceneSubsystem::ToggleSelected(URuntimeMeshSceneObject* SceneObject)
{
	BeginSelectionChange();

	if (SelectedSceneObjects.Contains(SceneObject))
	{
		SelectedSceneObjects.Remove(SceneObject);
		SceneObject->ClearHighlightMaterial();
	}
	else
	{
		SelectedSceneObjects.Add(SceneObject);
		SceneObject->SetToHighlightMaterial(this->SelectedMaterial);
	}

	EndSelectionChange();
	OnSelectionModified.Broadcast(this);
}

void URuntimeMeshSceneSubsystem::ClearSelection()
{
	if (SelectedSceneObjects.Num() > 0)
	{
		BeginSelectionChange();

		for (URuntimeMeshSceneObject* SelectedSceneObject : SelectedSceneObjects)
		{
			SelectedSceneObject->ClearHighlightMaterial();
		}
		SelectedSceneObjects.Reset();

		EndSelectionChange();
		OnSelectionModified.Broadcast(this);
	}
}

URuntimeMeshSceneObject* URuntimeMeshSceneSubsystem::FindNearestHitObject(FVector RayOrigin, FVector RayDirection, FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle, FVector& TriBaryCoordinates, float MaxDistance)
{
	URuntimeMeshSceneObject* FoundHit = nullptr;
	float MinHitDistance = TNumericLimits<float>::Max();

	for (URuntimeMeshSceneObject* SceneObject : SceneObjects)
	{
		FVector HitPoint, BaryCoordinates;
		float SceneObjectHitDistance;
		int32 SceneObjectNearestTriangle;
		if (SceneObject->IntersectRay(RayOrigin, RayDirection, HitPoint, SceneObjectHitDistance, SceneObjectNearestTriangle, BaryCoordinates, MaxDistance))
		{
			if (SceneObjectHitDistance < MinHitDistance)
			{
				MinHitDistance = SceneObjectHitDistance;
				WorldHitPoint = HitPoint;
				HitDistance = SceneObjectHitDistance;
				NearestTriangle = SceneObjectNearestTriangle;
				TriBaryCoordinates = BaryCoordinates;
				FoundHit = SceneObject;
			}
		}
	}

	return FoundHit;
}

void URuntimeMeshSceneSubsystem::SetSelectionInternal(const TArray<URuntimeMeshSceneObject*>& NewSceneObjects)
{
	if (SelectedSceneObjects.Num() > 0)
	{
		for (URuntimeMeshSceneObject* SceneObject : SelectedSceneObjects)
		{
			SceneObject->ClearHighlightMaterial();
		}
		SelectedSceneObjects.Reset();
	}

	for (URuntimeMeshSceneObject* SceneObject : NewSceneObjects)
	{
		if (SceneObjects.Contains(SceneObject))
		{
			if (SelectedSceneObjects.Contains(SceneObject) == false)
			{
				SelectedSceneObjects.Add(SceneObject);
				SceneObject->SetToHighlightMaterial(this->SelectedMaterial);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[URuntimeMeschSceneSubsystem::SetSelectionInternal] tried to select nonexitent SceneObject"));
		}
	}

	OnSelectionModified.Broadcast(this);
}

void URuntimeMeshSceneSubsystem::BeginSelectionChange()
{
	check(!ActiveSelectionChange);

	ActiveSelectionChange = MakeUnique<FMeshSceneSelectionChange>();
	ActiveSelectionChange->OldSelection = SelectedSceneObjects;
}

void URuntimeMeshSceneSubsystem::EndSelectionChange()
{
	check(ActiveSelectionChange);

	if (SelectedSceneObjects != ActiveSelectionChange->OldSelection)
	{
		ActiveSelectionChange->NewSelection = SelectedSceneObjects;

		if (TransactionsAPI)
		{
			TransactionsAPI->AppendChange(this, MoveTemp(ActiveSelectionChange), LOCTEXT("SelectionChange", "SelectionChange"));
		}
	}

	ActiveSelectionChange = nullptr;
}

void FMeshSceneSelectionChange::Apply(UObject* Object)
{
	if (URuntimeMeshSceneSubsystem* Subsystem = Cast<URuntimeMeshSceneSubsystem>(Object))
	{
		Subsystem->SetSelectionInternal(NewSelection);
	}
}

void FMeshSceneSelectionChange::Revert(UObject* Object)
{
	if (URuntimeMeshSceneSubsystem* Subsystem = Cast<URuntimeMeshSceneSubsystem>(Object))
	{
		Subsystem->SetSelectionInternal(OldSelection);
	}
}
