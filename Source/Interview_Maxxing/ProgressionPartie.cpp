#include "ProgressionPartie.h"
#include "InterfacePartie.h"
#include "EtiquetteData.h"
#include "MailData.h"
#include "MessageEmbauche.h"
#include "StyleInterface.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Dom/JsonObject.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TimerManager.h"
#include "UObject/UnrealType.h"

#define LOCTEXT_NAMESPACE "ProgressionPartie"

namespace ProgressionInterne
{
	const TCHAR* CheminOffres = TEXT("/Game/Data/DT_Offres_Mails.DT_Offres_Mails");
	const TCHAR* NiveauChambre = TEXT("Chambre");
	const TCHAR* NiveauEntretien = TEXT("MeetingRoom");
	const TCHAR* NiveauBureau = TEXT("Office");

	// Valeurs de l'énumération Blueprint EEtatProgression
	constexpr uint8 EtatPCAutorise = 0;
	constexpr uint8 EtatSommeilRequis = 1;
	constexpr uint8 EtatEntretienRequis = 2;
	constexpr uint8 EtatTestTechRequis = 3;

	// --- Lecture des variables du GameInstance Blueprint par réflexion ---

	bool LireBool(const UObject* Objet, FName Nom)
	{
		const FBoolProperty* Prop = Objet ? FindFProperty<FBoolProperty>(Objet->GetClass(), Nom) : nullptr;
		return Prop && Prop->GetPropertyValue_InContainer(Objet);
	}

	uint8 LireEnum(const UObject* Objet, FName Nom)
	{
		const FProperty* Prop = Objet ? FindFProperty<FProperty>(Objet->GetClass(), Nom) : nullptr;
		if (const FByteProperty* Octet = CastField<FByteProperty>(Prop))
		{
			return Octet->GetPropertyValue_InContainer(Objet);
		}
		if (const FEnumProperty* Enum = CastField<FEnumProperty>(Prop))
		{
			return static_cast<uint8>(Enum->GetUnderlyingProperty()->GetSignedIntPropertyValue(Enum->ContainerPtrToValuePtr<void>(Objet)));
		}
		return 0;
	}

	FName LireName(const UObject* Objet, FName Nom)
	{
		const FNameProperty* Prop = Objet ? FindFProperty<FNameProperty>(Objet->GetClass(), Nom) : nullptr;
		return Prop ? Prop->GetPropertyValue_InContainer(Objet) : NAME_None;
	}

	TArray<FName> LireNames(const UObject* Objet, FName Nom)
	{
		TArray<FName> Resultat;
		const FArrayProperty* Prop = Objet ? FindFProperty<FArrayProperty>(Objet->GetClass(), Nom) : nullptr;
		if (Prop && CastField<FNameProperty>(Prop->Inner))
		{
			FScriptArrayHelper Tableau(Prop, Prop->ContainerPtrToValuePtr<void>(Objet));
			for (int32 Index = 0; Index < Tableau.Num(); ++Index)
			{
				Resultat.Add(*reinterpret_cast<const FName*>(Tableau.GetRawPtr(Index)));
			}
		}
		return Resultat;
	}

	const UDataTable* Offres()
	{
		return LoadObject<UDataTable>(nullptr, CheminOffres);
	}
}

using namespace ProgressionInterne;

// ===========================================================================
// Monde
// ===========================================================================

bool UProgressionMonde::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UProgressionMonde::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	if (UProgressionPartie* Progression = UProgressionPartie::Get(&InWorld))
	{
		Progression->SurDebutNiveau(&InWorld);
	}
}

// ===========================================================================
// Cycle de vie
// ===========================================================================

UProgressionPartie* UProgressionPartie::Get(const UObject* WorldContextObject)
{
	const UWorld* Monde = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const UGameInstance* GI = Monde ? Monde->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UProgressionPartie>() : nullptr;
}

void UProgressionPartie::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ChargerFaits();
}

