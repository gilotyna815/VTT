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
#include "MeshScene/RuntimeMeshSceneObject.h"

#include "RuntimeMeshSceneSubsystem.generated.h"

class FMeshSceneSelectionChange;
class FAddRemoveSceneObjectChange;

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
class RUNTIMETOOLSSYSTEM_API URuntimeMeshSceneSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	static void InitializeSingleton(URuntimeMeshSceneSubsystem* Subsystem);
	static URuntimeMeshSceneSubsystem* Get();

protected:
	static URuntimeMeshSceneSubsystem* InstanceSingleton;

public:
	virtual void Deinitialize() override;


	virtual void SetCurrentTransactionsAPI(IToolsContextTransactionsAPI* TransactionsAPI);

public:

	UPROPERTY()
	UMaterialInterface* StandardMaterial;

	UPROPERTY()
	UMaterialInterface* SelectedMaterial;

public:

	UFUNCTION(BlueprintCallable)
	URuntimeMeshSceneObject* CreateNewSceneObject();

	UFUNCTION(BlueprintCallable)
	URuntimeMeshSceneObject* FindSceneObjectByActor(AActor* Actor);

	UFUNCTION(BlueprintCallable)
	bool DeleteSelectedSceneObjects();
	bool DeleteSelectedSceneObjects(AActor* SkipActor);

public:

	UFUNCTION(BlueprintCallable, Category="URuntimeMeshSceneSubsystem")
	TArray<URuntimeMeshSceneObject*> GetSelection() const {	return SelectedSceneObjects; }

	UFUNCTION(BlueprintCallable, Category = "URuntimeMeshSceneSubsystem")
	void SetSelected(URuntimeMeshSceneObject* SceneObject, bool bDeselect = false, bool bDeselectOthers = true);

	UFUNCTION(BlueprintCallable, Category = "URuntimeMeshSceneSubsystem")
	void ToggleSelected(URuntimeMeshSceneObject* SceneObject);

	UFUNCTION(BlueprintCallable, Category="URuntimeMeshSceneSubsystem")
	void ClearSelection();

	DECLARE_MULTICAST_DELEGATE_OneParam(FMeshSceneSelectionChangedEvent, URuntimeMeshSceneSubsystem*)
	FMeshSceneSelectionChangedEvent OnSelectionModified;

public:

	UFUNCTION(BlueprintCallable, Category = "URuntimeMeshSceneSubsystem")
	URuntimeMeshSceneObject* FindNearestHitObject(FVector RayOrigin, FVector RayDirection, FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle, FVector& TriBaryCoordinates, float MaxDistance = 0);

protected:
	IToolsContextTransactionsAPI* TransactionsAPI = nullptr;

	UPROPERTY()
	TArray<URuntimeMeshSceneObject*> SceneObjects;

	void AddSceneObjectInternal(URuntimeMeshSceneObject* Object, bool bIsUndoRedo);
	void RemoveSceneObjectInternal(URuntimeMeshSceneObject* Object, bool bIsUndoRedo);

	UPROPERTY()
	TArray<URuntimeMeshSceneObject*> SelectedSceneObjects;

	void SetSelectionInternal(const TArray<URuntimeMeshSceneObject*>& NewSceneObjects);

	TUniquePtr<FMeshSceneSelectionChange> ActiveSelectionChange;
	void BeginSelectionChange();
	void EndSelectionChange();

	friend class FMeshSceneSelectionChange;
	friend class FAddRemoveSceneObjectChange;
};

/**
 * FMeshSelectionChange represents a reversible change to UMeshSelectionSet
 */
class RUNTIMETOOLSSYSTEM_API FMeshSceneSelectionChange : public FToolCommandChange
{
public:
	TArray<URuntimeMeshSceneObject*> OldSelection;
	TArray<URuntimeMeshSceneObject*> NewSelection;

	virtual void Apply(UObject* Object) override;
	virtual void Revert(UObject* Object) override;
	virtual FString ToString() const override { return TEXT("FMeshSceneSelectionChange"); }
};

class RUNTIMETOOLSSYSTEM_API FAddRemoveSceneObjectChange : public FToolCommandChange
{
public:
	URuntimeMeshSceneObject* SceneObject;
	bool bAdded = true;

public:
	virtual void Apply(UObject* Object) override;
	virtual void Revert(UObject* Object) override;
	virtual FString ToString() const override { return TEXT("FAddRemoveSceneObjectChange"); }
};