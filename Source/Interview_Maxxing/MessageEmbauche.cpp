#include "MessageEmbauche.h"
#include "MailData.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "MessageEmbauche"

namespace
{
	const TCHAR* CheminOffresEmbauche = TEXT("/Game/Data/DT_Offres_Mails.DT_Offres_Mails");
	const TCHAR* CheminSonEmbauche = TEXT("/Game/Sounds/good_result_notification_Cue.good_result_notification_Cue");
	const TCHAR* CheminSonRefus = TEXT("/Game/Sounds/bad_result_notification_Cue.bad_result_notification_Cue");

	const FLinearColor CouleurTexteMessage(0.92f, 0.92f, 0.92f);
	const FLinearColor CouleurMessageReussite(0.15f, 0.9f, 0.25f);
	const FLinearColor CouleurMessageEchec(0.95f, 0.15f, 0.12f);
}

UMessageEmbauche* UMessageEmbauche::AfficherMessageEmbauche(UObject* WorldContextObject, bool bEmbauche, FName IdOffre)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return nullptr;
	}

	// Nom de l'entreprise à partir de l'offre à laquelle le joueur a postulé
	FText NomEntreprise = LOCTEXT("EntrepriseInconnue", "L'entreprise");
	if (const UDataTable* Offres = LoadObject<UDataTable>(nullptr, CheminOffresEmbauche))
	{
		if (const FMailDataRow* Offre = Offres->FindRow<FMailDataRow>(IdOffre, TEXT("MessageEmbauche"), false))
		{
			if (!Offre->NomEntreprise.IsEmpty())
			{
				NomEntreprise = Offre->NomEntreprise;
			}
		}
	}

	UMessageEmbauche* Message = CreateWidget<UMessageEmbauche>(PC, UMessageEmbauche::StaticClass());
	if (!Message)
	{
		return nullptr;
	}

	Message->Configurer(bEmbauche, NomEntreprise);
	Message->AddToViewport(50);

	if (USoundBase* Son = LoadObject<USoundBase>(nullptr, bEmbauche ? CheminSonEmbauche : CheminSonRefus))
	{
		UGameplayStatics::PlaySound2D(World, Son, 0.3f);
	}

	// Le joueur lit le message sans pouvoir se déplacer
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(Message->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
	PC->SetIgnoreMoveInput(true);
	PC->SetIgnoreLookInput(true);

	return Message;
}

void UMessageEmbauche::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);

	if (WidgetTree && !WidgetTree->RootWidget)
	{
		ConstruireInterface();
	}
}

