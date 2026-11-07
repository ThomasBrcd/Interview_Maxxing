#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProgressionPartie.h"
#include "InterfacePartie.generated.h"

class UTextBlock;
class UButton;
class UVerticalBox;
class UMenuParametres;

/**
 * Écran titre affiché au lancement du jeu, par-dessus la chambre.
 * Nouvelle partie / Paramètres / Quitter. Interface construite en C++.
 */
UCLASS()
class INTERVIEW_MAXXING_API UEcranTitre : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

private:
	/** Avec l'image Content/Ui/Menus/T_Titre_Fond : seuls l'USP et les boutons sont ajoutés dans son cadre */
	void ConstruireAvecImage(class UTexture2D* Fond);

	/** Sans image : titre, USP, boutons et crédits construits en code */
	void ConstruireSansImage();

	void AjouterBoutons(UVerticalBox* Colonne, float Echelle, float Largeur, EHorizontalAlignment Alignement);

	UFUNCTION()
	void OnNouvellePartie();

	UFUNCTION()
	void OnParametres();

	UFUNCTION()
	void OnParametresFermes();

	UFUNCTION()
	void OnQuitter();

	UPROPERTY()
	TObjectPtr<UMenuParametres> MenuParametres;
};

/**
 * Courte histoire affichée au début d'une nouvelle partie : contexte, puis consignes.
 * Le joueur avance page par page, ou passe directement au jeu.
 */
UCLASS()
class INTERVIEW_MAXXING_API UHistoireIntro : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	void AfficherPage(int32 Index);

	UFUNCTION()
	void OnSuivant();

	UFUNCTION()
	void OnPasser();

	void Terminer();

	struct FPage
	{
		FText Titre;
		FText Texte;
	};
	TArray<FPage> Pages;
	int32 PageActuelle = 0;

	UPROPERTY()
	TObjectPtr<UTextBlock> TexteTitre;

	UPROPERTY()
	TObjectPtr<UTextBlock> TexteCorps;

	UPROPERTY()
	TObjectPtr<UTextBlock> TexteCompteur;

	UPROPERTY()
	TObjectPtr<UTextBlock> TexteBoutonSuivant;
};

/**
 * Fin de partie : embauché (victoire) ou plus aucune offre disponible (défaite).
 * Récapitule le parcours du joueur (candidatures, jours, étapes atteintes) avec un fait réel sourcé.
 */
UCLASS()
class INTERVIEW_MAXXING_API UEcranFinPartie : public UUserWidget
{
	GENERATED_BODY()

public:
	void Configurer(bool bVictoire, const FText& EntrepriseEmbauche);

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void OnRetourMenu();

	void AjouterStat(class UHorizontalBox* Ligne, int32 Valeur, const FText& Libelle, const FLinearColor& Couleur);

	UPROPERTY()
	TObjectPtr<UVerticalBox> Contenu;
};

/**
 * Ligne d'objectif en haut à gauche de l'écran : jour en cours et prochaine action à faire.
 */
UCLASS()
class INTERVIEW_MAXXING_API UObjectifEcran : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> TexteJour;

	UPROPERTY()
	TObjectPtr<UTextBlock> TexteObjectif;

	FText DernierObjectif;
	float TempsDepuisChangement = 10.f;
};

/**
 * Carte « Le saviez-vous ? » : un fait réel sur le recrutement dans l'informatique, avec sa source.
 * Affichée quelques secondes en bas à droite, sans bloquer le jeu.
 */
UCLASS()
class INTERVIEW_MAXXING_API UCarteFait : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Contexte : titre de la carte. Retour : explication du résultat (optionnelle). Fait : fait réel sourcé (optionnel). */
	void Configurer(const FText& Contexte, bool bPositif, const FText& Retour, const FFaitReel* Fait);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> TexteContexte;

	UPROPERTY()
	TObjectPtr<UTextBlock> TexteRetour;

	UPROPERTY()
	TObjectPtr<UTextBlock> TitreFait;

	UPROPERTY()
	TObjectPtr<UTextBlock> TexteFait;

	UPROPERTY()
	TObjectPtr<UTextBlock> TexteSource;

	float Duree = 16.f;
	float TempsRestant = 16.f;
};
