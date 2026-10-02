#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InterviewDataTypes.h"
#include "InterviewManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTempsEcouleSignature);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INTERVIEW_MAXXING_API UInterviewManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInterviewManager();

	// Charge le JSON depuis le dossier Content
	UFUNCTION(BlueprintCallable, Category = "Interview")
	bool ChargerQuestionsDepuisJSON(FString SousDossierEtNomFichier);

	// Tire une question au sort en filtrant les questions de bluff si nécessaire
	UFUNCTION(BlueprintCallable, Category = "Interview")
	FQuestion ObtenirProchaineQuestion(bool bJoueurABluffe);

	// Stocke toutes les données chargées
	UPROPERTY(BlueprintReadOnly, Category = "Interview")
	FThemeQuiz ThemeActuel;

	// Score actuel du joueur
	UPROPERTY(BlueprintReadOnly, Category = "Interview")
	int32 ScoreActuel = 0;

	// La question en train d'être posée
	UPROPERTY(BlueprintReadOnly, Category = "Interview")
	FQuestion QuestionEnCours;

	UPROPERTY(BlueprintReadWrite, Category = "Interview")
    FTimerHandle TimerHandle_Entretien;

	// Compteurs
	UPROPERTY(BlueprintReadOnly, Category = "Interview")
	int32 QuestionsPosees = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Interview")
	int32 TotalQuestions = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Interview")
	int32 ScoreMaxPossible = 0;

	// File d'attente pré-calculée
	TArray<FQuestion> FileAttenteQuestions;

	UFUNCTION(BlueprintCallable, Category = "Interview")
	void InitialiserEntretien(bool bJoueurABluffe);

	// Démarre une nouvelle question et lance le timer
	UFUNCTION(BlueprintCallable, Category = "Interview")
	void DemarrerQuestion();

	// Vérifie la réponse choisie depuis l'interface (UI)
	UFUNCTION(BlueprintCallable, Category = "Interview")
	bool VerifierReponse(int32 IndexReponse);

	// Fonction appelée automatiquement si le temps est écoulé
	UFUNCTION(BlueprintCallable, Category = "Interview")
	void TempsEcoule();

	// Événement appelé quand le timer arrive à zéro
	UPROPERTY(BlueprintAssignable, Category = "Interview")
	FOnTempsEcouleSignature OnTempsEcouleEvent;
};