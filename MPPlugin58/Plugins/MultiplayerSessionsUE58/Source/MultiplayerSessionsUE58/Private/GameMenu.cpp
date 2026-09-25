// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMenu.h"
#include "MultiplayerSessionsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "steam/steam_api.h"

#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/SizeBox.h"

#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"

#include "FriendItem.h"


bool UGameMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (btn_invfriends)
	{
		btn_invfriends->OnClicked.AddDynamic(this, &UGameMenu::OnInviteFriendsClicked);
	}

	if (txt_search)
	{
		txt_search->OnTextChanged.AddDynamic(this, &UGameMenu::OnSearchTextChanged);
		txt_search->OnTextCommitted.AddDynamic(this, &UGameMenu::OnTextEnter);
	}

	if (btn_invfriendssteam)
	{
		btn_invfriendssteam->OnClicked.AddDynamic(this, &UGameMenu::OnInviteFriendsSteamClicked);
	}

	if (btn_friendback)
	{
		btn_friendback->OnClicked.AddDynamic(this, &UGameMenu::OnFriendBackClicked);
	}

	return true;
}

void UGameMenu::NativeDestruct()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnReadFriendsListComplete.RemoveAll(this);
	}

	if (btn_invfriends)
	{
		btn_invfriends->OnClicked.RemoveDynamic(this, &UGameMenu::OnInviteFriendsClicked);
	}

	if (txt_search)
	{
		txt_search->OnTextChanged.RemoveDynamic(this, &UGameMenu::OnSearchTextChanged);
		txt_search->OnTextCommitted.RemoveDynamic(this, &UGameMenu::OnTextEnter);
	}

	if (btn_invfriendssteam)
	{
		btn_invfriendssteam->OnClicked.RemoveDynamic(this, &UGameMenu::OnInviteFriendsSteamClicked);
	}

	if (btn_friendback)
	{
		btn_friendback->OnClicked.RemoveDynamic(this, &UGameMenu::OnFriendBackClicked);
	}

	Super::NativeDestruct();
}

void UGameMenu::GameMenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Hidden);
	SetIsFocusable(true);
	szb_friendcontainer->SetVisibility(ESlateVisibility::Hidden);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnReadFriendsListComplete.AddUObject(this, &UGameMenu::OnReadFriendsList);
	}
}

void UGameMenu::OnInviteFriendsClicked()
{
	if (!MultiplayerSessionsSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("MultiplayerSessionsSubsystem is not valid"));
		return;
	}

	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Invite Friends button clicked"));
	}

	MultiplayerSessionsSubsystem->FriendsList(0);
	szb_pausemenu->SetVisibility(ESlateVisibility::Hidden);
}

void UGameMenu::OnInviteFriendsSteamClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Invite Friends Steam button clicked"));
	}
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
	if (OnlineSub)
	{
		IOnlineExternalUIPtr ExternalUI = OnlineSub->GetExternalUIInterface();
		if (ExternalUI.IsValid())
		{
			ExternalUI->ShowInviteUI(0,NAME_GameSession);
		}
	}
}

void UGameMenu::OnFriendBackClicked()
{
	txt_search->SetText(FText::GetEmpty());
	CurrentFilter = FString();
	RebuildFriendList();
	szb_pausemenu->SetVisibility(ESlateVisibility::Visible);
	szb_friendcontainer->SetVisibility(ESlateVisibility::Hidden);
}

void UGameMenu::OnSearchTextChanged(const FText& Text)
{
	CurrentFilter = Text.ToString().ToLower();
	RebuildFriendList();
}

void UGameMenu::OnTextEnter(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}

	// Apply the current filter and rebuild so the first row reflects the search
	CurrentFilter = Text.ToString().TrimStartAndEnd().ToLower();
	RebuildFriendList();

	// Now invite the top entry, if any
	if (!sb_friendslist)
	{
		return;
	}

	UWidget* FirstChild = sb_friendslist->GetChildAt(0);
	UFriendItem* TopEntry = Cast<UFriendItem>(FirstChild);
	if (!TopEntry)
	{
		UE_LOG(LogTemp, Warning, TEXT("No matching friend to invite"));
		return;
	}

	TopEntry->InviteFriend();
}

