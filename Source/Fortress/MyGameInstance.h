// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"          // 이 줄 추가
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class FORTRESS_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public: 
	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void CreateSession();

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void FindAndJoinSession();

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	void OnCreateSessionComplete(FName SessionName, bool bSuccess);
	void OnFindSessionsComplete(bool bSuccess);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
