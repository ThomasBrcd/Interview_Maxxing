#pragma once

#include "CoreMinimal.h"
#include "InterviewDataTypes.generated.h"

USTRUCT(BlueprintType)
struct FReponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interview")
    FString texte;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interview")
    bool correcte = false;
};

USTRUCT(BlueprintType)
struct FQuestion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interview")
    FString id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interview")
    FString categorie;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interview")
    int32 difficulte = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interview")
    int32 temps_secondes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interview")
    FString question;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interview")
    TArray<FReponse> reponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interview")
    FString explication;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interview")
    FString etiquette_liee;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interview")
    bool reservee_bluff = false;
};

USTRUCT(BlueprintType)
struct FThemeQuiz
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interview")
    FString theme;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interview")
    TArray<FQuestion> questions;
};