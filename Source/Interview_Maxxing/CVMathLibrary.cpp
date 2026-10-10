// Fill out your copyright notice in the Description page of Project Settings.


#include "CVMathLibrary.h"

void UCVMathLibrary::CalculerScoreFinal(UDataTable* DT_Etiquettes, UDataTable* DT_Mails, const TArray<FName>& EtiquettesChoisies, FName ID_Entreprise, int32& ScoreTotal, bool& bEstAccepte)
{
    ScoreTotal = 0;
    bEstAccepte = false;
    if (!DT_Etiquettes || !DT_Mails) return;

    FString ContextString;

    FMailDataRow* CompanyData = DT_Mails->FindRow<FMailDataRow>(ID_Entreprise, ContextString);
    if (!CompanyData) return;

    float TotalTech = 0.0f, TotalCrea = 0.0f, TotalRig = 0.0f, TotalPoly = 0.0f, TotalExp = 0.0f;

    for (const FName& EtiquetteID : EtiquettesChoisies)
    {
        FEtiquetteRow* TagData = DT_Etiquettes->FindRow<FEtiquetteRow>(EtiquetteID, ContextString);
        if (TagData)
        {
            TotalTech += TagData->Stats.Technique;
            TotalCrea += TagData->Stats.Creativite;
            TotalRig  += TagData->Stats.Rigueur;
            TotalPoly += TagData->Stats.Polyvalence;
            TotalExp  += TagData->Stats.Experience;
        }
    }

    float FinalScore = 0.0f;
    FinalScore += TotalTech * CompanyData->MultiplicateursStats.Technique;
    FinalScore += TotalCrea * CompanyData->MultiplicateursStats.Creativite;
    FinalScore += TotalRig  * CompanyData->MultiplicateursStats.Rigueur;
    FinalScore += TotalPoly * CompanyData->MultiplicateursStats.Polyvalence;
    FinalScore += TotalExp  * CompanyData->MultiplicateursStats.Experience;

    ScoreTotal = FMath::RoundToInt(FinalScore);
    bEstAccepte = (ScoreTotal >= CompanyData->ScoreSeuilReussite);
}