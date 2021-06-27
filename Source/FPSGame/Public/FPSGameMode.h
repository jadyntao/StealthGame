// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSGameMode.generated.h"

//P039 GameMode相关的函数只能在服务端中运行，而客户端上没有GameMode的实例
//因此，completemission函数中取消玩家输入，改变目标视角等功能在客户端中都无法运行，同时FPSExtractionZone.cpp中53行的GM语句都会失效
//解决办法之一是创建组播函数，由于gamemode并不是可以进行复制的地方，因此基于game state base 创建一个新的C++类，命名为FPSGameState（与FPSGameMode类似，但能用于复制函数、变量等等）
//创建了game state（FPSGameState.h），在其上使用组播函数；
//P040 同时，创建了新的palyercontroller，来实现无论是客户端还是服务端完成任务，各自都能显示“任务完成”
UCLASS()
class AFPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:

	//SpectatingViewpointClass观察视点类，具体在蓝图中进行了设置，
	//将BP_GameMode中Details-Spectating设置为BP_SpectatingViewpoint
	//而BP_SpectatingViewpoint是在蓝图中设置好的一个固定位置的摄像头视角
	UPROPERTY(EditDefaultsOnly, Category = "Spectating")
	TSubclassOf<AActor> SpectatingViewpointClass;

public:

	AFPSGameMode();

	//任务结束
	void CompleteMission(APawn* InstigatorPawn, bool bMissionSuccess);

	//BlueprintImplementableEvent（蓝图可实现事件），具体功能在蓝图上实现
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void OnMissionCompleted(APawn* InstigatorPawn, bool bMissionSuccess);
};



