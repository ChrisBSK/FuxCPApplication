#include "CantusProblem.h"
/*
//==============================================================================
  CantusProblem

  Modèle central représentant un problème de contrepoint.

  Contient :
   - le Cantus Firmus
   - les contrepoints
   - les paramètres du solveur
   - les vecteurs de coûts calculés

  Utilisé par le GenerationService pour construire le problème Fux/Gecode.
//==============================================================================
*/

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
    melodicCosts = settings.buildMelodicCosts();
    generalCosts = settings.buildGeneralCosts();
    specificCosts = settings.buildSpecificCosts();
    importanceCosts = settings.buildImportanceCosts();

    /*std::cout << "melodicCosts size = "
          << melodicCosts.size()
          << std::endl;*/
}
