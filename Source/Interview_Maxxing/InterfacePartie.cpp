#include "InterfacePartie.h"
#include "MenuParametres.h"
#include "StyleInterface.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Engine/Texture2D.h"
#include "Components/ButtonSlot.h"
#include "Components/ScaleBox.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

#define LOCTEXT_NAMESPACE "InterfacePartie"

using namespace StyleInterface;

namespace
{
	/** Image de fond de l'écran titre (décor, logo et cadre des boutons), facultative */
	const TCHAR* CheminFondTitre = TEXT("/Game/Ui/Menus/T_Titre_Fond.T_Titre_Fond");

	/** Racine plein écran avec un fond assombri, renvoie l'overlay racine */
	UOverlay* CreerRacineAssombrie(UWidgetTree* Arbre, float Opacite)
	{
		UOverlay* Racine = Arbre->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Racine"));
		Arbre->RootWidget = Racine;

		UBorder* Fond = Arbre->ConstructWidget<UBorder>(UBorder::StaticClass());
		Fond->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, Opacite));
		if (UOverlaySlot* SlotFond = Racine->AddChildToOverlay(Fond))
		{
			SlotFond->SetHorizontalAlignment(HAlign_Fill);
			SlotFond->SetVerticalAlignment(VAlign_Fill);
		}
		return Racine;
	}

	/** Panneau centré de largeur fixe, renvoie la colonne de contenu */
	UVerticalBox* CreerPanneauCentral(UWidgetTree* Arbre, UOverlay* Racine, float Largeur, const FMargin& Marge)
	{
		USizeBox* Taille = Arbre->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Taille->SetWidthOverride(Largeur);
		if (UOverlaySlot* SlotTaille = Racine->AddChildToOverlay(Taille))
		{
			SlotTaille->SetHorizontalAlignment(HAlign_Center);
			SlotTaille->SetVerticalAlignment(VAlign_Center);
		}
		UBorder* Panneau = CreerPanneau(Arbre, Marge);
		AppliquerCadre(Panneau, FMargin(Marge.Left * 0.6f, Marge.Top * 0.8f, Marge.Right * 0.6f, Marge.Bottom * 0.6f), 0.65f);
		Taille->AddChild(Panneau);
		UVerticalBox* Colonne = Arbre->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		Panneau->SetContent(Colonne);
		return Colonne;
	}

	UVerticalBoxSlot* Ajouter(UVerticalBox* Colonne, UWidget* Widget, const FMargin& Marge = FMargin(0.f), EHorizontalAlignment Alignement = HAlign_Fill)
	{
		UVerticalBoxSlot* SlotAjoute = Colonne->AddChildToVerticalBox(Widget);
		if (SlotAjoute)
		{
			SlotAjoute->SetPadding(Marge);
			SlotAjoute->SetHorizontalAlignment(Alignement);
		}
		return SlotAjoute;
	}

	FText NomDomaine(const FString& Domaine)
	{
		if (Domaine.Equals(TEXT("jeu3d"), ESearchCase::IgnoreCase))
		{
			return LOCTEXT("DomaineJeu3D", "Jeu vidéo 3D");
		}
		if (Domaine.Equals(TEXT("reseaux"), ESearchCase::IgnoreCase))
		{
			return LOCTEXT("DomaineReseaux", "Réseaux");
		}
		return FText::FromString(Domaine);
	}

	FText TexteResultat(EResultatCandidature Resultat, FLinearColor& Couleur)
	{
		switch (Resultat)
		{
		case EResultatCandidature::CVRefuse:
			Couleur = Echec();
			return LOCTEXT("ResCV", "CV non retenu");
		case EResultatCandidature::EntretienRate:
			Couleur = Echec();
			return LOCTEXT("ResEntretien", "Entretien raté");
		case EResultatCandidature::TestRate:
			Couleur = Echec();
			return LOCTEXT("ResTest", "Test technique raté");
		case EResultatCandidature::Embauche:
			Couleur = Reussite();
			return LOCTEXT("ResEmbauche", "Embauché");
		default:
			Couleur = TexteSecondaire();
			return LOCTEXT("ResEnCours", "En cours");
		}
	}
}

// ===========================================================================
// Écran titre
// ===========================================================================

void UEcranTitre::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	if (UTexture2D* Fond = LoadObject<UTexture2D>(nullptr, CheminFondTitre))
	{
		ConstruireAvecImage(Fond);
	}
	else
	{
		ConstruireSansImage();
	}
}

