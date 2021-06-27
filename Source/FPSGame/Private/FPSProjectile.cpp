// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

AFPSProjectile::AFPSProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AFPSProjectile::OnHit);	// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;

	//P034 设置复制品，服务端代码中在生成发射物时为客户端窗口复制同样的效果
	//但是，客户端的动作不能复制到服务端（只有服务端具有命令权，而客户端仅负责模拟游戏世界的内容）
	//需要用到UFUNCTION(Server, Reliable, WithValidation)来确保代码只在服务器上运行，而不是客户端
	SetReplicates(true);
	//P034 设置复制品移动
	SetReplicateMovement(true);
}


void AFPSProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
	}
	//P034 检查玩家是否在服务器上运行代码或是否主机玩家正在操作
	//不希望在客户端上销毁actor，因为客户端并不拥有actor，他们只是模拟生成了服务器的指令。
	//因此，将Destroy()放入该if条件中，通过replicate让服务器决定何时在游戏中销毁actor
	if (Role == ROLE_Authority)
	{
		//P027Instigator可以确保MakeNoise函数调用时确定是否具有PawnNoiseEmitterComponent人形体声音发射器组件，从而发出真实声音
		//在AFPSCharacter::Fire()开枪函数中给Instigator=this赋值。
		MakeNoise(1.0f, Instigator);

		Destroy();
	}
}