#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "ProgressionPartie.generated.h"

class UUserWidget;
class UObjectifEcran;

/** Étape atteinte par une candidature */
UENUM(BlueprintType)
enum class EEtapeCandidature : uint8
{
	AttenteReponseCV,
	Entretien,
	TestTechnique,
	Terminee
};

/** Issue d'une candidature */
UENUM(BlueprintType)
enum class EResultatCandidature : uint8
{
	EnCours,
	CVRefuse,
	EntretienRate,
	TestRate,
	Embauche
};

/** Une candidature du joueur, pour le récapitulatif de fin de partie */
USTRUCT(BlueprintType)
struct FTentativeCandidature
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	FName IdOffre;

	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	FText Entreprise;

	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	FString Domaine;

	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	int32 JourDepart = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	int32 JourFin = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	EEtapeCandidature Etape = EEtapeCandidature::AttenteReponseCV;

	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	EResultatCandidature Resultat = EResultatCandidature::EnCours;
};

/** Fait réel sourcé sur le recrutement (Content/Data/faits_recrutement.json) */
struct FFaitReel
{
	FString Id;
	FString Etape;
	FString Texte;
	FString Source;
	FString Url;
};

/**
 * Suit la partie d'un bout à l'autre, indépendamment des changements de niveau :
 * - écran titre au lancement, histoire d'introduction, retour au menu ;
 * - compteur de jours et historique des candidatures (récap de fin) ;
 * - ligne d'objectif et cartes « Le saviez-vous ? » ;
 * - fin de partie : embauche (victoire) ou plus aucune offre (défaite).
 *
 * L'état du jeu reste dans le GameInstance Blueprint (GI_InterviewMaxxing) : ce sous-système
 * le lit par réflexion et détecte les changements d'étape, sans modifier la boucle existante.
 */
UCLASS()
class INTERVIEW_MAXXING_API UProgressionPartie : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UProgressionPartie* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Appelé au début de chaque niveau (voir UProgressionMonde) */
	void SurDebutNiveau(UWorld* Monde);

	// --- Menus -----------------------------------------------------------

	/** Depuis l'écran titre : remet tout à zéro et affiche l'histoire d'introduction */
	void NouvellePartie();

	/** Fin de l'introduction : le joueur prend le contrôle */
	void CommencerPartie();

	/** Remet la partie à zéro et revient à l'écran titre */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void RetourMenuPrincipal();

	// --- Événements de la boucle de jeu (appelables depuis les Blueprints) --

	/** Le test technique est terminé et le joueur a reçu la réponse de l'entreprise */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void EnregistrerDecisionFinale(bool bEmbauche, FText Entreprise);

	/** Le message de refus a été fermé : vérifie s'il reste des offres */
	void VerifierDefaite();

	// --- Lecture ---------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetJour() const { return Jour; }

	const TArray<FTentativeCandidature>& GetTentatives() const { return Tentatives; }

	/** Texte de la ligne d'objectif (vide si rien à afficher) */
	FText TexteObjectif() const;

	/** Nombre d'offres auxquelles le joueur n'a pas encore postulé */
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 NombreOffresRestantes() const;

	/** Un fait réel pour cette étape (cv, entretien, test_jeu3d, test_reseaux, general), sans répétition tant que possible */
	const FFaitReel* TirerFait(const FString& Etape);

	/** Affiche une carte non bloquante : résultat d'une étape, explication et fait réel « Le saviez-vous ? » */
	void AfficherCarteFait(const FString& Etape, const FText& Contexte, bool bPositif, const FText& Retour, float Delai = 0.f);

	bool EstPartieTerminee() const { return bPartieTerminee; }

private:
	/** Lit l'état du GameInstance et détecte les passages d'une étape à l'autre */
	void Observer();

	void AfficherEcranFin(bool bVictoire, const FText& Entreprise);
	void AfficherEcranTitre();
	void ReinitialiserGameInstance();
	void ChargerFaits();
	FTentativeCandidature* TentativeEnCours();
	FText NomEntreprise(FName IdOffre) const;

	/** Explique au joueur pourquoi son CV a été retenu ou refusé (mêmes règles que UCVMathLibrary::CalculerScoreFinal) */
	FText ExpliquerResultatCV(FName IdOffre, bool bRetenu) const;
	FString DomaineOffre(FName IdOffre) const;
	APlayerController* JoueurLocal() const;

	int32 Jour = 1;
	TArray<FTentativeCandidature> Tentatives;

	bool bAfficherTitre = true;
	bool bTitreOuIntroAffiche = false;
	bool bPartieTerminee = false;

	/** Durée pendant laquelle les conditions de défaite sont réunies (évite de couper un récap qui s'ouvre) */
	float TempsDefaitePrete = 0.f;

	// Dernier état observé dans le GameInstance
	bool bEtatConnu = false;
	uint8 EtatPrecedent = 0;
	FString NiveauActuel;
	FString NiveauPrecedent;

	TArray<FFaitReel> Faits;
	TSet<FString> FaitsMontres;

	TWeakObjectPtr<UWorld> MondeActuel;
	TWeakObjectPtr<UObjectifEcran> Objectif;
	FTimerHandle MinuterieObservation;
};

/** Prévient UProgressionPartie à chaque début de niveau (y compris en PIE) */
UCLASS()
class INTERVIEW_MAXXING_API UProgressionMonde : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
};
