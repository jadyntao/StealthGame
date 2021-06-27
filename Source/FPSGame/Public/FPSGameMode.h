// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSGameMode.generated.h"

//P039 GameMode��صĺ���ֻ���ڷ���������У����ͻ�����û��GameMode��ʵ��
//��ˣ�completemission������ȡ��������룬�ı�Ŀ���ӽǵȹ����ڿͻ����ж��޷����У�ͬʱFPSExtractionZone.cpp��53�е�GM��䶼��ʧЧ
//����취֮һ�Ǵ����鲥����������gamemode�����ǿ��Խ��и��Ƶĵط�����˻���game state base ����һ���µ�C++�࣬����ΪFPSGameState����FPSGameMode���ƣ��������ڸ��ƺ����������ȵȣ�
//������game state��FPSGameState.h����������ʹ���鲥������
//P040 ͬʱ���������µ�palyercontroller����ʵ�������ǿͻ��˻��Ƿ����������񣬸��Զ�����ʾ��������ɡ�
UCLASS()
class AFPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:

	//SpectatingViewpointClass�۲��ӵ��࣬��������ͼ�н��������ã�
	//��BP_GameMode��Details-Spectating����ΪBP_SpectatingViewpoint
	//��BP_SpectatingViewpoint������ͼ�����úõ�һ���̶�λ�õ�����ͷ�ӽ�
	UPROPERTY(EditDefaultsOnly, Category = "Spectating")
	TSubclassOf<AActor> SpectatingViewpointClass;

public:

	AFPSGameMode();

	//�������
	void CompleteMission(APawn* InstigatorPawn, bool bMissionSuccess);

	//BlueprintImplementableEvent����ͼ��ʵ���¼��������幦������ͼ��ʵ��
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void OnMissionCompleted(APawn* InstigatorPawn, bool bMissionSuccess);
};



