//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
  CantusProblem.cpp

  Modèle central représentant un problème de contrepoint.

  Contient :
   - le Cantus Firmus
   - les contrepoints
   - les paramètres du solveur
   - les vecteurs de coûts calculés(ou re-calculés)
   - la sérialisation du problème en ValueTree (utilisé pour les Sauvegarde XML)

  Utilisé par le GenerationService pour construire le problème Fux/Gecode.
//==============================================================================
*/

#include "CantusProblem.h"

namespace
{
    /*
        =====================================================================
        Identifiants des noeuds utilisés par toValueTree()/restoreFromValueTree().

        un seul identifiant, réutilisé côté écriture
        et côté lecture, garantit qu'on lit toujours ce qu'on a écrit.
        =====================================================================
    */
    const juce::Identifier idRoot              { "CantusProblemState" };
    const juce::Identifier idCantusFirmus       { "CantusFirmus" };
    const juce::Identifier idVoices             { "Voices" };
    const juce::Identifier idVoice              { "Voice" };
    const juce::Identifier idGlobalSettings     { "GlobalSettings" };
    const juce::Identifier idImportance         { "Importance" };
    const juce::Identifier idCounterpointParams { "CounterpointParams" };
    const juce::Identifier idParams             { "Params" };
    const juce::Identifier idShapeAssignments   { "ShapeAssignments" };
    const juce::Identifier idShape              { "Shape" };

    /*
        Transforme un vecteur de nombres en une seule chaîne de texte,
        les valeurs étant séparées par un espace.

        Utilisé pour stocker un vecteur entier (Cantus Firmus, importance,
        valeurs d'une shape...) dans une seule propriété de ValueTree,
        quelle que soit sa longueur : rien n'est jamais figé à un nombre
        précis de voix ou de valeurs.
    */
    template <typename NumberType>
    juce::String numbersToString(const std::vector<NumberType>& values)
    {
        juce::StringArray parts;

        for (auto value : values)
            parts.add(juce::String(value));

        return parts.joinIntoString(" ");
    }

    /*
        Reconstruit un vecteur d'entiers à partir d'une chaîne de texte
        produite par numbersToString().
    */
    std::vector<int> stringToIntVector(const juce::String& text)
    {
        std::vector<int> result;
        auto tokens = juce::StringArray::fromTokens(text, " ", "");
        tokens.removeEmptyStrings();

        for (const auto& token : tokens)
            result.push_back(token.getIntValue());

        return result;
    }

    /*
        Reconstruit un vecteur de doubles à partir d'une chaîne de texte
        produite par numbersToString().
    */
    std::vector<double> stringToDoubleVector(const juce::String& text)
    {
        std::vector<double> result;
        auto tokens = juce::StringArray::fromTokens(text, " ", "");
        tokens.removeEmptyStrings();

        for (const auto& token : tokens)
            result.push_back(token.getDoubleValue());

        return result;
    }
}

//==============================================================================
// Construction
//==============================================================================

/*
    Initialise un problème vide avec les paramètres par défaut
    et calcule les vecteurs de coûts associés.
*/
CantusProblem::CantusProblem()
{
    voiceCount = 0;
    recalculateCosts();
}

//==============================================================================
// Données musicales
// Gestion du Cantus Firmus et des contrepoints
//==============================================================================

/*
    Remplace complètement la structure musicale du problème.
*/
void CantusProblem::setVoices(const Voices& v)
{
    voices = v;
}

/*
    Retourne l'ensemble des voix du problème.
*/
const CantusProblem::Voices& CantusProblem::getVoices() const
{
    return voices;
}

/*
    Retourne le Cantus Firmus.
*/
const std::vector<int>& CantusProblem::getCantusFirmus() const
{
    return voices.cf;
}

/*
    Retourne la liste des contrepoints.
*/
const std::vector<CantusProblem::Counterpoint>& CantusProblem::getCounterpoints() const
{
    return voices.counterpoints;
}

/*
    Retourne le nombre de contrepoints.
*/
size_t CantusProblem::getCounterpointCount() const
{
    return voices.counterpoints.size();
}

/*
    Définit le nombre total de voix (CF inclus).
*/
void CantusProblem::setVoiceCount(int count)
{
    voiceCount = count;
}

