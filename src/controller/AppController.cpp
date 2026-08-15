#include "AppController.h"

#include <algorithm>

#include <juce_audio_basics/juce_audio_basics.h>

#include "../ui/leftPanel/LeftPanel.h"
#include "../ui/optionsPanel/OptionsPanel.h"


/*
//==============================================================================
   AppController

  Coordonne les interactions entre l'UI,
  le modèle musical et le moteur de génération.

  Responsable de :
   - construire le problème à générer
   - lancer la génération
   - recevoir les résultats du solveur
   - notifier l'interface utilisateur
//==============================================================================
*/

//==============================================================================
// CONSTRUCTEUR  -  Initialise le contrôleur principal de l'application.
//==============================================================================
AppController::AppController() = default;

//==============================================================================
// ACCÈS AU MODÈLE COURANT EN ECRITURE
// Utilisé par l'UI pour construire ou modifier le problème.
//==============================================================================

CantusProblem& AppController::getProblem()
{
    return problem;
}

//==============================================================================
// ACCÈS AU MODÈLE COURANT EN LECTURE SEULE
//==============================================================================
const CantusProblem& AppController::getProblem() const
{
    return problem;
}


//==============================================================================
// SYNCHRONISATION UI (LeftPanel <-> OptionsPanel)
// Accès aux paramètres UI des voix.
//==============================================================================

std::vector<AppController::VoiceSettings>& AppController::getVoiceSettings()
{
    return voiceSettings;
}

//==============================================================================
// Accès en lecture seule aux paramètres UI des voix.
//==============================================================================
const std::vector<AppController::VoiceSettings>& AppController::getVoiceSettings() const
{
    return voiceSettings;
}


//==============================================================================
// CONNEXION DES COMPOSANTS
//==============================================================================

//==============================================================================
// Connexion au LeftPanel
// Permet de notifier l'UI après la génération.
//==============================================================================

void AppController::setLeftPanel(LeftPanel* panel)
{
    leftPanel = panel;
}

//==============================================================================
// Connexion au service de génération.
// Le service exécute le solveur dans un thread séparé.
//==============================================================================
void AppController::setGenerationService(GenerationService* service)
{
    generationService = service;
}


//==============================================================================
// GÉNÉRATION
//==============================================================================

//==============================================================================
// Lance une génération.
// Vérifie l'état courant, copie le problème, puis démarre le thread solveur.
//==============================================================================
void AppController::startGeneration(const juce::String& outputPath)
{
    generationState.setProperty("generationStatus", "idle", nullptr);

    if (generationService == nullptr)
    {
        generationState.setProperty("generationError",
                                    juce::String::fromUTF8("Service de génération indisponible."),
                                    nullptr);

        generationState.setProperty("generationStatus", "error", nullptr);
        return;
    }

    if (problem.isEmpty())
    {
        generationState.setProperty("generationError",
                                    juce::String::fromUTF8("Le problème est vide.\n\nEntrez un problème complet."),
                                    nullptr);

        generationState.setProperty("generationStatus", "warning", nullptr);
        return;
    }

    generationState.setProperty("generationStatus", "generating", nullptr);

    CantusProblem copyProblem = problem;

    bool started = generationService->startGeneration(copyProblem, outputPath, this);

    if (!started)
    {
        generationState.setProperty("generationError",
                                    generationService->getLastError(),
                                    nullptr);

        generationState.setProperty("generationStatus",
                                    generationService->isInputValidationError()
                                        ? "warning"
                                        : "error",
                                    nullptr);
    }
}

//==============================================================================
// Lance une autre solution pour le dernier problème généré.
//==============================================================================
void AppController::startNextSolution(const juce::String& outputPath)
{
    generationState.setProperty("generationStatus", "idle", nullptr);

    if (generationService == nullptr)
    {
        generationState.setProperty("generationError",
                                    juce::String::fromUTF8("Service de génération indisponible."),
                                    nullptr);

        generationState.setProperty("generationStatus", "error", nullptr);
        return;
    }

    generationState.setProperty("generationStatus", "generating", nullptr);

    bool started = generationService->startNextSolution(outputPath, this);

    if (!started)
    {
        generationState.setProperty("generationError",
                                    generationService->getLastError(),
                                    nullptr);

        generationState.setProperty("generationStatus",
                                    generationService->isInputValidationError()
                                        ? "warning"
                                        : "error",
                                    nullptr);

        /*
            Next solution peut échouer avant de lancer le thread.

            Exemple :
            l'utilisateur appuie sur "Next solution" alors qu'aucune première
            solution n'a encore été générée.

            Dans ce cas, on affiche directement une alerte claire au lieu
            d'attendre un retour de génération qui n'arrivera pas.
        */
        if (generationService->isInputValidationError())
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                juce::String::fromUTF8("Next solution"),
                generationService->getLastError());
        }
    }
}

