#pragma once

#include "CoreMinimal.h"

class UWidgetTree;
class UTextBlock;
class UButton;
class UBorder;
class APlayerController;
class UUserWidget;

/**
 * Style commun des interfaces construites en C++ (écran titre, fins de partie, objectif...).
 * Mêmes couleurs et formes que le menu des paramètres et le message d'embauche.
 */
namespace StyleInterface
{
	FLinearColor Texte();
	FLinearColor TexteSecondaire();
	FLinearColor Accent();
	FLinearColor Reussite();
	FLinearColor Echec();
	FLinearColor FondPanneau();

	UTextBlock* CreerTexte(UWidgetTree* Arbre, const FText& Message, int32 Taille, bool bGras = false, const FLinearColor& Couleur = Texte());

	/** true si l'image de fenêtre Content/Ui/Menus/T_Cadre est présente : les panneaux deviennent des fenêtres Windows XP */
	bool CadrePersonnalise();

	/**
	 * Habille un panneau avec la fenêtre T_Cadre (9-slice). Les bords sont toujours dessinés à la taille de l'image :
	 * pour un petit panneau, réduire le panneau entier avec SetRenderScale. Marge : espace entre le bord blanc et le contenu.
	 * Ne fait rien si l'image est absente.
	 */
	void AppliquerCadre(UBorder* Panneau, const FMargin& Marge, float Echelle);

	/** Fond d'un encadré à l'intérieur d'un panneau (ex : « Dans la réalité ») */
	FLinearColor FondEncadre();

	/** true si les images de boutons (Content/Ui/Menus/T_Bouton_*) sont présentes dans le projet */
	bool BoutonsPersonnalises();

	/** Applique les images de boutons et un texte clair, si elles existent */
	void AppliquerStyleBouton(UButton* Bouton, UTextBlock* Libelle);

	/** Bouton clair (principal) ou sombre (secondaire) avec son libellé */
	UButton* CreerBouton(UWidgetTree* Arbre, const FText& Libelle, bool bPrincipal = true, int32 TailleTexte = 18);

	/** Panneau arrondi semi-opaque */
	UBorder* CreerPanneau(UWidgetTree* Arbre, const FMargin& Marge, const FLinearColor& Couleur = FondPanneau(), float Rayon = 14.f);

	/** Donne la souris au joueur et bloque le personnage pendant qu'une interface est ouverte */
	void PasserEnModeInterface(APlayerController* PC, UUserWidget* Focus);

	/** Rend le contrôle du personnage au joueur */
	void RevenirEnModeJeu(APlayerController* PC);
}
