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
    // =========================
    // 1. Vérification sécurité
    // =========================
    if (generationService == nullptr)
        return;

    // =========================
    // 2. Injection paramètres solveur
    // =========================
    // (Ces paramètres définissent le comportement du solveur Fux)

    ConstraintSettings settings;

    settings.soft.melodic   = {0, 1, 1, 576, 2, 2, 2, 1};
    settings.soft.general   = {4, 1, 1, 2, 2, 2, 8, 1};
    settings.soft.specific  = {8, 4, 0, 2, 1, 8, 50};
    settings.soft.importance= {8,7,5,2,9,3,14,12,6,11,4,10,1,13};
    settings.borrowMode = 1;
    problem.setSettings(settings);


    // =========================
    // Copie du problème
    // =========================

    // On copie le modèle pour éviter tout accès concurrent
    // entre le thread UI et le thread de génération
    CantusProblem copyProblem = problem;

    // =========================
    // Lancement du thread
    // =========================
    bool started = generationService->startGeneration(copyProblem, outputPath, this);

    if (!started)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Erreur",
            "Impossible de lancer la génération.\nPeut-être déjà en cours ?");
    }
}


//==============================================================================
// CALLBACK THREAD → UI
//==============================================================================

void AppController::handleAsyncUpdate()
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
        // =========================
        // Gestion erreur / aucun résultat
        // =========================
        juce::String errorMsg = generationService->getLastError();

        if (errorMsg.isEmpty())
            errorMsg = juce::String::fromUTF8("Aucune solution trouvée.");

        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            juce::String::fromUTF8("Résultat"),
            errorMsg);
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