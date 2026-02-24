# [InteractFlow] 로그 분석 요약

## 로그의 실제 시나리오 (정정)

**이 로그는 호스트가 진입한 경우가 아니라, 실제로는 일반 클라이언트만 OnTriggerEnter에 진입한 직후까지의 흐름을 담은 로그이다.** 진입한 쪽 = 일반 클라이언트(클라이언트 창에서 조종하는 플레이어). 아래 표와 결론은 이 전제로 재해석한 내용이다.

## 이번 로그 해석 (일반 클라이언트 진입 기준)

| 순서 | 로그 | 해석 |
|------|------|------|
| 1 | OnTriggerEnter - **Client** overlap, OtherActor=BP_JHSSpaceShip_**C_0** | **일반 클라이언트** 쪽에서 감지된 오버랩. 클라이언트 기준 내 Pawn = C_0. **진입한 것은 일반 클라이언트.** |
| 2 | Client path: ServerReportTriggerEnter RPC, 로컬 PlayerId=257 NetMode=3 | RPC를 **보낸 쪽 = 진입한 일반 클라이언트**. (257/NetMode=3은 PIE 로그 병합 등으로 호스트 창 값이 섞였을 수 있음. 발신 주체는 일반 클라이언트.) |
| 3 | ServerReportTriggerEnter_Implementation - Server received RPC | 서버가 RPC 수신. |
| 4 | ExecuteServerTriggerEnter - NetMode=2 OtherActor=C_**1** CallerPlayerId=**257** | 서버가 **일반 클라이언트가 보낸** RPC 처리. 서버가 보는 진입한 플레이어 Pawn = C_1, 서버가 읽은 ID = **257**. |
| 5 | MulticastOnTriggerEnter_Implementation - ENTRY **NetMode=2** CallerPlayerId=257 | 멀티캐스트가 **일반 클라이언트 창(NetMode=2)** 에서 실행됨. |
| 6 | **SKIP (CallerPlayerId=257 != localPlayerId=256)** | **같은 일반 클라이언트 머신**에서 로컬 256 → 257과 불일치로 SKIP. **진입한 본인 창에서 UI 갱신 누락.** |
| 7 | OnTriggerEnter - **Server** overlap, OtherActor=C_**1** | 서버에서도 오버랩 발생 (서버 기준 해당 Pawn = C_1). |

## 결론

- **진입한 쪽(정정)**: 실제 시나리오는 **일반 클라이언트만** OnTriggerEnter에 진입한 직후 로그이다. 진입한 쪽 = **일반 클라이언트** (클라이언트 Pawn C_0). (이전 문서는 호스트 진입으로 잘못 해석함.)
- **멀티캐스트를 받은 쪽**: 로그 5~6은 **NetMode=2(클라이언트)** 에서 실행되었고, 그쪽 로컬 PlayerId=**256**.
- 따라서 **CallerPlayerId(257) ≠ 로컬(256)** 인 이유는 같은 한 명(일반 클라이언트)에 대해 서버가 보는 ID(257)와 해당 클라이언트의 로컬 ID(256)가 달라서이며, **진입한 본인(일반 클라이언트) 창에서 SKIP**되어 그 창의 UI에 반영되지 않음.

즉, 이 로그는 **일반 클라이언트가 진입했는데, 그 일반 클라이언트 창에서 257!=256으로 SKIP되어 UI가 갱신되지 않는 상황**을 보여준다.

## 일반 클라이언트 진입 시 UI 미반영 원인 (위 로그가 해당 케이스)

**일반 클라이언트(NetMode=2, 로컬 PlayerId=256)가** 트리거에 진입했을 때:

1. 서버는 **그 클라이언트의 Pawn**에 대해 `GetPlayerState()->GetPlayerId()`를 호출해 **CallerPlayerId**를 얻습니다.
2. 엔진의 `APlayerState::GetPlayerId()`는 **서버/클라이언트마다 다를 수 있는 "리터럴 PlayerId"**를 반환할 수 있음.
3. 그래서 서버가 얻은 값(예: 257)과, **해당 클라이언트가 자기 자신에 대해 얻는 로컬 PlayerId(256)** 가 달라질 수 있음.
4. 멀티캐스트 수신 시 `CallerPlayerId(257) != localPlayerId(256)` 이 되어 **같은 클라이언트인데도 SKIP** → UI 미반영.

따라서 **원인은 "서버에서 본 PlayerId"와 "클라이언트에서 본 (같은 플레이어의) PlayerId"가 불일치하는 것**으로 보는 것이 타당합니다.

## 수정 방향 제안

- **GetPlayerId()** 에 의존하지 말고, **서버와 클라이언트가 동일하게 쓸 수 있는 식별자**를 사용하는 것이 안전합니다.
  - 예: **서버가 할당한 ID를 담은 복제 프로퍼티**
    - `JHSPlayerState` 등에 `UPROPERTY(Replicated)` 인 `ServerAssignedPlayerId`(또는 기존 PlayerId를 서버에서만 설정하고 복제)를 두고,
    - 서버는 이 값으로 `CallerPlayerId`를 채워 멀티캐스트하고,
    - 클라이언트는 **같은 복제된 값**으로 비교하면, 같은 플레이어에 대해 서버/클라이언트가 동일한 ID를 사용하게 됩니다.
  - 또는 **GameState::PlayerArray 인덱스**, **NetConnection/NetPlayerIndex** 등 네트워크적으로 일치하는 값을 사용할 수 있습니다.

위와 같이 서버가 정한 ID를 한 번 복제해서 쓰는 방식으로 통일하면, 현재 로그에서 보이는 **257 vs 256** 같은 불일치가 제거되고, 일반 클라이언트 진입 시에도 UI가 정상 반영될 가능성이 높습니다.

### 적용한 수정 (PlayerArray 인덱스)

- 멀티캐스트 필터만 **GameState::PlayerArray 인덱스**(CallerPlayerIndex)로 비교하도록 변경함.
- 서버: RPC 발신측 Pawn의 PlayerState를 PlayerArray에서 찾아 인덱스를 멀티캐스트 인자로 전달.
- 수신측: 로컬 PlayerState의 PlayerArray 인덱스와 비교해 일치할 때만 OnInteractable 호출.
- ChangeInteractState/UI는 기존처럼 GetPlayerId() 사용.