//==============================================================================
// CALLBACK THREAD --> UI

// Callback de fin de génération.
// Appelé sur le thread UI après la fin du solveur.
// Affiche le résultat et met à jour generationState.
//==============================================================================
void AppController::handleAsyncUpdate()
{
    if (generationService == nullptr)
        return;

    bool success = generationService->getLastGenerationSuccess();

    if (success)
    {
        juce::String midiPath = generationService->getLastGeneratedMidiPath();

        juce::AlertWindow::showOkCancelBox(
            juce::AlertWindow::QuestionIcon,
            juce::String::fromUTF8("Solution trouvée"),
            juce::String::fromUTF8("Une solution existe.\n\nVoulez-vous générer la solution ?"),
            juce::String::fromUTF8("Oui"),
            juce::String::fromUTF8("Non"),
            nullptr,
            juce::ModalCallbackFunction::create(
                [this, midiPath](int result)
                {
                    if (result == 1 && midiPath.isNotEmpty())
                    {
                        generationState.setProperty("midiFilePath", midiPath, nullptr);
                        generationState.setProperty("generationStatus", "completed", nullptr);
                    }
                }));
    }
    else
    {
        generationState.setProperty("generationError",
                                    generationService->getLastError(),
                                    nullptr);

        generationState.setProperty("generationStatus",
                                    generationService->isInputValidationError()
                                        ? "warning"
                                        : "error",
                                    nullptr);

        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            juce::String::fromUTF8("Résultat"),
            juce::String::fromUTF8("Aucune solution n'existe."));
    }
}

//==============================================================================
// Met à jour les paramètres d'une voix de contrepoint.
// Appelé lorsque l'utilisateur change l'espèce ou le type.
//==============================================================================
void AppController::updateVoice(int index, int species, int type)
{
    if (index < 0)
        return;

    if (index >= (int)voiceSettings.size())
        return;
    // Override les valeurs par défaut des voix
    voiceSettings[index].species = species; // 1 par défaut
    voiceSettings[index].type    = type; // 0 par défaut
}

//==============================================================================
// Indique si une génération est actuellement en cours.
//==============================================================================
bool AppController::isGenerating() const
{
    if (generationService == nullptr)
        return false;

    return generationService->isGenerating();
}

//==============================================================================
// CONFIGURATIONS SAUVEGARDÉES
//
// Une configuration sauvegardée est un fichier XML : le ValueTree produit
// par CantusProblem::toValueTree(), auquel on ajoute juste un nom et une
// date avant de l'écrire sur le disque.
//
//
//
// AppController ne connaît pas
// le détail de ce que contient un problème : il se contente de déléguer
// à CantusProblem, puis de gérer le fichier.
//==============================================================================

/*
    Retourne le dossier utilisé pour stocker les configurations, et le crée
    s'il n'existe pas encore.

    Emplacement standard des données d'application sur macOS :
    ~/Library/Application Support/Fuxophone/SavedConfigurations/
*/
juce::File AppController::getSavedConfigurationsDirectory()
{
    auto directory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Fuxophone")
        .getChildFile("SavedConfigurations");

    if (! directory.exists())
        directory.createDirectory();

    return directory;
}

