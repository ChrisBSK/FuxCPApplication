#include "AppController.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include "../ui/leftPanel/LeftPanel.h"
#include "../model/ConstraintsSettings.h"


//==============================================================================
// CONSTRUCTEURS
//==============================================================================

AppController::AppController() = default;


//==============================================================================
// ACCÈS MODÈLE
//==============================================================================

CantusProblem& AppController::getProblem()
{
    return problem;
}

const CantusProblem& AppController::getProblem() const
{
    return problem;
}


//==============================================================================
// SYNCHRONISATION UI (LeftPanel <-> OptionsPanel)
//==============================================================================

std::vector<AppController::VoiceSettings>& AppController::getVoiceSettings()
{
    return voiceSettings;
}

const std::vector<AppController::VoiceSettings>& AppController::getVoiceSettings() const
{
    return voiceSettings;
}


//==============================================================================
// CONNEXION DES COMPOSANTS
//==============================================================================

void AppController::setLeftPanel(LeftPanel* panel)
{
    leftPanel = panel;
}

void AppController::setGenerationService(GenerationService* service)
{
    generationService = service;
}


//==============================================================================
// GÉNÉRATION
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
// CALLBACK THREAD → UI
//==============================================================================

/*void AppController::handleAsyncUpdate()
{
    // =========================
    // Résultat du solveur
    // =========================
    if (generationService == nullptr)
        return;

    if (generationService->getLastGenerationSuccess())
    {

        // Récupération du fichier MIDI
        juce::File file(generationService->getLastGeneratedMidiPath());


        // Envoi au LeftPanel (UI)
        if (file.existsAsFile() && leftPanel != nullptr)
        {
            leftPanel->onGenerationFinished(file);
        }


        // Feedback utilisateur
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            juce::String::fromUTF8("Résultat"),
            juce::String::fromUTF8("Une solution existe !"));
    }
    else
    {

        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            juce::String::fromUTF8("Résultat"),
            juce::String::fromUTF8("Aucune solution n'existe !"));
    }
}*/

void AppController::handleAsyncUpdate()
{
    if (generationService == nullptr)
        return;

    bool success = generationService->getLastGenerationSuccess();

    if (success)
    {
        juce::String midiPath = generationService->getLastGeneratedMidiPath();

        if (midiPath.isNotEmpty())
        {
            generationState.setProperty("midiFilePath", midiPath, nullptr);
        }

        generationState.setProperty("generationStatus", "completed", nullptr);

        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            juce::String::fromUTF8("Résultat"),
            juce::String::fromUTF8("Une solution a été trouvée."));
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

bool AppController::isGenerating() const
{
    if (generationService == nullptr)
        return false;

    return generationService->isGenerating();
}


void AppController::updateSettings(const ConstraintSettings& newSettings) {
    currentSettings = newSettings;
    problem.setSettings(currentSettings);  // Met à jour les paramètres et recalcule les coûts

    std::cout << "Settings updated. Leap Penalty: " << currentSettings.leapPenalty << std::endl;
    std::cout << "Melodic costs: ";
    for (int cost : problem.getMelodicCosts()) {
        std::cout << cost << " ";
    }
    std::cout << std::endl;
}