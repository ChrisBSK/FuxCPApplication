#pragma once

#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "components/ColumnBox.h"
#include "components/ClickableTitle.h"
#include "components/SolverPriorityList.h"
#include "components/ArrowIconButton.h"
#include "components/VoiceWorkspace.h"
#include "components/MinimizationModePanel.h"

class LeftPanel;
class AppController;

/*
//==============================================================================
  OptionsPanel

  Panneau de configuration de la génération.

  Permet de :
  - choisir les paramètres des voix
  - modifier les contraintes de génération
  - lancer la génération via le LeftPanel
  - synchroniser l'interface avec l'AppController
//==============================================================================
*/

class OptionsPanel : public juce::Component
{
public:
    OptionsPanel();

    // Rendu et disposition
    void paint(juce::Graphics&) override;
    void resized() override;

    // Met à jour l'affichage des voix selon le nombre sélectionné
    void setNumVoices(int numVoices);

    // Remet les contrôles de génération dans l'état initial.
    void clearGenerationInputs();

    // Limite de la grande zone de travail, en coordonnées de OptionsPanel.
    juce::Rectangle<int> getWorkspaceBounds() const;

    // Limite de la colonne gauche, en coordonnées de OptionsPanel.
    juce::Rectangle<int> getLeftPanelBounds() const;

    // Hauteur laissée libre en bas pour le clavier.
    void setLowerReservedHeight(int height);

    // Connexions externes
    void setAppController(AppController* app_controller);
    void setLeftPanel(LeftPanel* panel);
    LeftPanel* getLeftPanel() const;

private:
    // Références externes
    LeftPanel* leftPanel = nullptr;
    AppController* appController = nullptr;

    // Zone de travail
    VoiceWorkspace voiceWorkspace;
    MinimizationModePanel minimizationModePanel;

    ColumnBox workspaceColumn;
    ColumnBox searchColumn;



    // Ancienne colonne Solver Priorities.
    // Conservée pour pouvoir la réutiliser plus tard.
    SolverPriorityList solverPriorityList;
    ArrowIconButton movePriorityUpButton { ArrowIconButton::Direction::up };
    ArrowIconButton movePriorityDownButton { ArrowIconButton::Direction::down };

    // Boutons d'action
    juce::TextButton generateButton;
    juce::TextButton nextSolutionButton;
    juce::TextButton clearButton;

    // État visuel de la colonne Search
    bool showLexicographicPriorities = true;
    juce::Rectangle<int> workspaceBounds;
    juce::Rectangle<int> leftPanelBounds;
    int lowerReservedHeight = 0;

    // Initialisation de l'interface
    void setupColumns();
    void setupButtons();
    void setupSolverPriorities();
    void setupMinimizationModePanel();

    // Interactions utilisateur
    void updateSolverPriorityVisibility();

    // Applique au modèle la valeur envoyée par un slider de coût.
    void applyCostSliderValueToSettings(int counterpointIndex,
                                        VoiceWorkspace::CostSliderTarget target,
                                        double value);

    // Applique au modèle la shape et ses 4 valeurs envoyées par l'interface.
    void applyShapeToSettings(int counterpointIndex,
                              VoiceWorkspace::CostSliderTarget target,
                              int shapeId,
                              const std::vector<double>& values);

    // Layout interne
    void layoutMinimizationModePanel(juce::Rectangle<int> columnBounds);
    void layoutButtons(juce::Rectangle<int> buttonArea);



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OptionsPanel)
};