/*
    Sauvegarde l'état complet du problème courant sous le nom donné par
    l'utilisateur.

    On part du ValueTree renvoyé par CantusProblem::toValueTree() (qui ne
    connaît que le problème lui-même), et on lui ajoute deux propriétés
    supplémentaires - le nom choisi et la date - avant de l'écrire en XML.



    C'est ce même ValueTree, avec ces mêmes propriétés, que loadConfiguration()
    et getSavedConfigurations() relisent ensuite.
*/
bool AppController::saveConfiguration(const juce::String& name)
{
    auto state = problem.toValueTree();

    const auto now = juce::Time::getCurrentTime();

    // Nom affiché tel quel dans la liste des configurations.
    state.setProperty("name", name, nullptr);
    // Texte affiché sous le nom, dans la liste.
    state.setProperty("dateDisplay", now.toString(true, true, false, true), nullptr);

    /*
        Le nom du fichier sur le disque n'est jamais montré à l'utilisateur :
        seule la propriété "name" ci-dessus l'est.


        -On simplifie le nom choisi pour qu'il reste un nom de fichier valide

        -On ajoute l'horodatage pour être certain de ne jamais écraser une ancienne
        sauvegarde qui porterait le même nom.
    */
    auto fileName = juce::File::createLegalFileName(name.isNotEmpty() ? name : "Configuration")
        + "_" + juce::String(now.toMilliseconds()) + ".xml";

    auto file = getSavedConfigurationsDirectory().getChildFile(fileName);

    if (auto xml = state.createXml())
        return xml->writeTo(file);

    return false;
}

/*
    Parcourt le dossier de sauvegarde et lit, pour chaque fichier, juste de
    quoi remplir une ligne de la liste (nom + date) : pas besoin de
    reconstruire tout le problème seulement pour l'afficher.
*/
std::vector<AppController::SavedConfigurationInfo> AppController::getSavedConfigurations() const
{
    std::vector<SavedConfigurationInfo> entries;

    for (const auto& file : getSavedConfigurationsDirectory().findChildFiles(juce::File::findFiles, false, "*.xml"))
    {
        auto xml = juce::XmlDocument::parse(file);

        if (xml == nullptr)
            continue;

        auto state = juce::ValueTree::fromXml(*xml);

        if (! state.isValid())
            continue;

        SavedConfigurationInfo entry;
        entry.name = state.getProperty("name", "Sans nom").toString();
        entry.dateDisplay = state.getProperty("dateDisplay", "").toString();
        entry.file = file;

        entries.push_back(entry);
    }

    // La configuration sauvegardée le plus récemment apparaît en premier.
    std::sort(entries.begin(), entries.end(), [](const SavedConfigurationInfo& a, const SavedConfigurationInfo& b)
    {
        return a.file.getLastModificationTime() > b.file.getLastModificationTime();
    });

    return entries;
}

/*
    Charge une configuration sauvegardée et remplace le problème courant.

    Toute la reconstruction du problème (Cantus Firmus, voix, réglages) est
    déléguée à CantusProblem::restoreFromValueTree() : AppController se
    contente de relire le fichier XML et de refaire ensuite le lien avec
    voiceSettings, qui sert uniquement à la synchronisation UI.
*/
bool AppController::loadConfiguration(const juce::File& file)
{
    auto xml = juce::XmlDocument::parse(file);

    if (xml == nullptr)
        return false;

    auto state = juce::ValueTree::fromXml(*xml);

    if (! state.isValid())
        return false;

    problem.restoreFromValueTree(state);

    /*
        voiceSettings sert uniquement à synchroniser LeftPanel <-> OptionsPanel
        avant un Generate : on le reconstruit à partir des voix qui viennent
        d'être chargées, pour que l'espèce et le type de chaque contrepoint
        restent corrects si l'utilisateur relance une génération sans rien
        modifier.
    */
    const auto& counterpoints = problem.getCounterpoints();
    voiceSettings.assign(counterpoints.size(), {});

    for (size_t i = 0; i < counterpoints.size(); ++i)
    {
        voiceSettings[i].species = counterpoints[i].species;
        voiceSettings[i].type    = counterpoints[i].type;
    }

    return true;
}

/*
    Supprime définitivement le fichier d'une configuration sauvegardée.

    Une simple suppression de fichier : rien à faire côté CantusProblem ou
    ValueTree, la configuration supprimée n'était de toute façon pas
    chargée en mémoire (seule sa fiche name/date l'était, pour la liste).
*/
bool AppController::deleteConfiguration(const juce::File& file)
{
    return file.deleteFile();
}
