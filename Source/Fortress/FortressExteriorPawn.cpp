#include "FortressExteriorPawn.h"
#include "FortressPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

AFortressExteriorPawn::AFortressExteriorPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	//ChassisMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChassisMesh"));
	//RootComponent = ChassisMesh;

	// ���� �ùķ��̼� OFF
	//ChassisMesh->SetSimulatePhysics(false);

	// �⺻�� ����
	MaxMoveSpeed = 800.f;
	TurnSpeed = 90.f;
	RideHeight = 50.f;
	GroundTraceLength = 500.f;
	LocationInterpSpeed = 15.f;
	RotationInterpSpeed = 10.f;

	MoveInput = 0.f;
	TurnInput = 0.f;

	EngineSpeedMultiplier = 1.f;

	bReplicates = true;
}

void AFortressExteriorPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AFortressExteriorPawn::Server_SetMoveInput_Implementation(float Value)
{
	MoveInput = Value;
}

void AFortressExteriorPawn::Server_SetTurnInput_Implementation(float Value)
{
	TurnInput = Value;
}

void AFortressExteriorPawn::BeginPlay()
{
	Super::BeginPlay();

}

// �Է½�ȣ �����
void AFortressExteriorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 1. IMC ���
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
			else
			{
				// �ѱ� ���ڵ� ���� ������ ���� ����� ����
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("[ERROR] DefaultMappingContext is MISSING in Blueprint!"));
			}
		}
	}

	// 2. �׼� ���ε�
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFortressExteriorPawn::Move);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AFortressExteriorPawn::Move);
		}
		else
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("[ERROR] MoveAction is MISSING in Blueprint!"));
		}

		if (TurnAction)
		{
			EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &AFortressExteriorPawn::Turn);
			EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Completed, this, &AFortressExteriorPawn::Turn);
		}
		else
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("[ERROR] TurnAction is MISSING in Blueprint!"));
		}

		if (ExitAction)
		{
			EnhancedInputComponent->BindAction(ExitAction, ETriggerEvent::Started, this, &AFortressExteriorPawn::ExitVehicle);
		}
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("[ERROR] Enhanced Input Cast Failed!"));
	}
}

void AFortressExteriorPawn::Move(const FInputActionValue& Value)
{
	MoveInput = Value.Get<float>();
}

void AFortressExteriorPawn::Turn(const FInputActionValue& Value)
{
	TurnInput = Value.Get<float>();
}

void AFortressExteriorPawn::ExitVehicle(const FInputActionValue& Value)
{
	if (AFortressPlayerController* PC = Cast<AFortressPlayerController>(GetController()))
	{
		PC->Server_ExitVehicle();
	}
}

void AFortressExteriorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 회전(조향)
	if (FMath::Abs(TurnInput) > 0.01f)
	{
		FRotator DeltaRotation = FRotator(0.f, TurnInput * (TurnSpeed * EngineSpeedMultiplier) * DeltaTime, 0.f);
		AddActorLocalRotation(DeltaRotation);
	}

	// 2. 바닥 탐지 (완전히 삭제!!!)
	// Z축(높이)과 기울기(Pitch, Roll)는 언리얼 물리 엔진에 전적으로 맡깁니다.

	// 3. 전진/후진 (Sweep 켜서 산에 막히게 함)
	float TargetSpeed = MoveInput * (MaxMoveSpeed * EngineSpeedMultiplier);
	float Rate = (FMath::Abs(MoveInput) > 0.01f) ? MoveAcceleration : MoveDeceleration;
	CurrentMoveSpeed = FMath::FInterpConstantTo(CurrentMoveSpeed, TargetSpeed, DeltaTime, Rate);

	if (FMath::Abs(CurrentMoveSpeed) > 0.1f)
	{
		// 탱크가 바라보는 앞쪽으로 이동
		FVector DeltaLocation = GetActorForwardVector() * CurrentMoveSpeed * DeltaTime;

		// 🌟 핵심: 혹시라도 튕겨서 위로 뜨지 않게 이동 방향의 Z축을 0으로 묶어버립니다.
		DeltaLocation.Z = 0.f;

		// Sweep(true) 덕분에 산벽에 닿으면 전진을 멈춤
		AddActorWorldOffset(DeltaLocation, true);
	}

	// ==========================================
	// 4. 디버깅용 AcceptanceRadius 원 그리기
	// ==========================================
#if !UE_BUILD_SHIPPING // 게임 출시 버전에서는 그려지지 않게 보호
	// 원이 바닥에 파묻히지 않도록 탱크 중심에서 위로 살짝(예: 100cm) 올려서 그립니다.
	FVector CircleCenter = GetActorLocation() + FVector(0.f, 0.f, 100.f);

	// AcceptanceRadius 변수가 헤더에 선언되어 있다고 가정합니다. (예: 600.f)
	DrawDebugCircle(
		GetWorld(),
		CircleCenter,           // 원의 중심점
		AcceptanceRadius,       // 원의 반지름 (도달 허용 거리)
		50,                     // 각진 정도 (50이면 부드러운 원이 됨)
		FColor::Cyan,           // 원 색상 (눈에 잘 띄는 시안색)
		false,                  // 매 프레임 그릴 것이므로 영구적(Persistent) 유지 안 함
		-1.f,                   // 수명 (-1은 1프레임만 유지)
		0,                      // 우선순위
		5.f,                    // 선 두께
		FVector::RightVector,   // X축 방향 (바닥과 평행한 원을 만들기 위함)
		FVector::ForwardVector, // Y축 방향 (바닥과 평행한 원을 만들기 위함)
		false                   // 축(Axis) 표시 여부
	);
#endif
}