// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "VTTSaveGame.generated.h"

/**
 * Base SaveGame object for VTT.
 */
UCLASS(abstract, Blueprintable, BlueprintType)
class SAVESYSTEM_API UVTTSaveGame : public ULocalPlayerSaveGame
{
	GENERATED_BODY()
};