/*
    Retourne le nombre total de voix.
*/
int CantusProblem::getVoiceCount() const
{
    return voiceCount;
}

//==============================================================================
// Conversion pour le solveur
// Extraction des données attendues par Fux/Gecode
//==============================================================================

/*
    Construit la liste des espèces des contrepoints.
*/
std::vector<int> CantusProblem::getSpeciesList() const
{
    std::vector<int> result;
    result.reserve(voices.counterpoints.size());

    for (const auto& cp : voices.counterpoints)
        result.push_back(cp.species);

    return result;
}

/*
    Construit la liste des types de voix.
*/
std::vector<int> CantusProblem::getVoiceTypes() const
{
    std::vector<int> result;
    result.reserve(voices.counterpoints.size());

    for (const auto& cp : voices.counterpoints)
        result.push_back(cp.type);

    return result;
}

//==============================================================================
// Paramètres de contraintes
// Gestion des réglages et des coûts du solveur
//==============================================================================

/*
    Met à jour les paramètres du solveur
    puis recalcule tous les vecteurs de coûts.
*/
void CantusProblem::setSettings(const ConstraintSettings& s)
{
    settings = s;
    recalculateCosts();
}

/*
    Accès aux paramètres du solveur.
*/
ConstraintSettings& CantusProblem::getSettings()
{
    return settings;
}

/*
    Accès en lecture seule aux paramètres du solveur.
*/
const ConstraintSettings& CantusProblem::getSettings() const
{
    return settings;
}

//==============================================================================
// Métadonnées
// Informations descriptives du problème
//==============================================================================

/*
    Définit le titre du problème.
*/
void CantusProblem::setTitle(const juce::String& newTitle)
{
    title = newTitle;
}

/*
    Retourne le titre du problème.
*/
juce::String CantusProblem::getTitle() const
{
    return title;
}

//==============================================================================
// Validation
// Vérification de la cohérence minimale du problème
//==============================================================================

/*
    Vérifie que le problème contient
    un Cantus Firmus et au moins un contrepoint.
*/
bool CantusProblem::isEmpty() const
{
    return voices.cf.empty() || voices.counterpoints.empty();
}

//==============================================================================
// Accès aux coûts calculés
//==============================================================================

/*
    Retourne les coûts mélodiques.
*/
const std::vector<int>& CantusProblem::getMelodicCosts() const {
    return melodicCosts;
}

/*
    Retourne les coûts généraux.
*/
const std::vector<int>& CantusProblem::getGeneralCosts() const {
    return generalCosts;
}

/*
    Retourne les coûts spécifiques aux espèces.
*/
const std::vector<int>& CantusProblem::getSpecificCosts() const {
    return specificCosts;
}

/*
    Retourne les priorités d'optimisation.
*/
const std::vector<int>& CantusProblem::getImportanceCosts() const {
    return importanceCosts;
}

//==============================================================================
// Recalcul des coûts
// Synchronisation entre les paramètres et le solveur
//==============================================================================
void CantusProblem::recalculateCosts() {
    melodicCosts = settings.buildMelodicCosts(static_cast<int>(voices.cf.size()));
    generalCosts = settings.buildGeneralCosts();
    specificCosts = settings.buildSpecificCosts();
    importanceCosts = settings.buildImportanceCosts();

    /*std::cout << "melodicCosts size = "
          << melodicCosts.size()
          << std::endl;*/
}

//==============================================================================
// Sauvegarde / Chargement (ValueTree)
//
// toValueTree() range tout l'état du problème dans un seul ValueTree.
// restoreFromValueTree() fait l'inverse : elle relit ce ValueTree et
// réécrit chaque champ du problème.
//
// Les deux méthodes se répondent noeud par noeud, dans le même ordre,
// pour rester faciles à comparer et à faire évoluer ensemble.
//==============================================================================

