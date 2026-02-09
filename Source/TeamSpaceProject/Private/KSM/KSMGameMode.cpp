#include "KSM/KSMGameMode.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"

void AKSMGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    //OnlineSubsystem에서 세션 인터페이스 가져오기
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("No OnlineSubsystem found"));
        return;
    }

    IOnlineSessionPtr Sessions = Subsystem->GetSessionInterface();
    if (!Sessions.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Session Interface invalid"));
        return;
    }

    //이 조건을 제거하거나 수정
    if (NewPlayer)
    {
        APlayerState* PlayerState = NewPlayer->PlayerState;
        if (PlayerState && PlayerState->GetUniqueId().IsValid())
        {
            TSharedPtr<const FUniqueNetId> UniqueId = PlayerState->GetUniqueId().GetUniqueNetId();
            if (UniqueId.IsValid())
            {
                //세션에 플레이어 등록
                bool bSuccess = Sessions->RegisterPlayer(NAME_GameSession, *UniqueId, false);

                if (bSuccess)
                {
                    UE_LOG(LogTemp, Log, TEXT("Player %s registered to session (IsLocal: %d)"),
                        *UniqueId->ToString(), NewPlayer->IsLocalController());
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to register player %s to session"),
                        *UniqueId->ToString());
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("UniqueId is invalid for player %s"),
                    *NewPlayer->GetName());
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerState is null or UniqueId invalid for %s"),
                *NewPlayer->GetName());
        }
    }
}