void UGameMenu::OnReadFriendsList(int32 LocalPlayerNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReadFriendsList failed: %s"), *ErrorStr);
		return;
	}

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (!OnlineSub) return;

	IOnlineFriendsPtr FriendsInterface = OnlineSub->GetFriendsInterface();
	if (!FriendsInterface.IsValid()) return;

	TArray<TSharedRef<FOnlineFriend>> FriendList;
	if (!FriendsInterface->GetFriendsList(LocalPlayerNum, ListName, FriendList))
		return;

	const uint32 MyAppId = GetCurrentAppId();

	CachedFriends.Empty();
	CachedFriends.Reserve(FriendList.Num());

	for (const TSharedRef<FOnlineFriend>& Friend : FriendList)
	{
		FFriendData Data;
		Data.DisplayName = Friend->GetDisplayName();
		Data.UserId = Friend->GetUserId();

		const FOnlineUserPresence& Presence = Friend->GetPresence();
		Data.bIsOnline = Presence.bIsOnline;

		Data.bIsPlayingThisGame = Presence.bIsPlaying &&
			Presence.GetAppId() == FString::FromInt(MyAppId);
		Data.PlayingGameName = Presence.Status.StatusStr;

		CachedFriends.Add(MoveTemp(Data));
	}

	RebuildFriendList();

#if 0
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, TEXT("OnReadFriendsList called"));
	}

	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReadFriendsList failed: %s"), *ErrorStr);
		return;
	}

	if (!sb_friendslist)
	{
		UE_LOG(LogTemp, Warning, TEXT("FriendListContainer is not bound"));
		return;
	}

	sb_friendslist->ClearChildren();

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (!OnlineSub)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnlineSubsystem is not valid"));
		return;
	}

	IOnlineFriendsPtr FriendsInterface = OnlineSub->GetFriendsInterface();
	if (!FriendsInterface.IsValid()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("FriendsInterface is not valid"));
		return;
	}

	TArray<TSharedRef<FOnlineFriend>> FriendList;
	if (!FriendsInterface->GetFriendsList(LocalPlayerNum, ListName, FriendList))
	{
		UE_LOG(LogTemp, Warning, TEXT("GetFriendsList failed for LocalPlayerNum: %d"), LocalPlayerNum);
		return;
	}

	for (const TSharedRef<FOnlineFriend>& Friend : FriendList)
	{
		const FString DisplayName = Friend->GetDisplayName();
		const TSharedPtr<const FUniqueNetId> UserId = Friend->GetUserId();
		const FOnlineUserPresence& Presence = Friend->GetPresence();

		if (!UserId.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Friend %s has invalid UserId"), *DisplayName);
			continue; // or skip avatar, still add entry
		}

		// Create the entry widget
		UFriendItem* Entry = CreateWidget<UFriendItem>(
			GetOwningPlayer(), FriendItem);
		if (!Entry) continue;

		// Convert FUniqueNetId to SteamID
		uint64 SteamId = FCString::Strtoui64(*UserId->ToString(), nullptr, 10);
		CSteamID SteamID(SteamId);

		// Request user info first (Steam caches asynchronously)
		SteamFriends()->RequestUserInformation(SteamID, false);

		// Get avatar handle
		int AvatarHandle = SteamFriends()->GetMediumFriendAvatar(SteamID);
		//if (AvatarHandle == -1) continue; // Not ready yet, skip this frame

		UTexture2D* AvatarTexture = nullptr;

		// Get dimensions and raw data
		uint32 Width = 0, Height = 0;
		if (SteamUtils()->GetImageSize(AvatarHandle, &Width, &Height) && Width > 0)
		{
			TArray<uint8> RawData;
			RawData.SetNumUninitialized(Width * Height * 4);

			if (SteamUtils()->GetImageRGBA(AvatarHandle, RawData.GetData(), RawData.Num()))
			{
				UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
				void* MipData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				FMemory::Memcpy(MipData, RawData.GetData(), RawData.Num());
				Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
				Texture->UpdateResource();

				if (Texture)
				{
					AvatarTexture = Texture;
				}
			}
		}

		Entry->Setup(DisplayName, AvatarTexture, Presence.bIsOnline, UserId);

		// Add it to the container
		sb_friendslist->AddChild(Entry);
	}

	sb_friendslist->SetVisibility(ESlateVisibility::Visible);
