#include "MenuParametres.h"
#include "ProgressionPartie.h"
#include "StyleInterface.h"
#include "InterviewGameUserSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/ComboBoxString.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "MenuParametres"

namespace
{
	// Même ordre que les options de ComboMode
	const EWindowMode::Type ModesAffichage[] = { EWindowMode::Fullscreen, EWindowMode::WindowedFullscreen, EWindowMode::Windowed };

}

void UMenuParametres::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);

	// Un Blueprint enfant avec son propre design garde son arbre de widgets
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		ConstruireInterface();
	}
}

void UMenuParametres::NativeConstruct()
{
	Super::NativeConstruct();
	ChargerValeurs();
}

FReply UMenuParametres::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::P)
	{
		OnRetour();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UMenuParametres::SetProposerMenuPrincipal(bool bProposer)
{
	if (BoutonMenuPrincipal)
	{
		BoutonMenuPrincipal->SetVisibility(bProposer ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UMenuParametres::OnMenuPrincipal()
{
	UProgressionPartie* Progression = UProgressionPartie::Get(this);
	Fermer();
	if (Progression)
	{
		Progression->RetourMenuPrincipal();
	}
}

void UMenuParametres::Fermer()
{
	OnFerme.Broadcast();
	RemoveFromParent();
}

// ---------------------------------------------------------------------------
// Construction de l'interface
// ---------------------------------------------------------------------------

UTextBlock* UMenuParametres::CreerTexte(const FText& Texte, int32 Taille, bool bGras)
{
	UTextBlock* Bloc = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Bloc->SetText(Texte);
	Bloc->SetFont(FCoreStyle::GetDefaultFontStyle(bGras ? "Bold" : "Regular", Taille));
	Bloc->SetColorAndOpacity(FSlateColor(StyleInterface::Texte()));
	return Bloc;
}

UButton* UMenuParametres::CreerBouton(const FText& Libelle)
{
	UButton* Bouton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	UTextBlock* Texte = CreerTexte(Libelle, 18, true);
	Texte->SetColorAndOpacity(FSlateColor(FLinearColor(0.05f, 0.05f, 0.05f)));
	if (UButtonSlot* SlotBouton = Cast<UButtonSlot>(Bouton->AddChild(Texte)))
	{
		SlotBouton->SetPadding(FMargin(28.f, 8.f));
	}
	StyleInterface::AppliquerStyleBouton(Bouton, Texte);
	return Bouton;
}

void UMenuParametres::AjouterLigne(UVerticalBox* Colonne, const FText& Libelle, UWidget* Controle)
{
	UHorizontalBox* Ligne = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

	if (UHorizontalBoxSlot* SlotLibelle = Ligne->AddChildToHorizontalBox(CreerTexte(Libelle, 20)))
	{
		SlotLibelle->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		SlotLibelle->SetVerticalAlignment(VAlign_Center);
	}

	USizeBox* ZoneControle = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	ZoneControle->SetWidthOverride(340.f);
	ZoneControle->AddChild(Controle);
	if (UHorizontalBoxSlot* SlotControle = Ligne->AddChildToHorizontalBox(ZoneControle))
	{
		SlotControle->SetVerticalAlignment(VAlign_Center);
	}

	if (UVerticalBoxSlot* SlotLigne = Colonne->AddChildToVerticalBox(Ligne))
	{
		SlotLigne->SetPadding(FMargin(0.f, 10.f));
	}
}

void UMenuParametres::ConstruireInterface()
{
	UOverlay* Racine = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Racine"));
	WidgetTree->RootWidget = Racine;

	// Fond assombri sur tout l'écran
	UBorder* Fond = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Fond->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.65f));
	if (UOverlaySlot* SlotFond = Racine->AddChildToOverlay(Fond))
	{
		SlotFond->SetHorizontalAlignment(HAlign_Fill);
		SlotFond->SetVerticalAlignment(VAlign_Fill);
	}

	// Panneau central
	USizeBox* TaillePanneau = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	TaillePanneau->SetWidthOverride(720.f);
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
	Panneau->SetPadding(FMargin(40.f, 32.f));
	StyleInterface::AppliquerCadre(Panneau, FMargin(28.f, 24.f, 28.f, 20.f), 0.65f);
	TaillePanneau->AddChild(Panneau);

	UVerticalBox* Colonne = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Panneau->SetContent(Colonne);

	if (UVerticalBoxSlot* SlotTitre = Colonne->AddChildToVerticalBox(CreerTexte(LOCTEXT("Titre", "Paramètres"), 34, true)))
	{
		SlotTitre->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));
	}

	// Mode d'affichage
	ComboMode = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass());
	ComboMode->AddOption(LOCTEXT("PleinEcran", "Plein écran").ToString());
	ComboMode->AddOption(LOCTEXT("SansBordure", "Fenêtré sans bordure").ToString());
	ComboMode->AddOption(LOCTEXT("Fenetre", "Fenêtré").ToString());
	ComboMode->OnSelectionChanged.AddDynamic(this, &UMenuParametres::OnModeChange);
	AjouterLigne(Colonne, LOCTEXT("ModeAffichage", "Mode d'affichage"), ComboMode);

	// Résolution
	ComboResolution = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass());
	AjouterLigne(Colonne, LOCTEXT("Resolution", "Résolution"), ComboResolution);

	// Sensibilité : slider + valeur
	UHorizontalBox* LigneSensibilite = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	SliderSensibilite = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass());
	SliderSensibilite->SetMinValue(UInterviewGameUserSettings::SensibiliteMin);
	SliderSensibilite->SetMaxValue(UInterviewGameUserSettings::SensibiliteMax);
	SliderSensibilite->SetStepSize(0.05f);
	SliderSensibilite->OnValueChanged.AddDynamic(this, &UMenuParametres::OnSensibiliteChange);
	if (UHorizontalBoxSlot* SlotSlider = LigneSensibilite->AddChildToHorizontalBox(SliderSensibilite))
	{
		SlotSlider->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		SlotSlider->SetVerticalAlignment(VAlign_Center);
	}
	TexteSensibilite = CreerTexte(FText::GetEmpty(), 18);
	TexteSensibilite->SetMinDesiredWidth(56.f);
	TexteSensibilite->SetJustification(ETextJustify::Right);
	if (UHorizontalBoxSlot* SlotValeur = LigneSensibilite->AddChildToHorizontalBox(TexteSensibilite))
	{
		SlotValeur->SetPadding(FMargin(12.f, 0.f, 0.f, 0.f));
		SlotValeur->SetVerticalAlignment(VAlign_Center);
	}
	AjouterLigne(Colonne, LOCTEXT("Sensibilite", "Sensibilité de la souris"), LigneSensibilite);

	// Aide
	TexteAide = CreerTexte(FText::GetEmpty(), 14);
	TexteAide->SetColorAndOpacity(FSlateColor(StyleInterface::TexteSecondaire()));
	TexteAide->SetAutoWrapText(true);
	if (UVerticalBoxSlot* SlotAide = Colonne->AddChildToVerticalBox(TexteAide))
	{
		SlotAide->SetPadding(FMargin(0.f, 6.f, 0.f, 20.f));
	}

	// Boutons
	UHorizontalBox* LigneBoutons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

	// Retour à l'écran titre, seulement en cours de partie
	BoutonMenuPrincipal = CreerBouton(LOCTEXT("MenuPrincipal", "Menu principal"));
	if (!StyleInterface::BoutonsPersonnalises())
	{
		BoutonMenuPrincipal->SetBackgroundColor(FLinearColor(0.75f, 0.3f, 0.25f, 1.f));
	}
	BoutonMenuPrincipal->OnClicked.AddDynamic(this, &UMenuParametres::OnMenuPrincipal);
	BoutonMenuPrincipal->SetVisibility(ESlateVisibility::Collapsed);
	if (UHorizontalBoxSlot* SlotMenu = LigneBoutons->AddChildToHorizontalBox(BoutonMenuPrincipal))
	{
		SlotMenu->SetPadding(FMargin(0.f, 0.f, 36.f, 0.f));
	}

	UButton* BoutonRetour = CreerBouton(LOCTEXT("Retour", "Retour"));
	BoutonRetour->OnClicked.AddDynamic(this, &UMenuParametres::OnRetour);
	UButton* BoutonAppliquer = CreerBouton(LOCTEXT("Appliquer", "Appliquer"));
	BoutonAppliquer->OnClicked.AddDynamic(this, &UMenuParametres::OnAppliquer);
	LigneBoutons->AddChildToHorizontalBox(BoutonRetour);
	if (UHorizontalBoxSlot* SlotAppliquer = LigneBoutons->AddChildToHorizontalBox(BoutonAppliquer))
	{
		SlotAppliquer->SetPadding(FMargin(12.f, 0.f, 0.f, 0.f));
	}
	if (UVerticalBoxSlot* SlotBoutons = Colonne->AddChildToVerticalBox(LigneBoutons))
	{
		SlotBoutons->SetHorizontalAlignment(HAlign_Right);
	}
}

