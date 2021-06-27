// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSAIGuard.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "FPSGameMode.h"
#include "Net/UnrealNetwork.h"
#include "AI/Navigation/NavigationSystem.h"


// Sets default values
AFPSAIGuard::AFPSAIGuard()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//创建默认子对象，类型为UPawnSensingComponent人形体感应组件，因为这不是一个场景组件，也没有层级排序，因此不需要设置附件
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

	//绑定事件，添加动态函数
	PawnSensingComp->OnSeePawn.AddDynamic(this, &AFPSAIGuard::OnPawnSeen);
	PawnSensingComp->OnHearNoise.AddDynamic(this, &AFPSAIGuard::OnNoiseHeard);

	GuardState = EAIState::Idle;
}

// Called when the game starts or when spawned
void AFPSAIGuard::BeginPlay()
{
	Super::BeginPlay();
	
	OriginalRotation = GetActorRotation();

	if (bPatrol)
	{
		MoveToNextPatrolPoint();
	}
}

void AFPSAIGuard::OnPawnSeen(APawn* SeenPawn)
{
	if (SeenPawn == nullptr)
	{
		return;
	}
	//画出调试球体函数，在守卫看到人形体时，在人形体的位置上画出3D球体
	DrawDebugSphere(GetWorld(), SeenPawn->GetActorLocation(), 32.0f, 12, FColor::Red, false, 10.0f);

	AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		////P029如果玩家被守卫看到，则在告知GameMode，游戏失败并结束
		GM->CompleteMission(SeenPawn, false);
	}

	SetGuardState(EAIState::Alerted);

	// Stop Movement if Patrolling
	AController* Controller = GetController();
	if (Controller)
	{
		Controller->StopMovement();
	}
}


void AFPSAIGuard::OnNoiseHeard(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
	//P30 警觉状态优先级高于怀疑状态
	if (GuardState == EAIState::Alerted)
	{
		return;
	}

	DrawDebugSphere(GetWorld(), Location, 32.0f, 12, FColor::Green, false, 10.0f);

	//P028守卫听到声音后，转向声音的位置
	FVector Direction = Location - GetActorLocation();
	Direction.Normalize();

	FRotator NewLookAt = FRotationMatrix::MakeFromX(Direction).Rotator();//只改变yaw角
	NewLookAt.Pitch = 0.0f;
	NewLookAt.Roll = 0.0f;

	SetActorRotation(NewLookAt);

	//P028需求：守卫转向后，经过指定时间后回到原来的位置
	//GetWorldTimerManager获取世界场景时钟管理器
	GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrientation);//停止原有定时器的句柄
	GetWorldTimerManager().SetTimer(TimerHandle_ResetOrientation, this, &AFPSAIGuard::ResetOrientation, 3.0f);

	SetGuardState(EAIState::Suspicious);

	// Stop Movement if Patrolling
	//当守卫处于怀疑状态时，通过AI控制的一个内置函数来实现停止游走
	AController* Controller = GetController();
	if (Controller)
	{
		Controller->StopMovement();
	}
}


void AFPSAIGuard::ResetOrientation()
{
	if (GuardState == EAIState::Alerted)
	{
		return;
	}

	SetActorRotation(OriginalRotation);

	SetGuardState(EAIState::Idle);

	// Stopped investigating...if we are a patrolling pawn, pick a new patrol point to move to
	//P031
	if (bPatrol)
	{
		MoveToNextPatrolPoint();
	}
}

//P037 当GuardState发生改变时，会调用onRep_GuardState函数，而且只在客户端上进行
//在onRep_GuardState函数中调用OnStateChanged蓝图函数，可以确保在客户端中也能对其UMG进行更新
void AFPSAIGuard::OnRep_GuardState()
{
	OnStateChanged(GuardState);
}

//P030 改变守卫的状态
void AFPSAIGuard::SetGuardState(EAIState NewState)
{
	if (GuardState == NewState)
	{
		return;
	}

	GuardState = NewState;
	
	//P037 确保服务端也能对UMG进行更新，在这里调用了OnRep_GuardState函数
	OnRep_GuardState();

	//P037 上面使用了OnRep_GuardState函数，这里的OnStateChanged就可以注释掉了
	//OnStateChanged(GuardState);
}

//P031
// Called every frame
void AFPSAIGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Patrol Goal Checks
	if (CurrentPatrolPoint)
	{
		FVector Delta = GetActorLocation() - CurrentPatrolPoint->GetActorLocation();
		float DistanceToGoal = Delta.Size();
		//UE_LOG(LogTemp, Warning, TEXT("GetActorLocation:%f,%f,%f"), GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);
		//UE_LOG(LogTemp, Warning, TEXT("GetActorLocatio2:%f,%f,%f"), CurrentPatrolPoint->GetActorLocation().X, CurrentPatrolPoint->GetActorLocation().Y, CurrentPatrolPoint->GetActorLocation().Z);
		//UE_LOG(LogTemp, Warning, TEXT("DistanceToGoal:%f"), Delta.Size());
		// Check if we are within 50 units of our goal, if so - pick a new patrol point
		//P031在默认地图上，距离就是达不到100，而在challenge/AIpatrol地图上就可以，原因还未找出？？？？？？？？

		if(DistanceToGoal < 50)
		{
			//UE_LOG(LogTemp, Warning, TEXT("DistanceToGoal < 50"));
			MoveToNextPatrolPoint();
		}
	}
}

//P031
void AFPSAIGuard::MoveToNextPatrolPoint()
{
	// Assign next patrol point.
	if (CurrentPatrolPoint == nullptr || CurrentPatrolPoint == SecondPatrolPoint)
	{
		//UE_LOG(LogTemp, Warning, TEXT("CurrentPatrolPoint = FirstPatrolPoint"));
		CurrentPatrolPoint = FirstPatrolPoint;
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("CurrentPatrolPoint = SecondPatrolPoint"));
		CurrentPatrolPoint = SecondPatrolPoint;
	}

	UNavigationSystem::SimpleMoveToActor(GetController(), CurrentPatrolPoint);
}


//P037 getLifetimeReplicatedProps在其生命周期内复制，设置同步的具体规则
//配合ReplicatedUsing的设置，该函数用于实现同步所有客户端
void AFPSAIGuard::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSAIGuard, GuardState);
}
