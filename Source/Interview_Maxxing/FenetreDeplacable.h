#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FenetreDeplacable.generated.h"

/**
 * Fenêtre que le joueur peut déplacer à la souris (ex : les terminaux du test réseau).
 *
 * Le widget occupe tout l'écran mais seule la fenêtre dessinée dans son CanvasPanel racine
 * réagit à la souris. Les enfants du CanvasPanel sont déplacés ensemble :
 * - à l'ouverture, les fenêtres s'empilent en cascade pour ne pas se recouvrir exactement ;
 * - un clic sur la fenêtre la passe au premier plan ;
 * - un clic maintenu sur une zone libre de la fenêtre la déplace, le point attrapé reste sous le curseur.
 */
UCLASS(Abstract)
class INTERVIEW_MAXXING_API UFenetreDeplacable : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Passe la fenêtre devant les autres fenêtres de l'écran */
	UFUNCTION(BlueprintCallable, Category = "Fenetre")
	void MettreAuPremierPlan();

protected:
	/** Nom du champ de saisie qui reprend le focus clavier après un clic sur la fenêtre (vide = aucun) */
	UPROPERTY(EditDefaultsOnly, Category = "Fenetre")
	FName ChampSaisie = TEXT("Input_Commande");

	/** Décalage entre deux fenêtres ouvertes l'une après l'autre */
	UPROPERTY(EditDefaultsOnly, Category = "Fenetre")
	FVector2D DecalageCascade = FVector2D(36.f, 36.f);

	/** Nombre de crans de cascade avant de revenir à la position de départ */
	UPROPERTY(EditDefaultsOnly, Category = "Fenetre", meta = (ClampMin = "1"))
	int32 NombreCransCascade = 6;

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	/** Déplace tous les éléments du CanvasPanel racine */
	void DeplacerFenetre(const FVector2D& Delta);

	/** Position du curseur dans l'espace du widget, bornée à l'écran */
	static FVector2D PositionSourisLocale(const FGeometry& MyGeometry);

	static bool BoutonGaucheEnfonce();

	void DonnerFocusAuChampSaisie();

	bool bPlacee = false;
	bool bEnDeplacement = false;
	/** Clic dans la fenêtre (champ de saisie, bouton) : passage au premier plan une fois le clic relâché */
	bool bPremierPlanDemande = false;
	FVector2D DernierePositionSouris = FVector2D::ZeroVector;

	/** Dernière fenêtre passée au premier plan (évite de la ré-empiler à chaque clic) */
	static TWeakObjectPtr<UFenetreDeplacable> FenetreAuPremierPlan;
};
