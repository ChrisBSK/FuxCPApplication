#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

#include "../../service/GenerationService.h"
#include "../optionsPanel/OptionsPanel.h"


/*
==============================================================================
    LeftPanel

    Panneau principal de saisie utilisateur. (sur la gauche de l'interface)

    Rôle :
    - récupérer le Cantus Firmus
    - choisir le nombre de voix
    - transmettre les paramètres de contrepoint au modèle
    - lancer la génération via l'AppController
    - afficher le fichier MIDI généré dans la zone de drag & drop

    Ce composant reste une couche UI :
    il ne résout pas le problème et ne contient pas de logique solveur.
==============================================================================
*/

class AppController;
class OptionsPanel;

/*
// ======================================================
   Composant Drag & Drop pour fichier MIDI

   MidiFileItem: Élément visuel représentant le fichier MIDI généré.
   Peut être glissé vers une application externe.
//==============================================================================
*/
class MidiFileItem : public juce::Component
{
public:
    juce::File file;

    // Dessine l'icône MIDI affichée dans la zone de sortie.
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

    // Lance le drag & drop externe du fichier MIDI.
    void mouseDrag(const juce::MouseEvent&) override
    {
        if (file.existsAsFile())
        {
            juce::DragAndDropContainer::performExternalDragDropOfFiles(
                { file.getFullPathName() }, true);
        }
    }

    // Présent pour initialiser correctement le geste de drag.
    void mouseDown(const juce::MouseEvent&) override{}
};

/*
//==============================================================================
   LeftPanel

   Interface d'entrée et de sortie principale.
   Construit le problème musical depuis l'UI et affiche le résultat généré.
//==============================================================================
*/
class LeftPanel : public juce::Component, private juce::ValueTree::Listener
{
public:
    // Construction
    explicit LeftPanel(AppController& controller);
    ~LeftPanel() override;

    // Rendu et Layout
    void paint(juce::Graphics &g) override;
    void resized() override;

    // Lance la construction du problème puis la génération.
    void triggerGeneration();

    // Affiche une boîte de dialogue utilisateur.
    void showAlert(juce::AlertWindow::AlertIconType icon,
                   const juce::String& title,
                   const juce::String& message);

    // Connecte le panneau d'options associé.
    void setOptionsPanel(OptionsPanel* panel)
    {
        optionsPanel = panel;
    }

    // Affiche le fichier MIDI généré dans la zone de drag & drop.
    void onGenerationFinished(const juce::File& file);

    // Prépare le chemin du futur fichier MIDI.
    void prepareOutputFile();

    // Ajoute une note jouée depuis le clavier virtuel au Cantus Firmus.
    void addNoteFromKeyboard(int midiNote);

    // Met à jour l'affichage textuel du Cantus Firmus.
    void updateCantusDisplay();

    /*
    //==============================================================================
      --> Donne accès au service de génération possédé par le LeftPanel. (LANCE LA GENERATION)

       Permet à l'AppController de déclencher une génération
       sans posséder directement le service.
    //==============================================================================
    */
    GenerationService& getGenerationService()
    {
        return generationService;
    }

    /*
    //==============================================================================
     --> Connecte le LeftPanel à l'état de génération.

         Utilisé pour être notifié automatiquement lorsqu'une
         génération démarre, échoue ou se termine.
         Connecte le LeftPanel à l'état de génération de l'AppController.
    //==============================================================================
    */
    void connectToGenerationState(juce::ValueTree state);




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
    // Synchronisation UI
    // =========================

    // Adapte l'interface des contrepoints au nombre de voix choisi.
    void updateVoiceSpeciesUI(int numVoices);

    // =========================
    // Génération
    // =========================

    // Service chargé d'exécuter la génération.
    GenerationService generationService;

    // État de génération partagé avec l'AppController.
    juce::ValueTree generationState;

    // Réagit aux changements d'état de la génération.
    void valueTreePropertyChanged(juce::ValueTree& tree,
                                  const juce::Identifier& property) override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LeftPanel)
};