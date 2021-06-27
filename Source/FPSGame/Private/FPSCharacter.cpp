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

	//P035 当在客户端上操作的玩家时，更新函数既在服务器上，又在客户端上运行，即会运行两次。
	//只希望在不操作角色时运行这个函数，而在实际操作角色时，是通过玩家输入控件来改变旋转度，不让这个复制变量来决定
	//IsLocallyControlled()表示玩家是否受本地控制，加上这个if条件可以使玩家不受本地控制时进行执行更新
	if (!IsLocallyControlled())
	{
		FRotator NewRot = CameraComponent->RelativeRotation;
		//P035 RemoteViewPitch按Alt+G，可以看到其类型为uint8不能为负值，而FRotator中的为float
		//在Pawn.cpp中SetRelativeRotation使用了如下的转换
		NewRot.Pitch = RemoteViewPitch * 360.0f / 255.0f;

		CameraComponent->SetRelativeRotation(NewRot);
	}
}

//P034 当在客户端中进行开火操作时，只会在当下窗口生成发射物，
//即AFPSCharacter::Fire()仅在客户端中执行，无法命令服务器产生同样的发射物
//通过服务器函数的ServerFire()来确保发射物生成在服务器中执行，并通过FPSProjectile.cpp中的SetReplicates(true)复制到客户端中
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
		//枪口位置、枪口旋转度
		FVector MuzzleLocation = GunMeshComponent->GetSocketLocation("Muzzle");
		FRotator MuzzleRotation = GunMeshComponent->GetSocketRotation("Muzzle");

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		//P027生成发射物时，会用到此Instigator，使其能在OnHit被调用时合理设置。
		ActorSpawnParams.Instigator = this;

		// spawn the projectile at the muzzle
		GetWorld()->SpawnActor<AFPSProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);
	}
}

//P034 Withvalidation关键词的实现函数，在服务器端进行完整性检查时用到
//当客户端发起return false反馈或请求时，如出现外挂或其他问题时，服务器会强制断开连接。
//一般情况下，就return true 除非需要进行其他的专项检查
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
	//P038 默认设置，同步到所有客户端
	DOREPLIFETIME(AFPSCharacter, bIsCarryingObjective);

	//P038 另一种方法，让数据传输至任一控制角色的机器上，能够节省带宽
	//DOREPLIFETIME_CONDITION(AFPSCharacter, bIsCarryingObjective, COND_OwnerOnly);
}
