#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EtiquetteData.h"
#include "MailData.generated.h" 

// Structure pour les poids de l'entreprise (Combien vaut chaque stat chez eux)
USTRUCT(BlueprintType)
struct FPoidsStats 
{ 
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Technique = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Creativite = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Rigueur = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Polyvalence = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Experience = 1.0f;
};

// La structure complète pour la DataTable des mails
USTRUCT(BlueprintType)
struct FMailDataRow : public FTableRowBase 
{ 
    GENERATED_BODY()
    
    // UI - Textes
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText NomEntreprise;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText ObjetMail;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DescriptionCourte;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DescriptionLongue;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText CritereRecherche;
    
    // GAMEPLAY - Domaine (lié à EDomaineTech créé précédemment pour charger les bonnes questions)
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EDomaineTech DomainePrincipal;
    
    // GAMEPLAY - Calcul du score
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FPoidsStats MultiplicateursStats;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ScoreSeuilReussite = 100;
};