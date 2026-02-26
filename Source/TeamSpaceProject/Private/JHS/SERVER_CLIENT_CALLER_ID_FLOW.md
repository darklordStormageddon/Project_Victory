# 언리얼 리슨 서버 - 서버/클라이언트 호출자 ID 판별 로직 문서

## 개요
이 문서는 언리얼 리슨 서버 환경에서 서버와 클라이언트 간 호출자를 구별하기 위한 AssignedPlayerId 시스템의 전체 흐름을 설명합니다.

---

## 1. 시스템 아키텍처

### 핵심 컴포넌트
- **AJHSGameMode**: 서버 전용, 플레이어 접속 시 ID 발급
- **AJHSGameState**: 서버/클라이언트 모두 존재, 게임 상태 관리
- **UPlayerStateGroup**: 플레이어 등록 및 ID 발급 담당
- **AJHSPlayerState**: 플레이어별 상태, AssignedPlayerId 복제
- **AJHSPlayerController**: 플레이어 컨트롤러, AssignedPlayerId 복제
- **UInteractableComponent**: 인터랙션 대상 (레벨 액터에 부착)
- **UInteracterComponent**: 인터랙션 주체 (Pawn에 부착)

### 복제 구조
```
서버에서 발급 → PlayerState 복제 → PlayerController 복제 → 모든 클라이언트
```

---

## 2. ID 발급 및 복제 단계

### 2.1 플레이어 접속 및 ID 발급

#### **메서드: AJHSGameMode::PostLogin**
- **호출 시점**: 플레이어가 서버에 접속할 때 (서버 전용)
- **위치**: `JHSGameMode.cpp:33`
- **역할**: 새로 접속한 플레이어에게 고유 ID를 발급하고 등록

**실행 흐름:**
```cpp
void AJHSGameMode::PostLogin(APlayerController* NewPlayer)
{
    // 1. 부모 클래스 PostLogin 호출
    Super::PostLogin(NewPlayer);
    
    // 2. PlayerState 가져오기
    AJHSPlayerState* _jhsPS = NewPlayer->GetPlayerState<AJHSPlayerState>();
    
    // 3. GameState 및 PlayerStateGroup 가져오기
    AJHSGameState* _gameState = GetGameState<AJHSGameState>();
    
    // 4. 플레이어 등록 시도 (ID 발급)
    E_REGIST_ERROR_TYPE _errorType = E_REGIST_ERROR_TYPE::NONE;
    _gameState->GetPlayerStateGroup()->TryRegistPlayer(_jhsPS, _playerName, _errorType);
    
    // 5. 발급된 ID를 PlayerController에도 저장
    const int32 _assignedId = _jhsPS->GetAssignedPlayerId();
    AJHSPlayerController* _jhsPC = Cast<AJHSPlayerController>(NewPlayer);
    _jhsPC->SetAssignedPlayerId(_assignedId);
}
```

---

### 2.2 PlayerStateGroup에서 ID 발급

#### **메서드: UPlayerStateGroup::TryRegistPlayer**
- **호출자**: AJHSGameMode::PostLogin
- **위치**: `PlayerStateGroup.cpp:53`
- **역할**: 실제 ID 발급 및 플레이어 데이터 생성

**실행 흐름:**
```cpp
bool UPlayerStateGroup::TryRegistPlayer(TObjectPtr<AJHSPlayerState> PlayerState, 
                                        FString PlayerName, 
                                        E_REGIST_ERROR_TYPE& ErrorType)
{
    // 1. 중복 검사 (UID, 이름)
    if (_playerStateMap.Contains(_playerID)) { /* 중복 UID */ }
    
    // 2. AssignedPlayerId 발급 (현재 등록된 플레이어 수)
    const int32 _assignedId = _playerStateMap.Num();
    
    // 3. FPlayerStateData 생성
    FPlayerStateData _newPlayerData;
    _newPlayerData.PlayerUID = _playerID;
    _newPlayerData.AssignedPlayerId = _assignedId;  // ★ 여기서 ID 발급
    _newPlayerData.PlayerName = PlayerName;
    _newPlayerData.PlayerIndex = _assignedId;
    
    // 4. PlayerState에 AssignedPlayerId 설정 (복제됨)
    AJHSPlayerState* _jhsPS = Cast<AJHSPlayerState>(PlayerState);
    _jhsPS->SetAssignedPlayerId(_assignedId);
    
    // 5. 맵에 추가
    _playerStateMap.Add(_newPlayerData.PlayerUID, _newPlayerData);
    return true;
}
```

**발급 규칙:**
- AssignedPlayerId = 현재 `_playerStateMap`의 크기
- 0부터 시작하는 순차적인 인덱스
- 한 번 발급되면 변경되지 않음

---

### 2.3 PlayerState에서 ID 복제

#### **메서드: AJHSPlayerState::SetAssignedPlayerId**
- **호출자**: UPlayerStateGroup::TryRegistPlayer
- **위치**: `JHSPlayerState.cpp:19`

