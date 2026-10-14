#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EtiquetteData.generated.h"

UENUM(BlueprintType)
enum class ECategorieEtiquette : uint8 { 
    Domaine, Experience, Formation, Competence, Loisir, Langue, Contact 
};

UENUM(BlueprintType)
enum class EDomaineTech : uint8 { 
    Aucun, General, Reseaux, Cyber, Web, Jeu3D, IA, Cloud, Embarque 
};

USTRUCT(BlueprintType)
struct FStatsCV 
{ 
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Technique = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Creativite = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Rigueur = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Polyvalence = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Experience = 0;
};

USTRUCT(BlueprintType)
struct FEtiquetteRow : public FTableRowBase 
{ 
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Texte;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) ECategorieEtiquette Categorie;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EDomaineTech Domaine;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FStatsCV Stats;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Prestige = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDepart = false;
};