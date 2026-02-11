# UIGazeInteract: UIBase 내부 UInteractableButton 검출 로직 (단계별)

## 개요

`UIGazeInteract`는 매 틱마다 **카메라 전방 LineTrace** → **히트된 WidgetComponent의 루트 위젯**을 기준으로, 그 **내부**에 있는 `UInteractableButton` 중 **히트 위치를 포함하는 버튼 하나**를 골라 포커스/클릭 대상으로 사용합니다.

---

## 1단계: LineTrace로 UI 히트 판정

**위치:** `TickComponent()` (UIGazeInteract.cpp)

1. 카메라 위치·방향으로 `_distance`만큼 `LineTraceSingleByChannel` 수행.
2. `_traceChannel`(기본 `ECC_Visibility`)로 히트된 컴포넌트/액터 획득.
3. 히트가 없으면 이후 단계 없이 포커스 해제(`_UpdateFocus(nullptr)`).

**결과:** `FHitResult _hitResult` (히트된 컴포넌트, 액터, `ImpactPoint` 등).

---

## 2단계: WidgetComponent 및 루트 위젯 확보

**위치:** `_FindInteractableFromHit(HitResult)` 시작부

1. **WidgetComponent 획득**
   - `HitResult.GetComponent()`를 `UWidgetComponent*`로 캐스트.
   - 실패 시 `HitResult.GetActor()->FindComponentByClass<UWidgetComponent>()`로 한 번 더 시도.
2. **루트 위젯 획득**
   - `_widgetComponent->GetWidget()` → `UUserWidget* _rootWidget`.
   - 이 `_rootWidget`이 해당 UI의 **최상위 UserWidget**(예: UIPanelShop)이며, **모든 좌표/계층의 기준**이 됨.

**결과:** `_rootWidget` (UIBase 상속 위젯 인스턴스).

---

## 3단계: 히트 위치를 “루트 위젯 로컬” 2D로 변환

**위치:** `_FindInteractableFromHit()` 내부

1. `_widgetComponent->GetLocalHitLocation(HitResult.ImpactPoint, _localHitLocation)` 호출.
2. 월드 3D `ImpactPoint`가 **해당 WidgetComponent가 그리는 위젯의 2D 로컬 좌표**로 변환됨.
3. 이 “위젯 로컬”은 **루트 UserWidget을 기준으로 한 좌표계**로 간주됨(엔진 문서/동작 기준).
4. `_rootWidget->GetCachedGeometry()`로 **루트의 지오메트리**를 얻어, 이후 “루트 로컬 ↔ 절대” 변환에 사용.

**결과:**  
- `FVector2D _localHitLocation` (루트 로컬 2D).  
- `FGeometry _rootGeometry` (루트 지오메트리).

---

## 4단계: 루트 아래 “모든 위젯” 수집 (BFS)

**위치:** `_FindInteractableFromHit()` — “시각적 루트부터 패널 자식으로 BFS 수집” 블록

목적: **블루프린트에만 있는 위젯**이 아니라, **런타임에 AddChild로 붙인 위젯**(예: HB_Category 아래 UPurchaseCategory, BTN_Category)까지 포함해 후보 목록을 만듦.

1. **BFS 시드 결정**
   - `_rootWidget->WidgetTree`와 `WidgetTree->RootWidget`이 유효하면 → `_treeRoot = WidgetTree->RootWidget` (실제 시각적 루트, 예: Canvas Panel).
   - 그렇지 않으면 → `_treeRoot = _rootWidget`.
   - 이유: `UUserWidget`은 `UPanelWidget`이 아니라서, 루트를 시드로 넣어도 `GetChildAt()`으로 자식을 수집할 수 없음. 따라서 **패널인 RootWidget**을 시드로 사용.

2. **BFS로 위젯 수집**
   - `_allWidgets.Add(_treeRoot)`로 시드 추가.
   - `_allWidgets`를 인덱스로 순회하면서:
     - `_allWidgets[_i]`를 `UPanelWidget*`로 캐스트.
     - 성공 시 `GetChildrenCount()`, `GetChildAt(_c)`로 **직계 자식만** 리스트에 추가.
   - 이렇게 **트리 전체**를 한 번만 순회하며, 동적 추가된 자식(UPurchaseCategory, 그 안의 BTN_Category 등)도 모두 포함.

**결과:** `TArray<UWidget*> _allWidgets` — 루트(시각적 루트) 및 그 아래 모든 자손 위젯.