```cpp
void AJHSPlayerState::SetAssignedPlayerId(int32 AssignedPlayerId)
{
    _assignedPlayerId = AssignedPlayerId;  // Replicated 속성
}

void AJHSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AJHSPlayerState, _assignedPlayerId);  // ★ 복제 설정
}
```

**복제 특성:**
- `UPROPERTY(Replicated)` 속성
- 서버에서 설정하면 모든 클라이언트에 자동 복제
- 각 클라이언트는 모든 플레이어의 AssignedPlayerId를 알 수 있음

---

### 2.4 PlayerController에서 ID 복제

#### **메서드: AJHSPlayerController::SetAssignedPlayerId**
- **호출자**: AJHSGameMode::PostLogin
- **위치**: `JHSPlayerController.cpp:25`

```cpp
void AJHSPlayerController::SetAssignedPlayerId(int32 AssignedPlayerId)
{
    _assignedPlayerId = AssignedPlayerId;  // Replicated 속성
}

void AJHSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AJHSPlayerController, _assignedPlayerId);  // ★ 복제 설정
}
```

**복제 이유:**
- PlayerState와 별도로 PlayerController에도 저장
- UI/Interact 로직에서 빠른 접근을 위해
- 주석: "서버 PostLogin 시 PlayerStateGroup에서 발급·복제된 식별 ID. UI/Interact 호출자 비교용."

---

## 3. 인터랙션에서 호출자 판별

### 3.1 트리거 진입 (클라이언트 → 서버)

#### **메서드: UInteractableComponent::OnTriggerEnter**
- **호출 시점**: 플레이어가 인터랙션 트리거에 진입할 때
- **위치**: `InteractableComponent.cpp:155`
- **네트워크 흐름**: 
  - 서버: 직접 ExecuteServerTriggerEnter 호출
  - 클라이언트: ServerReportTriggerEnter RPC로 서버에 알림

```cpp
void UInteractableComponent::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, 
                                            AActor* OtherActor, ...)
{
    // Interacter 찾기
    UInteracterComponent* _foundInteracter = OtherActor->FindComponentByClass<UInteracterComponent>();
    
    // 서버에서 실행
    if (_owner->HasAuthority())
    {
        ExecuteServerTriggerEnter(OtherActor);  // ★ 직접 호출
        return;
    }
    
    // 클라이언트에서 실행 (로컬 조종 중인 Pawn만)
    if (!_otherPawn->IsLocallyControlled())
        return;
    
    // 서버에 알림 (Pawn 소유 컴포넌트이므로 RPC 가능)
    _foundInteracter->ServerReportTriggerEnter(this);  // ★ Server RPC
}
```

**설계 포인트:**
- InteractableComponent는 레벨 액터에 부착 → owning connection 없음 → 직접 Server RPC 불가
- InteracterComponent는 Pawn에 부착 → owning connection 있음 → Server RPC 가능
- 따라서 클라이언트는 InteracterComponent를 통해 서버에 알림

---

#### **메서드: UInteracterComponent::ServerReportTriggerEnter_Implementation**
- **RPC 타입**: Server, Reliable
- **위치**: `InteracterComponent.cpp:142`

```cpp
void UInteracterComponent::ServerReportTriggerEnter_Implementation(UInteractableComponent* Interactable)
{
    AActor* _ownerPawn = GetOwner();  // Pawn
    Interactable->ExecuteServerTriggerEnter(_ownerPawn);  // ★ 서버에서 실행
}
```

---

### 3.2 서버에서 호출자 ID 추출 및 Client RPC

#### **메서드: UInteractableComponent::ExecuteServerTriggerEnter**
- **호출자**: 서버 권한 컨텍스트
- **위치**: `InteractableComponent.cpp:232`
- **역할**: 서버에서 호출자 ID를 추출하고, 해당 클라이언트에만 알림

```cpp
void UInteractableComponent::ExecuteServerTriggerEnter(AActor* OtherActor)
{
    UInteracterComponent* _foundInteracter = OtherActor->FindComponentByClass<UInteracterComponent>();
    
    // ★ 호출자 AssignedPlayerId 추출
    APawn* _otherPawnForId = Cast<APawn>(OtherActor);
    AJHSPlayerController* _callerJHSPC = Cast<AJHSPlayerController>(_otherPawnForId->GetController());
    const int32 _callerAssignedId = _callerJHSPC->GetAssignedPlayerId();  // ★ 여기서 ID 가져옴
    
    _interacter = _foundInteracter;
    E_INTERACT_TYPE _typeForEnter = _isWorldSpaceUI ? E_INTERACT_TYPE::Handle : E_INTERACT_TYPE::Seat;
    
    // ★ 해당 클라이언트에만 Client RPC 전송 (복제 타이밍에 의존하지 않음)
    _callerJHSPC->ClientInteractableTriggerEnter(this, _typeForEnter);
}
```

