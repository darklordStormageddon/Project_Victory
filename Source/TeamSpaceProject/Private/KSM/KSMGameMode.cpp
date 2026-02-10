#include "KSM/KSMGameMode.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"

void AKSMGameMode::PreLogin(const FString& Options, const FString& Address,
    const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

    UE_LOG(LogTemp, Warning, TEXT("PreLogin: %s from %s"), *UniqueId.ToString(), *Address);
}

FString AKSMGameMode::InitNewPlayer(APlayerController* NewPlayerController,
    const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
    FString Result = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

    UE_LOG(LogTemp, Warning, TEXT("InitNewPlayer: %s"), *UniqueId.ToString());

    return Result;
}

void AKSMGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    UE_LOG(LogTemp, Warning, TEXT("PostLogin called"));

    // 클라이언트를 세션에 등록
    if (NewPlayer && NewPlayer->PlayerState)
    {
        IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
        if (OnlineSub)
        {
            IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
            if (Sessions.IsValid())
            {
                TSharedPtr<const FUniqueNetId> UserId = NewPlayer->PlayerState->GetUniqueId().GetUniqueNetId();
                if (UserId.IsValid())
                {
                    bool bWasInvited = false; // Steam에서는 false
                    bool bSuccess = Sessions->RegisterPlayer(NAME_GameSession, *UserId, bWasInvited);

                    UE_LOG(LogTemp, Warning, TEXT("RegisterPlayer in PostLogin: %d for %s"),
                        bSuccess, *UserId->ToString());
                }
            }
        }
    }
}