// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"

#include "MultiplayerSessionsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);

DECLARE_MULTICAST_DELEGATE_ThreeParams(FMultiplayerOnFindFriendSessionComplete, int32 LocalPlayerNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>&  SessionResults);
DECLARE_MULTICAST_DELEGATE_FourParams(FMultiplayerOnSessionInviteReceived, const FUniqueNetId& LocalUserId, const FUniqueNetId& PersonInviting, const FString& AppId, const FOnlineSessionSearchResult& SearchResult);
DECLARE_MULTICAST_DELEGATE_FourParams(FMultiplayerOnSessionUserInviteAccepted, bool bWasSuccessful, int32 LocalPlayerNum, FUniqueNetIdPtr PersonInvited, const FOnlineSessionSearchResult& SearchResult);
DECLARE_MULTICAST_DELEGATE_FourParams(FMultiplayerOnReadFriendsListComplete, int32 LocalPlayerNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONSUE58_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSessionsSubsystem();
	
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();

	void FindFriendSession(int32 LocalPlayerNum, const FUniqueNetId& FriendUniqueNetId);
	void SendSessionInviteToFriend(int32 LocalPlayerNum, const FUniqueNetId& FriendUniqueNetId);
	void FriendsList(int32 LocalPlayerNum);

	// Custom Delegates for the Menu class to bind to
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;

	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

	FMultiplayerOnFindFriendSessionComplete MultiplayerOnFindFriendSessionComplete;
	FMultiplayerOnSessionInviteReceived MultiplayerOnSessionInviteReceived;
	FMultiplayerOnSessionUserInviteAccepted MultiplayerOnSessionUserInviteAccepted;
	FMultiplayerOnReadFriendsListComplete MultiplayerOnReadFriendsListComplete;

protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

	void OnFindFriendSessionComplete(int32 LocalPlayerNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SessionResults);
	void OnSessionInviteReceived(const FUniqueNetId& LocalUserId, const FUniqueNetId& PersonInviting, const FString& AppId, const FOnlineSessionSearchResult& SearchResult);
	void OnSessionUserInviteAccepted(bool bWasSuccessful, int32 LocalPlayerNum, FUniqueNetIdPtr PersonInvited, const FOnlineSessionSearchResult& SearchResult);
	void OnReadFriendsListComplete(int32 LocalPlayerNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

private:
	IOnlineSessionPtr SessionInterface;
	IOnlineFriendsPtr FriendsInterface;

	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;

	FOnFindFriendSessionCompleteDelegate FindFriendSessionCompleteDelegate;
	FOnSessionInviteReceivedDelegate SessionInviteReceivedDelegate;
	FOnSessionUserInviteAcceptedDelegate SessionUserInviteAcceptedDelegate;

	FOnReadFriendsListComplete ReadFriendsListCompleteDelegate;

	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;
	FDelegateHandle FindFriendSessionCompleteDelegateHandle;
	FDelegateHandle SessionInviteReceivedDelegateHandle;
	FDelegateHandle SessionUserInviteAcceptedDelegateHandle;
	FDelegateHandle ReadFriendsListCompleteDelegateHandle;

	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;

};