// ---------------------------------------------------------------------------
// Valeurs
// ---------------------------------------------------------------------------

void UMenuParametres::ChargerValeurs()
{
	UInterviewGameUserSettings* Reglages = UInterviewGameUserSettings::GetInterviewGameUserSettings();
	if (!Reglages || !ComboMode)
	{
		return;
	}

	RemplirResolutions();

	const int32 NbModes = UE_ARRAY_COUNT(ModesAffichage);
	for (int32 Index = 0; Index < NbModes; ++Index)
	{
		if (ModesAffichage[Index] == Reglages->GetFullscreenMode())
		{
			ComboMode->SetSelectedIndex(Index);
		}
	}

	SliderSensibilite->SetValue(Reglages->GetSensibiliteSouris());
	MettreAJourTexteSensibilite();
}

void UMenuParametres::RemplirResolutions()
{
	UInterviewGameUserSettings* Reglages = UInterviewGameUserSettings::GetInterviewGameUserSettings();

	Resolutions.Reset();
	TArray<FIntPoint> Liste;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Liste);
	Resolutions.Append(Liste);
	Liste.Reset();
	UKismetSystemLibrary::GetConvenientWindowedResolutions(Liste);
	Resolutions.Append(Liste);
	Resolutions.Add(Reglages->GetDesktopResolution());
	Resolutions.Add(Reglages->GetScreenResolution());

	// Sans doublon, de la plus grande à la plus petite
	TSet<FIntPoint> Uniques(Resolutions);
	Resolutions = Uniques.Array();
	Resolutions.RemoveAll([](const FIntPoint& Res) { return Res.X <= 0 || Res.Y <= 0; });
	Resolutions.Sort([](const FIntPoint& A, const FIntPoint& B) { return A.X != B.X ? A.X > B.X : A.Y > B.Y; });

	ComboResolution->ClearOptions();
	const FIntPoint Actuelle = Reglages->GetScreenResolution();
	for (int32 Index = 0; Index < Resolutions.Num(); ++Index)
	{
		ComboResolution->AddOption(FString::Printf(TEXT("%d x %d"), Resolutions[Index].X, Resolutions[Index].Y));
		if (Resolutions[Index] == Actuelle)
		{
			ComboResolution->SetSelectedIndex(Index);
		}
	}
}

