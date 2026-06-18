#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "FortressExteriorPawn.generated.h"

UCLASS()
class AFortressExteriorPawn : public APawn
{
	GENERATED_BODY()

public:
	AFortressExteriorPawn();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	// ��ü�� ���� �޽� (���� �ùķ��̼��� �ݵ�� ���� �մϴ�)
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	//class UStaticMeshComponent* ChassisMesh;

	// ==========================================
	// �̵� �� ���� ���� ����
	// ==========================================
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxMoveSpeed; // ����/���� �ӵ�

	UPROPERTY(EditAnywhere, Category = "Movement")
	float TurnSpeed; // ���ڸ� ȸ��(����) �ӵ�

	// How fast the tank ramps up to MaxMoveSpeed (units/sec²)
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveAcceleration = 400.f;

	// How fast the tank brakes to a stop when no input (units/sec²)
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveDeceleration = 600.f;

	// Runtime: actual current speed this frame (cosmetic, not replicated)
	float CurrentMoveSpeed = 0.f;

	UPROPERTY(EditAnywhere, Category = "Hover")
	float RideHeight; // �ٴڿ��� ��ü�� ��� ����

	UPROPERTY(EditAnywhere, Category = "Hover")
	float GroundTraceLength; // �ٴ��� Ž���� ������ ����

	UPROPERTY(EditAnywhere, Category = "Hover")
	float LocationInterpSpeed; // ���� ���̿� �������� �ε巯�� ����

	UPROPERTY(EditAnywhere, Category = "Hover")
	float RotationInterpSpeed; // ���� ���⿡ �������� �ε巯�� ����

	// ==========================================
	// ���� �Է� (Enhanced Input) ����
	// ==========================================
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* TurnAction;

	// Bound to "E" — calls Server_ExitVehicle on the controller
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* ExitAction;

	void ChooseNewTargetPart();

	// ���� �Է°� ����� ����
	float MoveInput;
	float TurnInput;

	// �Է� ó�� �Լ�
	void Move(const FInputActionValue& Value);
	void Turn(const FInputActionValue& Value);
	void ExitVehicle(const FInputActionValue& Value);

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Server RPCs - called by BP_TankSeat to drive this pawn (multiplayer-ready)
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Input")
	void Server_SetMoveInput(float Value);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Input")
	void Server_SetTurnInput(float Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AcceptanceRadius = 600.f;;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float EngineSpeedMultiplier;

	// 탱크 부위가 멀쩡한지 확인하는 BP 이벤트
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Combat")
	bool IsPartAlive(FName PartTag);
};