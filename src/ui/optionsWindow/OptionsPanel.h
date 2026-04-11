#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../controller/AppController.h"

/**
 * @brief Panneau d’options avancées pour la configuration des voix (Vue).
 *
 * Rôle :
 * - Affiche et permet de modifier les paramètres de chaque voix
 *   (espèce et type)
 * - Synchronise les contrôles UI avec le modèle partagé (VoiceSettings)
 * - Visualise les voix actives selon le nombre de voix sélectionné
 *
 * Responsabilités :
 * - Gérer l’affichage et l’organisation des VoiceBox
 * - Connecter les ComboBox au modèle (binding UI ↔ données)
 *
 * Ne contient PAS :
 * - de logique métier (règles musicales, génération)
 * - de traitement audio
 */

// ========= display composantes =====
class StyledLabel : public juce::Label
{
public:
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // fond vert
        g.setColour(juce::Colour(0xff2f4f4f));
        g.fillRect(bounds);

        // texte
        g.setColour(juce::Colours::white);
        g.setFont(getFont());

        g.drawText(getText(),
                   getLocalBounds(),
                   getJustificationType(),
                   true);
    }
};

// =============================
// VoiceBox
// =============================
class VoiceBox : public juce::Component
{
public:
    VoiceBox(const juce::String& name);

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool isActive = false;
    juce::ComboBox speciesBox;
    juce::ComboBox typeBox;

    AppController::VoiceSettings* settings = nullptr;


private:
    juce::Label title;


};

// =============================
// OptionsPanel
// =============================
class OptionsPanel : public juce::Component
{
public:
    OptionsPanel();

    void paint(juce::Graphics& g) override;
    void resized() override;
    void setNumVoices(int numVoices);

    void setVoiceSettings(std::vector<AppController::VoiceSettings> &settings);

    void updateVoice(int index, int species, int type);


private:
    class ColumnBox : public juce::Component
    {
    public:
        bool isActive = false;

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);

            // fond
            g.setColour(juce::Colours::darkgrey.darker(0.3f));
            g.fillRoundedRectangle(bounds, 10.0f);

            // contour
            if (isActive)
                g.setColour(juce::Colour(0xff2f4f4f)); // couleur colonne actives
            else
                g.setColour(juce::Colours::white.withAlpha(0.2f));

            g.drawRoundedRectangle(bounds, 10.0f, 2.0f);
        }
    };

ColumnBox column1, column2, column3, column4;

    class ClickableTitle : public juce::Label
    {
    public:
        std::function<void()> onClick;

        void mouseDown(const juce::MouseEvent&) override
        {
            if (onClick)
                onClick();
        }
    };

    ClickableTitle title1, title2, title3, title4;

    VoiceBox box1 { "Voix 1" };
    VoiceBox box2 { "Voix 2" };
    VoiceBox box3 { "Voix 3" };
    VoiceBox box4 { "Voix 4" };

    juce::TextButton generate;
    juce::TextButton cancel;

    // Composantes UI (widgets) - Melodic Constraints (2e colonne)

    StyledLabel melodicMaxLeapLabel;
    StyledLabel melodicStepBiasLabel;
    StyledLabel melodicRepetitionLabel;
    StyledLabel melodicDirectionLabel;

    juce::Slider melodicMaxLeapSlider;
    juce::Slider melodicStepBiasSlider;
    juce::ToggleButton melodicRepetitionToggle;
    juce::ComboBox melodicDirectionBox;

    // ===== Display column active =======
    int activeColumn = 0;
    void updateActiveColumn(int columnIndex);

    int hoveredColumn = 0; //posibilité d'extension




    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OptionsPanel)
};