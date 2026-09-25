// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"


//#include "MultiplayerSessionsSubsystem.h"

//#include "OnlineSubsystem.h"
//#include "Interfaces/OnlineSessionInterface.h"
//#include "Interfaces/OnlineFriendsInterface.h"

#include "FriendItem.generated.h"

class UTextBlock;
class UImage;
class UButton;

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONSUE58_API UFriendItem : public UUserWidget
{
	GENERATED_BODY()

public:
    // Called right after creating the widget to fill it with data
    void Setup(const FString& InDisplayName,
        UTexture2D* InAvatar,
        bool bInIsOnline,
        TSharedPtr<const FUniqueNetId> InUserId);

    UFUNCTION(BlueprintCallable, Category = "Friends")
    void SetPlayingThisGame(bool bPlaying, const FString& PlayingGameName);

    UFUNCTION(BlueprintCallable, Category = "Friends")
    void InviteFriend();

    // The unique net id, kept so we can invite this friend later
    TSharedPtr<const FUniqueNetId> GetUserId() const { return UserId; }

	void SetImgAvatar(UTexture2D* InAvatar);

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* txt_name;

    UPROPERTY(meta = (BindWidget))
    UImage* img_avatar;

    // Optional: a button on the row to invite this friend
    UPROPERTY(meta = (BindWidgetOptional))
    UButton* btn_invite;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* txt_status;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* txt_alias;

    UPROPERTY(BlueprintReadOnly, Category = "Friends")
    bool bIsOnline = false;

    UPROPERTY(BlueprintReadOnly, Category = "Friends")
    bool bIsPlayingThisGame = false;

    UPROPERTY()
    UTexture2D* AvatarTexture = nullptr;

private:
    TSharedPtr<const FUniqueNetId> UserId;

    UFUNCTION()
    void OnInviteClicked();

    //IOnlineFriendsPtr FriendsInterface;

};