/*
    Construit un ValueTree représentant l'état complet du problème :
    Cantus Firmus, voix (espèce/type de chaque contrepoint) et tous les
    réglages du solveur contenus dans ConstraintSettings.

    Chaque groupe de données (voix, priorités, shapes...) est écrit dans
    son propre noeud, avec autant d'enfants que nécessaire : la structure
    s'adapte donc au nombre réel de voix ou de shapes, sans jamais supposer
    un nombre fixe à l'avance.
*/
juce::ValueTree CantusProblem::toValueTree() const
{
    juce::ValueTree root(idRoot);
    root.setProperty("title", title, nullptr);

    // Cantus Firmus
    juce::ValueTree cantusFirmusNode(idCantusFirmus);
    cantusFirmusNode.setProperty("notes", numbersToString(voices.cf), nullptr);
    root.addChild(cantusFirmusNode, -1, nullptr);

    // Voix (une entrée par contrepoint, dans leur ordre réel)
    juce::ValueTree voicesNode(idVoices);
    voicesNode.setProperty("count", voiceCount, nullptr);

    for (const auto& counterpoint : voices.counterpoints)
    {
        juce::ValueTree voiceNode(idVoice);
        voiceNode.setProperty("species", counterpoint.species, nullptr);
        voiceNode.setProperty("type", counterpoint.type, nullptr);
        voicesNode.addChild(voiceNode, -1, nullptr);
    }
    root.addChild(voicesNode, -1, nullptr);

    //  Réglages globaux du solveur

    // Les enums sont stockés tels quels (static_cast en int) : ConstraintSettings
    // reste la seule source de vérité sur leur signification.
    juce::ValueTree globalSettingsNode(idGlobalSettings);
    globalSettingsNode.setProperty("borrowMode", settings.getBorrowMode(), nullptr);
    globalSettingsNode.setProperty("searchMethod",
                                   static_cast<int>(settings.getSearchMethod()),
                                   nullptr);
    globalSettingsNode.setProperty("minimizationMethod",
                                   static_cast<int>(settings.getMinimizationMethod()),
                                   nullptr);
    root.addChild(globalSettingsNode, -1, nullptr);

    // Ordre d'importance des priorités
    juce::ValueTree importanceNode(idImportance);
    importanceNode.setProperty("costs", numbersToString(settings.getImportanceCosts()),
        nullptr);
    root.addChild(importanceNode, -1, nullptr);

    // Sliders de coûts, un groupe de valeurs par contrepoint
    juce::ValueTree counterpointParamsNode(idCounterpointParams);

    for (const auto& params : settings.getAllCounterpointCostParameters())
    {
        juce::ValueTree paramsNode(idParams);
        paramsNode.setProperty("melodyMovement", params.melodyMovement, nullptr);
        paramsNode.setProperty("intervalColour", params.intervalColour, nullptr);
        paramsNode.setProperty("perfectIntervals", params.perfectIntervals, nullptr);
        counterpointParamsNode.addChild(paramsNode, -1, nullptr);
    }
    root.addChild(counterpointParamsNode, -1, nullptr);

    // Shapes assignées (aucune, une ou plusieurs par contrepoint)
    juce::ValueTree shapeAssignmentsNode(idShapeAssignments);

    for (const auto& assignment : settings.getShapeAssignments())
    {
        juce::ValueTree shapeNode(idShape);
        shapeNode.setProperty("voiceIndex", assignment.voiceIndex, nullptr);
        shapeNode.setProperty("target", static_cast<int>(assignment.target), nullptr);
        shapeNode.setProperty("shape", static_cast<int>(assignment.shape), nullptr);
        shapeNode.setProperty("values", numbersToString(assignment.controlValues),
            nullptr);
        shapeAssignmentsNode.addChild(shapeNode, -1, nullptr);
    }
    root.addChild(shapeAssignmentsNode, -1, nullptr);

    return root;
}

