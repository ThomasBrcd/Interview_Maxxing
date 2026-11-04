#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CVWidgetLibrary.generated.h"

class UWidget;
class UTextBlock;

UCLASS()
class UCVWidgetLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Adapte le texte d'une étiquette posée dans un trou du CV :
     * couleur lisible selon le fond, retour à la ligne à la largeur du cadre,
     * et police réduite jusqu'à ce que le texte tienne en hauteur.
     */
    UFUNCTION(BlueprintCallable, Category = "CV|Etiquette", meta = (AdvancedDisplay = "TailleMax,TailleMin,MargeInterieure"))
    static void AjusterTexteAuCadre(UWidget* Cadre, UTextBlock* Texte, bool bFondSombre, float TailleMax = 12.f, float TailleMin = 7.f, float MargeInterieure = 20.f);

    /**
     * Fait tenir un texte dans une zone fixe : retour à la ligne à la largeur donnée,
     * taille de police max par défaut, réduite seulement si le texte dépasse en hauteur.
     */
    UFUNCTION(BlueprintCallable, Category = "UI|Texte")
    static void AjusterTexteDansZone(UTextBlock* Texte, float Largeur, float Hauteur, float TailleMax = 28.f, float TailleMin = 12.f);
};