**중요 포인트:**
- 서버에서 Pawn → Controller → AssignedPlayerId 추출
- 복제된 속성이므로 서버에서 읽기 가능
- 해당 PlayerController에만 Client RPC 전송

---

#### **메서드: AJHSPlayerController::ClientInteractableTriggerEnter_Implementation**
- **RPC 타입**: Client, Reliable
- **위치**: `JHSPlayerController.cpp:30`

```cpp
void AJHSPlayerController::ClientInteractableTriggerEnter_Implementation(
    UInteractableComponent* Interactable, 
    E_INTERACT_TYPE InteractType)
{
    if (IsValid(Interactable))
        Interactable->ExecuteTriggerEnterForLocalPlayer(InteractType);  // ★ 로컬 실행
}
```

---

### 3.3 로컬 플레이어에서 실행

#### **메서드: UInteractableComponent::ExecuteTriggerEnterForLocalPlayer**
- **호출자**: Client RPC 수신 측
- **위치**: `InteractableComponent.cpp:189`

```cpp
void UInteractableComponent::ExecuteTriggerEnterForLocalPlayer(E_INTERACT_TYPE InteractType)
{
    // 로컬 PlayerController 가져오기
    AJHSPlayerController* _localController = nullptr;
    GetOrCacheLocalPlayerController(_localController);
    
    // 로컬 Pawn의 Interacter 가져오기
    APawn* _localPawn = _localController->GetPawn();
    UInteracterComponent* _foundInteracter = _localPawn->FindComponentByClass<UInteracterComponent>();
    
    // ★ Interacter에게 인터랙션 가능 알림
    _interacter = _foundInteracter;
    _foundInteracter->OnInteractable(this, InteractType);
}
```

---

### 3.4 로컬 PlayerController 캐싱 (PIE 리슨 서버 대응)

#### **메서드: UInteractableComponent::GetOrCacheLocalPlayerController**
- **위치**: `InteractableComponent.cpp:98`
- **역할**: PIE 리슨 서버 환경에서 올바른 로컬 PC 찾기

```cpp
bool UInteractableComponent::GetOrCacheLocalPlayerController(AJHSPlayerController*& OutController)
{
    // 캐시 유효성 검사 (월드별로 올바른 로컬 PC 사용)
    UWorld* _world = GetWorld();
    if (_cachedLocalPlayerController.IsValid() && 
        _cachedLocalPlayerController->IsLocalPlayerController() &&
        _cachedLocalPlayerController->GetWorld() == _world)
    {
        OutController = _cachedLocalPlayerController.Get();
        return true;
    }
    _cachedLocalPlayerController.Reset();
    
    // ★ GetFirstPlayerController()는 호스트만 반환 → 리슨 서버에서 클라이언트 창은 잘못된 PC
    // ★ 따라서 iterator로 순회하여 IsLocalPlayerController()인 PC 찾기
    for (FConstPlayerControllerIterator _it = _world->GetPlayerControllerIterator(); _it; ++_it)
    {
        APlayerController* _candidate = _it->Get();
        if (_candidate->IsLocalPlayerController())
        {
            _pc = _candidate;
            break;
        }
    }
    
    AJHSPlayerController* _controller = Cast<AJHSPlayerController>(_pc);
    _cachedLocalPlayerController = _controller;
    OutController = _controller;
    return true;
}
```

**PIE 리슨 서버 문제:**
- `GetFirstPlayerController()`는 항상 첫 번째 PC(호스트)를 반환
- 클라이언트 창에서는 잘못된 PC가 선택됨
- 해결: `IsLocalPlayerController()` 체크로 현재 컨텍스트의 로컬 PC 찾기

---

## 4. 인터랙션 입력 처리

### 4.1 인터랙션 입력 시도

#### **메서드: UInteracterComponent::TryInteractInput**
- **호출 시점**: 플레이어가 인터랙션 키 입력
- **위치**: `InteracterComponent.cpp:114`

```cpp
bool UInteracterComponent::TryInteractInput(bool& OutIsInterupt, bool& OutIsInteractEnter)
{
    // ★ 로컬 플레이어만 반응 (서버의 다른 플레이어는 무시)
    if (!_playerController->IsLocalPlayerController())
        return false;
    
    if (!_interactable->TryInteract(_playerController, OutIsInterupt, OutIsInteractEnter))
        return false;
    
    // UI 토글 (월드 스페이스 UI가 아닐 때만)
    if (!_interactable->IsWorldSpaceUI() && _uiManager != nullptr)
    {
        if (OutIsInteractEnter)
            _uiManager->CloseUI(_playerUI);
        else
            _uiManager->OpenUI(_playerUI);
    }
    
    return true;
}
```

---

#### **메서드: UInteractableComponent::TryInteract**
- **호출자**: UInteracterComponent::TryInteractInput
- **위치**: `InteractableComponent.cpp:403`

