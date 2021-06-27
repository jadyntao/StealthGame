// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSAIGuard.generated.h"

class UPawnSensingComponent;

//��Ҫ��FPSGame.Build.cs�ļ��е�PublicDependencyModuleNames(��������ģ��)�����AIModule��AIģ�飩
//��unreal�д����µ���ͼ�࣬ʹ��AIGuard�࣬����ΪBP_Guard
//��ͼ��ĳ�ʼ�����ã�
//1����Mesh(Inherited)����е�Details-Mesh-Static Mesh��ѡ��DwarfGrunt(���˲���)�������壻
//2����Mesh(Inherited)����е�Details-Animation���ж������ã���Animation modeѡ��Use Animation Asset��ʹ�ö�����Դ������Anim to play��ѡ��Ldle����ɢ������


//P030 ������״̬���ڹ��캯������ΪĬ��״̬��OnNoiseHeard�������ʱ��Ϊ���ɣ�OnPawnSeen�������ʱ��Ϊ����������״̬���ȼ����ڻ���״̬
//UENUM��Ϊ��ͼ���ͣ�ʹ���ܹ�����ͼ����ʾ��ʹ��
//����WBP_GuardState��ͼ�ؼ��������úö�Ӧ����λ�úʹ�С�ȵȣ���BP_Guard�����Widget�����
//��BP_Guard���¼�ͼ�������У�����Widget��ȡ��WBP_GuardState����ͨ������OnStateChanged��ȡ������״̬�����ö�Ӧ��ʾ���ݣ�����WBP_GuardState��text����


UENUM(BlueprintType)
enum class EAIState : uint8
{
	Idle,//Ĭ��״̬

	Suspicious,//����

	Alerted//����
};


UCLASS()
class FPSGAME_API AFPSAIGuard : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFPSAIGuard();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//�������Ӧ���
	//Details-AI�е�һЩ���������Բ�������Ұ��Χ����Ұ�뾶�����ڵ�����Hearing Threshold�����ڵ�����LOSHearing Threshold
	//Details-Events�е�һЩ�ɴ����¼���On See Pawn����������ʱ��On hear noise ��������ʱ
	//ʵ��Ч���еõ����������Ӧ������ڻ����Ż���ֻ��On See Pawn û����ʱ���ܴ��� On hear noise
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UPawnSensingComponent* PawnSensingComp;

	//UFUNCTION()�������ǣ����ߴ��룬�ú������¼������˰�
	UFUNCTION()
	void OnPawnSeen(APawn* SeenPawn);

	//P026��Ҫ��FPSCharacter�ļ���������NoiseEmitterComponent�����������������������������������ܹ���������
	//�ù��ܲ��Դ��룺������FPSCharacter���͵�BP_player��ͼ���е�Event Tick�¼�ͼ������Pawn Make Noise ��������������
	UFUNCTION()
	void OnNoiseHeard(APawn* NoiseInstigator, const FVector& Location, float Volume);

	FRotator OriginalRotation;

	UFUNCTION()
	void ResetOrientation();

	FTimerHandle TimerHandle_ResetOrientation;

	//P037 ��AI��صĺ���ֻ���ڷ������Ͻ����߼����㣬�����ڿͻ����ϡ�
	//UPROPERTY(ReplicatedUsing=OnRep_GuardState)����GuardState�����ı�ʱ�������onRep_GuardState����������ֻ�ڿͻ����Ͻ���
	//��onRep_GuardState�����е���OnStateChanged��ͼ����������ȷ���ڿͻ�����Ҳ�ܶ���UMG���и���
	//����.cpp��ʵ��getLifetimeReplicatedProps���������������ڸ��ƣ���������������ͬ���ľ������
	UPROPERTY(ReplicatedUsing=OnRep_GuardState)
	EAIState GuardState;

	UFUNCTION()
	void OnRep_GuardState();

	void SetGuardState(EAIState NewState);

	//����һ���ɵ��õĺ�������Ϊ��ͼ���ã��Ӷ�������Ӧ�����ڶ���UMG���и���
	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
	void OnStateChanged(EAIState NewState);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


protected:

	// CHALLENGE CODE	
	//P31����Ѳ��

	/* Let the guard go on patrol */
	UPROPERTY(EditInstanceOnly, Category = "AI")
	bool bPatrol;

	/* First of two patrol points to patrol between */
	UPROPERTY(EditInstanceOnly, Category = "AI", meta = (EditCondition="bPatrol"))
	AActor* FirstPatrolPoint;

	/* Second of two patrol points to patrol between */
	UPROPERTY(EditInstanceOnly, Category = "AI", meta = (EditCondition = "bPatrol"))
	AActor* SecondPatrolPoint;
	
	// The current point the actor is either moving to or standing at
	AActor* CurrentPatrolPoint;

	void MoveToNextPatrolPoint();

	
};