/*
    Relit un ValueTree produit par toValueTree() et remplace l'état actuel
    du problème par celui qu'il décrit.

    La lecture suit exactement le même ordre que l'écriture, noeud par
    noeud, pour rester simple à vérifier.
*/
void CantusProblem::restoreFromValueTree(const juce::ValueTree& state)
{
    // Un ValueTree invalide ou d'un autre type ne doit rien modifier :
    // on préfère ne rien faire plutôt que d'écraser le problème actuel
    // avec des données incohérentes.
    if (! state.isValid() || state.getType() != idRoot)
        return;

    title = state.getProperty("title", "").toString();

    // Cantus Firmus
    Voices restoredVoices;

    if (auto cantusFirmusNode = state.getChildWithName(idCantusFirmus); cantusFirmusNode.isValid())
        restoredVoices.cf = stringToIntVector(cantusFirmusNode.getProperty("notes").toString());

    // Voix

    // On reconstruit la liste des contrepoints en parcourant les enfants
    // "Voice" dans leur ordre d'origine. (Comme ça a été sauvegardé)
    if (auto voicesNode = state.getChildWithName(idVoices); voicesNode.isValid())
    {
        voiceCount = (int) voicesNode.getProperty("count", 0);

        for (int i = 0; i < voicesNode.getNumChildren(); ++i)
        {
            auto voiceNode = voicesNode.getChild(i);

            Counterpoint counterpoint;
            counterpoint.species = (int) voiceNode.getProperty("species", 1);
            counterpoint.type    = (int) voiceNode.getProperty("type", 0);

            restoredVoices.counterpoints.push_back(counterpoint);
        }
    }

    voices = restoredVoices;

    //  Réglages globaux (BorrowMode, Méthode de recherche et Méthode de minimisation)
    if (auto globalSettingsNode = state.getChildWithName(idGlobalSettings); globalSettingsNode.isValid())
    {
        settings.setBorrowMode((int) globalSettingsNode.getProperty("borrowMode", 1));

        settings.setSearchMethod(static_cast<ConstraintSettings::SearchMethod>(
            (int) globalSettingsNode.getProperty("searchMethod",
                                                 static_cast<int>(ConstraintSettings::SearchMethod::bab))));

        settings.setMinimizationMethod(static_cast<ConstraintSettings::MinimizationMethod>(
            (int) globalSettingsNode.getProperty("minimizationMethod",
                                                 static_cast<int>(ConstraintSettings::
                                                     MinimizationMethod::lexicographic))));
    }

    //  Ordre d'importance
    if (auto importanceNode = state.getChildWithName(idImportance); importanceNode.isValid())
        settings.setImportanceCosts(stringToIntVector(importanceNode.getProperty("costs").toString()));

    // Sliders de coûts par contrepoint

    // setCounterpointCount prépare d'abord une case par contrepoint,
    // pour que chaque setCounterpoint*() ci-dessous écrive au bon endroit.
    settings.setCounterpointCount((int) restoredVoices.counterpoints.size());

    if (auto counterpointParamsNode = state.getChildWithName(idCounterpointParams); counterpointParamsNode.isValid())
    {
        for (int i = 0; i < counterpointParamsNode.getNumChildren(); ++i)
        {
            auto paramsNode = counterpointParamsNode.getChild(i);

            settings.setCounterpointMelodyMovement(i, (double) paramsNode.getProperty("melodyMovement",
                0.0));
            settings.setCounterpointIntervalColour(i, (double) paramsNode.getProperty("intervalColour",
                0.0));
            settings.setCounterpointPerfectIntervals(i, (double) paramsNode.getProperty("perfectIntervals",
                0.0));
        }
    }

    //  Shapes assignées

    // On repart d'une liste vide avant de réajouter uniquement les shapes
    // réellement présentes dans le ValueTree.
    settings.setShapeAssignments({});

    if (auto shapeAssignmentsNode = state.getChildWithName(idShapeAssignments); shapeAssignmentsNode.isValid())
    {
        for (int i = 0; i < shapeAssignmentsNode.getNumChildren(); ++i)
        {
            auto shapeNode = shapeAssignmentsNode.getChild(i);

            const int voiceIndex = (int) shapeNode.getProperty("voiceIndex", 0);

            const auto target = static_cast<ConstraintSettings::ShapeCostTarget>(
                (int) shapeNode.getProperty("target", 0));

            const auto shape = static_cast<ConstraintSettings::ShapeType>(
                (int) shapeNode.getProperty("shape", 0));

            settings.setShapeAssignment(voiceIndex,
                                        target,
                                        shape,
                                        stringToDoubleVector(
                                            shapeNode.getProperty("values").toString()));
        }
    }

    // Les vecteurs de coûts envoyés au solveur dépendent des réglages
    // qu'on vient de restaurer : on les reconstruit avant de rendre la main.
    recalculateCosts();
}
