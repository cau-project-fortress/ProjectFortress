#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FortressGameMode.generated.h"

/**
 * 모든 게임모드의 부모 클래스입니다.
 * 생성자에서 Seamless Travel 설정을 담당합니다.
 */
UCLASS()
class FORTRESS_API AFortressGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFortressGameMode();
};