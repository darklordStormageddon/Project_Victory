// Fill out your copyright notice in the Description page of Project Settings.

#include "KSM/MyGameInstance.h"
#include "OnlineSessionSettings.h"

const static FName SESSION_NAME = TEXT("GameSession"); //채널명



void UMyGameInstance::Init()
{
	Super::Init();
	GameUniqueTag = TEXT("SpaceMiningSimulator");

	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (OSS)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("OSS : %s is Avaliable."), *OSS->GetSubsystemName().ToString());

		SessionInterface = OSS->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnCreateSessionComplete);

			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnDestroySessionComplete);

			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnFindSessionComplete);

			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnJoinSessionComplete);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not found subsystem."));
	}

	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UMyGameInstance::OnNetworkFailure);
	}

}



void UMyGameInstance::Host(FString ServerName)
{
	RoomName = ServerName;
	if (SessionInterface.IsValid())
	{
		auto AlreadyExsistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if (AlreadyExsistingSession)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s already exists. Destroying and recreating session."), *SESSION_NAME.ToString());
			// 비동기 콜백에서만 CreateSession() 호출되도록 변경
			bIsHostingAfterDestroy = true;
			SessionInterface->DestroySession(SESSION_NAME);
		}
		else
		{
			CreateSession();
		}
	}
}
void UMyGameInstance::CreateSession()
{
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings SessionSettings;

		if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
			SessionSettings.bIsLANMatch = true;
		else
			SessionSettings.bIsLANMatch = false;


		SessionSettings.NumPublicConnections = 4;
		SessionSettings.bUsesPresence = SessionSettings.bShouldAdvertise = true;
		SessionSettings.bAllowJoinInProgress = true;
		SessionSettings.bAllowJoinViaPresence = true;
		//게임 식별 태그
		SessionSettings.Set(FName("GameUniqueTag"), GameUniqueTag, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		//게임 접근 태그
		SessionSettings.Set(TEXT("Public"), bIsPublic, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		//세션 이름과 비밀번호
		SessionSettings.Set(TEXT("SessionName"), RoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionSettings.Set(TEXT("Password"), Password, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
	}
}

void UMyGameInstance::RefreshServerList()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Finding Session"));
		//세션 100개 최대 찾아온다.
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")), true, EOnlineComparisonOp::Equals);
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UMyGameInstance::Join(int Index)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Join Failed: SessionInterface is invalid"));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Join Failed: SessionInterface is invalid"));
		return;
	}

	if (!SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Join Failed: SessionSearch is invalid"));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Join Failed: SessionSearch is invalid"));
		return;
	}

	if (SessionSearch->SearchResults.Num() <= Index)
	{
		UE_LOG(LogTemp, Error, TEXT("Join Failed: Invalid Index %d (Total: %d)"), Index, SessionSearch->SearchResults.Num());
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Join Failed: Invalid Index %d"), Index));
		return;
	}

	// Join 전에 기존 세션 파괴
	auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
	if (ExistingSession)
	{
		UE_LOG(LogTemp, Warning, TEXT("Destroying existing session before joining"));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Destroying existing session..."));

		// 델리게이트 바인딩 - Join을 위한 파괴 완료 처리
		FOnDestroySessionCompleteDelegate DestroyDelegate;
		DestroyDelegate.BindLambda([this, Index](FName SessionName, bool bWasSuccessful)
			{
				if (bWasSuccessful)
				{
					UE_LOG(LogTemp, Warning, TEXT("Session destroyed successfully, now joining..."));
					if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Session destroyed, joining..."));

					// 세션 파괴 후 Join 재시도
					if (SessionSearch.IsValid() && SessionSearch->SearchResults.Num() > Index)
					{
						SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to destroy existing session"));
					if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to destroy session"));
				}
			});

		SessionInterface->OnDestroySessionCompleteDelegates.Add(DestroyDelegate);
		SessionInterface->DestroySession(SESSION_NAME);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Attempting to join session at index %d"), Index);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Joining session %d..."), Index));

	SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
}

