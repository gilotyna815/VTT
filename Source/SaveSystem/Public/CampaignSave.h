// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VTTSaveGame.h"
#include "CampaignSave.generated.h"

/**
 * SaveGame object containing data associated with a Campaign.
 */
UCLASS(Blueprintable, BlueprintType)
class SAVESYSTEM_API UCampaignSave : public UVTTSaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, Category = "SaveGame Metadata")
	FString CampaignName;
};