```cpp
bool UInteractableComponent::TryInteract(APlayerController* CallerController, 
                                         bool& OutIsInterupt, 
                                         bool& IsCloseUI)
{
    AJHSPlayerController* _jhsPC = Cast<AJHSPlayerController>(CallerController);
    
    // 월드 UI: 서버에 토글 요청 (Server RPC)
    if (_isWorldSpaceUI)
    {
        _jhsPC->ServerRequestToggleWorldUI(this);  // ★ Server RPC
        IsCloseUI = false;
        return true;
    }
    
    // 일반 UI: 로컬에서 토글
    _isInteract = !_isInteract;
    const int32 _callerAssignedId = _jhsPC->GetAssignedPlayerId();  // ★ 호출자 ID
    ChangeInteractState(_isInteract, _callerAssignedId, CallerController);
    
    IsCloseUI = _isInteract;
    return true;
}
```

---

### 4.2 인터랙션 상태 변경 (호출자 ID 전달)

#### **메서드: UInteractableComponent::ChangeInteractState**
- **위치**: `InteractableComponent.cpp:433`
- **역할**: 호출자 ID를 이벤트에 전달

```cpp
void UInteractableComponent::ChangeInteractState(bool IsInteract, 
                                                  int32 CallerPlayerId,  // ★ 호출자 ID
                                                  APlayerController* CallerController)
{
    _isInteract = IsInteract;
    
    if (_isInteract)
    {
        UUIBase* _openedUI = nullptr;
        if (_interactUIType != E_UI_TYPE::NONE && !_isWorldSpaceUI)
        {
            // ★ 로컬 플레이어이고 호출자 ID가 일치할 때만 UI 열기
            AJHSPlayerController* _localController = nullptr;
            if (GetOrCacheLocalPlayerController(_localController) && 
                _localController->GetAssignedPlayerId() == CallerPlayerId)  // ★ ID 비교
            {
                UUIManager* _callerUIManager = _localController->GetUIManager();
                _openedUI = _callerUIManager->OpenUI(_interactUIType);
            }
        }
        // ★ 이벤트 브로드캐스트 (호출자 ID 포함)
        OnInteractEnterAction.Broadcast(CallerPlayerId, _openedUI);
    }
    else
    {
        // UI 닫기 및 이벤트
        OnInteractExitAction.Broadcast(CallerPlayerId, _closedUI);
    }
}
```

**ID 사용 목적:**
- 로컬 플레이어의 AssignedPlayerId와 CallerPlayerId 비교
- 일치하는 경우에만 UI 열기
- 이벤트 리스너들이 어떤 플레이어가 인터랙션했는지 알 수 있음

---

## 5. 월드 스페이스 UI (멀티캐스트)

### 5.1 서버 토글 요청

#### **메서드: AJHSPlayerController::ServerRequestToggleWorldUI_Implementation**
- **RPC 타입**: Server, Reliable
- **위치**: `JHSPlayerController.cpp:42`

```cpp
void AJHSPlayerController::ServerRequestToggleWorldUI_Implementation(UInteractableComponent* Target)
{
    if (IsValid(Target))
        Target->AuthorityToggleWorldUI();  // ★ 서버 권한 토글
}
```

---

#### **메서드: UInteractableComponent::AuthorityToggleWorldUI**
- **호출자**: 서버 권한 컨텍스트
- **위치**: `InteractableComponent.cpp:356`

```cpp
void UInteractableComponent::AuthorityToggleWorldUI()
{
    AActor* _owner = GetOwner();
    if (!_owner->HasAuthority())  // 서버 체크
        return;
    
    _isInteract = !_isInteract;
    
    // ★ 멀티캐스트로 모든 클라이언트에 전파
    if (_isInteract)
        MulticastOpenWorldUI(_interactUIType, _owner, _worldUIRelativeLocation, _worldUIScale);
    else
        MulticastCloseWorldUI(_interactUIType);
}
```

---

### 5.2 멀티캐스트 수신 (모든 클라이언트)

#### **메서드: UInteractableComponent::MulticastOpenWorldUI_Implementation**
- **RPC 타입**: NetMulticast, Reliable
- **위치**: `InteractableComponent.cpp:368`

```cpp
void UInteractableComponent::MulticastOpenWorldUI_Implementation(E_UI_TYPE UIType, 
                                                                 AActor* OwnerActor, 
                                                                 FVector RelativeLocation, 
                                                                 float Scale)
{
    // ★ 모든 클라이언트에서 실행 (각자의 로컬 PC에서)
    AJHSPlayerController* _localController = nullptr;
    if (!GetOrCacheLocalPlayerController(_localController)) return;
    
    UUIManager* _localUIManager = _localController->GetUIManager();
    _localUIManager->OpenUIInWorldLocal(UIType, OwnerActor, RelativeLocation, Scale);
}
```

**멀티캐스트 특성:**
- 모든 클라이언트가 동시에 수신
- 각 클라이언트는 자신의 로컬 PC에서 UI 열기
- 호출자 구별 없이 모두에게 동일하게 표시

