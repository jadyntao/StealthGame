// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSAIGuard.generated.h"

class UPawnSensingComponent;

//需要在FPSGame.Build.cs文件中的PublicDependencyModuleNames(公开从属模块)中添加AIModule（AI模块）
//在unreal中创建新的蓝图类，使用AIGuard类，命名为BP_Guard
//蓝图类的初始化设置：
//1、在Mesh(Inherited)组件中的Details-Mesh-Static Mesh，选择DwarfGrunt(矮人步兵)的网格体；
//2、在Mesh(Inherited)组件中的Details-Animation进行动画设置，在Animation mode选择Use Animation Asset（使用动画资源）；在Anim to play中选择Ldle（闲散动画）


//P030 守卫的状态，在构造函数中设为默认状态，OnNoiseHeard听到玩家时变为怀疑，OnPawnSeen看到玩家时变为警觉，警觉状态优先级高于怀疑状态
//UENUM设为蓝图类型，使其能够在蓝图中显示并使用
//创建WBP_GuardState蓝图控件，并设置好对应文字位置和大小等等；在BP_Guard中添加Widget组件，
//在BP_Guard的事件图表设置中，基于Widget获取到WBP_GuardState，并通过函数OnStateChanged获取到守卫状态并设置对应显示内容，更新WBP_GuardState中text内容


UENUM(BlueprintType)
enum class EAIState : uint8
{
	Idle,//默认状态

	Suspicious,//怀疑

	Alerted//警觉
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
	
	//人形体感应组件
	//Details-AI中的一些可设置属性参数：视野范围、视野半径、有遮挡听距Hearing Threshold、无遮挡听距LOSHearing Threshold
	//Details-Events中的一些可触发事件，On See Pawn看到人形体时、On hear noise 听到声音时
	//实现效果中得到：人形体感应组件存在机制优化，只有On See Pawn 没触发时才能触发 On hear noise
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UPawnSensingComponent* PawnSensingComp;

	//UFUNCTION()的作用是，告诉代码，该函数与事件进行了绑定
	UFUNCTION()
	void OnPawnSeen(APawn* SeenPawn);

	//P026需要在FPSCharacter文件中添加组件NoiseEmitterComponent（声音发射器组件），以至于让玩家人物能够制造声音
	//该功能测试代码：可以在FPSCharacter类型的BP_player蓝图类中的Event Tick事件图中连接Pawn Make Noise 来持续制造声音
	UFUNCTION()
	void OnNoiseHeard(APawn* NoiseInstigator, const FVector& Location, float Volume);

	FRotator OriginalRotation;

	UFUNCTION()
	void ResetOrientation();

	FTimerHandle TimerHandle_ResetOrientation;

	//P037 与AI相关的函数只能在服务器上进行逻辑运算，而不在客户端上。
	//UPROPERTY(ReplicatedUsing=OnRep_GuardState)：当GuardState发生改变时，会调用onRep_GuardState函数，而且只在客户端上进行
	//在onRep_GuardState函数中调用OnStateChanged蓝图函数，可以确保在客户端中也能对其UMG进行更新
	//并在.cpp中实现getLifetimeReplicatedProps（在其生命周期内复制）函数，用于设置同步的具体规则
	UPROPERTY(ReplicatedUsing=OnRep_GuardState)
	EAIState GuardState;

	UFUNCTION()
	void OnRep_GuardState();

	void SetGuardState(EAIState NewState);

	//创建一个可调用的函数，能为蓝图所用，从而作出响应，用于对其UMG进行更新
	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
	void OnStateChanged(EAIState NewState);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


protected:

	// CHALLENGE CODE	
	//P31守卫巡逻

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