void UEcranTitre::AjouterBoutons(UVerticalBox* Colonne, float Echelle, float Largeur, EHorizontalAlignment Alignement)
{
	struct FEntree { FText Libelle; bool bPrincipal; FName Fonction; };
	const FEntree Entrees[] = {
		{ LOCTEXT("Nouvelle", "Nouvelle partie"), true, GET_FUNCTION_NAME_CHECKED(UEcranTitre, OnNouvellePartie) },
		{ LOCTEXT("Parametres", "Paramètres"), false, GET_FUNCTION_NAME_CHECKED(UEcranTitre, OnParametres) },
		{ LOCTEXT("Quitter", "Quitter"), false, GET_FUNCTION_NAME_CHECKED(UEcranTitre, OnQuitter) },
	};
	for (const FEntree& Entree : Entrees)
	{
		UButton* Bouton = CreerBouton(WidgetTree, Entree.Libelle, Entree.bPrincipal, FMath::RoundToInt(20.f * Echelle));
		if (UButtonSlot* SlotTexte = Cast<UButtonSlot>(Bouton->GetContentSlot()))
		{
			SlotTexte->SetPadding(FMargin(28.f * Echelle, 10.f * Echelle));
		}
		FScriptDelegate Delegue;
		Delegue.BindUFunction(this, Entree.Fonction);
		Bouton->OnClicked.Add(Delegue);
		USizeBox* LargeurBouton = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		LargeurBouton->SetWidthOverride(Largeur);
		LargeurBouton->AddChild(Bouton);
		Ajouter(Colonne, LargeurBouton, FMargin(0.f, 0.f, 0.f, 12.f * Echelle), Alignement);
	}
}

void UEcranTitre::ConstruireAvecImage(UTexture2D* Fond)
{
	// L'image (3840x2160) contient déjà le décor, le logo et le cadre de gauche.
	// Tout est construit dans l'espace de l'image puis mis à l'échelle de l'écran :
	// les boutons restent dans le cadre quelle que soit la résolution.
	constexpr float LargeurImage = 3840.f;
	constexpr float HauteurImage = 2160.f;
	constexpr float Echelle = 2.f; // l'image est conçue pour du 4K, l'interface de base pour du 1080p

	// Zone libre à l'intérieur du cadre de gauche (en pixels de l'image), au-dessus des crédits
	const FMargin ZoneCadre(70.f, 150.f, LargeurImage - 1380.f, HauteurImage - 1950.f);

	// Fond noir puis image entière : sur un écran qui n'est pas en 16:9, bandes noires plutôt que logo coupé
	UBorder* FondNoir = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Racine"));
	FondNoir->SetBrushColor(FLinearColor::Black);
	FondNoir->SetPadding(FMargin(0.f));
	FondNoir->SetHorizontalAlignment(HAlign_Fill);
	FondNoir->SetVerticalAlignment(VAlign_Fill);
	WidgetTree->RootWidget = FondNoir;

	UScaleBox* Echelonnage = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass());
	Echelonnage->SetStretch(EStretch::ScaleToFit);
	FondNoir->SetContent(Echelonnage);

	USizeBox* TailleImage = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	TailleImage->SetWidthOverride(LargeurImage);
	TailleImage->SetHeightOverride(HauteurImage);
	Echelonnage->AddChild(TailleImage);

	UOverlay* Calques = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	TailleImage->AddChild(Calques);

	UImage* Image = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	Image->SetBrushFromTexture(Fond);
	if (UOverlaySlot* SlotImage = Calques->AddChildToOverlay(Image))
	{
		SlotImage->SetHorizontalAlignment(HAlign_Fill);
		SlotImage->SetVerticalAlignment(VAlign_Fill);
	}

	UVerticalBox* Colonne = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	if (UOverlaySlot* SlotColonne = Calques->AddChildToOverlay(Colonne))
	{
		SlotColonne->SetPadding(ZoneCadre);
		SlotColonne->SetHorizontalAlignment(HAlign_Fill);
		SlotColonne->SetVerticalAlignment(VAlign_Center);
	}

	// L'USP du jeu, au-dessus des boutons
	UTextBlock* Accroche = CreerTexte(WidgetTree,
		LOCTEXT("Accroche", "Construisez votre CV étiquette par étiquette, adaptez-le à chaque entreprise, passez l'entretien puis un vrai test technique du métier. Décrochez un poste avant d'épuiser toutes les offres."),
		FMath::RoundToInt(17.f * Echelle), false, FLinearColor(0.92f, 0.92f, 0.92f));
	Accroche->SetAutoWrapText(true);
	Accroche->SetJustification(ETextJustify::Center);
	Ajouter(Colonne, Accroche, FMargin(60.f * Echelle, 0.f, 60.f * Echelle, 48.f * Echelle), HAlign_Fill);

	AjouterBoutons(Colonne, Echelle, 340.f * Echelle, HAlign_Center);
}

