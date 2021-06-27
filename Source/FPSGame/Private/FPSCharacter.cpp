// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSCharacter.h"
#include "FPSProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "Net/UnrealNetwork.h"


AFPSCharacter::AFPSCharacter()
{
	// Create a CameraComponent	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->RelativeLocation = FVector(0, 0, BaseEyeHeight); // Position the camera
	CameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1PComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	Mesh1PComponent->SetupAttachment(CameraComponent);
	Mesh1PComponent->CastShadow = false;
	Mesh1PComponent->RelativeRotation = FRotator(2.0f, -15.0f, 5.0f);
	Mesh1PComponent->RelativeLocation = FVector(0, 0, -160.0f);

	// Create a gun mesh component
	GunMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	GunMeshComponent->CastShadow = false;
	GunMeshComponent->SetupAttachment(Mesh1PComponent, "GripPoint");

	NoiseEmitterComponent = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("NoiseEmitter"));
}


void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::Fire);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}


void AFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//P035 ���ڿͻ����ϲ��������ʱ�����º������ڷ������ϣ����ڿͻ��������У������������Ρ�
	//ֻϣ���ڲ�������ɫʱ�����������������ʵ�ʲ�����ɫʱ����ͨ���������ؼ����ı���ת�ȣ�����������Ʊ���������
	//IsLocallyControlled()��ʾ����Ƿ��ܱ��ؿ��ƣ��������if��������ʹ��Ҳ��ܱ��ؿ���ʱ����ִ�и���
	if (!IsLocallyControlled())
	{
		FRotator NewRot = CameraComponent->RelativeRotation;
		//P035 RemoteViewPitch��Alt+G�����Կ���������Ϊuint8����Ϊ��ֵ����FRotator�е�Ϊfloat
		//��Pawn.cpp��SetRelativeRotationʹ�������µ�ת��
		NewRot.Pitch = RemoteViewPitch * 360.0f / 255.0f;

		CameraComponent->SetRelativeRotation(NewRot);
	}
}

//P034 ���ڿͻ����н��п������ʱ��ֻ���ڵ��´������ɷ����
//��AFPSCharacter::Fire()���ڿͻ�����ִ�У��޷��������������ͬ���ķ�����
//ͨ��������������ServerFire()��ȷ�������������ڷ�������ִ�У���ͨ��FPSProjectile.cpp�е�SetReplicates(true)���Ƶ��ͻ�����
void AFPSCharacter::Fire()
{
	ServerFire();

	// try and play the sound if specified
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1PComponent->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->PlaySlotAnimationAsDynamicMontage(FireAnimation, "Arms", 0.0f);
		}
	}
}


void AFPSCharacter::ServerFire_Implementation()
{
	// try and fire a projectile
	if (ProjectileClass)
	{
		//ǹ��λ�á�ǹ����ת��
		FVector MuzzleLocation = GunMeshComponent->GetSocketLocation("Muzzle");
		FRotator MuzzleRotation = GunMeshComponent->GetSocketRotation("Muzzle");

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		//P027���ɷ�����ʱ�����õ���Instigator��ʹ������OnHit������ʱ�������á�
		ActorSpawnParams.Instigator = this;

		// spawn the projectile at the muzzle
		GetWorld()->SpawnActor<AFPSProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);
	}
}

//P034 Withvalidation�ؼ��ʵ�ʵ�ֺ������ڷ������˽��������Լ��ʱ�õ�
//���ͻ��˷���return false����������ʱ���������һ���������ʱ����������ǿ�ƶϿ����ӡ�
//һ������£���return true ������Ҫ����������ר����
bool AFPSCharacter::ServerFire_Validate()
{
	return true;
}


void AFPSCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}


void AFPSCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}


void AFPSCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//P038 Ĭ�����ã�ͬ�������пͻ���
	DOREPLIFETIME(AFPSCharacter, bIsCarryingObjective);

	//P038 ��һ�ַ����������ݴ�������һ���ƽ�ɫ�Ļ����ϣ��ܹ���ʡ����
	//DOREPLIFETIME_CONDITION(AFPSCharacter, bIsCarryingObjective, COND_OwnerOnly);
}
