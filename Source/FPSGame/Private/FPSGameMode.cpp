// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSGameMode.h"
#include "FPSHUD.h"
#include "FPSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "FPSGameState.h"


AFPSGameMode::AFPSGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/BP_Player"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFPSHUD::StaticClass();

	//P039
	GameStateClass = AFPSGameState::StaticClass();
}

//P029增加了bool bMissionSuccess，并在BP_GameMode蓝图类中设置根据bMissionSuccess显示不同文字的节点控件
//在之前的设置（教学视频中有，本代码中看不到）中，BP_GameMode蓝图类中使用了名为WBP_GameOver的控件蓝图
//于是，在WBP_GameOver中Graph中创建变量bMissionSuccess，并在其Details中勾选Instance Editable （实例可编辑）
//并在其Designer界面中，对显示文本框的Details-Content-Create Binding, 使用节点根据bMissionSuccess显示不同文字
//回到BP_GameMode中，刷新节点Create WBP Game Over Widget，连接bMissionSuccess
void AFPSGameMode::CompleteMission(APawn* InstigatorPawn, bool bMissionSuccess)
{
	if (InstigatorPawn)
	{
		//P039 并不适用于多人游戏，客户端玩家无法应用，因此可以删除
		//InstigatorPawn->DisableInput(nullptr);

		if (SpectatingViewpointClass)
		{
			TArray<AActor*> ReturnedActors;
			//找到对应类型???
			UGameplayStatics::GetAllActorsOfClass(this, SpectatingViewpointClass, ReturnedActors);

			// Change viewtarget if any valid actor found
			//找到任何有效的actor后，更改目标视角
			if (ReturnedActors.Num() > 0)
			{
				//只使用数组的第一个
				AActor* NewViewTarget = ReturnedActors[0];

				for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
				{
					APlayerController* PC = It->Get();
					if (PC)
					{
						//设置新视角，BlendTime混合时间为0.5f，目标视角混合函数为Cubic（三次函数）
						PC->SetViewTargetWithBlend(NewViewTarget, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SpectatingViewpointClass is nullptr. Please update GameMode class with valid subclass. Cannot change spectating view target."));
		}
	}

	//P039 获取游戏状态，通过组播函数方式，通知客户端，实现multicastOnMissionComplete中的取消玩家输入的功能
	AFPSGameState* GS = GetGameState<AFPSGameState>();
	if (GS)
	{
		GS->MulticastOnMissionComplete(InstigatorPawn, bMissionSuccess);
	}


	OnMissionCompleted(InstigatorPawn, bMissionSuccess);
}
