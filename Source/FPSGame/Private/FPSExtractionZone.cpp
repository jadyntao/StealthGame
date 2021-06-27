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
	//创建默认子对象，并指定名字为OverlapComp，会在蓝图中显示
	OverlapComp = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapComp"));
	//设置碰撞为QueryOnly（只发出询问）
	OverlapComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//设置碰撞为忽略所有通道
	OverlapComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	//设置碰撞为仅对Pawn（人形体）响应
	OverlapComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	//设置盒体范围（合适的响应范围）
	OverlapComp->SetBoxExtent(FVector(200.0f));
	RootComponent = OverlapComp;

	//将默认隐藏设置改为false
	OverlapComp->SetHiddenInGame(false);

	OverlapComp->OnComponentBeginOverlap.AddDynamic(this, &AFPSExtractionZone::HandleOverlap);

	//创建默认子对象，类型为UDecalComponent（贴花组件），并指定名字为DecalComp，会在蓝图中显示
	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	//设置贴花尺寸
	DecalComp->DecalSize = FVector(200.0f, 200.0f, 200.0f);
	DecalComp->SetupAttachment(RootComponent);
}


void AFPSExtractionZone::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//触发重叠的对象强制转成AFPSCharacter类型
	AFPSCharacter* MyPawn = Cast<AFPSCharacter>(OtherActor);
	//判断AFPSCharacter类型的对象是否存在
	if (MyPawn == nullptr)
	{
		return;
	}
	//判断是否携带目标物体
	if (MyPawn->bIsCarryingObjective)
	{
		AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			//P029如果携带目标物体，则在通过FPSGameMode.cpp告知GameMode，游戏成功并结束
			GM->CompleteMission(MyPawn, true);
		}
	}
	//没有携带到目标物体，播放对应音效（音效在BP_ExtractionZone蓝图中Details - Sound）
	else
	{
		UGameplayStatics::PlaySound2D(this, ObjectiveMissingSound);
	}

	UE_LOG(LogTemp, Log, TEXT("Overlapped with extraction zone!"));
}
