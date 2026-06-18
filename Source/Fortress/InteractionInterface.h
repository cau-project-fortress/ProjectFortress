// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
// AActor 타입을 인식하기 위해 아래 헤더를 반드시 추가해야 합니다.
#include "GameFramework/Actor.h" 
#include "InteractionInterface.generated.h"

// BlueprintType을 추가해야 블루프린트 변수 등에서 이 인터페이스를 타입으로 쓸 수 있습니다.
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * */
class FORTRESS_API IInteractionInterface
{
	GENERATED_BODY()

public:
	// 1. virtual 키워드를 붙여야 상속받는 클래스에서 재정의가 가능합니다.
	// 2. BlueprintNativeEvent는 C++와 블루프린트 양쪽에서 구현할 수 있게 해줍니다.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void OnInteract(AActor* Interactor);
};