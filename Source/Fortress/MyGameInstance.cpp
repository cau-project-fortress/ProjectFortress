#include "MyGameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

void UMyGameInstance::Init()
{
    Super::Init();

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        SessionInterface = Subsystem->GetSessionInterface();

        if (SessionInterface.IsValid())
        {
            // 델리게이트 바인딩
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMyGameInstance::OnCreateSessionComplete);
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMyGameInstance::OnFindSessionsComplete);
            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMyGameInstance::OnJoinSessionComplete);
        }
    }
}

void UMyGameInstance::CreateSession()
{
    if (!SessionInterface.IsValid()) return;

    SessionInterface->DestroySession(NAME_GameSession);

    FOnlineSessionSettings Settings;
    Settings.bIsLANMatch = true;
    Settings.NumPublicConnections = 4;
    Settings.bShouldAdvertise = true;
    Settings.bUsesPresence = false;
    Settings.Set(FName("MapName"), FString("Lv_FirstPerson"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    SessionInterface->CreateSession(0, NAME_GameSession, Settings);
}

void UMyGameInstance::OnCreateSessionComplete(FName SessionName, bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Session Create Success! Travel to Level"));
        GetWorld()->ServerTravel("/FirstPerson/Lv_FirstPerson?listen");
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Session Create Failed"));
    }
}

void UMyGameInstance::FindAndJoinSession()
{
    if (!SessionInterface.IsValid()) return;

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->bIsLanQuery = true;
    SessionSearch->MaxSearchResults = 10;

    // 이 부분에서 SEARCH_PRESENCE 관련 에러가 날 수 있어 명시적으로 처리
    SessionSearch->QuerySettings.Set(FName(TEXT("SEARCH_PRESENCE")), false, EOnlineComparisonOp::Equals);

    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UMyGameInstance::OnFindSessionsComplete(bool bSuccess)
{
    if (bSuccess && SessionSearch.IsValid() && SessionSearch->SearchResults.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Found %d Sessions"), SessionSearch->SearchResults.Num());
        SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[0]);
    }
}

void UMyGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        FString URL;
        if (SessionInterface->GetResolvedConnectString(NAME_GameSession, URL))
        {
            APlayerController* PC = GetFirstLocalPlayerController();
            if (PC) PC->ClientTravel(URL, ETravelType::TRAVEL_Absolute);
        }
    }
}