void UEcranTitre::ConstruireSansImage()
{
	UOverlay* Racine = CreerRacineAssombrie(WidgetTree, 0.45f);

	// Colonne à gauche de l'écran, la chambre reste visible à droite
	USizeBox* Taille = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Taille->SetWidthOverride(640.f);
	if (UOverlaySlot* SlotTaille = Racine->AddChildToOverlay(Taille))
	{
		SlotTaille->SetHorizontalAlignment(HAlign_Left);
		SlotTaille->SetVerticalAlignment(VAlign_Fill);
	}
	UBorder* Bandeau = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Bandeau->SetBrushColor(FLinearColor(0.015f, 0.015f, 0.02f, 0.88f));
	Bandeau->SetPadding(FMargin(72.f, 0.f, 56.f, 0.f));
	Bandeau->SetVerticalAlignment(VAlign_Center);
	Taille->AddChild(Bandeau);

	UVerticalBox* Colonne = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Bandeau->SetContent(Colonne);

	Ajouter(Colonne, CreerTexte(WidgetTree, LOCTEXT("SurTitre", "UN JEU SÉRIEUX SUR LE RECRUTEMENT DANS L'INFORMATIQUE"), 13, true, Accent()), FMargin(0.f, 0.f, 0.f, 6.f));
	Ajouter(Colonne, CreerTexte(WidgetTree, LOCTEXT("Titre", "INTERVIEW\nMAXXING"), 64, true), FMargin(0.f, 0.f, 0.f, 18.f));

	UTextBlock* Accroche = CreerTexte(WidgetTree,
		LOCTEXT("Accroche", "Construisez votre CV étiquette par étiquette, adaptez-le à chaque entreprise, passez l'entretien puis un vrai test technique du métier. Décrochez un poste avant d'épuiser toutes les offres."),
		17, false, TexteSecondaire());
	Accroche->SetAutoWrapText(true);
	Ajouter(Colonne, Accroche, FMargin(0.f, 0.f, 0.f, 40.f));

	AjouterBoutons(Colonne, 1.f, 300.f, HAlign_Left);

	Ajouter(Colonne, CreerTexte(WidgetTree, LOCTEXT("Credits", "César Charbey & Thomas Barascud — UE Jeu Sérieux 2026, Université de Montpellier"), 12, false, TexteSecondaire()), FMargin(0.f, 48.f, 0.f, 0.f));
}

void UEcranTitre::OnNouvellePartie()
{
	if (UProgressionPartie* Progression = UProgressionPartie::Get(this))
	{
		Progression->NouvellePartie();
	}
	RemoveFromParent();
}

void UEcranTitre::OnParametres()
{
	if (MenuParametres)
	{
		return;
	}
	MenuParametres = CreateWidget<UMenuParametres>(GetOwningPlayer(), UMenuParametres::StaticClass());
	if (MenuParametres)
	{
		MenuParametres->OnFerme.AddDynamic(this, &UEcranTitre::OnParametresFermes);
		MenuParametres->AddToViewport(120);
		PasserEnModeInterface(GetOwningPlayer(), MenuParametres);
	}
}

void UEcranTitre::OnParametresFermes()
{
	MenuParametres = nullptr;
	PasserEnModeInterface(GetOwningPlayer(), this);
}

void UEcranTitre::OnQuitter()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

// ===========================================================================
// Histoire d'introduction
// ===========================================================================

