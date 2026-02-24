# 인터랙트 플로우 로그 순차 분석 (일반 클라이언트 UI 미반영)

## 제공 로그

```
LogTemp: [InteractFlow] OnTriggerEnter - Client overlap with OtherActor=BP_JHSSpaceShip_C_0
LogTemp: [InteractFlow] OnTriggerEnter - Client path: ServerReportTriggerEnter RPC (로컬 AssignedPlayerId=1 NetMode=3)
LogTemp: [InteractFlow] ServerReportTriggerEnter_Implementation - Server received RPC, Interactable=BP_InteractableExample_C_0
LogTemp: [InteractFlow] ExecuteServerTriggerEnter - NetMode=2 OtherActor=BP_JHSSpaceShip_C_1 CallerAssignedPlayerId=1
LogTemp: [InteractFlow] ExecuteServerTriggerEnter - calling MulticastOnTriggerEnter CallerAssignedPlayerId=1 InteractType=1
LogTemp: [InteractFlow] MulticastOnTriggerEnter_Implementation - ENTRY NetMode=2 CallerAssignedPlayerId=1 InteractType=1
LogTemp: [InteractFlow] MulticastOnTriggerEnter_Implementation - SKIP (no local PC with AssignedPlayerId=1, NetMode=2)
LogTemp: [InteractFlow] OnTriggerEnter - Server overlap with OtherActor=BP_JHSSpaceShip_C_1
LogTemp: [InteractFlow] OnTriggerEnter - Server path: ExecuteServerTriggerEnter
```

---

## 순차 분석

### 1단계: 클라이언트에서 트리거 진입 감지

| 로그 | 의미 |
|------|------|
| `OnTriggerEnter - Client overlap with OtherActor=BP_JHSSpaceShip_C_0` | 클라이언트 월드에서 **내 Pawn**이 트리거에 진입. |
| `Client path: ServerReportTriggerEnter RPC (로컬 AssignedPlayerId=1 NetMode=3)` | 비권한(클라이언트) 경로로, **AssignedPlayerId=1**을 로그에 찍고 서버로 RPC 전송. |

→ 클라이언트는 “내가 들어왔다”고 인식하고, **이 시점에는** 로컬 PC/Pawn 기준으로 AssignedPlayerId=1을 읽음.

---

### 2단계: 서버가 RPC 수신 및 처리

| 로그 | 의미 |
|------|------|
| `ServerReportTriggerEnter_Implementation - Server received RPC` | 서버가 `ServerReportTriggerEnter` 수신. |
| `ExecuteServerTriggerEnter - NetMode=2 OtherActor=BP_JHSSpaceShip_C_1 CallerAssignedPlayerId=1` | 서버가 **OtherActor(클라이언트 Pawn)**의 **Controller**에서 CallerAssignedPlayerId=1 취득. (서버는 PostLogin에서 이미 PC에 SetAssignedPlayerId(1) 호출함) |
| `calling MulticastOnTriggerEnter CallerAssignedPlayerId=1 InteractType=1` | 서버가 **MulticastOnTriggerEnter(1, InteractType)** 호출. |

→ 서버는 “1번 플레이어가 진입했다”고 정확히 알고, 멀티캐스트를 1번으로 보냄.

---

### 3단계: 멀티캐스트가 클라이언트(NetMode=2)에서 실행

| 로그 | 의미 |
|------|------|
| `MulticastOnTriggerEnter_Implementation - ENTRY NetMode=2 CallerAssignedPlayerId=1` | 멀티캐스트가 **NetMode=2(클라이언트)**에서 실행됨. 인자 CallerAssignedPlayerId=1. |
| `SKIP (no local PC with AssignedPlayerId=1, NetMode=2)` | **FindLocalPlayerControllerByAssignedId(World, 1, Out)** 실패. 즉, 이 월드에서 **IsLocalPlayerController() 이면서 GetAssignedPlayerId()==1** 인 PC를 **한 명도 찾지 못함**. |

→ **원인**: 클라이언트 측에서 “AssignedPlayerId==1인 로컬 PC”를 찾을 때 실패함.

