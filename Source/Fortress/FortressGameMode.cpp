#include "FortressGameMode.h"

AFortressGameMode::AFortressGameMode()
{
	// 멀티플레이어 레벨 전환 시 연결 유지를 위한 핵심 설정
	// 이 클래스를 상속받는 모든 BP_GameMode에 적용됩니다.
	bUseSeamlessTravel = true;
}