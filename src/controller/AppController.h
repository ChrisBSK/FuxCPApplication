//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   AppController.h

   Contrôleur principal de l'application.

   Assure la communication entre l'interface utilisateur,
   le modèle (CantusProblem) et le service de génération.
//==============================================================================
*/


#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "../model/CantusProblem.h"
#include "../service/GenerationService.h"


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


     //Lance la solution suivante du dernier problème généré.
    void startNextSolution(const juce::String& outputPath);


    // =========================
    // Accès modèle
    // =========================

    //Accès en écriture au problème utilisé par le LeftPanel pour construire le problème
    CantusProblem& getProblem();


    //Accès en lecture seule
    const CantusProblem& getProblem() const;


    // =========================
    // Synchronisation UI
    // =========================

    // Réglages d'une voix de contrepoint (espèce et tessiture),
    // utilisés pour synchroniser LeftPanel et OptionsPanel.
    struct VoiceSettings //Voix de contrepoint
    {
        int species = 1; // espèce du contrepoint (1 à 5)
        int type    = 0; // tessiture de la voix (Tenor/Alto/Soprano)
    };


    // Accès en écriture au vecteur de réglages par voix (species/type) affiché dans l'UI.
    std::vector<VoiceSettings>& getVoiceSettings();
    // Accès en lecture seule au vecteur de réglages par voix.
    const std::vector<VoiceSettings>& getVoiceSettings() const;


    // =========================
    // Connexions UI
    // =========================

    // Enregistre le LeftPanel à notifier lors des mises à jour du contrôleur.
    void setLeftPanel(LeftPanel* panel);

    // Enregistre le GenerationService utilisé pour lancer les générations.
    void setGenerationService(GenerationService* service);

    // Met à jour l'espèce et la tessiture de la voix à l'index donné.
    void updateVoice(int index, int species, int type);

    // Indique si une génération est actuellement en cours.
    bool isGenerating() const;

    // Accès en écriture à l'état de génération partagé avec l'UI.
    juce::ValueTree& getGenerationState()
    {
        return generationState;
    }

    // Accès en écriture aux réglages de contraintes du problème courant.
    ConstraintSettings& getConstraintSettings()
    {
        return problem.getSettings();
    }

    // Accès en lecture seule aux réglages de contraintes du problème courant.
    const ConstraintSettings& getConstraintSettings() const
    {
        return problem.getSettings();
    }

    /*
    // =========================
       Configurations sauvegardées

       Une configuration sauvegardée est un fichier XML sur disque : le
       ValueTree de CantusProblem (toValueTree), plus un nom et une date.

       Charger un fichier reconstruit ce ValueTree pour restaurer l'état
       du problème. AppController se contente de lire/écrire ces fichiers.
    // =========================
    */

    // Une ligne de la liste des configurations sauvegardées (nom, date, fichier).
    struct SavedConfigurationInfo
    {
        juce::String name;
        juce::String dateDisplay;
        juce::File file;
    };


    // Sauvegarde l'état complet du problème courant sous le nom donné (false si l'écriture échoue).
    bool saveConfiguration(const juce::String& name);

    // Retourne la liste des configurations sauvegardées, de la plus récente à la plus ancienne.
    std::vector<SavedConfigurationInfo> getSavedConfigurations() const;

    // Charge une configuration depuis ce fichier et remplace le problème courant
    // (false si le fichier est invalide/introuvable).
    bool loadConfiguration(const juce::File& file);

    // Supprime définitivement une configuration sauvegardée (false si la suppression échoue).
    bool deleteConfiguration(const juce::File& file);

private:
    // Dossier où sont stockées les configurations sauvegardées.
    // Créé automatiquement au premier appel s'il n'existe pas encore.
    static juce::File getSavedConfigurationsDirectory();


    // Modèle principal
    CantusProblem problem;


    // Synchronisation UI
    std::vector<VoiceSettings> voiceSettings;

    // Services externes
    GenerationService* generationService = nullptr;

    // UI callbacks
    LeftPanel* leftPanel = nullptr;

    // Callback appelé après la génération, une fois le résultat du thread solveur disponible.
    void handleAsyncUpdate() override;

    // Réglages de contraintes utilisés pour la génération en cours.
    ConstraintSettings currentSettings;

    // État de génération partagé avec l'UI (attente, succès, erreur), sans lien direct vers les composants.
    juce::ValueTree generationState { "GenerationState" };


    /*
    // =========================
       Fenêtre de progression (compte à rebours pendant la recherche)

       Vit ici plutôt que côté UI : AppController démarre déjà la génération
       et affiche son résultat, la fenêtre de progression suit donc le même
       cycle de vie, au même endroit.
    // =========================
    */

    // Fenêtre affichée tant que le solveur cherche une solution (nullptr si aucune génération en cours).
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
        horloge juce::Time::getMillisecondCounter()).

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
        Affiche le résultat de la génération (succès ou échec) et
        referme la fenêtre de progression.

        Appelée depuis handleAsyncUpdate(), après le délai minimum (3s)

        Les valeurs sont passées en paramètres plutôt que
        relues sur generationService à ce moment-là, pour rester fiable
        même si une nouvelle génération a déjà démarré entre-temps.
    */
    void showGenerationResult(bool success,
                              const juce::String& midiPath,
                              const juce::String& errorMessage,
                              bool isValidationError);
};
