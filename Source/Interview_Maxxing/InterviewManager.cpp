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

void UInterviewManager::InitialiserEntretien(bool bJoueurABluffe)
{
	ScoreActuel = 0;
	QuestionsPosees = 0;
	ScoreMaxPossible = 0;
	TotalQuestions = bJoueurABluffe ? 15 : 10;
	FileAttenteQuestions.Empty();
	
	TArray<FQuestion> QuestionsValides;
	
	// 1. Filtrer les questions de bluff
	for (const FQuestion& Q : ThemeActuel.questions)
	{
		if (!bJoueurABluffe && Q.reservee_bluff) continue;
		QuestionsValides.Add(Q);
	}
	
	// 2. Mélanger la liste entière (Fisher-Yates)
	for (int32 i = QuestionsValides.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		QuestionsValides.Swap(i, j);
	}
	
	// 3. Couper la liste pour ne garder que le nombre ciblé
	if (QuestionsValides.Num() > TotalQuestions)
	{
		QuestionsValides.SetNum(TotalQuestions);
	}
	else
	{
		TotalQuestions = QuestionsValides.Num(); // Sécurité si le JSON est trop court
	}
	
	// 4. Trier cette sélection par difficulté pour l'évolution graduelle
	QuestionsValides.Sort([](const FQuestion& A, const FQuestion& B) {
		return A.difficulte < B.difficulte;
	});
	
	// 5. Calculer le score max et remplir la file d'attente
	for (const FQuestion& Q : QuestionsValides)
	{
		FileAttenteQuestions.Add(Q);
		ScoreMaxPossible += (Q.difficulte * 10);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Entretien prêt : %d questions. Score Max : %d"), TotalQuestions, ScoreMaxPossible);
}

void UInterviewManager::DemarrerQuestion()
{
	UE_LOG(LogTemp, Error, TEXT("---> DemarrerQuestion a ete appele ! <---"));
	if (FileAttenteQuestions.Num() == 0)
	{
		// Plus de questions, on vide la structure pour signaler la fin à l'UI
		QuestionEnCours = FQuestion(); 
		return;
	}

	// On prend la première question de la file et on l'enlève
	QuestionEnCours = FileAttenteQuestions[0];
	FileAttenteQuestions.RemoveAt(0);
	
	QuestionsPosees++;

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Entretien);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Entretien, this, &UInterviewManager::TempsEcoule, QuestionEnCours.temps_secondes, false);
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