void UHistoireIntro::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	Pages = {
		{ LOCTEXT("P1Titre", "Juin, dans votre chambre"),
		  LOCTEXT("P1", "Votre diplôme d'informatique est enfin en poche. Il ne manque plus qu'une chose : un premier emploi.\n\nVotre boîte mail déborde d'offres de studios de jeux vidéo et d'entreprises réseau. Mais chaque recruteur a ses propres attentes, et une candidature ratée ne se rattrape pas.") },
		{ LOCTEXT("P2Titre", "Le parcours d'une candidature"),
		  LOCTEXT("P2", "1.  Sur votre ordinateur, lisez une offre et construisez un CV qui correspond à ce que l'entreprise recherche, puis postulez.\n\n2.  Allez dormir : le lendemain matin, vous saurez si votre CV a été retenu.\n\n3.  Si c'est le cas, prenez la porte pour passer l'entretien face au recruteur.\n\n4.  Entretien réussi ? Place au test technique sur le métier de l'entreprise, avec un guide pour vous aider.\n\n5.  De retour chez vous, l'entreprise vous donne sa décision.") },
		{ LOCTEXT("P3Titre", "Les règles"),
		  LOCTEXT("P3", "Une offre à laquelle vous avez postulé ne revient pas, que vous réussissiez ou non. Si vous épuisez toutes les offres sans être embauché, la partie est perdue.\n\nVotre objectif du moment est toujours affiché en haut à gauche de l'écran, et des faits réels sur le recrutement dans l'informatique apparaissent au fil de la partie.") },
		{ LOCTEXT("P4Titre", "Commandes"),
		  LOCTEXT("P4", "ZQSD : se déplacer\nSouris : regarder\nE : interagir (ordinateur, lit, porte, chaise)\nÉchap : paramètres et retour au menu\n\nBonne chance, et adaptez-vous à chaque entreprise !") },
	};

	UOverlay* Racine = CreerRacineAssombrie(WidgetTree, 0.75f);
	UVerticalBox* Colonne = CreerPanneauCentral(WidgetTree, Racine, 820.f, FMargin(52.f, 40.f));

	TexteCompteur = CreerTexte(WidgetTree, FText::GetEmpty(), 13, true, Accent());
	Ajouter(Colonne, TexteCompteur, FMargin(0.f, 0.f, 0.f, 6.f));

	TexteTitre = CreerTexte(WidgetTree, FText::GetEmpty(), 32, true);
	Ajouter(Colonne, TexteTitre, FMargin(0.f, 0.f, 0.f, 20.f));

	TexteCorps = CreerTexte(WidgetTree, FText::GetEmpty(), 19);
	TexteCorps->SetAutoWrapText(true);
	USizeBox* HauteurCorps = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	HauteurCorps->SetMinDesiredHeight(300.f);
	HauteurCorps->AddChild(TexteCorps);
	Ajouter(Colonne, HauteurCorps, FMargin(0.f, 0.f, 0.f, 28.f));

	UHorizontalBox* LigneBoutons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UButton* BoutonPasser = CreerBouton(WidgetTree, LOCTEXT("Passer", "Passer l'introduction"), false, 16);
	BoutonPasser->OnClicked.AddDynamic(this, &UHistoireIntro::OnPasser);
	LigneBoutons->AddChildToHorizontalBox(BoutonPasser);
	if (UHorizontalBoxSlot* SlotEspace = LigneBoutons->AddChildToHorizontalBox(WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass())))
	{
		SlotEspace->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	UButton* BoutonSuivant = CreerBouton(WidgetTree, LOCTEXT("Suivant", "Suivant"), true, 18);
	TexteBoutonSuivant = Cast<UTextBlock>(BoutonSuivant->GetContent());
	BoutonSuivant->OnClicked.AddDynamic(this, &UHistoireIntro::OnSuivant);
	LigneBoutons->AddChildToHorizontalBox(BoutonSuivant);
	Ajouter(Colonne, LigneBoutons);

	AfficherPage(0);
}

FReply UHistoireIntro::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Touche = InKeyEvent.GetKey();
	if (Touche == EKeys::Enter || Touche == EKeys::SpaceBar || Touche == EKeys::Right)
	{
		OnSuivant();
		return FReply::Handled();
	}
	if (Touche == EKeys::Left && PageActuelle > 0)
	{
		AfficherPage(PageActuelle - 1);
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UHistoireIntro::AfficherPage(int32 Index)
{
	PageActuelle = FMath::Clamp(Index, 0, Pages.Num() - 1);
	TexteTitre->SetText(Pages[PageActuelle].Titre);
	TexteCorps->SetText(Pages[PageActuelle].Texte);
	TexteCompteur->SetText(FText::Format(LOCTEXT("Compteur", "{0} / {1}"), PageActuelle + 1, Pages.Num()));
	if (TexteBoutonSuivant)
	{
		TexteBoutonSuivant->SetText(PageActuelle == Pages.Num() - 1 ? LOCTEXT("Commencer", "Commencer") : LOCTEXT("Suivant", "Suivant"));
	}
}

void UHistoireIntro::OnSuivant()
{
	if (PageActuelle < Pages.Num() - 1)
	{
		AfficherPage(PageActuelle + 1);
	}
	else
	{
		Terminer();
	}
}

void UHistoireIntro::OnPasser()
{
	Terminer();
}

void UHistoireIntro::Terminer()
{
	RemoveFromParent();
	if (UProgressionPartie* Progression = UProgressionPartie::Get(this))
	{
		Progression->CommencerPartie();
	}
}

// ===========================================================================
// Fin de partie
// ===========================================================================

void UEcranFinPartie::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}
	UOverlay* Racine = CreerRacineAssombrie(WidgetTree, 0.8f);
	Contenu = CreerPanneauCentral(WidgetTree, Racine, 900.f, FMargin(48.f, 36.f));
}