void UMyGameInstance::OnCreateSessionComplete(FName InSessionName, bool IsSuccess)
{
	if (!IsSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not Createsession"));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to create session"));
		return;
	}

	UEngine* Engine = GetEngine();
	if (!Engine) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UE_LOG(LogTemp, Warning, TEXT("Session created successfully, traveling to Lobby"));
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Session created! Traveling..."));

	//레벨(맵)
	World->ServerTravel("/Game/Import/Maps/Lobby?listen");
}


void UMyGameInstance::StartSession()
{
	if (SessionInterface.IsValid())
		SessionInterface->StartSession(SESSION_NAME);
}

void UMyGameInstance::OnDestroySessionComplete(FName InSessionName, bool IsSuccess)
{
	if (IsSuccess == true && bIsHostingAfterDestroy)
	{
		bIsHostingAfterDestroy = false;
		CreateSession();
	}
}

void UMyGameInstance::OnFindSessionComplete(bool IsSuccess)
{
	if (IsSuccess && SessionSearch.IsValid())
	{
		ServerNames.Empty();
		int temp_index = 0;

		UE_LOG(LogTemp, Warning, TEXT("Found %d sessions"), SessionSearch->SearchResults.Num());
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Found %d sessions"), SessionSearch->SearchResults.Num()));

		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Display, TEXT("Found Session name : %s"), *SearchResult.GetSessionIdStr());
			UE_LOG(LogTemp, Display, TEXT("Ping : %d"), SearchResult.PingInMs);

			FString tempName;
			SearchResult.Session.SessionSettings.Get(FName("SessionName"), tempName);
			if (!SearchName.IsEmpty() && SearchName != tempName)
				continue;

			FServerData ServerData;
			ServerData.CurrentPlayers = SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections;
			ServerData.HostUserName = SearchResult.Session.OwningUserName;
			SearchResult.Session.SessionSettings.Get(FName("Password"), ServerData.Password);
			SearchResult.Session.SessionSettings.Get(FName("SessionName"), ServerData.Name);
			SearchResult.Session.SessionSettings.Get(FName("Public"), ServerData.Accessibility);
			ServerData.SearchResultIndex = temp_index;

			temp_index++;
			ServerNames.Add(ServerData);
		}

		UE_LOG(LogTemp, Warning, TEXT("Finished Finding Session"));
		OnSessionListUpdated.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find sessions"));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to find sessions"));
	}
}

void UMyGameInstance::OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type InResult)
{
	UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionComplete called with result: %d"), (int32)InResult);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Join Result: %d"), (int32)InResult));

	if (SessionInterface.IsValid() == false)
	{
		UE_LOG(LogTemp, Error, TEXT("SessionInterface is invalid"));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("SessionInterface is invalid"));
		return;
	}

	if (InResult != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to join session. Result: %d"), (int32)InResult);

		FString ErrorMsg = TEXT("Join Failed: ");
		switch (InResult)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:
			ErrorMsg += TEXT("Session is full");
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			ErrorMsg += TEXT("Session does not exist");
			break;
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
			ErrorMsg += TEXT("Could not retrieve address");
			break;
		case EOnJoinSessionCompleteResult::AlreadyInSession:
			ErrorMsg += TEXT("Already in session");
			break;
		case EOnJoinSessionCompleteResult::UnknownError:
			ErrorMsg += TEXT("Unknown error");
			break;
		}

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, ErrorMsg);
		return;
	}

	FString Address;
	if (!SessionInterface->GetResolvedConnectString(InSessionName, Address))
		return;

	UE_LOG(LogTemp, Warning, TEXT("Joining session at: %s"), *Address);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Traveling to: %s"), *Address));

	UEngine* Engine = GetEngine();
	if (!Engine)
		return;

	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr)
		return;

	PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void UMyGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Error, TEXT("Network Failure - Type: %d, Error: %s"), (int32)FailureType, *ErrorString);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Network Error: %s"), *ErrorString));
}