---

## 6. 멀티캐스트에서 호출자 판별 (사용되지 않는 경로)

### 6.1 AssignedPlayerId로 호출자 판별

#### **메서드: UInteractableComponent::MulticastOnTriggerEnter_Implementation**
- **RPC 타입**: NetMulticast, Reliable
- **위치**: `InteractableComponent.cpp:267`
- **상태**: 코드에 존재하지만 현재 호출되지 않음

```cpp
void UInteractableComponent::MulticastOnTriggerEnter_Implementation(int32 CallerAssignedPlayerId, 
                                                                     E_INTERACT_TYPE InteractType)
{
    UWorld* _world = GetWorld();
    
    // ★ CallerAssignedPlayerId에 해당하는 PC를 찾고, 그 PC가 로컬일 때만 처리
    AJHSPlayerController* _localController = nullptr;
    if (!FindLocalPlayerControllerByAssignedId(_world, CallerAssignedPlayerId, _localController))
        return;
    
    APawn* _localPawn = _localController->GetPawn();
    UInteracterComponent* _foundInteracter = _localPawn->FindComponentByClass<UInteracterComponent>();
    
    _interacter = _foundInteracter;
    _foundInteracter->OnInteractable(this, InteractType);
}
```

---

#### **메서드: UInteractableComponent::FindLocalPlayerControllerByAssignedId**
- **위치**: `InteractableComponent.cpp:136`
- **역할**: AssignedPlayerId로 로컬 PlayerController 찾기

```cpp
bool UInteractableComponent::FindLocalPlayerControllerByAssignedId(UWorld* World, 
                                                                    int32 AssignedPlayerId, 
                                                                    AJHSPlayerController*& OutController)
{
    OutController = nullptr;
    if (AssignedPlayerId < 0)
        return false;
    
    // ★ 모든 PlayerController 순회
    for (FConstPlayerControllerIterator _it = World->GetPlayerControllerIterator(); _it; ++_it)
    {
        APlayerController* _pc = _it->Get();
        
        // ★ 로컬 PlayerController만 체크
        if (!_pc->IsLocalPlayerController())
            continue;
        
        AJHSPlayerController* _jhsPC = Cast<AJHSPlayerController>(_pc);
        
        // ★ AssignedPlayerId 일치 여부 확인
        if (_jhsPC->GetAssignedPlayerId() != AssignedPlayerId)
            continue;
        
        OutController = _jhsPC;
        return true;
    }
    return false;
}
```

**판별 로직:**
1. 모든 PlayerController를 순회
2. 로컬 PlayerController만 필터링 (`IsLocalPlayerController()`)
3. AssignedPlayerId가 일치하는지 확인
4. 일치하면 해당 PC가 호출자의 로컬 PC

**사용 목적:**
- 멀티캐스트 RPC는 모든 클라이언트에 전송됨
- 각 클라이언트에서 "이 호출이 나한테 해당하는가?"를 판별
- AssignedPlayerId를 사용하여 호출자 클라이언트만 반응

---

## 7. 트리거 이탈 (Exit) 흐름

### 7.1 트리거 이탈 감지

#### **메서드: UInteractableComponent::OnTriggerExit**
- **위치**: `InteractableComponent.cpp:289`

```cpp
void UInteractableComponent::OnTriggerExit(UPrimitiveComponent* OverlappedComponent, 
                                           AActor* OtherActor, ...)
{
    APawn* _otherPawn = Cast<APawn>(OtherActor);
    
    // 서버: 직접 실행
    if (_owner->HasAuthority())
    {
        ExecuteServerTriggerExit(OtherActor);
        return;
    }
    
    // 클라이언트: 로컬 조종 중인 Pawn만 서버에 알림
    if (!_otherPawn->IsLocallyControlled())
        return;
    
    UInteracterComponent* _foundInteracter = OtherActor->FindComponentByClass<UInteracterComponent>();
    _foundInteracter->ServerReportTriggerExit(this);  // ★ Server RPC
}
```

---

#### **메서드: UInteracterComponent::ServerReportTriggerExit_Implementation**
- **RPC 타입**: Server, Reliable
- **위치**: `InteracterComponent.cpp:152`

```cpp
void UInteracterComponent::ServerReportTriggerExit_Implementation(UInteractableComponent* Interactable)
{
    AActor* _ownerPawn = GetOwner();
    Interactable->ExecuteServerTriggerExit(_ownerPawn);
}
```

---

### 7.2 서버에서 이탈 처리

#### **메서드: UInteractableComponent::ExecuteServerTriggerExit**
- **위치**: `InteractableComponent.cpp:315`

