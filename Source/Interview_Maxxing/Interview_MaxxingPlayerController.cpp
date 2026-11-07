// Copyright Epic Games, Inc. All Rights Reserved.


#include "Interview_MaxxingPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Interview_MaxxingCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "Interview_Maxxing.h"
#include "MenuParametres.h"
#include "Widgets/Input/SVirtualJoystick.h"

AInterview_MaxxingPlayerController::AInterview_MaxxingPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AInterview_MaxxingCameraManager::StaticClass();

	MenuParametresClass = UMenuParametres::StaticClass();
}

void AInterview_MaxxingPlayerController::BeginPlay()
{
	Super::BeginPlay();

	
	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogInterview_Maxxing, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AInterview_MaxxingPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}

		// Menu des paramètres : Échap en jeu, P en plus car Échap arrête le PIE dans l'éditeur
		for (const FKey& Touche : { EKeys::Escape, EKeys::P })
		{
			FInputKeyBinding& Binding = InputComponent->BindKey(Touche, IE_Pressed, this, &AInterview_MaxxingPlayerController::BasculerMenuParametres);
			Binding.bExecuteWhenPaused = true;
		}
	}

}

void AInterview_MaxxingPlayerController::BasculerMenuParametres()
{
	if (MenuParametres)
	{
		MenuParametres->Fermer();
		return;
	}

	// Seulement en déplacement libre : les autres interfaces (PC, entretien, récap...) gèrent déjà la souris
	if (bShowMouseCursor || !MenuParametresClass)
	{
		return;
	}

	MenuParametres = CreateWidget<UMenuParametres>(this, MenuParametresClass);
	if (!MenuParametres)
	{
		return;
	}

	MenuParametres->OnFerme.AddDynamic(this, &AInterview_MaxxingPlayerController::OnMenuParametresFerme);
	MenuParametres->SetProposerMenuPrincipal(true);
	MenuParametres->AddToViewport(100);

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(MenuParametres->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	bShowMouseCursor = true;

	SetPause(true);
}

void AInterview_MaxxingPlayerController::OnMenuParametresFerme()
{
	MenuParametres = nullptr;

	SetPause(false);
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}

bool AInterview_MaxxingPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
