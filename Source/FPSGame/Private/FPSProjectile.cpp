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

	//P034 ���ø���Ʒ������˴����������ɷ�����ʱΪ�ͻ��˴��ڸ���ͬ����Ч��
	//���ǣ��ͻ��˵Ķ������ܸ��Ƶ�����ˣ�ֻ�з���˾�������Ȩ�����ͻ��˽�����ģ����Ϸ��������ݣ�
	//��Ҫ�õ�UFUNCTION(Server, Reliable, WithValidation)��ȷ������ֻ�ڷ����������У������ǿͻ���
	SetReplicates(true);
	//P034 ���ø���Ʒ�ƶ�
	SetReplicateMovement(true);
}


void AFPSProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
	}
	//P034 �������Ƿ��ڷ����������д�����Ƿ�����������ڲ���
	//��ϣ���ڿͻ���������actor����Ϊ�ͻ��˲���ӵ��actor������ֻ��ģ�������˷�������ָ�
	//��ˣ���Destroy()�����if�����У�ͨ��replicate�÷�����������ʱ����Ϸ������actor
	if (Role == ROLE_Authority)
	{
		//P027Instigator����ȷ��MakeNoise��������ʱȷ���Ƿ����PawnNoiseEmitterComponent����������������������Ӷ�������ʵ����
		//��AFPSCharacter::Fire()��ǹ�����и�Instigator=this��ֵ��
		MakeNoise(1.0f, Instigator);

		Destroy();
	}
}