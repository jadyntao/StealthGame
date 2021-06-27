// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSPlayerController.generated.h"


//P040 在原来的逻辑中，FPSGameMode.cpp中在完成任务时通过蓝图函数OnMissionCompleted实现UI显示，而由于FPSGameMode只在服务端中运行，导致客户端上无法正常显示
//假设有两个玩家时，playercontroller的话服务器上就有两个，每个客户端就只有一个。而pawn的话，服务器上有两个，各个客户端上也有两个
//于是，通过PlayerController来实现获取各自客户端的palyercontroller，并各自显示UI
UCLASS()
class FPSGAME_API AFPSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerController")
	void OnMissionCompleted(APawn* InstigatorPawn, bool bMissionSuccess);
	
	
};
