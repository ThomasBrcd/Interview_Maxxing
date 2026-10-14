#include "CVMathLibrary.h"
#include "Math/UnrealMathUtility.h"

void UCVMathLibrary::CalculerScoreFinal(UDataTable* DT_Etiquettes, UDataTable* DT_Mails, const TArray<FName>& EtiquettesChoisies, FName ID_Entreprise, int32& ScoreTotal, bool& bEstAccepte, TArray<FName>& BonnesEtiquettes, TArray<FName>& MoyennesEtiquettes, TArray<FName>& MauvaisesEtiquettes)
{
    ScoreTotal = 0;
    bEstAccepte = false;
    
    BonnesEtiquettes.Empty();
    MoyennesEtiquettes.Empty();
    MauvaisesEtiquettes.Empty();

    if (!DT_Etiquettes || !DT_Mails) 
    {
        return;
    }

    FString ContextString;
    FMailDataRow* CompanyData = DT_Mails->FindRow<FMailDataRow>(ID_Entreprise, ContextString);
    
    if (!CompanyData) 
    {
        return;
    }

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

            bool bHasNegativeStat = (TagData->Stats.Technique < 0 || TagData->Stats.Creativite < 0 || 
                                     TagData->Stats.Rigueur < 0 || TagData->Stats.Polyvalence < 0 || 
                                     TagData->Stats.Experience < 0);

            if (bHasNegativeStat)
            {
                MauvaisesEtiquettes.Add(EtiquetteID);
            }
            else if (TagData->Domaine == CompanyData->DomainePrincipal)
            {
                BonnesEtiquettes.Add(EtiquetteID);
            }
            else if (TagData->Domaine == EDomaineTech::General || TagData->Domaine == EDomaineTech::Aucun)
            {
                MoyennesEtiquettes.Add(EtiquetteID);
            }
            else
            {
                MauvaisesEtiquettes.Add(EtiquetteID);
            }
        }
    }

    float FinalScore = 0.0f;
    FinalScore += TotalTech * CompanyData->MultiplicateursStats.Technique;
    FinalScore += TotalCrea * CompanyData->MultiplicateursStats.Creativite;
    FinalScore += TotalRig  * CompanyData->MultiplicateursStats.Rigueur;
    FinalScore += TotalPoly * CompanyData->MultiplicateursStats.Polyvalence;
    FinalScore += TotalExp  * CompanyData->MultiplicateursStats.Experience;

    float PenaliteParErreur = 10.0f;
    FinalScore -= (MauvaisesEtiquettes.Num() * PenaliteParErreur);
    FinalScore = FMath::Max(0.0f, FinalScore);

    ScoreTotal = FMath::RoundToInt(FinalScore);

    bool bScoreAtteint = (ScoreTotal >= CompanyData->ScoreSeuilReussite);
    bool bAuMoinsUneBonne = (BonnesEtiquettes.Num() > 0);
    bool bPasTropDeFautes = (MauvaisesEtiquettes.Num() < 3); 

    bEstAccepte = (bScoreAtteint && bAuMoinsUneBonne && bPasTropDeFautes);
}