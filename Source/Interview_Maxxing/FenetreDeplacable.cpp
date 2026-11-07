#include "FenetreDeplacable.h"
#include "Blueprint/GameViewportSubsystem.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Framework/Application/SlateApplication.h"
#include "UObject/UObjectIterator.h"

TWeakObjectPtr<UFenetreDeplacable> UFenetreDeplacable::FenetreAuPremierPlan;

void UFenetreDeplacable::NativeConstruct()
{
	Super::NativeConstruct();

	// Les zones vides du widget plein écran laissent passer les clics vers ce qui est derrière
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (UWidget* Racine = GetRootWidget())
	{
		Racine->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	// Construct est rappelé à chaque passage au premier plan : la cascade n'est faite qu'à l'ouverture
	if (bPlacee)
	{
		return;
	}
	bPlacee = true;

	int32 AutresFenetres = 0;
	for (TObjectIterator<UFenetreDeplacable> It; It; ++It)
	{
		UFenetreDeplacable* Autre = *It;
		if (Autre != this && Autre->GetWorld() == GetWorld() && Autre->bPlacee && Autre->IsInViewport())
		{
			++AutresFenetres;
		}
	}

	DeplacerFenetre(DecalageCascade * static_cast<float>(AutresFenetres % FMath::Max(1, NombreCransCascade)));
	FenetreAuPremierPlan = this;
}

void UFenetreDeplacable::MettreAuPremierPlan()
{
	if (FenetreAuPremierPlan.Get() == this)
	{
		return;
	}
	FenetreAuPremierPlan = this;

	// Le viewport dessine ses widgets dans l'ordre d'ajout : on ré-ajoute la fenêtre pour la passer devant
	UGameViewportSubsystem* Viewport = UGameViewportSubsystem::Get();
	if (Viewport && IsInViewport())
	{
		const FGameViewportWidgetSlot Emplacement = Viewport->GetWidgetSlot(this);
		Viewport->RemoveWidget(this);
		Viewport->AddWidget(this, Emplacement);
	}
}

void UFenetreDeplacable::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Le déplacement lit directement la souris : il ne dépend pas d'une capture qui serait perdue
	// quand la fenêtre change de plan, et s'arrête dès que le bouton est relâché, où que soit le curseur
	if (bEnDeplacement)
	{
		if (BoutonGaucheEnfonce())
		{
			const FVector2D PositionSouris = PositionSourisLocale(MyGeometry);
			DeplacerFenetre(PositionSouris - DernierePositionSouris);
			DernierePositionSouris = PositionSouris;
		}
		else
		{
			bEnDeplacement = false;
			DonnerFocusAuChampSaisie();
		}
	}

	// Ré-empiler la fenêtre pendant un clic ferait perdre ce clic au champ de saisie ou au bouton :
	// on attend qu'il soit relâché
	if (bPremierPlanDemande && !BoutonGaucheEnfonce())
	{
		bPremierPlanDemande = false;
		if (IsInViewport() && FenetreAuPremierPlan.Get() != this)
		{
			MettreAuPremierPlan();
			DonnerFocusAuChampSaisie();
		}
	}
}

FReply UFenetreDeplacable::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	bPremierPlanDemande = true;
	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UFenetreDeplacable::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Clic gauche sur une zone de la fenêtre qui n'a pas consommé le clic (fond, historique) : on attrape la fenêtre
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bEnDeplacement = true;
		bPremierPlanDemande = false;
		DernierePositionSouris = PositionSourisLocale(InGeometry);
		MettreAuPremierPlan();
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UFenetreDeplacable::DeplacerFenetre(const FVector2D& Delta)
{
	if (Delta.IsNearlyZero())
	{
		return;
	}

	if (const UCanvasPanel* Canvas = Cast<UCanvasPanel>(GetRootWidget()))
	{
		for (UPanelSlot* Emplacement : Canvas->GetSlots())
		{
			if (UCanvasPanelSlot* SlotCanvas = Cast<UCanvasPanelSlot>(Emplacement))
			{
				SlotCanvas->SetPosition(SlotCanvas->GetPosition() + Delta);
			}
		}
	}
}

FVector2D UFenetreDeplacable::PositionSourisLocale(const FGeometry& MyGeometry)
{
	// Espace local du widget plein écran : l'échelle DPI est déjà prise en compte.
	// Le point attrapé ne peut pas sortir de l'écran, la fenêtre reste donc toujours récupérable.
	const FVector2D Taille = MyGeometry.GetLocalSize();
	const FVector2D Position = MyGeometry.AbsoluteToLocal(FSlateApplication::Get().GetCursorPos());
	return FVector2D(FMath::Clamp(Position.X, 0.0, Taille.X), FMath::Clamp(Position.Y, 0.0, Taille.Y));
}

bool UFenetreDeplacable::BoutonGaucheEnfonce()
{
	return FSlateApplication::IsInitialized() && FSlateApplication::Get().GetPressedMouseButtons().Contains(EKeys::LeftMouseButton);
}

void UFenetreDeplacable::DonnerFocusAuChampSaisie()
{
	if (ChampSaisie.IsNone() || !IsInViewport())
	{
		return;
	}
	if (UWidget* Champ = GetWidgetFromName(ChampSaisie))
	{
		Champ->SetKeyboardFocus();
	}
}
