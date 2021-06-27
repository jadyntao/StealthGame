// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSPlayerController.generated.h"


//P040 ��ԭ�����߼��У�FPSGameMode.cpp�����������ʱͨ����ͼ����OnMissionCompletedʵ��UI��ʾ��������FPSGameModeֻ�ڷ���������У����¿ͻ������޷�������ʾ
//�������������ʱ��playercontroller�Ļ��������Ͼ���������ÿ���ͻ��˾�ֻ��һ������pawn�Ļ������������������������ͻ�����Ҳ������
//���ǣ�ͨ��PlayerController��ʵ�ֻ�ȡ���Կͻ��˵�palyercontroller����������ʾUI
UCLASS()
class FPSGAME_API AFPSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerController")
	void OnMissionCompleted(APawn* InstigatorPawn, bool bMissionSuccess);
	
	
};
