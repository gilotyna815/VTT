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
#include "Misc/Change.h"
#include "SceneHistoryManager.generated.h"

/**
 * FChangeHistoryRecord is an (UObject, FCommandChange, Description) tuple, as a UStruct so that the UObject can be kept alive for GC purposes 
 */
USTRUCT()
struct FChangeHistoryRecord
{
	GENERATED_BODY()

	UPROPERTY()
	UObject* TargetObject = nullptr;

	UPROPERTY()
	FText Description;

	// UStruct needs to be copyable so we need to wrap the TUniquePtr in a TSharedPtr. Gross.
	struct FChangeWrapper
	{
		TUniquePtr<FCommandChange> Change;
	};

	TSharedPtr<FChangeWrapper> ChangeWrapper;
};

/**
 * FChangeHistoryTransaction stores a list of FChangeHistoryRecords
 */
USTRUCT()
struct FChangeHistoryTransaction
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FChangeHistoryRecord> Records;

	UPROPERTY()
	FText Descritpion;

	/** @return true if all contained FCommandChanges have Expired. In this case, the entire Transaction will have no effect, and should be skipped in Undo/Redo. */
	bool HasExpired() const;
};

/**
 * USceneHistoryManager provides a simple undo/redo history stack.
 * In this context a "Transaction" is not a UObject property transaction, but simply a list of FCommandChange objects.
 * Transaction terminology is used here to make the relationship clear with various InteractiveToolsFrameworkAPIs.
 * 
 * Note that FCommandChange's are paired with UObjects, similar to the Editor Transaction system.
 * This is necessary because the Change implementations generally do not store the target UObject (because that's how the Editor works).
 * The FChangeHistoryTransaction and FChangeHistoryRecord structs are UStructs, stored in UProperties, so they will keep these UObjects and prevent from being garbage collected (again, similar to the UE Editor)
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API USceneHistoryManager : public UObject
{
	GENERATED_BODY()
	
public:

	/** Open a new Transaction, ie list of (UObject, FCommandChange) pairs */
	void BeginTransaction(const FText& Description);

	/** @return true if we are inside an open Transaction */
	UFUNCTION(BlueprintCallable)
	bool IsBuildingTransaction() const { return BeginTransactionDepth > 0; }

protected:

	// undo history, stored as a set of transactions, which are themselves list of (UObject, FCommandChange) pairs
	UPROPERTY()
	TArray<FChangeHistoryTransaction> Transactions;

	// current index is Transactions list, will be Transactions.Num() unless user is Undo()/Redo()-ing
	int32 CurrentIndex = 0;

	// remove any elements of Transactions list beyond CurrentIndex (called if we are in Undo state and a new transaction is opened)
	void TruncateHistory();

	// transaction currently being built
	UPROPERTY()
	FChangeHistoryTransaction ActiveTransaction;

	int BeginTransactionDepth = 0;
};
