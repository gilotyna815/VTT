// Fill out your copyright notice in the Description page of Project Settings.


#include "FogOfWarCPP.h"

// Sets default values
AFogOfWarCPP::AFogOfWarCPP()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFogOfWarCPP::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFogOfWarCPP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFogOfWarCPP::CheckLinesOfSight()
{

}