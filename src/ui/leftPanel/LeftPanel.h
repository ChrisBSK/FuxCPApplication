#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

#include "../../service/GenerationService.h"
#include "../optionsPanel/OptionsPanel.h"

/*
==============================================================================
    LeftPanel

    Rôle :
    Ce composant représente l’interface principale d’entrée utilisateur.

    Responsabilités :
    - Saisir le Cantus Firmus (suite de notes MIDI ou noms de notes)
    - Choisir le nombre de voix (cantus firmus inclus)
    - Configurer les contrepoints (espèce et type) via des ComboBox
    - Construire un problème musical à partir des entrées utilisateur
    - Déclencher la génération via le AppController
    - Afficher le fichier MIDI généré et permettre son drag & drop

    IMPORTANT :
    - Le Cantus Firmus n’est PAS une voix
    - Les espèces et types ne concernent QUE les contrepoints
    - Aucune logique de génération ou de résolution ici (UI uniquement)

==============================================================================
*/

class AppController;
class OptionsPanel;

// ======================================================
// Composant Drag & Drop pour fichier MIDI
// ======================================================
class MidiFileItem : public juce::Component
{
public:
    juce::File file;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        g.setColour(juce::Colours::darkgrey.brighter());
        g.fillRoundedRectangle(bounds, 6.0f);

        g.setColour(juce::Colours::white);
        g.drawText("MIDI",
                   getLocalBounds(),
                   juce::Justification::centred);
    }

    void mouseDrag(const juce::MouseEvent&) override
    {
        if (file.existsAsFile())
        {
            juce::DragAndDropContainer::performExternalDragDropOfFiles(
                { file.getFullPathName() }, true);
        }
    }

    void mouseDown(const juce::MouseEvent&) override
    {
        // nécessaire pour initier le drag
    }
};

// ======================================================
// LeftPanel : UI principale (entrée utilisateur)
// ======================================================
class LeftPanel : public juce::Component
{
public:
    explicit LeftPanel(AppController& controller);

    void paint(juce::Graphics &g) override;

    ~LeftPanel() override = default;

    // =========================
    // JUCE
    // =========================
    //void paint(juce::Graphics&) override;
    void resized() override;

    // =========================
    // Interaction utilisateur
    // =========================
    void triggerGeneration();

    void showAlert(juce::AlertWindow::AlertIconType icon,
                   const juce::String& title,
                   const juce::String& message);

    // =========================
    // Cantus Firmus
    // =========================
    /*juce::String getCantusText() const;
    void setCantusText(const juce::String& newText);*/

    // =========================
    // Liaison avec OptionsPanel
    // =========================
    void setOptionsPanel(OptionsPanel* panel)
    {
        optionsPanel = panel;
    }


    // =========================
    // Callback après génération
    // =========================
    void onGenerationFinished(const juce::File& file);

    void prepareOutputFile();

    void addNoteFromKeyboard(int midiNote);
    void updateCantusDisplay();

    GenerationService& getGenerationService()
    {
        return generationService;
    }





private:
    // =========================
    // Références
    // =========================
    AppController& appController;
    OptionsPanel* optionsPanel = nullptr;

    // =========================
    // UI - Cantus Firmus
    // =========================
    juce::TextEditor cfInput;
    juce::Label cfInputLabel;

    // =========================
    // UI - Nombre de voix
    // =========================
    juce::ComboBox numVoicesCB;
    juce::Label numVoicesCBLabel;


    // =========================
    // Drag & Drop MIDI
    // =========================
    juce::File midiOutFileToGenerate;
    std::unique_ptr<MidiFileItem> midiItem;

    // =========================
    // Helpers UI
    // =========================
    void updateVoiceSpeciesUI(int numVoices);

    GenerationService generationService;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LeftPanel)
};