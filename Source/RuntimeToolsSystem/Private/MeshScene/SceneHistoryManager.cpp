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


#include "MeshScene/SceneHistoryManager.h"

bool FChangeHistoryTransaction::HasExpired() const
{
	for (const FChangeHistoryRecord& Record : Records)
	{
		if (Record.ChangeWrapper->Change->HasExpired(Record.TargetObject) == false)
		{
			return false;
		}
	}
	return true;
}

void USceneHistoryManager::BeginTransaction(const FText& Description)
{
	if (BeginTransactionDepth != 0)
	{
		BeginTransactionDepth++;
	}
	else
	{
		TruncateHistory();

		ActiveTransaction = FChangeHistoryTransaction();
		ActiveTransaction.Descritpion = Description;

		BeginTransactionDepth++;
	}
}

void USceneHistoryManager::AppendChange(UObject* TargetObject, TUniquePtr<FCommandChange> Change, const FText& Description)
{
	bool bAutoCloseTransaction = false;
	if (ensure(BeginTransactionDepth > 0) == false)
	{
		BeginTransaction(Description);
		bAutoCloseTransaction = true;
	}

	FChangeHistoryRecord Record;
	Record.TargetObject = TargetObject;
	Record.Description = Description;
	Record.ChangeWrapper = MakeShared<FChangeHistoryRecord::FChangeWrapper>();
	Record.ChangeWrapper->Change = MoveTemp(Change);

	UE_LOG(LogTemp, Warning, TEXT("[HISTORY] %s"), *Record.Description.ToString());

	ActiveTransaction.Records.Add(MoveTemp(Record));

	if (bAutoCloseTransaction)
	{
		EndTransaction();
	}
}

void USceneHistoryManager::EndTransaction()
{
	if (ensure(BeginTransactionDepth > 0) == false) return;

	BeginTransactionDepth--;

	if (BeginTransactionDepth == 0)
	{
		if (ActiveTransaction.Records.Num() > 0)
		{
			Transactions.Add(MoveTemp(ActiveTransaction));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[EndTransactioon] Empty Transaction Record!"));
		}

		ActiveTransaction = FChangeHistoryTransaction();

		CurrentIndex = Transactions.Num();
	}
}

void USceneHistoryManager::TruncateHistory()
{
	// truncate history if we are in undo step
	if (CurrentIndex < Transactions.Num())
	{
		Transactions.SetNum(CurrentIndex);
	}
}

