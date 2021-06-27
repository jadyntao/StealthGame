// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSLaunchPad.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UArrowComponent;


UCLASS()
class FPSGAME_API AFPSLaunchPad : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSLaunchPad();

protected:

	//创建网格体组件
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	//创建盒体组件（用于重叠）
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* OverlapComp;

	//重叠触发函数
	// Marked with ufunction to bind to overlap event
	UFUNCTION()
	void OverlapLaunchPad(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/* Total impulse added to the character on overlap
		Marked 'EditInstanceOnly' to allow in-level editing of this property per instance. */
	//创建变量，作为弹射力度
	UPROPERTY(EditInstanceOnly, Category = "LaunchPad")
	float LaunchStrength;
	//EditInstanceOnly（仅编辑示例，将它们放置在关卡中时，都可以进行编辑）
	/* Angle added on top of actor rotation to launch the character. 
		Marked 'EditInstanceOnly' to allow in-level editing of this property per instance. */ 
	//创建变量，作为弹射角度
	UPROPERTY(EditInstanceOnly, Category = "LaunchPad")
	float LaunchPitchAngle;

	/* Effect to play when activating launch pad */
	UPROPERTY(EditDefaultsOnly, Category = "LaunchPad")
	UParticleSystem* ActivateLaunchPadEffect;

};
