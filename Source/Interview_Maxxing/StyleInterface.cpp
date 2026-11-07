#include "StyleInterface.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "Styling/CoreStyle.h"

namespace StyleInterface
{
	namespace
	{
		// Fenêtre T_Cadre : mesures en pixels sur l'image source 1640x1200, depuis les bords de l'image
		constexpr float LargeurSource = 1640.f, HauteurSource = 1200.f;
		// Découpe 9-slice : la barre de titre (icône à gauche, boutons à droite) ne s'étire qu'en largeur au centre
		constexpr float TrancheGauche = 70.f, TrancheHaut = 88.f, TrancheDroite = 136.f, TrancheBas = 46.f;
		// Zone sombre à l'intérieur de la fenêtre
		constexpr float InterieurGauche = 46.f, InterieurHaut = 86.f, InterieurDroite = 44.f, InterieurBas = 41.f;

		UTexture2D* ImageCadre()
		{
			return LoadObject<UTexture2D>(nullptr, TEXT("/Game/Ui/Menus/T_Cadre.T_Cadre"), nullptr, LOAD_NoWarn | LOAD_Quiet);
		}
	}

	bool CadrePersonnalise()
	{
		static TWeakObjectPtr<UTexture2D> Cache;
		static bool bCherche = false;
		if (!Cache.IsValid() && !bCherche)
		{
			Cache = ImageCadre();
			bCherche = true;
		}
		return Cache.IsValid();
	}

	void AppliquerCadre(UBorder* Panneau, const FMargin& Marge, float Echelle)
	{
		UTexture2D* Image = Panneau ? ImageCadre() : nullptr;
		if (!Image)
		{
			return;
		}
		// Slate dessine les bords d'un 9-slice à la taille réelle de la texture : Echelle n'a pas d'effet sur eux.
		// Pour un petit panneau, on réduit tout le panneau avec un RenderScale (voir l'objectif et la carte).
		(void)Echelle;
		const float Facteur = 1.f;

		FSlateBrush Brush;
		Brush.SetResourceObject(Image);
		Brush.DrawAs = ESlateBrushDrawType::Box;
		Brush.ImageSize = FVector2D(Image->GetSizeX(), Image->GetSizeY());
		Brush.Margin = FMargin(TrancheGauche / LargeurSource, TrancheHaut / HauteurSource, TrancheDroite / LargeurSource, TrancheBas / HauteurSource);

		Panneau->SetBrush(Brush);
		Panneau->SetBrushColor(FLinearColor::White);
		Panneau->SetPadding(FMargin(
			InterieurGauche * Facteur + Marge.Left,
			InterieurHaut * Facteur + Marge.Top,
			InterieurDroite * Facteur + Marge.Right,
			InterieurBas * Facteur + Marge.Bottom));
	}

	// Textes clairs : l'intérieur des panneaux (et de la fenêtre T_Cadre) est sombre
	FLinearColor Texte() { return FLinearColor(0.92f, 0.92f, 0.92f); }
	FLinearColor TexteSecondaire() { return FLinearColor(0.6f, 0.6f, 0.62f); }
	FLinearColor Accent() { return FLinearColor(0.25f, 0.62f, 1.f); }
	FLinearColor Reussite() { return FLinearColor(0.15f, 0.9f, 0.25f); }
	FLinearColor Echec() { return FLinearColor(0.95f, 0.2f, 0.15f); }
	FLinearColor FondEncadre() { return CadrePersonnalise() ? FLinearColor(0.02f, 0.06f, 0.16f, 1.f) : FLinearColor(0.08f, 0.1f, 0.14f, 1.f); }
	FLinearColor FondPanneau() { return FLinearColor(0.025f, 0.025f, 0.03f, 0.96f); }

	UTextBlock* CreerTexte(UWidgetTree* Arbre, const FText& Message, int32 Taille, bool bGras, const FLinearColor& Couleur)
	{
		UTextBlock* Bloc = Arbre->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Bloc->SetText(Message);
		Bloc->SetFont(FCoreStyle::GetDefaultFontStyle(bGras ? "Bold" : "Regular", Taille));
		Bloc->SetColorAndOpacity(FSlateColor(Couleur));
		return Bloc;
	}