---

## 5단계: 후보를 UInteractableButton으로 한정

**위치:** “실제 히트 위치를 포함하는 UInteractableButton만 선택” for 루프

1. `_allWidgets`를 순회.
2. 각 `_widget`에 대해 `Cast<UInteractableButton>(_widget)`.
3. 실패하면 스킵; 성공한 위젯만 **히트 검사 후보**로 사용.

**결과:** 후보 = “수집된 위젯 중 UInteractableButton인 것들” (BTN_SaleElement, BTN_Category 등).

---

## 6단계: “루트 로컬” 좌표계에서 히트 검사

**위치:** 같은 for 루프 안, 각 `_button`에 대해

목적: `_localHitLocation`(루트 로컬 2D)이 **해당 버튼의 사각형 안**에 있는지 판단.  
각 버튼의 지오메트리는 **자기 로컬 좌표계**이므로, **버튼 영역만 루트 로컬로 바꿔서** 비교함.

1. **버튼 지오메트리**
   - `_geometry = _button->GetCachedGeometry()`  
   - `_size = _geometry.GetLocalSize()` (버튼 로컬 크기).

2. **버튼 네 모서리를 “루트 로컬”로 변환**
   - 버튼 로컬 네 모서리: `(0,0)`, `(size.X, 0)`, `(size.X, size.Y)`, `(0, size.Y)`.
   - 각 점에 대해:
     - `_geometry.LocalToAbsolute(점)` → “절대”(스크린/렌더타겟) 좌표.
     - `_rootGeometry.AbsoluteToLocal(절대좌표)` → **루트 위젯 로컬** 좌표.
   - 네 점의 min/max로 **루트 로컬 기준 AABB** (`_min`, `_max`) 계산.

3. **포함 검사**
   - `_localHitLocation.X`가 `[_min.X, _max.X]` 안에 있고,  
     `_localHitLocation.Y`가 `[_min.Y, _max.Y]` 안에 있으면 **히트**.
   - 첫 번째로 히트된 버튼을 반환하고 종료.

**결과:** 히트된 `UInteractableButton*` 하나, 없으면 `nullptr`.

---

## 7단계: 포커스 갱신 및 클릭

**위치:** `TickComponent()` 마지막, `ClickFocused()`

1. **Tick**
   - `_FindInteractableFromHit(_hitResult)` 반환값을 `_UpdateFocus(_hitButton)`에 넘김.
   - 이전 포커스 버튼이 있으면 `Unfocus()`, 새 버튼이 있으면 `Focus()` 호출.
2. **클릭**
   - 입력 바인딩 등에서 `ClickFocused()` 호출 시, 현재 `_focusedButton`에 대해 `Click()` 호출.

---

## 요약: 데이터/좌표 흐름

- **LineTrace** → 월드 3D 히트.
- **GetLocalHitLocation** → 루트 위젯 2D 로컬 `_localHitLocation`.
- **BFS(WidgetTree->RootWidget 시드)** → `_allWidgets` (동적 자식 포함).
- **Cast UInteractableButton** → 후보 버튼만 필터.
- **버튼 LocalToAbsolute → Root AbsoluteToLocal** → 버튼 영역을 루트 로컬 AABB로 변환.
- **`_localHitLocation`이 AABB 안에 있으면** 해당 버튼이 “UIBase 내부에서 검출된 UInteractableButton”으로 선택됨.

---

## BTN_Category가 검출되지 않을 때 점검할 것

1. **BFS에 BTN_Category가 실제로 들어가는지**  
   - `WidgetTree->RootWidget`부터 `GetChildAt()`만으로 **HB_Category → UPurchaseCategory → … → BTN_Category**까지 이어지는지.  
   - 중간에 `UPanelWidget`이 아닌 위젯만 있는 구간이 있으면 그 아래는 수집되지 않음.

2. **좌표계 일치 여부**  
   - `_rootGeometry`와 각 버튼의 `_geometry`가 **같은 “절대” 공간**을 쓰는지(같은 WidgetComponent/같은 트리).  
   - 다르면 `AbsoluteToLocal` 변환 후 AABB가 잘못되어, 루트 로컬 `_localHitLocation`과 비교가 어긋날 수 있음.

3. **지오메트리 유효성**  
   - 동적 생성 직후 등에서 `GetCachedGeometry()`가 0 크기이거나 갱신 전이면, AABB가 잘못되거나 비어 있을 수 있음.