```cpp
void UInteractableComponent::ExecuteServerTriggerExit(AActor* OtherActor)
{
    UInteracterComponent* _foundInteracter = OtherActor->FindComponentByClass<UInteracterComponent>();
    
    // ★ 호출자 AssignedPlayerId 추출
    APawn* _otherPawn = Cast<APawn>(OtherActor);
    AJHSPlayerController* _callerJHSPC = Cast<AJHSPlayerController>(_otherPawn->GetController());
    const int32 _callerAssignedId = _callerJHSPC->GetAssignedPlayerId();
    
    _interacter = nullptr;
    
    // ★ 해당 클라이언트에만 Client RPC
    _callerJHSPC->ClientInteractableTriggerExit(this);
    
    // 인터랙트 중이었으면 상태 변경
    if (_isInteract)
        ChangeInteractState(false, _callerAssignedId, _callerJHSPC);
}
```

---

#### **메서드: AJHSPlayerController::ClientInteractableTriggerExit_Implementation**
- **RPC 타입**: Client, Reliable
- **위치**: `JHSPlayerController.cpp:36`

```cpp
void AJHSPlayerController::ClientInteractableTriggerExit_Implementation(UInteractableComponent* Interactable)
{
    if (IsValid(Interactable))
        Interactable->ExecuteTriggerExitForLocalPlayer();
}
```

---

#### **메서드: UInteractableComponent::ExecuteTriggerExitForLocalPlayer**
- **위치**: `InteractableComponent.cpp:204`

```cpp
void UInteractableComponent::ExecuteTriggerExitForLocalPlayer()
{
    AJHSPlayerController* _localController = nullptr;
    GetOrCacheLocalPlayerController(_localController);
    
    APawn* _localPawn = _localController->GetPawn();
    UInteracterComponent* _foundInteracter = _localPawn->FindComponentByClass<UInteracterComponent>();
    
    // Interacter에게 알림
    _foundInteracter->OnDisInteractable();
    _interacter = nullptr;
    _isInteract = false;
    
    // ★ 비월드 UI: 트리거 이탈 시 UI 닫기
    if (!_isWorldSpaceUI && _interactUIType != E_UI_TYPE::NONE)
    {
        UUIManager* _callerUIManager = _localController->GetUIManager();
        UUIBase* _closedUI = _callerUIManager->CloseUI(_interactUIType);
        OnInteractExitAction.Broadcast(_localController->GetAssignedPlayerId(), _closedUI);
    }
}
```

**특이점:**
- 트리거 이탈 시 해당 클라이언트의 UI를 닫아야 함
- 서버의 ChangeInteractState는 호스트만 대상
- 일반 클라이언트는 이 메서드에서 직접 UI 닫기

---

## 8. 이벤트 시스템에서 호출자 ID 사용

### 8.1 인터랙션 타입 변경 이벤트

#### **메서드: UInteracterComponent::ExecuteEventOnChangeInteractType**
- **위치**: `InteracterComponent.cpp:162`

```cpp
void UInteracterComponent::ExecuteEventOnChangeInteractType(E_INTERACT_TYPE InteractType)
{
    UEventOnChangeInteractType* _event = NewObject<UEventOnChangeInteractType>(this);
    
    _event->InteractType = InteractType;
    
    // ★ 호출자 PlayerID 설정
    AJHSPlayerController* _callerJHSPC = Cast<AJHSPlayerController>(_playerController);
    _event->PlayerID = _callerJHSPC ? _callerJHSPC->GetAssignedPlayerId() : -1;
    
    UEventManager::ExecuteEvent<UEventOnChangeInteractType>(_event);
}
```

**이벤트 활용:**
- 이벤트 리스너들이 어떤 플레이어가 인터랙션했는지 알 수 있음
- UI 업데이트, 게임 로직 등에서 활용
- 예: 특정 플레이어만 보는 UI, 플레이어별 통계

---

## 9. 전체 호출 흐름 요약

### 9.1 ID 발급 흐름 (서버 전용)
```
플레이어 접속
↓
AJHSGameMode::PostLogin (서버)
↓
UPlayerStateGroup::TryRegistPlayer
  ├─ AssignedPlayerId 발급 (순차 인덱스)
  └─ FPlayerStateData 생성
↓
AJHSPlayerState::SetAssignedPlayerId (복제)
↓
AJHSPlayerController::SetAssignedPlayerId (복제)
↓
모든 클라이언트에 복제됨
```

---

### 9.2 인터랙션 진입 흐름 (일반 UI)

#### 클라이언트 → 서버
```
UInteractableComponent::OnTriggerEnter (클라이언트)
  ├─ IsLocallyControlled() 체크
  └─ UInteracterComponent::ServerReportTriggerEnter (RPC)
↓
UInteracterComponent::ServerReportTriggerEnter_Implementation (서버)
↓
UInteractableComponent::ExecuteServerTriggerEnter (서버)
  ├─ GetAssignedPlayerId() → 호출자 ID 추출
  └─ AJHSPlayerController::ClientInteractableTriggerEnter (RPC)
↓
AJHSPlayerController::ClientInteractableTriggerEnter_Implementation (클라이언트)
↓
UInteractableComponent::ExecuteTriggerEnterForLocalPlayer (클라이언트)
  ├─ GetOrCacheLocalPlayerController() → 로컬 PC 찾기
  └─ UInteracterComponent::OnInteractable
```

