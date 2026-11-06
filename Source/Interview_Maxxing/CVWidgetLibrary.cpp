#include "CVWidgetLibrary.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"

void UCVWidgetLibrary::AjusterTexteAuCadre(UWidget* Cadre, UTextBlock* Texte, bool bFondSombre, float TailleMax, float TailleMin, float MargeInterieure)
{
    if (!Cadre || !Texte)
    {
        return;
    }

    // Texte clair sur fond sombre, texte foncé sur fond clair
    Texte->SetColorAndOpacity(FSlateColor(bFondSombre ? FLinearColor::White : FLinearColor(0.02f, 0.02f, 0.02f)));

    // Taille du cadre : on lit le slot du Canvas, fiable même si le design n'a pas encore été affiché
    FVector2D TailleCadre = FVector2D::ZeroVector;
    if (const UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Cadre->Slot))
    {
        TailleCadre = CanvasSlot->GetSize();
    }
    else
    {
        TailleCadre = Cadre->GetCachedGeometry().GetLocalSize();
    }

    const FVector2D Disponible = TailleCadre - FVector2D(MargeInterieure);
    if (Disponible.X <= 0.f || Disponible.Y <= 0.f)
    {
        return;
    }

    Texte->SetAutoWrapText(true);
    AjusterTexteDansZone(Texte, Disponible.X, Disponible.Y, TailleMax, TailleMin);
}

void UCVWidgetLibrary::AjusterTexteDansZone(UTextBlock* Texte, float Largeur, float Hauteur, float TailleMax, float TailleMin)
{
    if (!Texte || Largeur <= 0.f || Hauteur <= 0.f)
    {
        return;
    }

    Texte->SetWrapTextAt(Largeur);

    // On part de la taille max et on réduit la police jusqu'à ce que le texte tienne dans la hauteur
    FSlateFontInfo Police = Texte->GetFont();
    for (float Taille = TailleMax; Taille >= TailleMin; Taille -= 1.f)
    {
        Police.Size = Taille;
        Texte->SetFont(Police);
        Texte->ForceLayoutPrepass();

        if (Texte->GetDesiredSize().Y <= Hauteur)
        {
            break;
        }
    }
}