---

### 4단계: 왜 클라이언트에서 AssignedPlayerId=1인 PC를 못 찾는가?

- **서버**: PostLogin에서 `TryRegistPlayer` 후 `_jhsPC->SetAssignedPlayerId(1)` 호출 → 서버의 해당 PC에는 1이 들어감.
- **클라이언트**: 같은 PC는 **복제**로 존재하고, `_assignedPlayerId`는 **Replicated**라서 나중에 1로 갱신됨.
- **타이밍**:  
  - 멀티캐스트 RPC는 “바로” 해당 클라이언트로 전달되어 실행됨.  
  - 반면 **PlayerController의 _assignedPlayerId 복제**는 같은 틱이 아니거나, 복제 배치/우선순위에 따라 **한 틱(또는 그 이상) 뒤에** 도착할 수 있음.  
- 그래서 **멀티캐스트가 실행되는 순간**에는 클라이언트의 그 PC가 아직 **GetAssignedPlayerId() == -1(기본값)** 인 상태일 수 있음.  
- 그러면 **FindLocalPlayerControllerByAssignedId(World, 1, Out)** 는 “로컬이면서 ID==1”인 PC를 찾지 못해 false → **SKIP**.

정리하면, **“복제 타이밍”** 때문에 클라이언트에서 아직 1번으로 갱신되지 않은 PC만 보이는 것이 3단계 SKIP의 직접 원인이다.

---

### 5단계: SKIP 이후 결과

- `MulticastOnTriggerEnter_Implementation`에서 return 되므로:
  - `OnInteractable` 호출 없음  
  - `ExecuteEventOnChangeInteractType` 호출 없음  
  - **UIPanelPlayerFPS**의 `OnChangeInteractType` 호출 없음  
→ **일반 클라이언트 화면의 인터랙트 UI가 갱신되지 않음.**

---

### 6단계: 서버 쪽 로그

- `OnTriggerEnter - Server overlap` / `Server path: ExecuteServerTriggerEnter`  
  → 서버 월드에서도 동일 Pawn으로 오버랩이 발생해 서버 경로가 한 번 더 실행된 것. UI 미반영과는 별개.

---

## 결론

| 단계 | 현상 | 원인 |
|------|------|------|
| 1~2 | 클라이언트 RPC, 서버 처리, CallerAssignedPlayerId=1 | 정상. |
| 3 | NetMode=2에서 “no local PC with AssignedPlayerId=1” | 클라이언트의 해당 PC에 **아직 _assignedPlayerId 복제가 도착하지 않아** GetAssignedPlayerId()가 -1(또는 기본값)인 상태. |
| 4 | UI 미갱신 | 3에서 SKIP되어 OnInteractable → 이벤트 → UI 경로가 실행되지 않음. |

**근본 원인**: 멀티캐스트 수신 시점에, **클라이언트의 PlayerController._assignedPlayerId 복제가 아직 적용되지 않아** ID로 호출자를 찾는 방식이 실패함.  
**해결 방향**: ID 복제에 의존하지 않도록, **대상 클라이언트만 정확히 호출하는 방식**(해당 PlayerController에 대한 **Client RPC**)으로 변경.

---

## 적용한 수정

- **MulticastOnTriggerEnter** 대신 **Client RPC** 사용:
  - 서버: `ExecuteServerTriggerEnter`에서 `_callerJHSPC->ClientInteractableTriggerEnter(this, _typeForEnter)` 호출.
  - 해당 RPC는 **그 PC를 소유한 클라이언트에만** 전달되므로, AssignedPlayerId 복제 여부와 무관하게 올바른 클라이언트에서만 실행됨.
- 클라이언트: `ClientInteractableTriggerEnter_Implementation`에서 `Interactable->ExecuteTriggerEnterForLocalPlayer(InteractType)` 호출.
- `ExecuteTriggerEnterForLocalPlayer`: 로컬 PC → Pawn → InteracterComponent 찾아 `OnInteractable` 호출 (기존 멀티캐스트 성공 경로와 동일 로직, ID 비교 없음).