APlayerController* UProgressionPartie::JoueurLocal() const
{
	return MondeActuel.IsValid() ? MondeActuel->GetFirstPlayerController() : nullptr;
}

void UProgressionPartie::SurDebutNiveau(UWorld* Monde)
{
	MondeActuel = Monde;
	NiveauPrecedent = NiveauActuel;
	NiveauActuel = UGameplayStatics::GetCurrentLevelName(Monde, true);
	bTitreOuIntroAffiche = false;

	// Changement de pièce pendant une candidature : l'entretien est terminé
	if (FTentativeCandidature* Tentative = TentativeEnCours())
	{
		if (Tentative->Etape == EEtapeCandidature::Entretien && NiveauPrecedent == NiveauEntretien)
		{
			if (NiveauActuel == NiveauBureau)
			{
				Tentative->Etape = EEtapeCandidature::TestTechnique;
			}
			else if (NiveauActuel == NiveauChambre)
			{
				Tentative->Etape = EEtapeCandidature::Terminee;
				Tentative->Resultat = EResultatCandidature::EntretienRate;
				Tentative->JourFin = Jour;
			}
		}
	}

	Monde->GetTimerManager().SetTimer(MinuterieObservation, FTimerDelegate::CreateUObject(this, &UProgressionPartie::Observer), 0.25f, true, 0.3f);

	// Laisse les BeginPlay du niveau régler la souris avant d'ajouter nos interfaces
	FTimerHandle Attente;
	Monde->GetTimerManager().SetTimer(Attente, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		APlayerController* PC = JoueurLocal();
		if (!PC)
		{
			return;
		}
		if (UObjectifEcran* Ligne = CreateWidget<UObjectifEcran>(PC, UObjectifEcran::StaticClass()))
		{
			Ligne->AddToViewport(5);
			Objectif = Ligne;
		}
		if (bAfficherTitre && NiveauActuel == NiveauChambre)
		{
			AfficherEcranTitre();
		}
	}), 0.2f, false);
}

// ===========================================================================
// Menus
// ===========================================================================

void UProgressionPartie::AfficherEcranTitre()
{
	APlayerController* PC = JoueurLocal();
	if (!PC)
	{
		return;
	}
	if (UEcranTitre* Titre = CreateWidget<UEcranTitre>(PC, UEcranTitre::StaticClass()))
	{
		Titre->AddToViewport(60);
		StyleInterface::PasserEnModeInterface(PC, Titre);
		bTitreOuIntroAffiche = true;
	}
}

void UProgressionPartie::NouvellePartie()
{
	ReinitialiserGameInstance();
	Jour = 1;
	Tentatives.Reset();
	FaitsMontres.Reset();
	bPartieTerminee = false;
	bAfficherTitre = false;
	bEtatConnu = false;

	APlayerController* PC = JoueurLocal();
	if (UHistoireIntro* Intro = PC ? CreateWidget<UHistoireIntro>(PC, UHistoireIntro::StaticClass()) : nullptr)
	{
		Intro->AddToViewport(60);
		StyleInterface::PasserEnModeInterface(PC, Intro);
		bTitreOuIntroAffiche = true;
	}
	else
	{
		CommencerPartie();
	}
}

void UProgressionPartie::CommencerPartie()
{
	bTitreOuIntroAffiche = false;
	StyleInterface::RevenirEnModeJeu(JoueurLocal());
}

void UProgressionPartie::RetourMenuPrincipal()
{
	ReinitialiserGameInstance();
	Jour = 1;
	Tentatives.Reset();
	bPartieTerminee = false;
	bAfficherTitre = true;
	bEtatConnu = false;

	if (APlayerController* PC = JoueurLocal())
	{
		PC->SetPause(false);
	}
	if (MondeActuel.IsValid())
	{
		UGameplayStatics::OpenLevel(MondeActuel.Get(), FName(NiveauChambre));
	}
}

