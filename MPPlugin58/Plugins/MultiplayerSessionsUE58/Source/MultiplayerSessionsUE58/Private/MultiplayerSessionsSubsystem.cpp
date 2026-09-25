// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"

#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete)),
	FindFriendSessionCompleteDelegate(FOnFindFriendSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnFindFriendSessionComplete)),
	SessionUserInviteAcceptedDelegate(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &ThisClass::OnSessionUserInviteAccepted))
{
}

void UMultiplayerSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Acquire interfaces now — the OSS is initialized by this point
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		UE_LOG(LogTemp, Warning, TEXT("Initialize: OSS=%s"),
			*Subsystem->GetSubsystemName().ToString());

		SessionInterface = Subsystem->GetSessionInterface();
		FriendsInterface = Subsystem->GetFriendsInterface();
	}

	if (SessionInterface.IsValid())
	{
		SessionUserInviteAcceptedDelegateHandle =
			SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
				SessionUserInviteAcceptedDelegate);

		UE_LOG(LogTemp, Warning, TEXT("Invite delegates bound"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SessionInterface invalid — invite delegates not bound"));
	}
}

void UMultiplayerSessionsSubsystem::Deinitialize()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(
			SessionUserInviteAcceptedDelegateHandle);
	}
	Super::Deinitialize();
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;

		DestroySession();
	}

	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->BuildUniqueId = 1;

	const ULocalPlayer* LocalPlayer = GetGameInstance()->GetFirstLocalPlayerController()->GetLocalPlayer();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		//broadcast our own custom delegate
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	if(!SessionInterface->JoinSession(*GetGameInstance()->GetFirstLocalPlayerController()->GetLocalPlayer()->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}

}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnStartSessionComplete.Broadcast(false);
		return;
	}

	StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

	if (!SessionInterface->StartSession(NAME_GameSession))
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		MultiplayerOnStartSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::FindFriendSession(int32 LocalPlayerNum, const FUniqueNetId& FriendUniqueNetId)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SessionInterface is not valid"));
		MultiplayerOnFindFriendSessionComplete.Broadcast(LocalPlayerNum, false, TArray<FOnlineSessionSearchResult>());
		return;
	}

	FindFriendSessionCompleteDelegateHandle = SessionInterface->AddOnFindFriendSessionCompleteDelegate_Handle(LocalPlayerNum, FindFriendSessionCompleteDelegate);

	if (!SessionInterface->FindFriendSession(LocalPlayerNum, FriendUniqueNetId))
	{
		SessionInterface->ClearOnFindFriendSessionCompleteDelegate_Handle(LocalPlayerNum, FindFriendSessionCompleteDelegateHandle);
		MultiplayerOnFindFriendSessionComplete.Broadcast(LocalPlayerNum, false, TArray<FOnlineSessionSearchResult>());
		UE_LOG(LogTemp, Warning, TEXT("FindFriendSession call failed"));
	}
}

void UMultiplayerSessionsSubsystem::SendInviteToFriend(int32 LocalPlayerNum, const FUniqueNetId& FriendUniqueNetId)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SessionInterface is not valid"));
		return;
	}

	if (SessionInterface->SendSessionInviteToFriend(
		LocalPlayerNum, NAME_GameSession, FriendUniqueNetId))
	{
		UE_LOG(LogTemp, Display,
			TEXT("Sent session invite from LocalPlayerNum %d to friend %s"),
			LocalPlayerNum, *FriendUniqueNetId.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("SendSessionInviteToFriend FAILED for LocalPlayerNum %d, friend %s"),
			LocalPlayerNum, *FriendUniqueNetId.ToString());
	}
}

void UMultiplayerSessionsSubsystem::FriendsList(int32 LocalPlayerNum)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, TEXT("FriendsList called"));
	}

	if (!FriendsInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("FriendsInterface is not valid"));
		MultiplayerOnReadFriendsListComplete.Broadcast(LocalPlayerNum, false, TEXT(""), TEXT("FriendsInterface is not valid"));
		return;
	}

	// Create the delegate inline and pass it as the third argument.
	FOnReadFriendsListComplete ReadCompleteDelegate;
	ReadCompleteDelegate.BindUObject(this, &ThisClass::OnReadFriendsListComplete);

	if (!FriendsInterface->ReadFriendsList(LocalPlayerNum, TEXT("default"), ReadCompleteDelegate))
	{
		MultiplayerOnReadFriendsListComplete.Broadcast(LocalPlayerNum, false, TEXT("default"), TEXT("ReadFriendsList call failed"));
		UE_LOG(LogTemp, Warning, TEXT("ReadFriendsList call failed for LocalPlayerNum: %d"), LocalPlayerNum);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("ReadFriendsList called successfully for LocalPlayerNum: %d"), LocalPlayerNum);
	}
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	//broadcast our own custom delegate
	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (LastSessionSearch.IsValid() && LastSessionSearch->SearchResults.Num() <= 0)
	{
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	}

	MultiplayerOnStartSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindFriendSessionComplete(int32 LocalPlayerNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SessionResults)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindFriendSessionCompleteDelegate_Handle(LocalPlayerNum, FindFriendSessionCompleteDelegateHandle);
	}

	// Broadcast the custom delegate with the session results
	MultiplayerOnFindFriendSessionComplete.Broadcast(LocalPlayerNum, bWasSuccessful, SessionResults);

	if (bWasSuccessful && SessionResults.Num() > 0)
	{
		UE_LOG(LogTemp, Display, TEXT("Found friend session for LocalPlayerNum: %d with %d results"), LocalPlayerNum, SessionResults.Num());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find friend session or no results found for LocalPlayerNum: %d"), LocalPlayerNum);
	}
}

void UMultiplayerSessionsSubsystem::OnSessionUserInviteAccepted(bool bWasSuccessful, int32 LocalPlayerNum, FUniqueNetIdPtr PersonInvited, const FOnlineSessionSearchResult& SearchResult)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, TEXT("OnSessionUserInviteAccepted called"));
	}

	if (!bWasSuccessful || !SearchResult.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to accept session invite for LocalPlayerNum: %d"), LocalPlayerNum);
		MultiplayerOnSessionUserInviteAccepted.Broadcast(
			false, LocalPlayerNum, PersonInvited, SearchResult);
		return;
	}

	JoinSession(SearchResult);

	MultiplayerOnSessionUserInviteAccepted.Broadcast(
		bWasSuccessful, LocalPlayerNum, PersonInvited, SearchResult);
}

void UMultiplayerSessionsSubsystem::OnReadFriendsListComplete(int32 LocalPlayerNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, TEXT("OnReadFriendsListComplete called"));
	}

	// Broadcast the custom delegate with the result
	MultiplayerOnReadFriendsListComplete.Broadcast(LocalPlayerNum, bWasSuccessful, ListName, ErrorStr);

	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Display, TEXT("Successfully read friends list for LocalPlayerNum: %d, List: %s"), LocalPlayerNum, *ListName);

		// Retrieve the friends list after it has been read
		TArray<TSharedRef<FOnlineFriend>> FriendsList;
		if (FriendsInterface->GetFriendsList(LocalPlayerNum, ListName, FriendsList))
		{
			UE_LOG(LogTemp, Display, TEXT("Found %d friends in list %s"), FriendsList.Num(), *ListName);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to read friends list for LocalPlayerNum: %d, Error: %s"), LocalPlayerNum, *ErrorStr);
	}
}