EWindowMode::Type UMenuParametres::GetModeSelectionne() const
{
	const int32 Index = ComboMode ? ComboMode->GetSelectedIndex() : INDEX_NONE;
	return (Index >= 0 && Index < UE_ARRAY_COUNT(ModesAffichage)) ? ModesAffichage[Index] : EWindowMode::Fullscreen;
}

void UMenuParametres::MettreAJourTexteSensibilite()
{
	TexteSensibilite->SetText(FText::AsNumber(SliderSensibilite->GetValue(), &FNumberFormattingOptions().SetMinimumFractionalDigits(2).SetMaximumFractionalDigits(2)));
}

// ---------------------------------------------------------------------------
// Événements
// ---------------------------------------------------------------------------

void UMenuParametres::OnModeChange(FString Option, ESelectInfo::Type TypeSelection)
{
	// En sans bordure la fenêtre prend toujours la résolution du bureau
	const bool bSansBordure = GetModeSelectionne() == EWindowMode::WindowedFullscreen;
	ComboResolution->SetIsEnabled(!bSansBordure);

	switch (GetModeSelectionne())
	{
	case EWindowMode::WindowedFullscreen:
		TexteAide->SetText(LOCTEXT("AideSansBordure", "Le jeu occupe tout l'écran à sa résolution native, idéal pour les écrans ultra-larges."));
		break;
	case EWindowMode::Windowed:
		TexteAide->SetText(LOCTEXT("AideFenetre", "La fenêtre peut ensuite être déplacée et redimensionnée librement."));
		break;
	default:
		TexteAide->SetText(LOCTEXT("AidePleinEcran", "Plein écran exclusif à la résolution choisie."));
		break;
	}
}

void UMenuParametres::OnSensibiliteChange(float Valeur)
{
	// Appliquée tout de suite pour pouvoir la tester en sortant du menu
	if (UInterviewGameUserSettings* Reglages = UInterviewGameUserSettings::GetInterviewGameUserSettings())
	{
		Reglages->SetSensibiliteSouris(Valeur);
	}
	MettreAJourTexteSensibilite();
}

void UMenuParametres::OnAppliquer()
{
	UInterviewGameUserSettings* Reglages = UInterviewGameUserSettings::GetInterviewGameUserSettings();
	if (!Reglages)
	{
		return;
	}

	const EWindowMode::Type Mode = GetModeSelectionne();
	Reglages->SetFullscreenMode(Mode);

	if (Mode == EWindowMode::WindowedFullscreen)
	{
		Reglages->SetScreenResolution(Reglages->GetDesktopResolution());
	}
	else if (Resolutions.IsValidIndex(ComboResolution->GetSelectedIndex()))
	{
		Reglages->SetScreenResolution(Resolutions[ComboResolution->GetSelectedIndex()]);
	}

	Reglages->SetSensibiliteSouris(SliderSensibilite->GetValue());

	// Applique et sauvegarde dans GameUserSettings.ini
	Reglages->ApplySettings(false);

	// La liste dépend de la résolution courante
	RemplirResolutions();
}

void UMenuParametres::OnRetour()
{
	if (UInterviewGameUserSettings* Reglages = UInterviewGameUserSettings::GetInterviewGameUserSettings())
	{
		Reglages->SaveSettings();
	}
	Fermer();
}

#undef LOCTEXT_NAMESPACE
