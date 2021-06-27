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

//P029������bool bMissionSuccess������BP_GameMode��ͼ�������ø���bMissionSuccess��ʾ��ͬ���ֵĽڵ�ؼ�
//��֮ǰ�����ã���ѧ��Ƶ���У��������п��������У�BP_GameMode��ͼ����ʹ������ΪWBP_GameOver�Ŀؼ���ͼ
//���ǣ���WBP_GameOver��Graph�д�������bMissionSuccess��������Details�й�ѡInstance Editable ��ʵ���ɱ༭��
//������Designer�����У�����ʾ�ı����Details-Content-Create Binding, ʹ�ýڵ����bMissionSuccess��ʾ��ͬ����
//�ص�BP_GameMode�У�ˢ�½ڵ�Create WBP Game Over Widget������bMissionSuccess
void AFPSGameMode::CompleteMission(APawn* InstigatorPawn, bool bMissionSuccess)
{
	if (InstigatorPawn)
	{
		//P039 ���������ڶ�����Ϸ���ͻ�������޷�Ӧ�ã���˿���ɾ��
		//InstigatorPawn->DisableInput(nullptr);

		if (SpectatingViewpointClass)
		{
			TArray<AActor*> ReturnedActors;
			//�ҵ���Ӧ����???
			UGameplayStatics::GetAllActorsOfClass(this, SpectatingViewpointClass, ReturnedActors);

			// Change viewtarget if any valid actor found
			//�ҵ��κ���Ч��actor�󣬸���Ŀ���ӽ�
			if (ReturnedActors.Num() > 0)
			{
				//ֻʹ������ĵ�һ��
				AActor* NewViewTarget = ReturnedActors[0];

				for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
				{
					APlayerController* PC = It->Get();
					if (PC)
					{
						//�������ӽǣ�BlendTime���ʱ��Ϊ0.5f��Ŀ���ӽǻ�Ϻ���ΪCubic�����κ�����
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

	//P039 ��ȡ��Ϸ״̬��ͨ���鲥������ʽ��֪ͨ�ͻ��ˣ�ʵ��multicastOnMissionComplete�е�ȡ���������Ĺ���
	AFPSGameState* GS = GetGameState<AFPSGameState>();
	if (GS)
	{
		GS->MulticastOnMissionComplete(InstigatorPawn, bMissionSuccess);
	}


	OnMissionCompleted(InstigatorPawn, bMissionSuccess);
}
