#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "../service/GenerationService.h"

#include "../model/CantusFirmus.h"

/**
 * @brief Composant principal de l'UI.
 *
 * Contient :
 *  - un label (titre)
 *  - un champ de saisie (TextEditor)
 *  - un label de sortie qui reflète le texte saisi
 */
class MainComponent : public juce::Component
{
public:
    MainComponent();


    void paint (juce::Graphics& g) override;
    void resized() override;

private:

/*
    /// Label descriptif pour le champ texte.
    juce::Label inputLabel;
    /// Champ de saisie utilisateur.
    juce::TextEditor inputEditor;
    /// Label qui affiche le texte saisi (sortie).
    juce::Label outputLabel;
    /// Met à jour outputLabel à partir du texte de inputEditor.
    void refreshOutputFromInput();

    /// Label descriptif pour le 2e champ texte.
    juce::Label inputLabel2;

    /// 2e champ de saisie utilisateur.
    juce::TextEditor inputEditor2;

    /// 2e label qui affiche le texte saisi (sortie).
    juce::Label outputLabel2;

    /// Met à jour outputLabel2 à partir du texte de inputEditor2.
    void refreshOutputFromInput2();

    /// Bouton d'action (carré) en bas.
    juce::TextButton actionButton;

    /// Callback appelé au clic du bouton.
    void onActionButtonClicked();

    void handleGenerationFinished();

    GenerationService generationService;

    static std::vector<int> parseIntList(const juce::String& text, bool& ok);
    */

    // ================= UI =================

    juce::Label cfLabel;
    juce::TextEditor cfEditor;

    juce::Label speciesLabel;
    juce::ComboBox speciesBox;

    juce::Label voicesLabel;
    juce::ComboBox voicesBox;

    juce::TextButton generateButton;

    // ================= Service =================

    GenerationService generationService;

    // ================= Helpers =================

    std::vector<int> parseCantusFirmus(const juce::String& text);

    // Zone dédiée aux messages de statut
    juce::Label statusTitleLabel;  // "Statut:"
    juce::Label statusMessageLabel; // Le message lui-même



    juce::String outputPathToGenerate;

    // Bridge optionnel vers le controller
    //AppController* appController = nullptr;

    // Locks & état
    juce::CriticalSection callbackLock;
    std::atomic<bool> generationSuccess { false };
    juce::String lastError;

    juce::File midiOutFileToGenerate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