void UEcranFinPartie::AjouterStat(UHorizontalBox* Ligne, int32 Valeur, const FText& Libelle, const FLinearColor& Couleur)
{
	UVerticalBox* Bloc = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Ajouter(Bloc, CreerTexte(WidgetTree, FText::AsNumber(Valeur), 34, true, Couleur), FMargin(0.f), HAlign_Center);
	UTextBlock* TexteLibelle = CreerTexte(WidgetTree, Libelle, 14, false, TexteSecondaire());
	TexteLibelle->SetJustification(ETextJustify::Center);
	Ajouter(Bloc, TexteLibelle, FMargin(0.f), HAlign_Center);
	if (UHorizontalBoxSlot* SlotBloc = Ligne->AddChildToHorizontalBox(Bloc))
	{
		SlotBloc->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
}

void UEcranFinPartie::Configurer(bool bVictoire, const FText& EntrepriseEmbauche)
{
	UProgressionPartie* Progression = UProgressionPartie::Get(this);
	if (!Contenu || !Progression)
	{
		return;
	}
	Contenu->ClearChildren();

	const TArray<FTentativeCandidature>& Tentatives = Progression->GetTentatives();
	const int32 Jours = Progression->GetJour();

	int32 CVRetenus = 0, EntretiensReussis = 0, TestsPasses = 0;
	for (const FTentativeCandidature& Tentative : Tentatives)
	{
		CVRetenus += Tentative.Resultat != EResultatCandidature::CVRefuse && Tentative.Etape != EEtapeCandidature::AttenteReponseCV ? 1 : 0;
		EntretiensReussis += (Tentative.Resultat == EResultatCandidature::TestRate || Tentative.Resultat == EResultatCandidature::Embauche
			|| Tentative.Etape == EEtapeCandidature::TestTechnique) ? 1 : 0;
		TestsPasses += (Tentative.Resultat == EResultatCandidature::TestRate || Tentative.Resultat == EResultatCandidature::Embauche) ? 1 : 0;
	}

	// En-tête
	Ajouter(Contenu, CreerTexte(WidgetTree, bVictoire ? LOCTEXT("SurTitreVictoire", "FIN DE PARTIE — VICTOIRE") : LOCTEXT("SurTitreDefaite", "FIN DE PARTIE"), 13, true, Accent()), FMargin(0.f, 0.f, 0.f, 4.f), HAlign_Center);
	UTextBlock* Titre = CreerTexte(WidgetTree,
		bVictoire ? FText::Format(LOCTEXT("TitreVictoire", "Embauché chez {0} !"), EntrepriseEmbauche) : LOCTEXT("TitreDefaite", "Plus aucune offre disponible"),
		34, true, bVictoire ? Reussite() : Echec());
	Titre->SetJustification(ETextJustify::Center);
	Titre->SetAutoWrapText(true);
	Ajouter(Contenu, Titre, FMargin(0.f, 0.f, 0.f, 8.f), HAlign_Center);

	UTextBlock* SousTitre = CreerTexte(WidgetTree,
		bVictoire
			? FText::Format(LOCTEXT("SousTitreVictoire", "Il vous aura fallu {0} {0}|plural(one=jour,other=jours) et {1} {1}|plural(one=candidature,other=candidatures) pour décrocher votre premier poste."), Jours, Tentatives.Num())
			: FText::Format(LOCTEXT("SousTitreDefaite", "Après {0} {0}|plural(one=jour,other=jours) et {1} {1}|plural(one=candidature,other=candidatures), aucune entreprise ne vous a embauché. Relancez une partie pour tenter une autre stratégie."), Jours, Tentatives.Num()),
		17, false, TexteSecondaire());
	SousTitre->SetJustification(ETextJustify::Center);
	SousTitre->SetAutoWrapText(true);
	Ajouter(Contenu, SousTitre, FMargin(0.f, 0.f, 0.f, 22.f), HAlign_Fill);

	// Chiffres clés
	UHorizontalBox* LigneStats = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	AjouterStat(LigneStats, Jours, LOCTEXT("StatJours", "jours\nécoulés"), Texte());
	AjouterStat(LigneStats, Tentatives.Num(), LOCTEXT("StatCandidatures", "candidatures"), Texte());
	AjouterStat(LigneStats, CVRetenus, LOCTEXT("StatCV", "CV\nretenus"), Accent());
	AjouterStat(LigneStats, EntretiensReussis, LOCTEXT("StatEntretiens", "entretiens\nréussis"), Accent());
	AjouterStat(LigneStats, TestsPasses, LOCTEXT("StatTests", "tests\ntechniques"), Accent());
	Ajouter(Contenu, LigneStats, FMargin(0.f, 0.f, 0.f, 22.f));

	// Parcours candidature par candidature
	Ajouter(Contenu, CreerTexte(WidgetTree, LOCTEXT("Parcours", "Votre parcours"), 18, true), FMargin(0.f, 0.f, 0.f, 8.f));
	UScrollBox* Liste = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());
	USizeBox* HauteurListe = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	HauteurListe->SetMaxDesiredHeight(230.f);
	HauteurListe->AddChild(Liste);
	Ajouter(Contenu, HauteurListe, FMargin(0.f, 0.f, 0.f, 20.f));

	if (Tentatives.IsEmpty())
	{
		Liste->AddChild(CreerTexte(WidgetTree, LOCTEXT("AucuneCandidature", "Aucune candidature envoyée."), 15, false, TexteSecondaire()));
	}
	for (int32 Index = 0; Index < Tentatives.Num(); ++Index)
	{
		const FTentativeCandidature& Tentative = Tentatives[Index];
		UHorizontalBox* Ligne = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

		const int32 JourFin = Tentative.JourFin > 0 ? Tentative.JourFin : Jours;
		const FText Periode = JourFin > Tentative.JourDepart
			? FText::Format(LOCTEXT("Periode", "Jours {0} à {1}"), Tentative.JourDepart, JourFin)
			: FText::Format(LOCTEXT("PeriodeUnJour", "Jour {0}"), Tentative.JourDepart);
		UTextBlock* TextePeriode = CreerTexte(WidgetTree, Periode, 15, false, TexteSecondaire());
		TextePeriode->SetMinDesiredWidth(120.f);
		Ligne->AddChildToHorizontalBox(TextePeriode);

		UTextBlock* TexteEntreprise = CreerTexte(WidgetTree,
			FText::Format(LOCTEXT("LigneEntreprise", "{0}  ·  {1}"), Tentative.Entreprise, NomDomaine(Tentative.Domaine)), 15, true);
		if (UHorizontalBoxSlot* SlotEntreprise = Ligne->AddChildToHorizontalBox(TexteEntreprise))
		{
			SlotEntreprise->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}

		FLinearColor CouleurResultat;
		const FText Resultat = TexteResultat(Tentative.Resultat, CouleurResultat);
		Ligne->AddChildToHorizontalBox(CreerTexte(WidgetTree, Resultat, 15, true, CouleurResultat));

		if (UScrollBoxSlot* SlotLigne = Cast<UScrollBoxSlot>(Liste->AddChild(Ligne)))
		{
			SlotLigne->SetPadding(FMargin(0.f, 3.f));
		}
	}

	// Un fait réel sur le recrutement pour conclure
	if (const FFaitReel* Fait = Progression->TirerFait(TEXT("general")))
	{
		UBorder* Encadre = CreerPanneau(WidgetTree, FMargin(18.f, 12.f), FondEncadre(), 10.f);
		UVerticalBox* ColonneFait = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		Encadre->SetContent(ColonneFait);
		Ajouter(ColonneFait, CreerTexte(WidgetTree, LOCTEXT("DansLaRealite", "Dans la réalité"), 14, true, Accent()), FMargin(0.f, 0.f, 0.f, 4.f));
		UTextBlock* TexteFait = CreerTexte(WidgetTree, FText::FromString(Fait->Texte), 15);
		TexteFait->SetAutoWrapText(true);
		Ajouter(ColonneFait, TexteFait, FMargin(0.f, 0.f, 0.f, 4.f));
		UTextBlock* TexteSource = CreerTexte(WidgetTree, FText::Format(LOCTEXT("Source", "Source : {0}"), FText::FromString(Fait->Source)), 12, false, TexteSecondaire());
		TexteSource->SetAutoWrapText(true);
		Ajouter(ColonneFait, TexteSource);
		Ajouter(Contenu, Encadre, FMargin(0.f, 0.f, 0.f, 22.f));
	}

	UButton* BoutonMenu = CreerBouton(WidgetTree, LOCTEXT("RetourMenu", "Retour au menu principal"), true, 18);
	BoutonMenu->OnClicked.AddDynamic(this, &UEcranFinPartie::OnRetourMenu);
	Ajouter(Contenu, BoutonMenu, FMargin(0.f), HAlign_Center);
}