#### 인터랙션 입력
```
UInteracterComponent::TryInteractInput (로컬)
  └─ IsLocalPlayerController() 체크
↓
UInteractableComponent::TryInteract (로컬)
  └─ GetAssignedPlayerId() → 호출자 ID 추출
↓
UInteractableComponent::ChangeInteractState (로컬)
  ├─ GetAssignedPlayerId() == CallerPlayerId 비교
  ├─ 일치 시 UI 열기
  └─ OnInteractEnterAction.Broadcast(CallerPlayerId, ...)
```

---

### 9.3 인터랙션 진입 흐름 (월드 UI - 멀티캐스트)

```
UInteracterComponent::TryInteractInput (로컬)
↓
UInteractableComponent::TryInteract (로컬)
  └─ AJHSPlayerController::ServerRequestToggleWorldUI (RPC)
↓
AJHSPlayerController::ServerRequestToggleWorldUI_Implementation (서버)
↓
UInteractableComponent::AuthorityToggleWorldUI (서버)
  ├─ HasAuthority() 체크
  └─ MulticastOpenWorldUI (멀티캐스트 RPC)
↓
UInteractableComponent::MulticastOpenWorldUI_Implementation (모든 클라이언트)
  ├─ GetOrCacheLocalPlayerController() → 각자의 로컬 PC
  └─ UIManager::OpenUIInWorldLocal
```

---

### 9.4 트리거 이탈 흐름

```
UInteractableComponent::OnTriggerExit (클라이언트)
  └─ UInteracterComponent::ServerReportTriggerExit (RPC)
↓
UInteracterComponent::ServerReportTriggerExit_Implementation (서버)
↓
UInteractableComponent::ExecuteServerTriggerExit (서버)
  ├─ GetAssignedPlayerId() → 호출자 ID 추출
  ├─ AJHSPlayerController::ClientInteractableTriggerExit (RPC)
  └─ ChangeInteractState(false, _callerAssignedId, ...)
↓
AJHSPlayerController::ClientInteractableTriggerExit_Implementation (클라이언트)
↓
UInteractableComponent::ExecuteTriggerExitForLocalPlayer (클라이언트)
  ├─ UInteracterComponent::OnDisInteractable
  └─ UI 닫기 (비월드 UI인 경우)
```

---

## 10. 핵심 설계 원칙

### 10.1 서버 권한 (Authority)
- **ID 발급**: 서버에서만 발급 (PostLogin)
- **상태 변경**: 중요한 상태는 서버에서 관리
- **멀티캐스트**: 서버에서만 호출 가능

### 10.2 복제 (Replication)
- **AssignedPlayerId**: PlayerState와 PlayerController 모두 복제
- **자동 동기화**: 서버에서 설정하면 모든 클라이언트에 자동 전파
- **읽기 전용**: 클라이언트는 읽기만 가능

### 10.3 RPC 타입 선택
- **Client RPC**: 특정 클라이언트에만 전송 (복제 타이밍 무관)
- **Server RPC**: 클라이언트가 서버에 알림 (Pawn 소유 컴포넌트만 가능)
- **Multicast RPC**: 모든 클라이언트에 전송 (동일한 동작)

### 10.4 로컬 체크
- **IsLocalPlayerController()**: 현재 컨텍스트의 로컬 PC 판별
- **IsLocallyControlled()**: Pawn이 로컬에서 조종되는지 판별
- **PIE 대응**: GetFirstPlayerController() 대신 iterator 순회

### 10.5 호출자 판별 전략
1. **Server RPC 호출자**: RPC 호출 시점에 Controller 자동 전달
2. **Multicast + ID**: 모든 클라이언트에 ID 전달 → 각자 판별
3. **Client RPC**: 특정 클라이언트에만 전송 (판별 불필요)
4. **로컬 비교**: AssignedPlayerId 비교로 UI 표시 여부 결정

---

## 11. PIE 리슨 서버 특수 케이스

### 11.1 문제점
- `GetFirstPlayerController()`: 항상 첫 번째 PC(호스트) 반환
- 리슨 서버 클라이언트 창에서는 잘못된 PC 선택
- UI가 엉뚱한 플레이어에게 표시될 수 있음

### 11.2 해결 방법
```cpp
// ❌ 잘못된 방법
APlayerController* _pc = UGameplayStatics::GetPlayerController(GetWorld(), 0);

// ✅ 올바른 방법
for (FConstPlayerControllerIterator _it = World->GetPlayerControllerIterator(); _it; ++_it)
{
    APlayerController* _candidate = _it->Get();
    if (_candidate->IsLocalPlayerController())  // ★ 로컬 체크
    {
        _pc = _candidate;
        break;
    }
}
```

