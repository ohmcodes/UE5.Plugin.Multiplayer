// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "GameMenu.generated.h"

class UButton;
class UScrollBox;
class UEditableTextBox;
class UFriendItem;
class UMultiplayerSessionsSubsystem;
class USizeBox;

USTRUCT()
struct FFriendData
{
	GENERATED_BODY()

	UPROPERTY() FString DisplayName;
	UPROPERTY() UTexture2D* AvatarTexture;
	UPROPERTY() bool    bIsOnline = false;
	UPROPERTY() bool    bIsPlayingThisGame = false;
	UPROPERTY() FString PlayingGameName;

	// Non-reflected — TSharedPtr can't be a UPROPERTY
	TSharedPtr<const FUniqueNetId> UserId;
};

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONSUE58_API UGameMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void GameMenuSetup();

	UFUNCTION(BlueprintCallable, Category = "Friends")
	void RebuildFriendList();

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	class UButton* btn_invfriends;
	UPROPERTY(meta = (BindWidget))
	class UButton* btn_invfriendssteam;
	UPROPERTY(meta = (BindWidget))
	class UButton* btn_friendback;
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* sb_friendslist;
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* txt_search;
	UPROPERTY(meta = (BindWidget))
	class USizeBox* szb_pausemenu;
	UPROPERTY(meta = (BindWidget))
	class USizeBox* szb_friendcontainer;

	UPROPERTY(EditDefaultsOnly, Category = "Friends")
	TSubclassOf<UFriendItem> FriendItem;

	UFUNCTION()
	void OnInviteFriendsClicked();
	UFUNCTION()
	void OnInviteFriendsSteamClicked();
	UFUNCTION()
	void OnFriendBackClicked();

	UFUNCTION()
	void OnSearchTextChanged(const FText& Text);
	UFUNCTION()
	void OnTextEnter(const FText& Text, ETextCommit::Type CommitMethod);

	void OnReadFriendsList(int32 LocalPlayerNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

private:
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	UPROPERTY() 
	TArray<FFriendData> CachedFriends;

	FString CurrentFilter;

	uint32 GetCurrentAppId() const;
	
};



