// Fill out your copyright notice in the Description page of Project Settings.


#include "FogOfWar.h"
#include "Engine/TextureRenderTarget.h"

// Sets default values
AFogOfWar::AFogOfWar()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFogOfWar::BeginPlay()
{
	Super::BeginPlay();
	CreateTextureRender

	
}

// Called every frame
void AFogOfWar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