void UProgressionPartie::ReinitialiserGameInstance()
{
	// Remet chaque variable déclarée dans les Blueprints du GameInstance à sa valeur par défaut
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}
	const UObject* ParDefaut = GI->GetClass()->GetDefaultObject();
	for (TFieldIterator<FProperty> It(GI->GetClass()); It; ++It)
	{
		// Les dispatchers gardent leurs abonnés (liés au BeginPlay des acteurs du niveau)
		if (CastField<FMulticastDelegateProperty>(*It) || CastField<FDelegateProperty>(*It))
		{
			continue;
		}
		if (Cast<UBlueprintGeneratedClass>(It->GetOwnerClass()))
		{
			It->CopyCompleteValue_InContainer(GI, ParDefaut);
		}
	}
}

// ===========================================================================
// Observation de la boucle de jeu
// ===========================================================================

FTentativeCandidature* UProgressionPartie::TentativeEnCours()
{
	return Tentatives.Num() > 0 && Tentatives.Last().Resultat == EResultatCandidature::EnCours ? &Tentatives.Last() : nullptr;
}

void UProgressionPartie::Observer()
{
	const UGameInstance* GI = GetGameInstance();
	if (!GI || bPartieTerminee)
	{
		return;
	}

	// Certains Blueprints vident l'écran (Remove All Widgets) : la ligne d'objectif revient d'elle-même
	if (Objectif.IsValid() && !Objectif->IsInViewport())
	{
		Objectif->AddToViewport(5);
	}

	const uint8 Etat = LireEnum(GI, TEXT("Etat_Actuel"));
	if (!bEtatConnu)
	{
		EtatPrecedent = Etat;
		bEtatConnu = true;

		// Partie reprise en cours de candidature (niveau lancé directement pour tester) : on la suit quand même
		const FName IdOffre = LireName(GI, TEXT("Candidature_Entreprise"));
		if (Etat != EtatPCAutorise && !IdOffre.IsNone() && !TentativeEnCours())
		{
			FTentativeCandidature Reprise;
			Reprise.IdOffre = IdOffre;
			Reprise.Entreprise = NomEntreprise(IdOffre);
			Reprise.Domaine = DomaineOffre(IdOffre);
			Reprise.JourDepart = Jour;
			Reprise.Etape = Etat == EtatSommeilRequis ? EEtapeCandidature::AttenteReponseCV
				: Etat == EtatEntretienRequis ? EEtapeCandidature::Entretien : EEtapeCandidature::TestTechnique;
			Tentatives.Add(Reprise);
		}
		return;
	}

	if (Etat != EtatPrecedent)
	{
		// Candidature envoyée : le joueur doit maintenant dormir
		if (Etat == EtatSommeilRequis)
		{
			const FName IdOffre = LireName(GI, TEXT("Candidature_Entreprise"));
			if (!TentativeEnCours())
			{
				FTentativeCandidature Nouvelle;
				Nouvelle.IdOffre = IdOffre;
				Nouvelle.Entreprise = NomEntreprise(IdOffre);
				Nouvelle.Domaine = DomaineOffre(IdOffre);
				Nouvelle.JourDepart = Jour;
				Tentatives.Add(Nouvelle);
			}
		}

		// Une nuit est passée : réponse de l'entreprise au CV
		if (EtatPrecedent == EtatSommeilRequis)
		{
			++Jour;
			if (FTentativeCandidature* Tentative = TentativeEnCours())
			{
				const bool bRetenu = Etat == EtatEntretienRequis;
				if (bRetenu)
				{
					Tentative->Etape = EEtapeCandidature::Entretien;
				}
				else
				{
					Tentative->Etape = EEtapeCandidature::Terminee;
					Tentative->Resultat = EResultatCandidature::CVRefuse;
					Tentative->JourFin = Jour;
				}
				AfficherCarteFait(TEXT("cv"),
					FText::Format(bRetenu ? LOCTEXT("CarteCVRetenu", "{0} a retenu votre CV") : LOCTEXT("CarteCVRefuse", "{0} n'a pas retenu votre CV"), Tentative->Entreprise),
					bRetenu, ExpliquerResultatCV(Tentative->IdOffre, bRetenu), 1.5f);
			}
		}
		// Fin de l'entretien, dans la salle : réussi (test technique) ou raté
		if (EtatPrecedent == EtatEntretienRequis && NiveauActuel == NiveauEntretien)
		{
			if (FTentativeCandidature* Tentative = TentativeEnCours())
			{
				const bool bReussi = Etat == EtatTestTechRequis;
				Tentative->Etape = bReussi ? EEtapeCandidature::TestTechnique : EEtapeCandidature::Terminee;
				if (!bReussi)
				{
					Tentative->Resultat = EResultatCandidature::EntretienRate;
					Tentative->JourFin = Jour;
				}
				AfficherCarteFait(TEXT("entretien"),
					bReussi ? LOCTEXT("CarteEntretienReussi", "Entretien réussi") : LOCTEXT("CarteEntretienRate", "Entretien raté"),
					bReussi,
					bReussi ? LOCTEXT("RetourEntretienReussi", "Le recruteur a apprécié vos réponses. Prochaine étape : un test technique sur le métier de l'entreprise.")
						: LOCTEXT("RetourEntretienRate", "Trop de réponses inadaptées. Relisez les explications données après chaque erreur : elles valent pour les prochains entretiens."),
					1.f);
			}
		}

		EtatPrecedent = Etat;
	}

	// Défaite : plus aucune offre et plus de candidature en cours
	if (NiveauActuel == NiveauChambre && Etat == EtatPCAutorise && !bTitreOuIntroAffiche
		&& !LireBool(GI, TEXT("ResultatEmbaucheAAnnoncer")))
	{
		VerifierDefaite();
	}
	else
	{
		TempsDefaitePrete = 0.f;
	}
}

