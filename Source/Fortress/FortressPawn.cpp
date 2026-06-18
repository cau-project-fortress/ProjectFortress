// Fill out your copyright notice in the Description page of Project Settings.


#include "FortressPawn.h"

// Sets default values
AFortressPawn::AFortressPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFortressPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFortressPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AFortressPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AFortressPawn::Server_Move_Implementation(float X, float Y)
{
	// 빌드오류수정
}

