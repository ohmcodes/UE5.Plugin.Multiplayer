// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete)),
	FindFriendSessionCompleteDelegate(FOnFindFriendSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnFindFriendSessionComplete)),
	SessionInviteReceivedDelegate(FOnSessionInviteReceivedDelegate::CreateUObject(this, &ThisClass::OnSessionInviteReceived)),
	SessionUserInviteAcceptedDelegate(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &ThisClass::OnSessionUserInviteAccepted)),
	ReadFriendsListCompleteDelegate(FOnReadFriendsListComplete::CreateUObject(this, &ThisClass::OnReadFriendsListComplete))
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();
		FriendsInterface = Subsystem->GetFriendsInterface();
	}
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

void UMultiplayerSessionsSubsystem::FindFriendSession(int32 LocalPlayerNum, const FUniqueNetId& FriendUniqueNetId)
{
}

void UMultiplayerSessionsSubsystem::SendSessionInviteToFriend(int32 LocalPlayerNum, const FUniqueNetId& FriendUniqueNetId)
{
}

void UMultiplayerSessionsSubsystem::FriendsList(int32 LocalPlayerNum)
{

}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnFindFriendSessionComplete(int32 LocalPlayerNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SessionResults)
{
}

void UMultiplayerSessionsSubsystem::OnSessionInviteReceived(const FUniqueNetId& LocalUserId, const FUniqueNetId& PersonInviting, const FString& AppId, const FOnlineSessionSearchResult& SearchResult)
{
}

void UMultiplayerSessionsSubsystem::OnSessionUserInviteAccepted(bool bWasSuccessful, int32 LocalPlayerNum, FUniqueNetIdPtr PersonInvited, const FOnlineSessionSearchResult& SearchResult)
{
}

void UMultiplayerSessionsSubsystem::OnReadFriendsListComplete(int32 LocalPlayerNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
}
