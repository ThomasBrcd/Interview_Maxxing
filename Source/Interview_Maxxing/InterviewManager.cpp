#include "InterviewManager.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UInterviewManager::UInterviewManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UInterviewManager::ChargerQuestionsDepuisJSON(FString SousDossierEtNomFichier)
{
	FString JsonString;
	// Pointe vers le dossier Content de ton projet
	FString CheminComplet = FPaths::ProjectContentDir() + SousDossierEtNomFichier;

	if (FFileHelper::LoadFileToString(JsonString, *CheminComplet))
	{
		// Convertit le texte JSON en structure C++
		return FJsonObjectConverter::JsonObjectStringToUStruct<FThemeQuiz>(JsonString, &ThemeActuel, 0, 0);
	}
	
	UE_LOG(LogTemp, Error, TEXT("Impossible de charger le fichier JSON : %s"), *CheminComplet);
	return false;
}

FQuestion UInterviewManager::ObtenirProchaineQuestion(bool bJoueurABluffe)
{
	FQuestion QuestionVide;
	
	// On stocke les INDEX des questions valides pour pouvoir les supprimer ensuite
	TArray<int32> IndexPossibles;
	
	for (int32 i = 0; i < ThemeActuel.questions.Num(); i++)
	{
		if (!bJoueurABluffe && ThemeActuel.questions[i].reservee_bluff) 
		{
			continue;
		}
		IndexPossibles.Add(i);
	}

	if (IndexPossibles.Num() > 0)
	{
		int32 IndexAleatoire = FMath::RandRange(0, IndexPossibles.Num() - 1);
		int32 IndexReel = IndexPossibles[IndexAleatoire];
		
		FQuestion QuestionChoisie = ThemeActuel.questions[IndexReel];
		
		// ON SUPPRIME LA QUESTION DE LA LISTE POUR NE PLUS LA TIRER
		ThemeActuel.questions.RemoveAt(IndexReel); 
		
		return QuestionChoisie;
	}

	return QuestionVide;
}

void UInterviewManager::TempsEcoule()
{
	UE_LOG(LogTemp, Warning, TEXT("Le temps est écoulé pour cette question !"));
	// On prévient le Blueprint (l'interface UI) que le temps est écoulé
	OnTempsEcouleEvent.Broadcast();
}

void UInterviewManager::DemarrerQuestion(bool bJoueurABluffe)
{
	// On pioche la question
	QuestionEnCours = ObtenirProchaineQuestion(bJoueurABluffe);

	if (QuestionEnCours.id.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Fin du quiz ou aucune question trouvée."));
		return;
	}

	// On s'assure qu'aucun timer n'est déjà en cours
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Entretien);

	// On lance le timer avec le temps spécifique à cette question (ex: 30, 25 ou 20 secondes)
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_Entretien, 
		this, 
		&UInterviewManager::TempsEcoule, 
		QuestionEnCours.temps_secondes, 
		false // false = ne boucle pas
	);
	
	UE_LOG(LogTemp, Warning, TEXT("Nouvelle question : %s (Temps: %d s)"), *QuestionEnCours.question, QuestionEnCours.temps_secondes);
}

bool UInterviewManager::VerifierReponse(int32 IndexReponse)
{
	// On arrête le timer car le joueur a répondu
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Entretien);

	// Sécurité anti-crash
	if (!QuestionEnCours.reponses.IsValidIndex(IndexReponse))
	{
		return false;
	}

	bool bEstCorrect = QuestionEnCours.reponses[IndexReponse].correcte;

	if (bEstCorrect)
	{
		// On calcule les points (ex: difficulté 2 = 20 points)
		ScoreActuel += (QuestionEnCours.difficulte * 10);
		UE_LOG(LogTemp, Warning, TEXT("Bonne réponse ! Score : %d"), ScoreActuel);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Mauvaise réponse. Explication : %s"), *QuestionEnCours.explication);
	}

	return bEstCorrect;
}
