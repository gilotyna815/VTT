// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VTTSaveGame.h"
#include "PlayerSave.generated.h"

/**
 * SaveGame object containing data that should be the owning player enters the game.
 */
UCLASS(Blueprintable, BlueprintType)
class SAVESYSTEM_API UPlayerSave : public UVTTSaveGame
{
	GENERATED_BODY()
	
};