void UEcranFinPartie::OnRetourMenu()
{
	RemoveFromParent();
	if (UProgressionPartie* Progression = UProgressionPartie::Get(this))
	{
		Progression->RetourMenuPrincipal();
	}
}

// ===========================================================================
// Ligne d'objectif
// ===========================================================================

void UObjectifEcran::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UOverlay* Racine = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Racine"));
	WidgetTree->RootWidget = Racine;

	UBorder* Panneau = CreerPanneau(WidgetTree, FMargin(16.f, 10.f, 20.f, 12.f), FLinearColor(0.02f, 0.02f, 0.025f, 0.78f), 10.f);
	// Avec la fenêtre T_Cadre, le panneau est construit en grand puis réduit d'un bloc :
	// bords, barre de titre et texte gardent leurs proportions
	const float Reduction = CadrePersonnalise() ? 0.5f : 1.f;
	const float Agrandi = 1.f / Reduction;
	AppliquerCadre(Panneau, FMargin(16.f, 6.f, 20.f, 8.f), 1.f);
	Panneau->SetRenderTransformPivot(FVector2D(0.f, 0.f));
	Panneau->SetRenderScale(FVector2D(Reduction, Reduction));

	USizeBox* TaillePanneau = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	TaillePanneau->SetMinDesiredWidth(CadrePersonnalise() ? 840.f : 0.f);
	TaillePanneau->AddChild(Panneau);
	if (UOverlaySlot* SlotPanneau = Racine->AddChildToOverlay(TaillePanneau))
	{
		SlotPanneau->SetHorizontalAlignment(HAlign_Left);
		SlotPanneau->SetVerticalAlignment(VAlign_Top);
		SlotPanneau->SetPadding(FMargin(24.f, 24.f, 0.f, 0.f));
	}

	UVerticalBox* Colonne = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Panneau->SetContent(Colonne);
	TexteJour = CreerTexte(WidgetTree, FText::GetEmpty(), FMath::RoundToInt(13 * Agrandi), true, Accent());
	Ajouter(Colonne, TexteJour, FMargin(0.f, 0.f, 0.f, 2.f * Agrandi));
	TexteObjectif = CreerTexte(WidgetTree, FText::GetEmpty(), FMath::RoundToInt(17 * Agrandi), true);
	TexteObjectif->SetWrapTextAt(460.f * Agrandi);
	Ajouter(Colonne, TexteObjectif);

	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UObjectifEcran::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const UProgressionPartie* Progression = UProgressionPartie::Get(this);
	if (!Progression || !TexteObjectif)
	{
		return;
	}

	const FText Objectif = Progression->TexteObjectif();
	if (!Objectif.EqualTo(DernierObjectif))
	{
		DernierObjectif = Objectif;
		TexteObjectif->SetText(Objectif);
		TempsDepuisChangement = 0.f;
	}
	TexteJour->SetText(FText::Format(LOCTEXT("Jour", "JOUR {0}  ·  OBJECTIF"), Progression->GetJour()));

	// Petit flash quand l'objectif change, pour attirer l'œil
	TempsDepuisChangement += InDeltaTime;
	const float Flash = FMath::Clamp(1.f - TempsDepuisChangement / 1.5f, 0.f, 1.f);
	TexteObjectif->SetColorAndOpacity(FSlateColor(FMath::Lerp(StyleInterface::Texte(), FLinearColor(1.f, 0.85f, 0.2f), Flash)));

	// Masquée quand il n'y a rien à faire (menus, fin de partie) ou qu'une interface occupe l'écran (PC, test, récap)
	const APlayerController* PC = GetOwningPlayer();
	const bool bInterfaceOuverte = PC && PC->bShowMouseCursor;
	SetRenderOpacity(Objectif.IsEmpty() || bInterfaceOuverte ? 0.f : 1.f);
}

