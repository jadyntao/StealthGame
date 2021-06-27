// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class AFPSProjectile;
class USoundBase;
class UAnimSequence;
class UPawnNoiseEmitterComponent;

//P036 多人模式设置：
//拖入多个PlayerStart，游戏模式会尝试在关卡中找到此类actor所在的位置，选择作为游戏玩家的起点
//在World Settings中选择BP_GameMode作为游戏模式，Default Pawn选择BP_Player蓝图类

UCLASS()
class AFPSCharacter : public ACharacter
{
	GENERATED_BODY()

protected:

	/** Pawn mesh: 1st person view  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mesh")
	USkeletalMeshComponent* Mesh1PComponent;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* GunMeshComponent;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComponent;

	//P026 添加声音发射器组件
	//可以在BP_player蓝图类中的Event Tick事件图中连接Pawn Make Noise 来持续制造声音，来测试该组件的效果
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UPawnNoiseEmitterComponent* NoiseEmitterComponent;

public:
	AFPSCharacter();

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	TSubclassOf<AFPSProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditDefaultsOnly, Category="Gameplay")
	USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	UAnimSequence* FireAnimation;

	//P038 在FPSExtractionZone.cpp中对MyPawn->bIsCarryingObjective为真时，显示“任务完成”。
	//而在FPSObjectiveActor.cpp中，仅在服务器上执行bIsCarryingObjective=true，导致客户端玩家永远不可能
	//使用Replicated关键词将bIsCarryingObjective复制到客户端上，另外需要在.cpp中实现getlifetimereplicateprogs函数来指定同步规则
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Gameplay")
	bool bIsCarryingObjective;

protected:
	
	/** Fires a projectile. */
	void Fire();

	//P034 服务器、可靠连接（能确保连接至服务器）、进行验证
	//与普通函数不同，通过serverFire_implementation(函数名_实现)来实现，返回类型为void
	//WithValidation的关键词需要创建并实现serverFire_validate，返回类型为bool
	//FPSCharacter.generated.h会自动创建上述两个函数//
	//注释中都不能出现上述两个函数名，不然 会编译错误，具体原因未知！！！！
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles strafing movement, left and right */
	void MoveRight(float Val);

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1PComponent; }

	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return CameraComponent; }
	
	virtual void Tick(float DeltaTime) override;
};