#endif
}

uint32 UGameMenu::GetCurrentAppId() const
{
	if (SteamUtils())
	{
		return SteamUtils()->GetAppID();
	}
	return 0;
}

void UGameMenu::RebuildFriendList()
{
	if (!sb_friendslist)
	{
		UE_LOG(LogTemp, Warning, TEXT("sb_friendslist is not bound"));
		return;
	}

	sb_friendslist->ClearChildren();

	// 1) Filter
	TArray<FFriendData> Filtered;
	Filtered.Reserve(CachedFriends.Num());
	for (const FFriendData& F : CachedFriends)
	{
		if (CurrentFilter.IsEmpty() ||
			F.DisplayName.ToLower().Contains(CurrentFilter))
		{
			Filtered.Add(F);
		}
	}

	// 2) Sort: same-game > online > offline, then alphabetical
	Filtered.Sort([](const FFriendData& A, const FFriendData& B)
		{
			auto Rank = [](const FFriendData& F) -> int32
				{
					if (F.bIsPlayingThisGame) return 0;
					if (F.bIsOnline)          return 1;
					return 2;
				};

			const int32 RA = Rank(A);
			const int32 RB = Rank(B);
			if (RA != RB) return RA < RB;

			return A.DisplayName < B.DisplayName;
		});

	// 3) Build rows
	for (const FFriendData& F : Filtered)
	{
		if (!FriendItem)
		{
			UE_LOG(LogTemp, Warning, TEXT("FriendItem class not set on GameMenu BP"));
			break;
		}

		UFriendItem* Entry = CreateWidget<UFriendItem>(GetOwningPlayer(), FriendItem);
		if (!Entry) continue;

		Entry->Setup(F.DisplayName, F.AvatarTexture, F.bIsOnline, F.UserId);
		Entry->SetPlayingThisGame(F.bIsPlayingThisGame, F.PlayingGameName);
		
		// 4) Steam avatar (optional, best-effort)
		if (F.UserId.IsValid() && SteamFriends() && SteamUtils())
		{
			uint64 SteamId = FCString::Strtoui64(*F.UserId->ToString(), nullptr, 10);
			if (SteamId != 0)
			{
				CSteamID SteamID(SteamId);
				SteamFriends()->RequestUserInformation(SteamID, false);

				int32 AvatarHandle = SteamFriends()->GetMediumFriendAvatar(SteamID);
				if (AvatarHandle != -1)
				{
					uint32 Width = 0, Height = 0;
					if (SteamUtils()->GetImageSize(AvatarHandle, &Width, &Height) &&
						Width > 0 && Height > 0)
					{
						TArray<uint8> RawData;
						RawData.SetNumUninitialized(Width * Height * 4);

						if (SteamUtils()->GetImageRGBA(AvatarHandle, RawData.GetData(), RawData.Num()))
						{
							UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
							if (Texture)
							{
								void* MipData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
								FMemory::Memcpy(MipData, RawData.GetData(), RawData.Num());
								Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
								Texture->UpdateResource();

								Entry->SetImgAvatar(Texture);
							}
						}
					}
				}
			}
		}

		sb_friendslist->AddChild(Entry);
	}

	szb_friendcontainer->SetVisibility(ESlateVisibility::Visible);
}
