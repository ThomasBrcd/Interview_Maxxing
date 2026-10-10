#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "MailData.h"
#include "EtiquetteData.h"
#include "CVMathLibrary.generated.h"

UCLASS()
class UCVMathLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "CV Score")
    static void CalculerScoreFinal(
        UDataTable* DT_Etiquettes, 
        UDataTable* DT_Mails, 
        const TArray<FName>& EtiquettesChoisies, 
        FName ID_Entreprise, 
        int32& ScoreTotal, 
        bool& bEstAccepte);
};