void UProgressionPartie::VerifierDefaite()
{
	// On attend que le joueur ait fermé les interfaces ouvertes (message de l'entreprise, récap du matin...)
	// et qu'il ait repris la main depuis un moment : le récap du matin s'ouvre un peu après le réveil
	const APlayerController* PC = JoueurLocal();
	const bool bPrete = !bPartieTerminee && !bTitreOuIntroAffiche && !TentativeEnCours() && NombreOffresRestantes() == 0
		&& PC && !PC->bShowMouseCursor && !PC->IsMoveInputIgnored() && !PC->IsLookInputIgnored();
	TempsDefaitePrete = bPrete ? TempsDefaitePrete + 0.25f : 0.f;
	if (TempsDefaitePrete >= 2.f)
	{
		TempsDefaitePrete = 0.f;
		AfficherEcranFin(false, FText::GetEmpty());
	}
}

void UProgressionPartie::EnregistrerDecisionFinale(bool bEmbauche, FText Entreprise)
{
	FTentativeCandidature* Tentative = TentativeEnCours();
	if (!Tentative)
	{
		// Partie lancée directement dans un niveau (tests) : pas d'historique, mais l'embauche termine quand même la partie
		if (bEmbauche)
		{
			AfficherEcranFin(true, Entreprise);
		}
		return;
	}
	Tentative->Etape = EEtapeCandidature::Terminee;
	Tentative->Resultat = bEmbauche ? EResultatCandidature::Embauche : EResultatCandidature::TestRate;
	Tentative->JourFin = Jour;

	if (bEmbauche)
	{
		AfficherEcranFin(true, Tentative->Entreprise);
	}
	else
	{
		const FString Etape = Tentative->Domaine.Equals(TEXT("jeu3d"), ESearchCase::IgnoreCase) ? TEXT("test_jeu3d") : TEXT("test_reseaux");
		AfficherCarteFait(Etape, FText::Format(LOCTEXT("CarteTest", "{0} ne donne pas suite"), Tentative->Entreprise), false,
			LOCTEXT("RetourTest", "Votre test technique n'a pas atteint 50 %. Le guide du test résume les notions à maîtriser : elles reviendront dans les prochains tests."), 1.f);
	}
}

