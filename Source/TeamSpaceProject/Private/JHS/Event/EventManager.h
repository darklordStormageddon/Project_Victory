// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JHS/Event/CommonEventBase.h"
#include "Delegates/Delegate.h"
#include <type_traits>
#include "EventManager.generated.h"

/**
 * Unity C# EventManager와 동일한 구조의 이벤트 시스템
 * 타입 안전한 제네릭 이벤트 리스너 관리
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UEventManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UEventManager(const FObjectInitializer& ObjectInitializer);

	/**
	 * 이벤트 리스너 추가
	 * @tparam T 이벤트 타입 (UCommonEventBase를 상속받아야 함)
	 * @param Callback 이벤트 발생 시 호출될 콜백 함수
	 * @return 델리게이트 핸들 (제거 시 사용)
	 */
	template<typename T>
	FDelegateHandle AddListener(TFunction<void(T*)> Callback)
	{
		static_assert(std::is_base_of<UCommonEventBase, T>::value, "T must be derived from UCommonEventBase");

		UClass* _eventClass = T::StaticClass();
		
		// 내부 델리게이트 생성 (타입 캐스팅 포함)
		auto _internalDelegate = [Callback](UCommonEventBase* Event)
		{
			if (T* _typedEvent = Cast<T>(Event))
			{
				Callback(_typedEvent);
			}
		};

		// 타입별 델리게이트 맵에 추가하고 핸들 반환
		if (!_eventDelegates.Contains(_eventClass))
		{
			_eventDelegates.Add(_eventClass, TMulticastDelegate<void(UCommonEventBase*)>());
		}

		FDelegateHandle _handle = _eventDelegates[_eventClass].AddLambda(_internalDelegate);
		
		// 핸들을 룩업 맵에 저장
		_delegateLookup.Add(_handle, _eventClass);

		return _handle;
	}

	/**
	 * 이벤트 리스너 제거
	 * @tparam T 이벤트 타입
	 * @param Handle AddListener에서 반환된 델리게이트 핸들
	 */
	template<typename T>
	void DelListener(FDelegateHandle Handle)
	{
		static_assert(std::is_base_of<UCommonEventBase, T>::value, "T must be derived from UCommonEventBase");

		if (!_delegateLookup.Contains(Handle))
		{
			return;
		}

		UClass* _eventClass = _delegateLookup[Handle];
		if (_eventDelegates.Contains(_eventClass))
		{
			_eventDelegates[_eventClass].Remove(Handle);
			
			// 델리게이트가 비어있으면 맵에서 제거
			if (!_eventDelegates[_eventClass].IsBound())
			{
				_eventDelegates.Remove(_eventClass);
			}
		}

		_delegateLookup.Remove(Handle);
	}

	/**
	 * 이벤트 실행
	 * @tparam T 이벤트 타입
	 * @param Event 실행할 이벤트 객체
	 */
	template<typename T>
	static void ExecuteEvent(T* Event)
	{
		static_assert(std::is_base_of<UCommonEventBase, T>::value, "T must be derived from UCommonEventBase");

		if (Event == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("UEventManager: ExecuteEvent called with nullptr"));
			return;
		}

		UClass* _eventClass = T::StaticClass();
		if (_eventDelegates.Contains(_eventClass))
		{
			UE_LOG(LogTemp, Log, TEXT("[InteractFlow] EventManager::ExecuteEvent - broadcasting %s"), *_eventClass->GetName());
			_eventDelegates[_eventClass].Broadcast(Event);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[InteractFlow] EventManager::ExecuteEvent - NO listeners for %s (Broadcast skipped)"), *_eventClass->GetName());
		}
	}

	/**
	 * 모든 리스너 정리
	 */
	void Clean();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// 타입별 델리게이트 맵 (TMulticastDelegate는 UPROPERTY로 선언할 수 없음)
	static TMap<UClass*, TMulticastDelegate<void(UCommonEventBase*)>> _eventDelegates;

	// 델리게이트 핸들과 이벤트 클래스 매핑
	TMap<FDelegateHandle, UClass*> _delegateLookup;
};
