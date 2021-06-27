// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSExtractionZone.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "FPSCharacter.h"
#include "FPSGameMode.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AFPSExtractionZone::AFPSExtractionZone()
{
	//����Ĭ���Ӷ��󣬲�ָ������ΪOverlapComp��������ͼ����ʾ
	OverlapComp = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapComp"));
	//������ײΪQueryOnly��ֻ����ѯ�ʣ�
	OverlapComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//������ײΪ��������ͨ��
	OverlapComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	//������ײΪ����Pawn�������壩��Ӧ
	OverlapComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	//���ú��巶Χ�����ʵ���Ӧ��Χ��
	OverlapComp->SetBoxExtent(FVector(200.0f));
	RootComponent = OverlapComp;

	//��Ĭ���������ø�Ϊfalse
	OverlapComp->SetHiddenInGame(false);

	OverlapComp->OnComponentBeginOverlap.AddDynamic(this, &AFPSExtractionZone::HandleOverlap);

	//����Ĭ���Ӷ�������ΪUDecalComponent���������������ָ������ΪDecalComp��������ͼ����ʾ
	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	//���������ߴ�
	DecalComp->DecalSize = FVector(200.0f, 200.0f, 200.0f);
	DecalComp->SetupAttachment(RootComponent);
}


void AFPSExtractionZone::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//�����ص��Ķ���ǿ��ת��AFPSCharacter����
	AFPSCharacter* MyPawn = Cast<AFPSCharacter>(OtherActor);
	//�ж�AFPSCharacter���͵Ķ����Ƿ����
	if (MyPawn == nullptr)
	{
		return;
	}
	//�ж��Ƿ�Я��Ŀ������
	if (MyPawn->bIsCarryingObjective)
	{
		AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			//P029���Я��Ŀ�����壬����ͨ��FPSGameMode.cpp��֪GameMode����Ϸ�ɹ�������
			GM->CompleteMission(MyPawn, true);
		}
	}
	//û��Я����Ŀ�����壬���Ŷ�Ӧ��Ч����Ч��BP_ExtractionZone��ͼ��Details - Sound��
	else
	{
		UGameplayStatics::PlaySound2D(this, ObjectiveMissingSound);
	}

	UE_LOG(LogTemp, Log, TEXT("Overlapped with extraction zone!"));
}
