#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "../model/CantusProblem.h"
#include "../service/GenerationService.h"

/*
//==============================================================================
   AppController

   Contrôleur principal de l'application.

   Assure la communication entre l'interface utilisateur,
   le modèle (CantusProblem) et le service de génération.
//==============================================================================
*/

// Forward declaration
class LeftPanel;


class AppController : public juce::AsyncUpdater,
                      private juce::Timer
{
public:
    AppController();
    ~AppController() override;
    //explicit AppController(const juce::String& title);

    // =========================
    // Génération
    // =========================

    /**
     * Lance la génération d'un problème
     * - prend le modèle courant
     * - déclenche le GenerationService (thread)
     */
    void startGeneration(const juce::String& outputPath);

    /**
     * Lance la solution suivante du dernier problème généré.
     */

    void startNextSolution(const juce::String& outputPath);


    // =========================
    // Accès modèle
    // =========================

    /**
     * Accès en écriture au problème
     * utilisé par le LeftPanel pour construire le problème
     */
    CantusProblem& getProblem();

    /**
     * Accès en lecture seule
     */
    const CantusProblem& getProblem() const;


    // =========================
    // Synchronisation UI
    // =========================

    /**
     * Structure intermédiaire utilisée UNIQUEMENT pour synchroniser
     * LeftPanel <-> OptionsPanel
     */
    struct VoiceSettings
    {
        int species = 1;
        int type    = 0;
    };

    std::vector<VoiceSettings>& getVoiceSettings();
    const std::vector<VoiceSettings>& getVoiceSettings() const;


    // =========================
    // Connexions UI
    // =========================

    void setLeftPanel(LeftPanel* panel);
    void setGenerationService(GenerationService* service);

    void updateVoice(int index, int species, int type);
    bool isGenerating() const;


    juce::ValueTree& getGenerationState()
    {
        return generationState;
    }

    ConstraintSettings& getConstraintSettings()
    {
        return problem.getSettings();
    }

    const ConstraintSettings& getConstraintSettings() const
    {
        return problem.getSettings();
    }


    // =========================
    // Configurations sauvegardées
    //
    // Une configuration sauvegardée est un fichier XML qui contient le
    // ValueTree renvoyé par CantusProblem::toValueTree() (voir
    // CantusProblem.h), complété par un nom choisi par l'utilisateur et
    // une date. AppController ne fait que lire/écrire ce fichier : toute
    // la connaissance de "ce qu'est l'état du problème" reste dans
    // CantusProblem.
    // =========================

    /**
     * Une ligne de la liste des configurations sauvegardées, affichée par
     * la page "Saved configurations" : le nom choisi par l'utilisateur et
     * la date de sauvegarde.
     */
    struct SavedConfigurationInfo
    {
        juce::String name;
        juce::String dateDisplay;
        juce::File file;
    };

    /**
     * Sauvegarde l'état complet du problème courant (Cantus Firmus, voix,
     * réglages) sous le nom donné par l'utilisateur.
     *
     * Retourne false si l'écriture du fichier a échoué.
     */
    bool saveConfiguration(const juce::String& name);

    /**
     * Retourne la liste des configurations sauvegardées, triée de la plus
     * récente à la plus ancienne.
     */
    std::vector<SavedConfigurationInfo> getSavedConfigurations() const;

    /**
     * Remplace le problème courant par la configuration lue dans ce fichier.
     *
     * Reconstruit aussi voiceSettings à partir des voix chargées, pour que
     * le prochain Generate reste cohérent avec ce qui vient d'être restauré.
     *
     * Retourne false si le fichier est invalide ou introuvable.
     */
    bool loadConfiguration(const juce::File& file);

    /**
     * Supprime définitivement une configuration sauvegardée.
     *
     * Retourne false si le fichier n'a pas pu être supprimé.
     */
    bool deleteConfiguration(const juce::File& file);

private:
    // Dossier où sont stockées les configurations sauvegardées.
    // Créé automatiquement au premier appel s'il n'existe pas encore.
    static juce::File getSavedConfigurationsDirectory();

    // =========================
    // Modèle principal
    // =========================
    CantusProblem problem;

    // =========================
    // Synchronisation UI
    // =========================
    std::vector<VoiceSettings> voiceSettings;

    // =========================
    // Services externes
    // =========================
    GenerationService* generationService = nullptr;

    // =========================
    // UI callbacks
    // =========================
    LeftPanel* leftPanel = nullptr;

    /**
     * Callback appelé après la génération (thread → UI)
     */
    void handleAsyncUpdate() override;

    ConstraintSettings currentSettings;

    juce::ValueTree generationState { "GenerationState" };

    // =========================
    // Fenêtre de progression (compte à rebours pendant la recherche)
    //
    // Vit ici et pas côté UI : AppController est déjà celui qui démarre
    // la génération (startGeneration/startNextSolution) et qui affiche le
    // résultat (handleAsyncUpdate) via des AlertWindow. La fenêtre de
    // progression suit donc le même cycle de vie, au même endroit.
    // =========================

    // Fenêtre affichée tant que le solveur cherche une solution.
    // nullptr quand aucune génération n'est en cours.
    std::unique_ptr<juce::AlertWindow> generationProgressWindow;

    // Nombre de secondes restantes affiché dans la fenêtre de progression.
    int remainingSeconds = 0;

    // Ouvre la fenêtre de progression et démarre le compte à rebours.
    void showGenerationProgressWindow();

    // Ferme la fenêtre de progression et arrête le compte à rebours.
    void closeGenerationProgressWindow();

    // Construit le texte affiché dans la fenêtre, à partir de remainingSeconds.
    juce::String buildGenerationProgressMessage() const;

    // Appelé chaque seconde tant que la fenêtre de progression est ouverte.
    void timerCallback() override;

    /*
        Date de démarrage de la fenêtre de progression (en millisecondes,
        horloge de juce::Time::getMillisecondCounter()).

        Sert à garantir un affichage minimum (voir showGenerationResult) :
        sans ça, un solveur qui répond en quelques millisecondes qu'aucune
        solution n'existe ferait clignoter "Génération en cours" puis
        "Aucune solution n'existe" presque instantanément, ce qui ressemble
        à un bug plutôt qu'à un vrai résultat.
    */
    juce::uint32 generationStartTimeMs = 0;

    // Durée minimale (ms) pendant laquelle la fenêtre de progression reste
    // affichée, même si le solveur a déjà répondu.
    static constexpr int minimumProgressDisplayMs = 3000;  //(3s)

    /*
        Affiche réellement le résultat de la génération (succès ou échec) et
        referme la fenêtre de progression.

        Appelée depuis handleAsyncUpdate(), après le délai minimum décrit
        ci-dessus. Les valeurs sont passées en paramètres (plutôt que
        relues sur generationService à ce moment-là) pour rester fiable
        même si une nouvelle génération a déjà démarré entre-temps.
    */
    void showGenerationResult(bool success,
                              const juce::String& midiPath,
                              const juce::String& errorMessage,
                              bool isValidationError);
};
