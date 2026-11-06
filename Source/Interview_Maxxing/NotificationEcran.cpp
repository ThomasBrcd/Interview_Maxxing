#include "NotificationEcran.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Styling/CoreStyle.h"
#include "TimerManager.h"

TWeakObjectPtr<UNotificationEcran> UNotificationEcran::NotificationActuelle;

void UNotificationEcran::AfficherNotification(UObject* WorldContextObject, FText Message, float Duree)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	if (UNotificationEcran* Precedente = NotificationActuelle.Get())
	{
		Precedente->Fermer();
	}

	UNotificationEcran* Notification = CreateWidget<UNotificationEcran>(PC, UNotificationEcran::StaticClass());
	if (!Notification)
	{
		return;
	}

	if (Notification->Texte)
	{
		Notification->Texte->SetText(Message);
	}
	Notification->AddToViewport(60);
	NotificationActuelle = Notification;

	World->GetTimerManager().SetTimer(Notification->TimerFermeture, FTimerDelegate::CreateUObject(Notification, &UNotificationEcran::Fermer), FMath::Max(Duree, 0.5f), false);
}

void UNotificationEcran::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && !WidgetTree->RootWidget)
	{
		ConstruireInterface();
	}
}

void UNotificationEcran::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerFermeture);
	}
	Super::NativeDestruct();
}

void UNotificationEcran::ConstruireInterface()
{
	UOverlay* Racine = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Racine"));
	WidgetTree->RootWidget = Racine;
	Racine->SetVisibility(ESlateVisibility::HitTestInvisible);

	FSlateBrush BrushBandeau;
	BrushBandeau.DrawAs = ESlateBrushDrawType::RoundedBox;
	BrushBandeau.TintColor = FSlateColor(FLinearColor(0.025f, 0.025f, 0.03f, 0.9f));
	BrushBandeau.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
	BrushBandeau.OutlineSettings.CornerRadii = FVector4(10.f, 10.f, 10.f, 10.f);

	UBorder* Bandeau = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Bandeau->SetBrush(BrushBandeau);
	Bandeau->SetPadding(FMargin(28.f, 14.f));
	if (UOverlaySlot* SlotBandeau = Racine->AddChildToOverlay(Bandeau))
	{
		SlotBandeau->SetHorizontalAlignment(HAlign_Center);
		SlotBandeau->SetVerticalAlignment(VAlign_Top);
		SlotBandeau->SetPadding(FMargin(0.f, 90.f, 0.f, 0.f));
	}

	Texte = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Texte->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 22));
	Texte->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.95f, 0.95f)));
	Texte->SetJustification(ETextJustify::Center);
	Bandeau->SetContent(Texte);
}

void UNotificationEcran::Fermer()
{
	if (NotificationActuelle.Get() == this)
	{
		NotificationActuelle.Reset();
	}
	RemoveFromParent();
}
