#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_data_structures/juce_data_structures.h>

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


class AppController : public juce::AsyncUpdater
{
public:
    AppController();
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

};
