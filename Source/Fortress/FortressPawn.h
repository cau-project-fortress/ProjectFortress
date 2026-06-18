// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FortressPawn.generated.h"

UCLASS()
class FORTRESS_API AFortressPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AFortressPawn();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ── 컴포넌트 ──────────────────────────
	UPROPERTY(VisibleAnywhere) USceneComponent* Root;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* BodyMesh;   // 요새 본체

	// ── 이동 수치 ─────────────────────────
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float TurnSpeed = 80.f;

private:	
	// 입력값 저장
	float MoveInput = 0.f;
	float TurnInput = 0.f;

	// 입력 바인딩 함수
	void OnMoveForward(float Value);
	void OnTurnRight(float Value);

	// 서버 RPC (멀티플레이용)
	UFUNCTION(Server, Reliable)
	void Server_Move(float ForwardVal, float TurnVal);
};
