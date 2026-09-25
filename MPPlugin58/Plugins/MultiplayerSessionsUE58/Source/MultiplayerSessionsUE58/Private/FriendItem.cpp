// Fill out your copyright notice in the Description page of Project Settings.


#include "FriendItem.h"


//#include "HttpModule.h"
//#include "Interfaces/IHttpResponse.h"
//#include "ImageUtils.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"

#include "MultiplayerSessionsSubsystem.h"

void UFriendItem::Setup(const FString& InDisplayName,
    UTexture2D* InAvatar,
    bool bInIsOnline,
    TSharedPtr<const FUniqueNetId> InUserId)
{
    UserId = InUserId;
    bIsOnline = bInIsOnline;

    // Name
    if (txt_name)
    {
        txt_name->SetText(FText::FromString(InDisplayName));
        // Optionally grey out offline friends
        txt_name->SetColorAndOpacity(
            bIsOnline ? FLinearColor::Green : FLinearColor::Gray);
    }
#if 0
    // Avatar — download the URL asynchronously
    //if (img_avatar && !InAvatarURL.IsEmpty())
    //{
    //    TWeakObjectPtr<UFriendItem> WeakThis(this);

    //    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
    //        FHttpModule::Get().CreateRequest();
    //    Request->SetURL(InAvatarURL);
    //    Request->SetVerb(TEXT("GET"));
    //    Request->OnProcessRequestComplete().BindLambda(
    //        [WeakThis](FHttpRequestPtr, FHttpResponsePtr Response, bool bWasSuccessful)
    //        {
    //            if (!WeakThis.IsValid() || !bWasSuccessful || !Response.IsValid())
    //                return;

    //            const TArray<uint8>& Data = Response->GetContent();
    //            UTexture2D* Texture = FImageUtils::ImportBufferAsTexture2D(Data);
    //            if (Texture && WeakThis->img_avatar)
    //            {
    //                WeakThis->img_avatar->SetBrushFromTexture(Texture, true);
    //            }
    //        });
    //    Request->ProcessRequest();
    //}
#endif

	if (InAvatar && img_avatar)
	{
		img_avatar->SetBrushFromTexture(InAvatar, true);
	}

    // Hook up the optional invite button
    if (btn_invite)
    {
        btn_invite->OnClicked.AddDynamic(this, &UFriendItem::OnInviteClicked);
    }
}

void UFriendItem::SetPlayingThisGame(bool bPlaying, const FString& PlayingGameName)
{
    bIsPlayingThisGame = bPlaying;
    if (txt_status)
    {
		txt_status->SetText(PlayingGameName.IsEmpty()
            ? FText::GetEmpty()
            : FText::FromString(PlayingGameName));
        txt_status->SetColorAndOpacity(bPlaying ? FLinearColor::Green : FLinearColor::Gray);
    }
}

void UFriendItem::InviteFriend()
{
    OnInviteClicked();
}

void UFriendItem::SetImgAvatar(UTexture2D* InAvatar)
{
    if (img_avatar && InAvatar)
    {
        img_avatar->SetBrushFromTexture(InAvatar, true);
    }
}

void UFriendItem::OnInviteClicked()
{
    if (!UserId.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UserId is invalid"));
        return;
    }
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("No GameInstance"));
        return;
    }

    UMultiplayerSessionsSubsystem* Subsystem =
        GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();

    if (!Subsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Subsystem not valid"));
        return;
    }

    Subsystem->SendInviteToFriend(0, *UserId);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
            FString::Printf(TEXT("Invite sent to %s"), *txt_name->GetText().ToString()));
    }
}