	namespace
	{
		UTexture2D* ImageBouton(const TCHAR* Nom)
		{
			return LoadObject<UTexture2D>(nullptr, *FString::Printf(TEXT("/Game/Ui/Menus/%s.%s"), Nom, Nom), nullptr, LOAD_NoWarn | LOAD_Quiet);
		}

		FSlateBrush BrushBouton(UTexture2D* Image)
		{
			FSlateBrush Brush;
			Brush.SetResourceObject(Image);
			Brush.ImageSize = FVector2D(Image->GetSizeX(), Image->GetSizeY());
			// 9-slice : bords et coins préservés, seul le centre s'étire quand le bouton change de taille
			Brush.DrawAs = ESlateBrushDrawType::Box;
			Brush.Margin = FMargin(12.f / Image->GetSizeX(), 10.f / Image->GetSizeY());
			return Brush;
		}
	}

	bool BoutonsPersonnalises()
	{
		return ImageBouton(TEXT("T_Bouton_Normal")) != nullptr;
	}

	void AppliquerStyleBouton(UButton* Bouton, UTextBlock* Libelle)
	{
		UTexture2D* Normal = ImageBouton(TEXT("T_Bouton_Normal"));
		if (!Bouton || !Normal)
		{
			return;
		}
		UTexture2D* Survol = ImageBouton(TEXT("T_Bouton_Survol"));
		UTexture2D* Appui = ImageBouton(TEXT("T_Bouton_Appui"));

		FButtonStyle Style = Bouton->GetStyle();
		Style.SetNormal(BrushBouton(Normal));
		Style.SetHovered(BrushBouton(Survol ? Survol : Normal));
		Style.SetPressed(BrushBouton(Appui ? Appui : Normal));
		Style.SetNormalPadding(FMargin(0.f));
		Style.SetPressedPadding(FMargin(0.f, 2.f, 0.f, 0.f));
		Bouton->SetStyle(Style);
		Bouton->SetBackgroundColor(FLinearColor::White);

		if (Libelle)
		{
			Libelle->SetColorAndOpacity(FSlateColor(FLinearColor(0.97f, 0.97f, 1.f)));
			Libelle->SetShadowOffset(FVector2D(1.f, 1.f));
			Libelle->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.1f, 0.6f));
		}
	}

	UButton* CreerBouton(UWidgetTree* Arbre, const FText& Libelle, bool bPrincipal, int32 TailleTexte)
	{
		UButton* Bouton = Arbre->ConstructWidget<UButton>(UButton::StaticClass());
		if (!bPrincipal)
		{
			Bouton->SetBackgroundColor(FLinearColor(0.18f, 0.18f, 0.2f, 1.f));
		}
		UTextBlock* TexteBouton = CreerTexte(Arbre, Libelle, TailleTexte, true,
			bPrincipal ? FLinearColor(0.05f, 0.05f, 0.05f) : Texte());
		if (UButtonSlot* SlotTexte = Cast<UButtonSlot>(Bouton->AddChild(TexteBouton)))
		{
			SlotTexte->SetPadding(FMargin(28.f, 8.f));
		}
		AppliquerStyleBouton(Bouton, TexteBouton);
		return Bouton;
	}

	UBorder* CreerPanneau(UWidgetTree* Arbre, const FMargin& Marge, const FLinearColor& Couleur, float Rayon)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
		Brush.TintColor = FSlateColor(Couleur);
		Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
		Brush.OutlineSettings.CornerRadii = FVector4(Rayon, Rayon, Rayon, Rayon);

		UBorder* Panneau = Arbre->ConstructWidget<UBorder>(UBorder::StaticClass());
		Panneau->SetBrush(Brush);
		Panneau->SetPadding(Marge);
		return Panneau;
	}

	void PasserEnModeInterface(APlayerController* PC, UUserWidget* Focus)
	{
		if (!PC)
		{
			return;
		}
		FInputModeUIOnly InputMode;
		if (Focus)
		{
			InputMode.SetWidgetToFocus(Focus->TakeWidget());
		}
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}

	void RevenirEnModeJeu(APlayerController* PC)
	{
		if (!PC)
		{
			return;
		}
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
		PC->ResetIgnoreMoveInput();
		PC->ResetIgnoreLookInput();
	}
}
