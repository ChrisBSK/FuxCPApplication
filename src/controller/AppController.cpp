#include "AppController.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>

#include "../ui/leftPanel/leftPanel.h"
#include "../model/ConstraintsSettings.h"
#include "../model/ConstraintsApplier.h"
// =========================
// Constructeurs
// =========================


AppController::AppController()
{
}

AppController::AppController(const juce::String& title)
    : problem(title)
{
    generationState.setProperty("maxLeap", 5, nullptr);
}
juce::ValueTree& AppController::getGenerationState()
{
    return generationState;
}

const juce::ValueTree& AppController::getGenerationState() const
{
    return generationState;
}
// =========================
// Génération
// =========================

void AppController::startGeneration(const juce::String& outputPath)
{

    int maxLeap = (int) generationState.getProperty("maxLeap", 5);

    problem.getSettings().rules.maxLeap = maxLeap;

    auto costs = ConstraintApplier::apply(problem.getSettings());


    problem.setCostParameters(costs);
    problem.setBorrowMode(1);

    CantusProblem copyProblem = problem;
    // Lance la génération dans le thread worker
    bool started = generationService->startGeneration(copyProblem, outputPath, this);

    if (!started)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            juce::String::fromUTF8("Erreur"),
            juce::String::fromUTF8("Impossible de lancer la génération.\nPeut-être déjà en cours ?"));
    }
}

// =========================
// Accès modèle
// =========================

CantusProblem& AppController::getProblem()
{
    return problem;
}

const CantusProblem& AppController::getProblem() const
{
    return problem;
}

// =========================
// Callback thread → UI
// =========================

void AppController::handleAsyncUpdate()
{
    if (generationService->getLastGenerationSuccess())
    {
        juce::File file(generationService->getLastGeneratedMidiPath());

        if (file.existsAsFile() && leftPanel != nullptr)
        {
            leftPanel->onGenerationFinished(file);
        }

        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            juce::String::fromUTF8("Résultat"),
            juce::String::fromUTF8("Une solution existe !"));
    }
    else
    {
        juce::String errorMsg = generationService->getLastError();

        if (errorMsg.isEmpty())
            errorMsg = juce::String::fromUTF8("Aucune solution trouvée.");

        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            juce::String::fromUTF8("Résultat"),
            errorMsg);
    }
}