void UProgressionPartie::AfficherEcranFin(bool bVictoire, const FText& Entreprise)
{
	APlayerController* PC = JoueurLocal();
	if (!PC || bPartieTerminee)
	{
		return;
	}
	bPartieTerminee = true;

	// Les cartes en cours laissent la place à l'écran de fin
	TArray<UUserWidget*> Cartes;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC, Cartes, UCarteFait::StaticClass(), true);
	for (UUserWidget* Carte : Cartes)
	{
		Carte->RemoveFromParent();
	}

	if (UEcranFinPartie* Ecran = CreateWidget<UEcranFinPartie>(PC, UEcranFinPartie::StaticClass()))
	{
		Ecran->AddToViewport(70);
		Ecran->Configurer(bVictoire, Entreprise);
		StyleInterface::PasserEnModeInterface(PC, Ecran);
	}
}

// ===========================================================================
// Objectif
// ===========================================================================

FText UProgressionPartie::TexteObjectif() const
{
	const UGameInstance* GI = GetGameInstance();
	if (!GI || bPartieTerminee || bTitreOuIntroAffiche)
	{
		return FText::GetEmpty();
	}

	const uint8 Etat = LireEnum(GI, TEXT("Etat_Actuel"));

	if (NiveauActuel == NiveauChambre)
	{
		switch (Etat)
		{
		case EtatSommeilRequis:
			return LOCTEXT("ObjDormir", "Allez dormir dans votre lit : l'entreprise répondra demain matin");
		case EtatEntretienRequis:
			return LOCTEXT("ObjAllerEntretien", "Votre CV a été retenu ! Prenez la porte pour aller à l'entretien");
		case EtatTestTechRequis:
			return LOCTEXT("ObjAllerTest", "Prenez la porte pour aller passer le test technique");
		default:
			return FText::Format(LOCTEXT("ObjPostuler", "Postulez à une offre depuis votre ordinateur ({0} {0}|plural(one=offre restante,other=offres restantes))"), NombreOffresRestantes());
		}
	}
	if (NiveauActuel == NiveauEntretien)
	{
		if (Etat == EtatTestTechRequis || LireBool(GI, TEXT("A_Reussi_Entretien")))
		{
			return LOCTEXT("ObjEntretienReussi", "Entretien réussi ! Prenez la porte pour passer le test technique");
		}
		if (Etat == EtatEntretienRequis)
		{
			return LOCTEXT("ObjEntretien", "Asseyez-vous face au recruteur pour commencer l'entretien");
		}
		return LOCTEXT("ObjEntretienFini", "Entretien terminé : prenez la porte pour rentrer chez vous");
	}
	if (NiveauActuel == NiveauBureau)
	{
		// Le test se termine en renvoyant directement le joueur chez lui : dans le bureau, il reste toujours à faire
		return LOCTEXT("ObjTest", "Installez-vous à l'ordinateur et passez le test technique");
	}
	return FText::GetEmpty();
}

// ===========================================================================
// Offres
// ===========================================================================

int32 UProgressionPartie::NombreOffresRestantes() const
{
	const UDataTable* Table = Offres();
	if (!Table)
	{
		return 1;
	}
	const TArray<FName> Postulees = LireNames(GetGameInstance(), TEXT("OffresPostulees"));
	int32 Restantes = 0;
	for (const FName& Ligne : Table->GetRowNames())
	{
		Restantes += Postulees.Contains(Ligne) ? 0 : 1;
	}
	return Restantes;
}

FText UProgressionPartie::NomEntreprise(FName IdOffre) const
{
	if (const UDataTable* Table = Offres())
	{
		if (const FMailDataRow* Offre = Table->FindRow<FMailDataRow>(IdOffre, TEXT("Progression"), false))
		{
			if (!Offre->NomEntreprise.IsEmpty())
			{
				return Offre->NomEntreprise;
			}
		}
	}
	return FText::FromName(IdOffre);
}

