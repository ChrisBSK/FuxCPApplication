#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "components/ColumnBox.h"
#include "components/ClickableTitle.h"
#include "components/SolverPriorityList.h"
#include "components/ArrowIconButton.h"
#include "components/VoiceWorkspace.h"

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

    // Connexions externes
    void setAppController(AppController* app_controller);
    void setLeftPanel(LeftPanel* panel);
    LeftPanel* getLeftPanel() const;

private:
    // Références externes
    LeftPanel* leftPanel = nullptr;
    AppController* appController = nullptr;

    // Zone de travail + colonne Search
    ColumnBox column1, column2, column3, column4, column5;
    ClickableTitle title1, title2, title3, title4, title5;
    VoiceWorkspace voiceWorkspace;



    // Ancienne colonne Solver Priorities.
    // Conservée pour pouvoir la réutiliser plus tard.
    SolverPriorityList solverPriorityList;
    ArrowIconButton movePriorityUpButton { ArrowIconButton::Direction::up };
    ArrowIconButton movePriorityDownButton { ArrowIconButton::Direction::down };

    // Boutons d'action
    juce::TextButton generateButton;
    juce::TextButton clearButton;

    // État visuel de la colonne Search
    int activeColumn = 0;
    juce::Rectangle<int> workspaceBounds;

    // Initialisation de l'interface
    void setupColumns();
    void setupTitles();
    void setupButtons();
    void setupSolverPriorities();

    // Interactions utilisateur
    void setupColumnInteractions();

    void updateActiveColumn(int index);

    // Layout interne
    void layoutSolverPriorities(juce::Rectangle<int> listBounds);
    void layoutButtons(juce::Rectangle<int> bottomArea);



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OptionsPanel)
};
