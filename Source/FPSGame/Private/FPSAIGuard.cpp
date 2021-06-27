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
	//����Ĭ���Ӷ�������ΪUPawnSensingComponent�������Ӧ�������Ϊ�ⲻ��һ�����������Ҳû�в㼶������˲���Ҫ���ø���
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

	//���¼�����Ӷ�̬����
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
	//�����������庯��������������������ʱ�����������λ���ϻ���3D����
	DrawDebugSphere(GetWorld(), SeenPawn->GetActorLocation(), 32.0f, 12, FColor::Red, false, 10.0f);

	AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		////P029�����ұ��������������ڸ�֪GameMode����Ϸʧ�ܲ�����
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
	//P30 ����״̬���ȼ����ڻ���״̬
	if (GuardState == EAIState::Alerted)
	{
		return;
	}

	DrawDebugSphere(GetWorld(), Location, 32.0f, 12, FColor::Green, false, 10.0f);

	//P028��������������ת��������λ��
	FVector Direction = Location - GetActorLocation();
	Direction.Normalize();

	FRotator NewLookAt = FRotationMatrix::MakeFromX(Direction).Rotator();//ֻ�ı�yaw��
	NewLookAt.Pitch = 0.0f;
	NewLookAt.Roll = 0.0f;

	SetActorRotation(NewLookAt);

	//P028��������ת��󣬾���ָ��ʱ���ص�ԭ����λ��
	//GetWorldTimerManager��ȡ���糡��ʱ�ӹ�����
	GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrientation);//ֹͣԭ�ж�ʱ���ľ��
	GetWorldTimerManager().SetTimer(TimerHandle_ResetOrientation, this, &AFPSAIGuard::ResetOrientation, 3.0f);

	SetGuardState(EAIState::Suspicious);

	// Stop Movement if Patrolling
	//���������ڻ���״̬ʱ��ͨ��AI���Ƶ�һ�����ú�����ʵ��ֹͣ����
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

//P037 ��GuardState�����ı�ʱ�������onRep_GuardState����������ֻ�ڿͻ����Ͻ���
//��onRep_GuardState�����е���OnStateChanged��ͼ����������ȷ���ڿͻ�����Ҳ�ܶ���UMG���и���
void AFPSAIGuard::OnRep_GuardState()
{
	OnStateChanged(GuardState);
}

//P030 �ı�������״̬
void AFPSAIGuard::SetGuardState(EAIState NewState)
{
	if (GuardState == NewState)
	{
		return;
	}

	GuardState = NewState;
	
	//P037 ȷ�������Ҳ�ܶ�UMG���и��£������������OnRep_GuardState����
	OnRep_GuardState();

	//P037 ����ʹ����OnRep_GuardState�����������OnStateChanged�Ϳ���ע�͵���
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
		//P031��Ĭ�ϵ�ͼ�ϣ�������Ǵﲻ��100������challenge/AIpatrol��ͼ�ϾͿ��ԣ�ԭ��δ�ҳ�����������������

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


//P037 getLifetimeReplicatedProps�������������ڸ��ƣ�����ͬ���ľ������
//���ReplicatedUsing�����ã��ú�������ʵ��ͬ�����пͻ���
void AFPSAIGuard::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSAIGuard, GuardState);
}