FString UProgressionPartie::DomaineOffre(FName IdOffre) const
{
	if (const UDataTable* Table = Offres())
	{
		if (const FMailDataRow* Offre = Table->FindRow<FMailDataRow>(IdOffre, TEXT("Progression"), false))
		{
			switch (Offre->DomainePrincipal)
			{
			case EDomaineTech::Jeu3D: return TEXT("jeu3d");
			case EDomaineTech::Reseaux: return TEXT("reseaux");
			default: break;
			}
		}
	}
	// Identifiant de la forme mail_<domaine>_<entreprise>
	TArray<FString> Morceaux;
	IdOffre.ToString().ParseIntoArray(Morceaux, TEXT("_"));
	return Morceaux.Num() > 1 ? Morceaux[1] : FString();
}

FText UProgressionPartie::ExpliquerResultatCV(FName IdOffre, bool bRetenu) const
{
	const UGameInstance* GI = GetGameInstance();
	const UDataTable* Table = Offres();
	const FMailDataRow* Offre = Table ? Table->FindRow<FMailDataRow>(IdOffre, TEXT("Progression"), false) : nullptr;
	if (!GI || !Offre)
	{
		return FText::GetEmpty();
	}

	int32 Score = 0;
	if (const FIntProperty* Prop = FindFProperty<FIntProperty>(GI->GetClass(), TEXT("DernierScoreObtenu")))
	{
		Score = Prop->GetPropertyValue_InContainer(GI);
	}
	const int32 Bonnes = LireNames(GI, TEXT("Recap_BonnesEtiquettes")).Num();
	const int32 Mauvaises = LireNames(GI, TEXT("Recap_MauvaisesEtiquettes")).Num();
	const FText Domaine = DomaineOffre(IdOffre) == TEXT("jeu3d") ? LOCTEXT("DomJeu", "jeu vidéo 3D") : LOCTEXT("DomRes", "réseaux");

	// Qualité que l'entreprise valorise le plus
	const FPoidsStats& Poids = Offre->MultiplicateursStats;
	const TPair<float, FText> Qualites[] = {
		{ Poids.Technique, LOCTEXT("QTech", "la technique") },
		{ Poids.Creativite, LOCTEXT("QCrea", "la créativité") },
		{ Poids.Rigueur, LOCTEXT("QRig", "la rigueur") },
		{ Poids.Polyvalence, LOCTEXT("QPoly", "la polyvalence") },
		{ Poids.Experience, LOCTEXT("QExp", "l'expérience") },
	};
	const TPair<float, FText>* Prioritaire = &Qualites[0];
	for (const TPair<float, FText>& Qualite : Qualites)
	{
		Prioritaire = Qualite.Key > Prioritaire->Key ? &Qualite : Prioritaire;
	}

	if (bRetenu)
	{
		return FText::Format(LOCTEXT("ExplCVRetenu", "Score {0} pour {1} requis, avec {2} {2}|plural(one=compétence,other=compétences) du domaine {3}. Votre CV répondait à ce que l'offre demandait."),
			Score, Offre->ScoreSeuilReussite, Bonnes, Domaine);
	}

	TArray<FText> Raisons;
	if (Bonnes == 0)
	{
		Raisons.Add(FText::Format(LOCTEXT("RaisonDomaine", "Aucune étiquette de votre CV ne relève du domaine {0} recherché."), Domaine));
	}
	if (Mauvaises >= 3)
	{
		Raisons.Add(FText::Format(LOCTEXT("RaisonMauvaises", "{0} étiquettes hors sujet ou négatives : chacune retire des points et brouille votre profil."), Mauvaises));
	}
	if (Score < Offre->ScoreSeuilReussite)
	{
		Raisons.Add(FText::Format(LOCTEXT("RaisonScore", "Score {0} pour {1} requis."), Score, Offre->ScoreSeuilReussite));
	}
	Raisons.Add(FText::Format(LOCTEXT("Conseil", "Conseil : cette entreprise valorise surtout {0}. Relisez l'offre et choisissez les étiquettes qui y répondent."), Prioritaire->Value));
	return FText::Join(FText::FromString(TEXT("\n")), Raisons);
}