// ===========================================================================
// Carte « Le saviez-vous ? »
// ===========================================================================

void UCarteFait::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UOverlay* Racine = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Racine"));
	WidgetTree->RootWidget = Racine;

	// Avec la fenêtre T_Cadre, la carte est construite en grand puis réduite d'un bloc (coin bas droit fixe)
	const float Reduction = CadrePersonnalise() ? 0.6f : 1.f;
	const float A = 1.f / Reduction;

	USizeBox* Taille = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Taille->SetWidthOverride(480.f * A);
	if (UOverlaySlot* SlotTaille = Racine->AddChildToOverlay(Taille))
	{
		SlotTaille->SetHorizontalAlignment(HAlign_Right);
		SlotTaille->SetVerticalAlignment(VAlign_Bottom);
		SlotTaille->SetPadding(FMargin(0.f, 0.f, 28.f, 28.f));
	}
	Taille->SetRenderTransformPivot(FVector2D(1.f, 1.f));
	Taille->SetRenderScale(FVector2D(Reduction, Reduction));

	UBorder* Panneau = CreerPanneau(WidgetTree, FMargin(18.f, 14.f), FLinearColor(0.05f, 0.07f, 0.11f, 0.95f), 12.f);
	AppliquerCadre(Panneau, FMargin(18.f, 8.f, 18.f, 12.f), 1.f);
	Taille->AddChild(Panneau);
	UVerticalBox* Colonne = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Panneau->SetContent(Colonne);

	auto T = [A](int32 Taille) { return FMath::RoundToInt(Taille * A); };
	TexteContexte = CreerTexte(WidgetTree, FText::GetEmpty(), T(15), true, Accent());
	Ajouter(Colonne, TexteContexte, FMargin(0.f, 0.f, 0.f, 6.f * A));
	TexteRetour = CreerTexte(WidgetTree, FText::GetEmpty(), T(15));
	TexteRetour->SetAutoWrapText(true);
	Ajouter(Colonne, TexteRetour, FMargin(0.f, 0.f, 0.f, 10.f * A));
	TitreFait = CreerTexte(WidgetTree, LOCTEXT("Saviez", "LE SAVIEZ-VOUS ?"), T(12), true, Accent());
	Ajouter(Colonne, TitreFait, FMargin(0.f, 0.f, 0.f, 4.f * A));
	TexteFait = CreerTexte(WidgetTree, FText::GetEmpty(), T(15));
	TexteFait->SetAutoWrapText(true);
	Ajouter(Colonne, TexteFait, FMargin(0.f, 0.f, 0.f, 6.f * A));
	TexteSource = CreerTexte(WidgetTree, FText::GetEmpty(), T(12), false, TexteSecondaire());
	TexteSource->SetAutoWrapText(true);
	Ajouter(Colonne, TexteSource);

	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCarteFait::Configurer(const FText& Contexte, bool bPositif, const FText& Retour, const FFaitReel* Fait)
{
	TexteContexte->SetText(Contexte);
	TexteContexte->SetColorAndOpacity(FSlateColor(bPositif ? Reussite() : Echec()));

	TexteRetour->SetText(Retour);
	TexteRetour->SetVisibility(Retour.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);

	const ESlateVisibility VisibiliteFait = Fait ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
	TitreFait->SetVisibility(VisibiliteFait);
	TexteFait->SetVisibility(VisibiliteFait);
	TexteSource->SetVisibility(VisibiliteFait);
	if (Fait)
	{
		TexteFait->SetText(FText::FromString(Fait->Texte));
		TexteSource->SetText(FText::Format(LOCTEXT("SourceCarte", "Source : {0}"), FText::FromString(Fait->Source)));
	}

	// Plus de temps de lecture quand la carte explique un résultat
	Duree = Retour.IsEmpty() ? 16.f : 26.f;
	TempsRestant = Duree;
}

void UCarteFait::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Apparition et disparition en fondu
	TempsRestant -= InDeltaTime;
	SetRenderOpacity(FMath::Clamp(FMath::Min(TempsRestant, Duree - TempsRestant) / 0.6f, 0.f, 1.f));
	if (TempsRestant <= 0.f)
	{
		RemoveFromParent();
	}
}

#undef LOCTEXT_NAMESPACE
