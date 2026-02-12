# PlateContainer 상호작용 문제 원인 분석

## 현상
- **Shop의 UPlateContainer**: 선택 하이라이트(IMG_OnHover) 변경은 동작, 스크롤은 변동 없음
- **Container의 UPlateContainer**: 선택 하이라이트와 스크롤 모두 동작하지 않음

---

## 1. Container - 둘 다 동작하지 않는 원인

### 핵심: UI 표시 방식 차이

UIGazeInteract는 **라인트레이스**로 WidgetComponent에 충돌 시에만 InteractableUI를 찾습니다.

| 방식 | API | WidgetComponent | 라인트레이스 히트 |
|------|-----|-----------------|-------------------|
| 뷰포트 UI | `OpenUI()` → AddToViewport | 없음 | **불가** |
| 월드 스페이스 UI | `OpenUIInWorld()` | 생성됨 | **가능** |

**UIPanelContainer**가 `OpenUI()`로 뷰포트에 표시되면:
- 3D 공간에 WidgetComponent가 없음
- 카메라 라인트레이스가 UI에 닿지 않음
- `_FindInteractableFromHit`가 호출되지 않음
- Focus/ClickEnter/ProcessScrollInput 모두 실행되지 않음

**해결 방향**: UIPanelContainer를 `OpenUIInWorld()`로 표시하거나, 현재 `InteractableComponent::InitializeUIInteractable`에서 `IsWorldSpaceUI` 설정을 확인.

---

## 2. Shop - 하이라이트만 동작하고 스크롤이 안 되는 원인

### UIGazeInteract 동작 확인
- 선택 하이라이트 동작 → Focus/ClickEnter까지는 정상
- `_clickedInteractableUI`에 InteractableScrollBox가 설정됨
- `WantsScrollInput()` = true (스크롤 모드 진입 시)

### 마우스 휠 입력 미전달

스크롤은 `UIGazeInteract::Tick`에서 다음으로 처리:

```cpp
_wheelDelta = _pc->GetInputAxisValue(FName("MouseWheelAxis"));
_clicked->ProcessScrollInput(_wheelDelta);
```

프로젝트 설정:
- `DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent`
- Enhanced Input 사용 시 **기존 축 API**와 축 바인딩이 다를 수 있음
- `GetInputAxisValue("MouseWheelAxis")`가 Enhanced Input 하에서 **0을 반환**할 가능성이 높음

**해결 방향**:
1. Enhanced Input에서 Mouse Wheel 축을 바인딩하고, 그 입력을 UIGazeInteract로 전달
2. 또는 마우스 휠용 Legacy Input (Axis Mapping) 추가
3. 또는 `UEnhancedInputComponent`로 마우스 휠 액션을 바인딩한 뒤, 액션 콜백에서 UIGazeInteract에 델타 전달

---

## 3. 위젯 계층 (참고)

```
UIPanelShop / UIPanelContainer (WidgetComponent 루트)
  └─ SB_PlateContainer (USizeBox)
       └─ UPlateContainer (동적 추가)
            └─ SB_Items (UInteractableScrollBox)  ← Focus/Click/Scroll 대상
                 ├─ SCRL_ScrollBox
                 └─ IMG_OnHover
```

`_FindInteractableFromHit`의 BFS가 동적 자식(UPlateContainer)까지 탐색하므로, 월드 스페이스 UI일 때는 InteractableScrollBox 검출에는 문제 없음.

---

## 4. 요약 및 권장 수정

| 문제 | 원인 | 수정 방향 |
|------|------|-----------|
| Container 전체 미동작 | 뷰포트 UI로 표시 → WidgetComponent 없음 | `OpenUIInWorld`로 월드 스페이스 표시 |
| Shop 스크롤 미동작 | Enhanced Input 하에서 MouseWheelAxis 미전달 | 마우스 휠 입력을 Enhanced Input 또는 Legacy로 바인딩 후 UIGazeInteract로 전달 |