// ===========================================================================
// Faits réels
// ===========================================================================

void UProgressionPartie::ChargerFaits()
{
	FString Contenu;
	const FString Chemin = FPaths::ProjectContentDir() / TEXT("Data/faits_recrutement.json");
	if (!FFileHelper::LoadFileToString(Contenu, *Chemin))
	{
		UE_LOG(LogTemp, Warning, TEXT("Faits réels introuvables : %s"), *Chemin);
		return;
	}

	TSharedPtr<FJsonObject> Racine;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Contenu), Racine) || !Racine.IsValid())
	{
		return;
	}
	const TArray<TSharedPtr<FJsonValue>>* Liste = nullptr;
	if (!Racine->TryGetArrayField(TEXT("faits"), Liste))
	{
		return;
	}
	for (const TSharedPtr<FJsonValue>& Valeur : *Liste)
	{
		const TSharedPtr<FJsonObject> Objet = Valeur->AsObject();
		if (!Objet.IsValid())
		{
			continue;
		}
		FFaitReel Fait;
		Fait.Id = Objet->GetStringField(TEXT("id"));
		Fait.Etape = Objet->GetStringField(TEXT("etape"));
		Fait.Texte = Objet->GetStringField(TEXT("texte"));
		Fait.Source = Objet->GetStringField(TEXT("source"));
		Objet->TryGetStringField(TEXT("url"), Fait.Url);
		Faits.Add(Fait);
	}
}

const FFaitReel* UProgressionPartie::TirerFait(const FString& Etape)
{
	TArray<const FFaitReel*> Candidats;
	for (const FFaitReel& Fait : Faits)
	{
		if (Fait.Etape == Etape && !FaitsMontres.Contains(Fait.Id))
		{
			Candidats.Add(&Fait);
		}
	}
	// Tous déjà vus : on recommence le tirage pour cette étape
	if (Candidats.IsEmpty())
	{
		for (const FFaitReel& Fait : Faits)
		{
			if (Fait.Etape == Etape)
			{
				FaitsMontres.Remove(Fait.Id);
				Candidats.Add(&Fait);
			}
		}
	}
	if (Candidats.IsEmpty())
	{
		return nullptr;
	}
	const FFaitReel* Choisi = Candidats[FMath::RandRange(0, Candidats.Num() - 1)];
	FaitsMontres.Add(Choisi->Id);
	return Choisi;
}

void UProgressionPartie::AfficherCarteFait(const FString& Etape, const FText& Contexte, bool bPositif, const FText& Retour, float Delai)
{
	if (!MondeActuel.IsValid())
	{
		return;
	}
	auto Afficher = [this, Etape, Contexte, bPositif, Retour]()
	{
		APlayerController* PC = JoueurLocal();
		if (!PC || bPartieTerminee)
		{
			return;
		}
		// Une seule carte à la fois
		TArray<UUserWidget*> Anciennes;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC, Anciennes, UCarteFait::StaticClass(), true);
		for (UUserWidget* Ancienne : Anciennes)
		{
			Ancienne->RemoveFromParent();
		}
		if (UCarteFait* Carte = CreateWidget<UCarteFait>(PC, UCarteFait::StaticClass()))
		{
			Carte->Configurer(Contexte, bPositif, Retour, TirerFait(Etape));
			Carte->AddToViewport(90);
		}
	};

	if (Delai <= 0.f)
	{
		Afficher();
		return;
	}
	FTimerHandle Attente;
	MondeActuel->GetTimerManager().SetTimer(Attente, FTimerDelegate::CreateWeakLambda(this, Afficher), Delai, false);
}

#undef LOCTEXT_NAMESPACE
