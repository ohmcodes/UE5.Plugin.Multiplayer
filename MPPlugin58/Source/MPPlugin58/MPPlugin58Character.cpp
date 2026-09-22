// Copyright Epic Games, Inc. All Rights Reserved.

#include "MPPlugin58Character.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "MPPlugin58.h"
#include "Kismet/GameplayStatics.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"


AMPPlugin58Character::AMPPlugin58Character():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &AMPPlugin58Character::OnCreateSessionComplete)),
	FindSessionCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &AMPPlugin58Character::OnFindSessionComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &AMPPlugin58Character::OnJoinSessionComplete))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();

	if (OnlineSubsystem)
	{
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
		UE_LOG(LogMPPlugin58, Log, TEXT("Online Subsystem: %s"), *OnlineSubsystem->GetSubsystemName().ToString());
	}
	else
	{
		UE_LOG(LogMPPlugin58, Warning, TEXT("No Online Subsystem found!"));
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 
			5.f, 
			FColor::Green, 
			FString::Printf(TEXT("Online Subsystem Initialized: %s"), *OnlineSubsystem->GetSubsystemName().ToString())
		);

	}
}

void AMPPlugin58Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMPPlugin58Character::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMPPlugin58Character::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMPPlugin58Character::Look);
	}
	else
	{
		UE_LOG(LogMPPlugin58, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMPPlugin58Character::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AMPPlugin58Character::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AMPPlugin58Character::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AMPPlugin58Character::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AMPPlugin58Character::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AMPPlugin58Character::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AMPPlugin58Character::CreateGameSession()
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}

	auto ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);

	if (ExistingSession != nullptr)
	{
		OnlineSessionInterface->DestroySession(NAME_GameSession);
	}

	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());

	SessionSettings->bIsLANMatch = false;
	SessionSettings->NumPublicConnections = 4;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->Set(FName("MatchType"), FString("FreeForAll"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings);
}

void AMPPlugin58Character::JoinGameSession()
{
	if (!OnlineSessionInterface.IsValid())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				20.f,
				FColor::Red,
				FString(TEXT("Online Session Interface is not valid!"))
			);
		}
		
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			20.f,
			FColor::Green,
			FString(TEXT("Joining Session"))
		);
	}

	OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegate);

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = 1000;
	SessionSearch->bIsLanQuery = false;
	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
}

void AMPPlugin58Character::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogMPPlugin58, Log, TEXT("Session created successfully: %s"), *SessionName.ToString());

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				20.f,
				FColor::Blue,
				FString::Printf(TEXT("Session created successfully: %s"), *SessionName.ToString())
			);

		}

		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(FString("/Game/Levels/Lobby?listen"));
		}

	}
	else
	{
		UE_LOG(LogMPPlugin58, Error, TEXT("Failed to create session: %s"), *SessionName.ToString());

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				20.f,
				FColor::Red,
				FString(TEXT("Failed to create session"))
			);

		}
	}

}

void AMPPlugin58Character::OnFindSessionComplete(bool bWasSuccessful)
{
	if (!OnlineSessionInterface.IsValid())
	{
		UE_LOG(LogMPPlugin58, Log, TEXT("Online Session Interface is not valid!"));

		return;
	}

	for (auto Result : SessionSearch->SearchResults)
	{
		FString Id = Result.GetSessionIdStr();
		FString User = Result.Session.OwningUserName;
		FString MatchType;
		Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);

		UE_LOG(LogMPPlugin58, Log, TEXT("Found session Id: %s, Owning User: %s"), *Id, *User);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				20.f,
				FColor::Green,
				FString::Printf(TEXT("Found session Id: %s, Owning User: %s, MatchType: %s"), *Id, *User, *MatchType)
			);
		}
		if (MatchType == "FreeForAll")
		{
			UE_LOG(LogMPPlugin58, Log, TEXT("Found FreeForAll"));

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					20.f,
					FColor::Green,
					FString::Printf(TEXT("Joining session Id: %s, Owning User: %s, MatchType: %s"), *Id, *User, *MatchType)
				);
			}
			
			OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

			const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Result);

			break;
		}
	}
}

void AMPPlugin58Character::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}
	FString ConnectInfo;
	if(OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectInfo))
	{
		UE_LOG(LogMPPlugin58, Log, TEXT("Resolved Connect String: %s"), *ConnectInfo);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				20.f,
				FColor::Yellow,
				FString::Printf(TEXT("Resolved Connect String: %s"), *ConnectInfo)
			);
		}
	}

	APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController(GetWorld());

	if (PlayerController)
	{
		UE_LOG(LogMPPlugin58, Log, TEXT("Traveling"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				20.f,
				FColor::Green,
				FString(TEXT("Traveling"))
			);
		}
		PlayerController->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
	}
	else
	{
		UE_LOG(LogMPPlugin58, Error, TEXT("PlayerController is null!"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				20.f,
				FColor::Red,
				FString(TEXT("PlayerController is null!"))
			);
		}
	}
}