FReply UMessageEmbauche::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Touche = InKeyEvent.GetKey();
	if (Touche == EKeys::Enter || Touche == EKeys::SpaceBar || Touche == EKeys::Escape)
	{
		OnContinuer();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UMessageEmbauche::ConstruireInterface()
{
	UOverlay* Racine = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Racine"));
	WidgetTree->RootWidget = Racine;

	// Fond assombri sur tout l'écran
	UBorder* Fond = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Fond->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.6f));
	if (UOverlaySlot* SlotFond = Racine->AddChildToOverlay(Fond))
	{
		SlotFond->SetHorizontalAlignment(HAlign_Fill);
		SlotFond->SetVerticalAlignment(VAlign_Fill);
	}

	// Panneau central
	USizeBox* TaillePanneau = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	TaillePanneau->SetWidthOverride(760.f);
	if (UOverlaySlot* SlotPanneau = Racine->AddChildToOverlay(TaillePanneau))
	{
		SlotPanneau->SetHorizontalAlignment(HAlign_Center);
		SlotPanneau->SetVerticalAlignment(VAlign_Center);
	}

	FSlateBrush BrushPanneau;
	BrushPanneau.DrawAs = ESlateBrushDrawType::RoundedBox;
	BrushPanneau.TintColor = FSlateColor(FLinearColor(0.025f, 0.025f, 0.03f, 0.96f));
	BrushPanneau.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
	BrushPanneau.OutlineSettings.CornerRadii = FVector4(14.f, 14.f, 14.f, 14.f);

	UBorder* Panneau = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Panneau->SetBrush(BrushPanneau);
	Panneau->SetPadding(FMargin(44.f, 36.f));
	TaillePanneau->AddChild(Panneau);

	UVerticalBox* Colonne = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Panneau->SetContent(Colonne);

	UTextBlock* Entete = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Entete->SetText(LOCTEXT("Entete", "Nouveau message"));
	Entete->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 16));
	Entete->SetColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)));
	if (UVerticalBoxSlot* SlotEntete = Colonne->AddChildToVerticalBox(Entete))
	{
		SlotEntete->SetHorizontalAlignment(HAlign_Center);
		SlotEntete->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
	}

	TexteTitre = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TexteTitre->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 34));
	TexteTitre->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* SlotTitre = Colonne->AddChildToVerticalBox(TexteTitre))
	{
		SlotTitre->SetHorizontalAlignment(HAlign_Center);
		SlotTitre->SetPadding(FMargin(0.f, 0.f, 0.f, 20.f));
	}

	TexteMessage = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TexteMessage->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 20));
	TexteMessage->SetColorAndOpacity(FSlateColor(CouleurTexteMessage));
	TexteMessage->SetJustification(ETextJustify::Center);
	TexteMessage->SetAutoWrapText(true);
	if (UVerticalBoxSlot* SlotMessage = Colonne->AddChildToVerticalBox(TexteMessage))
	{
		SlotMessage->SetPadding(FMargin(0.f, 0.f, 0.f, 28.f));
	}

	BoutonContinuer = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	UTextBlock* TexteBouton = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TexteBouton->SetText(LOCTEXT("Continuer", "Continuer"));
	TexteBouton->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 18));
	TexteBouton->SetColorAndOpacity(FSlateColor(FLinearColor(0.05f, 0.05f, 0.05f)));
	if (UButtonSlot* SlotTexteBouton = Cast<UButtonSlot>(BoutonContinuer->AddChild(TexteBouton)))
	{
		SlotTexteBouton->SetPadding(FMargin(32.f, 8.f));
	}
	BoutonContinuer->OnClicked.AddDynamic(this, &UMessageEmbauche::OnContinuer);
	if (UVerticalBoxSlot* SlotBouton = Colonne->AddChildToVerticalBox(BoutonContinuer))
	{
		SlotBouton->SetHorizontalAlignment(HAlign_Center);
	}
}

void UMessageEmbauche::Configurer(bool bEmbauche, const FText& NomEntreprise)
{
	if (!TexteTitre || !TexteMessage)
	{
		return;
	}

	if (bEmbauche)
	{
		TexteTitre->SetText(LOCTEXT("TitreEmbauche", "Vous êtes embauché !"));
		TexteTitre->SetColorAndOpacity(FSlateColor(CouleurMessageReussite));
		TexteMessage->SetText(FText::Format(
			LOCTEXT("MessageEmbauche", "{0} a étudié votre test technique et souhaite vous accueillir dans son équipe.\nFélicitations, le poste est à vous !"),
			NomEntreprise));
	}
	else
	{
		TexteTitre->SetText(LOCTEXT("TitreRefus", "Candidature non retenue"));
		TexteTitre->SetColorAndOpacity(FSlateColor(CouleurMessageEchec));
		TexteMessage->SetText(FText::Format(
			LOCTEXT("MessageRefus", "{0} vous remercie pour votre temps, mais votre test technique n'a pas été suffisant.\nRetentez votre chance avec une autre offre !"),
			NomEntreprise));
	}
}

void UMessageEmbauche::OnContinuer()
{
	Fermer();
}

void UMessageEmbauche::Fermer()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
		PC->ResetIgnoreMoveInput();
		PC->ResetIgnoreLookInput();
	}
	RemoveFromParent();
}

#undef LOCTEXT_NAMESPACE
