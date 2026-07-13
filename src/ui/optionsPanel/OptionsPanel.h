#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "components/VoiceBox.h"
#include "components/ColumnBox.h"
#include "components/ClickableTitle.h"
#include "components/ParameterColumn.h"
#include "components/ParameterFactory.h"
#include "components/SolverPriorityList.h"
#include "components/ArrowIconButton.h"

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

    // Connexions externes
    void setAppController(AppController* app_controller);
    void setLeftPanel(LeftPanel* panel);
    LeftPanel* getLeftPanel() const;

private:
    // Références externes
    LeftPanel* leftPanel = nullptr;
    AppController* appController = nullptr;

    // Colonnes principales
    ColumnBox column1, column2, column3, column4, column5;
    ClickableTitle title1, title2, title3, title4, title5;

    // Voix affichées dans la première colonne
    VoiceBox box1 { "Cantus Firmus" };
    VoiceBox box2 { "Contrepoint 1" };
    VoiceBox box3 { "Contrepoint 2" };
    VoiceBox box4 { "Contrepoint 3" };

    // Paramètres configurables par colonne
    ParameterColumn basicColumn;
    ParameterColumn melodicColumn;
    ParameterColumn harmonicColumn;
    ParameterColumn otherColumn;
    ParameterColumn solverColumn;

    // Repères visuels du vecteur importance envoyé au solveur
    SolverPriorityList solverPriorityList;
    ArrowIconButton movePriorityUpButton { ArrowIconButton::Direction::up };
    ArrowIconButton movePriorityDownButton { ArrowIconButton::Direction::down };

    // Boutons d'action
    juce::TextButton generateButton;
    juce::TextButton cancel;

    // État visuel des colonnes
    int activeColumn = 0;
    int hoveredColumn = 0;

    // -1 = aucune voix sélectionnée.
    int selectedVoiceIndex = -1;

    // Initialisation de l'interface
    void setupColumns();
    void setupTitles();
    void setupVoiceBoxes();
    void setupButtons();
    void setupBasicControls();
    void setupMelodicControls();
    void setupHarmonicControls();
    void setupSolverPriorities();

    // Interactions utilisateur
    void setupColumnInteractions();
    void setupHover(ClickableTitle& title,
                    ColumnBox& column,
                    int index);

    void updateActiveColumn(int index);
    void updateSelectedVoiceVisuals();

    // Layout interne
    void layoutVoiceColumn(juce::Rectangle<int> bounds);
    void layoutSolverPriorities(juce::Rectangle<int> listBounds,
                                juce::Rectangle<int> arrowBounds);
    void layoutButtons(juce::Rectangle<int> bottomArea);

    // Ajoute un paramètre UI dans une colonne donnée
    template <typename ControlType>
    ControlType* addParameter(ParameterColumn& column,
                              const juce::String& label,
                              std::unique_ptr<ControlType> control)
    {
        auto* controlPtr = control.get();

        column.addParameter(*this, label, std::move(control));

        return controlPtr;
    }

    juce::Slider* addSliderParameter(ParameterColumn& column,
                                     const juce::String& label,
                                     double min,
                                     double max,
                                     double interval,
                                     double defaultValue,
                                     const juce::String& leftEndpoint,
                                     const juce::String& rightEndpoint)
    {
        auto slider = ParameterFactory::slider(min, max, interval, defaultValue);
        auto* sliderPtr = slider.get();

        column.addParameter(*this,
                            label,
                            std::move(slider),
                            BoxParameter::EndpointLabels { leftEndpoint, rightEndpoint });

        return sliderPtr;
    }

    OnOffSwitchButton* addSwitchParameter(ParameterColumn& column,
                                          const juce::String& label,
                                          bool defaultValue)
    {
        auto button = ParameterFactory::onOffSwitch(defaultValue);
        auto* buttonPtr = button.get();

        column.addParameter(*this, label, std::move(button));

        return buttonPtr;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OptionsPanel)
};
