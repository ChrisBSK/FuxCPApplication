#include "AppController.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include "../ui/leftPanel/LeftPanel.h"
#include "../ui/optionsPanel/OptionsPanel.h"


/*
//==============================================================================
   AppController

  Coordonne les interactions entre l'UI,
  le modèle musical et le moteur de génération.

  Responsable de :
   - construire le problème à générer
   - lancer la génération
   - recevoir les résultats du solveur
   - notifier l'interface utilisateur
//==============================================================================
*/

//==============================================================================
// CONSTRUCTEUR  -  Initialise le contrôleur principal de l'application.
//==============================================================================
AppController::AppController() = default;

//==============================================================================
// ACCÈS AU MODÈLE COURANT EN ECRITURE
// Utilisé par l'UI pour construire ou modifier le problème.
//==============================================================================

CantusProblem& AppController::getProblem()
{
    return problem;
}

//==============================================================================
// ACCÈS AU MODÈLE COURANT EN LECTURE SEULE
//==============================================================================
const CantusProblem& AppController::getProblem() const
{
    return problem;
}


//==============================================================================
// SYNCHRONISATION UI (LeftPanel <-> OptionsPanel)
// Accès aux paramètres UI des voix.
//==============================================================================

std::vector<AppController::VoiceSettings>& AppController::getVoiceSettings()
{
    return voiceSettings;
}

//==============================================================================
// Accès en lecture seule aux paramètres UI des voix.
//==============================================================================
const std::vector<AppController::VoiceSettings>& AppController::getVoiceSettings() const
{
    return voiceSettings;
}


//==============================================================================
// CONNEXION DES COMPOSANTS
//==============================================================================

//==============================================================================
// Connexion au LeftPanel
// Permet de notifier l'UI après la génération.
//==============================================================================

void AppController::setLeftPanel(LeftPanel* panel)
{
    leftPanel = panel;
}

//==============================================================================
// Connexion au service de génération.
// Le service exécute le solveur dans un thread séparé.
//==============================================================================
void AppController::setGenerationService(GenerationService* service)
{
    generationService = service;
}


//==============================================================================
// GÉNÉRATION
//==============================================================================

//==============================================================================
// Lance une génération.
// Vérifie l'état courant, copie le problème, puis démarre le thread solveur.
//==============================================================================
void AppController::startGeneration(const juce::String& outputPath)
{
    generationState.setProperty("generationStatus", "idle", nullptr);

    if (generationService == nullptr)
    {
        generationState.setProperty("generationError",
                                    juce::String::fromUTF8("Service de génération indisponible."),
                                    nullptr);

        generationState.setProperty("generationStatus", "error", nullptr);
        return;
    }

    if (problem.isEmpty())
    {
        generationState.setProperty("generationError",
                                    juce::String::fromUTF8("Le problème est vide.\n\nEntrez un problème complet."),
                                    nullptr);

        generationState.setProperty("generationStatus", "warning", nullptr);
        return;
    }

    generationState.setProperty("generationStatus", "generating", nullptr);

    CantusProblem copyProblem = problem;

    bool started = generationService->startGeneration(copyProblem, outputPath, this);

    if (!started)
    {
        generationState.setProperty("generationError",
                                    generationService->getLastError(),
                                    nullptr);

        generationState.setProperty("generationStatus",
                                    generationService->isInputValidationError()
                                        ? "warning"
                                        : "error",
                                    nullptr);
    }
}

//==============================================================================
// CALLBACK THREAD --> UI

// Callback de fin de génération.
// Appelé sur le thread UI après la fin du solveur.
// Affiche le résultat et met à jour generationState.
//==============================================================================
void AppController::handleAsyncUpdate()
{
    if (generationService == nullptr)
        return;

    bool success = generationService->getLastGenerationSuccess();

    if (success)
    {
        juce::String midiPath = generationService->getLastGeneratedMidiPath();

        juce::AlertWindow::showOkCancelBox(
            juce::AlertWindow::QuestionIcon,
            juce::String::fromUTF8("Solution trouvée"),
            juce::String::fromUTF8("Une solution existe.\n\nVoulez-vous générer la solution ?"),
            juce::String::fromUTF8("Oui"),
            juce::String::fromUTF8("Non"),
            nullptr,
            juce::ModalCallbackFunction::create(
                [this, midiPath](int result)
                {
                    if (result == 1 && midiPath.isNotEmpty())
                    {
                        generationState.setProperty("midiFilePath", midiPath, nullptr);
                        generationState.setProperty("generationStatus", "completed", nullptr);
                    }
                }));
    }
    else
    {
        generationState.setProperty("generationError",
                                    generationService->getLastError(),
                                    nullptr);

        generationState.setProperty("generationStatus",
                                    generationService->isInputValidationError()
                                        ? "warning"
                                        : "error",
                                    nullptr);

        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            juce::String::fromUTF8("Résultat"),
            juce::String::fromUTF8("Aucune solution n'existe."));
    }
}

//==============================================================================
// Met à jour les paramètres d'une voix de contrepoint.
// Appelé lorsque l'utilisateur change l'espèce ou le type.
//==============================================================================
void AppController::updateVoice(int index, int species, int type)
{
    if (index < 0)
        return;

    if (index >= (int)voiceSettings.size())
        return;
    // Override les valeurs par défaut des voix
    voiceSettings[index].species = species; // 1 par défaut
    voiceSettings[index].type    = type; // 0 par défaut
}

//==============================================================================
// Indique si une génération est actuellement en cours.
//==============================================================================
bool AppController::isGenerating() const
{
    if (generationService == nullptr)
        return false;

    return generationService->isGenerating();
}


//==============================================================================
// tentative de modification des sliders
//==============================================================================
void AppController::updateSettings(const ConstraintSettings& newSettings) {
    currentSettings = newSettings;
    problem.setSettings(currentSettings);  // Met à jour les paramètres et recalcule les coûts

    /*std::cout << "Settings updated. Leap Penalty: " << currentSettings.leapPenalty << std::endl;
    std::cout << "Melodic costs: ";
    for (int cost : problem.getMelodicCosts()) {
        std::cout << cost << " ";
    }
    std::cout << std::endl;*/
}