### 11.3 캐싱 전략
- 로컬 PC를 캐시하여 매번 검색하지 않음
- 월드 변경 시 캐시 무효화 (`GetWorld() == _world`)
- TWeakObjectPtr 사용으로 안전성 확보

---

## 12. 사용 예시 및 활용

### 12.1 호출자별 다른 UI 표시
```cpp
void UInteractableComponent::ChangeInteractState(bool IsInteract, 
                                                  int32 CallerPlayerId, ...)
{
    // 로컬 플레이어의 ID와 호출자 ID 비교
    if (_localController->GetAssignedPlayerId() == CallerPlayerId)
    {
        // ★ 호출자에게만 UI 표시
        _uiManager->OpenUI(_interactUIType);
    }
    
    // 이벤트는 모든 클라이언트에 전파 (관찰자용)
    OnInteractEnterAction.Broadcast(CallerPlayerId, _openedUI);
}
```

### 12.2 이벤트 리스너에서 호출자 구별
```cpp
void OnPlayerInteract(int32 CallerPlayerId, UUIBase* OpenedUI)
{
    AJHSPlayerController* _localPC = GetLocalPlayerController();
    
    if (_localPC->GetAssignedPlayerId() == CallerPlayerId)
    {
        // ★ 내가 인터랙션한 경우
        UE_LOG(LogTemp, Log, TEXT("I interacted!"));
    }
    else
    {
        // ★ 다른 플레이어가 인터랙션한 경우
        UE_LOG(LogTemp, Log, TEXT("Player %d interacted"), CallerPlayerId);
    }
}
```

### 12.3 멀티캐스트에서 호출자만 반응
```cpp
void UInteractableComponent::MulticastOnTriggerEnter_Implementation(int32 CallerAssignedPlayerId, ...)
{
    // ★ CallerAssignedPlayerId에 해당하는 로컬 PC 찾기
    AJHSPlayerController* _localController = nullptr;
    if (!FindLocalPlayerControllerByAssignedId(GetWorld(), CallerAssignedPlayerId, _localController))
        return;  // 다른 플레이어의 클라이언트에서는 조기 리턴
    
    // ★ 호출자 클라이언트에서만 실행
    _foundInteracter->OnInteractable(this, InteractType);
}
```

---

## 13. 디버깅 팁

### 13.1 로그로 네트워크 모드 확인
```cpp
UWorld* _world = GetWorld();
const ENetMode _netMode = _world ? _world->GetNetMode() : NM_Standalone;

UE_LOG(LogTemp, Log, TEXT("NetMode=%d, CallerID=%d"), 
       (int32)_netMode, CallerAssignedId);

// NM_Standalone = 0 (싱글플레이)
// NM_DedicatedServer = 1 (데디케이티드 서버)
// NM_ListenServer = 2 (리슨 서버 - 호스트)
// NM_Client = 3 (클라이언트)
```

### 13.2 로컬 PC 확인
```cpp
APlayerController* _pc = GetPlayerController();
bool _isLocal = _pc && _pc->IsLocalPlayerController();
int32 _assignedId = Cast<AJHSPlayerController>(_pc)->GetAssignedPlayerId();

UE_LOG(LogTemp, Log, TEXT("IsLocal=%d, AssignedId=%d"), 
       _isLocal, _assignedId);
```

### 13.3 Authority 확인
```cpp
AActor* _owner = GetOwner();
bool _hasAuthority = _owner && _owner->HasAuthority();

UE_LOG(LogTemp, Log, TEXT("HasAuthority=%d"), _hasAuthority);
```

---

## 14. 결론

### 핵심 요약
1. **AssignedPlayerId**는 서버에서 접속 시 발급 (순차 인덱스)
2. **PlayerState와 PlayerController**에 모두 복제되어 저장
3. **Server RPC**로 서버에 알림 → 서버에서 호출자 ID 추출
4. **Client RPC**로 특정 클라이언트에만 결과 전달
5. **Multicast RPC**로 모든 클라이언트에 전달 (ID 포함)
6. **로컬 PC 비교**로 UI 표시 여부 결정
7. **이벤트 시스템**에서 호출자 ID 활용

### 장점
- **복제 타이밍 독립**: Client RPC로 즉시 전달
- **명확한 호출자 구별**: AssignedPlayerId 기반
- **PIE 대응**: IsLocalPlayerController() 체크
- **확장 가능**: 이벤트 시스템으로 다양한 리스너 지원

### 주의사항
- InteractableComponent는 레벨 액터 → Server RPC 불가
- InteracterComponent는 Pawn 소유 → Server RPC 가능
- GetFirstPlayerController()는 리슨 서버에서 문제 발생
- 로컬 PC 캐싱으로 성능 최적화 필요

---

**작성 기준 날짜**: 2026-02-25  
**분석 대상 경로**: `C:\Users\jhsro\source\repos\Unreal\TeamSpace\Source\TeamSpaceProject\Private\JHS`
