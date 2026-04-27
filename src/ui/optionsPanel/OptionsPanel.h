#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "components/VoiceBox.h"
#include "components/ColumnBox.h"
#include "components/ClickableTitle.h"
#include "components/StyledLabel.h"


class LeftPanel;
class AppController;

/**
 * =============================================================================
 * OptionsPanel
 *
 * Rôle :
 * - Affichage des voix + paramètres
 * - Synchronisation UI avec LeftPanel
 * - Gestion du layout
 *
 * =============================================================================
 */
class OptionsPanel : public juce::Component
{
public:
    OptionsPanel();

    void paint(juce::Graphics&) override;
    void resized() override;

    // =========================
    // Synchronisation UI
    // =========================

    void setNumVoices(int numVoices);

    void setAppController(AppController * app_controller);

    // Hyper important car il permet de construire le leftpanel
    // et du coup d'être utilisé lors de la génération initiée
    // par OptionsPanel (generateButton.onCLick)
    //
    void setLeftPanel(LeftPanel* panel);
    LeftPanel* getLeftPanel() const;

private:
    LeftPanel* leftPanel = nullptr;
    AppController* appController = nullptr;

    // =========================
    // Colonnes
    // =========================

    ColumnBox column1, column2, column3, column4;
    ClickableTitle title1, title2, title3, title4;

    int activeColumn = 0;
    int hoveredColumn = 0;

    void updateActiveColumn(int index);

    // =========================
    // Voix
    // =========================

    VoiceBox box1 { "Cantus Firmus" };
    VoiceBox box2 { "Contrepoint 1" };
    VoiceBox box3 { "Contrepoint 2" };
    VoiceBox box4 { "Contrepoint 3" };

    // =========================
    // Contraintes mélodiques
    // =========================

    StyledLabel melodicMaxLeapLabel;
    juce::Slider melodicMaxLeapSlider;

    StyledLabel melodicStepBiasLabel;
    juce::Slider melodicStepBiasSlider;

    StyledLabel melodicRepetitionLabel;
    juce::ToggleButton melodicRepetitionToggle;

    StyledLabel melodicDirectionLabel;
    juce::ComboBox melodicDirectionBox;

    // =========================
    // Boutons
    // =========================

    juce::TextButton generateButton;
    juce::TextButton cancel;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OptionsPanel)
};