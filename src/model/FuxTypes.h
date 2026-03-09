// ============================================================================
// FuxTypes.h
// ----------------------------------------------------------------------------
// Définitions centralisées, *génériques* des types & constantes pour le modèle
// FuxCP.
//
// Objectif :
//  - Fournir un seul endroit propre et structuré pour les enums, constantes
//    d’intervalles musicaux, constantes de mouvement, et petites tables globales
//    utilisées dans le modèle.
//  - Rester générique : tout est exprimé en demi-tons / conventions MIDI, et
//    les tables sont faciles à étendre sans modifier la logique.
//
// Remarques :
//  - Le projet utilise une grille temporelle de 4 "ticks" par mesure (voir
//    Voice.cpp).
//  - La correspondance espèce -> notes par mesure est donnée dans
//    `notesPerMeasure`.
//  - Les gammes sont représentées ici comme des *boucles d’intervalles*
//    (cf. Utilities.cpp), c.-à-d. une suite de pas qui boucle sur l’octave.
// ============================================================================

#ifndef FUXCP_MODEL_FUXTYPES_H
#define FUXCP_MODEL_FUXTYPES_H

#include <array>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fuxcp {

// ---------------------------------------------------------------------------
// Constantes musicales de base (en demi-tons)
// ---------------------------------------------------------------------------

// Utiliser un namespace dédié évite de polluer l’espace global (Gecode, etc.).
constexpr int PERFECT_OCTAVE = 12;

// Noms d’intervalles en demi-tons (classe d’intervalle absolue).
// Convention utilisée partout : unisson = 0, octave = 12.
constexpr int UNISSON          = 0;
constexpr int MINOR_SECOND     = 1;
constexpr int MAJOR_SECOND     = 2;
constexpr int MINOR_THIRD      = 3;
constexpr int MAJOR_THIRD      = 4;
constexpr int PERFECT_FOURTH   = 5;
constexpr int TRITONE          = 6;  // même distance de classe que A4 / d5
constexpr int AUGMENTED_FOURTH = 6;  // alias parfois utilisé
constexpr int PERFECT_FIFTH    = 7;
constexpr int MINOR_SIXTH      = 8;
constexpr int MAJOR_SIXTH      = 9;
constexpr int MINOR_SEVENTH    = 10;
constexpr int MAJOR_SEVENTH    = 11;

// ---------------------------------------------------------------------------
// Espèces et conventions de nombre de voix
// ---------------------------------------------------------------------------

// Enum des espèces utilisées dans le projet.
// IMPORTANT : `problem_wrapper.cpp` convertit les entiers 1..5 vers ces espèces.
// Le cantus firmus n’est pas converti par cette fonction, mais il existe comme
// espèce dans le modèle.
enum Species : int {
  CANTUS_FIRMUS   = 0,
  FIRST_SPECIES   = 1,
  SECOND_SPECIES  = 2,
  THIRD_SPECIES   = 3,
  FOURTH_SPECIES  = 4,
  FIFTH_SPECIES   = 5
};

// Constantes de nombre de voix (utilisées dans CounterpointUtils.cpp).
constexpr int TWO_VOICES   = 2;
constexpr int THREE_VOICES = 3;
constexpr int FOUR_VOICES  = 4;

// ---------------------------------------------------------------------------
// Constantes de mouvement (motion)
// ---------------------------------------------------------------------------
// Le code utilise ces noms symboliques à plusieurs endroits.
// NOTE : certains tableaux utilisent -1 pour indiquer "non applicable"
// (par exemple quand la voix est la basse).
constexpr int CONTRARY_MOTION  = 1;
constexpr int OBLIQUE_MOTION   = 2;
constexpr int PARALLEL_MOTION  = 3;

// ---------------------------------------------------------------------------
// Notes par mesure pour la grille de 4 "ticks" utilisée par Voice.cpp
// ---------------------------------------------------------------------------
// Le modèle utilise une grille de 4 unités par mesure :
//   - FIRST_SPECIES  : 1 note/mesure  -> pas = 4/1 = 4
//   - SECOND_SPECIES : 2 notes/mesure -> pas = 4/2 = 2
//   - THIRD_SPECIES  : 4 notes/mesure -> pas = 4/4 = 1
//   - FOURTH_SPECIES : généralement 2 notes/mesure (syncope)
//   - FIFTH_SPECIES  : espèce mixte ; beaucoup d’implémentations l’alignent
//     sur une grille "4" et une espèce par note pilote les règles.
//
// Si ton FIFTH_SPECIES utilise une autre densité de grille, change la valeur
// ci-dessous à un seul endroit.
inline const std::unordered_map<int, int> notesPerMeasure = {
  {CANTUS_FIRMUS,  1},
  {FIRST_SPECIES,  1},
  {SECOND_SPECIES, 2},
  {THIRD_SPECIES,  4},
  {FOURTH_SPECIES, 2},
  {FIFTH_SPECIES,  4}
};

// ---------------------------------------------------------------------------
// Ensembles consonance / dissonance (classes de hauteur mod 12)
// ---------------------------------------------------------------------------
// Utilisés pour les restrictions de domaine et les booléens "isConsonance".
//
// IMPORTANT :
// - Certaines parties calculent les intervalles harmoniques avec un `%12` et
//   peuvent produire des valeurs négatives selon la formule ; le code utilise
//   souvent abs(...) pour tester la consonance.
// - On stocke donc ici les ensembles en [0..11] ; le code peut faire abs(...)
//   ou une normalisation mod 12 selon le cas.

// Consonances parfaites (classe de hauteur).
inline const std::array<int, 2> PERFECT_CONSONANCES = {
  UNISSON, PERFECT_FIFTH
  // L’octave est un unisson mod 12 ; quand il faut +/-12, c’est géré ailleurs.
};

// Consonances (classe de hauteur).
inline const std::array<int, 6> CONSONANCES = {
  UNISSON,
  MINOR_THIRD,
  MAJOR_THIRD,
  PERFECT_FIFTH,
  MINOR_SIXTH,
  MAJOR_SIXTH
  // L’octave se réduit à UNISSON mod 12.
};

// Dissonances usuelles (classe de hauteur).
inline const std::array<int, 4> DISSONANCES = {
  MINOR_SECOND,
  MAJOR_SECOND,
  TRITONE,
  MINOR_SEVENTH
  // MAJOR_SEVENTH est aussi dissonant ; certains jeux de règles le traitent à part.
};

// TRIAD (classes de hauteur) : triade en position fondamentale au niveau harmonique.
// Utilisé notamment dans Stratum.cpp pour la fin (accord final).
inline const std::array<int, 3> TRIAD = {
  UNISSON, MAJOR_THIRD, PERFECT_FIFTH
};

// ---------------------------------------------------------------------------
// Définition des gammes comme *boucles d’intervalles* (Utilities.cpp)
// ---------------------------------------------------------------------------
// Une gamme ici n’est pas un ensemble de classes de hauteur, mais une suite de
// pas (demi-tons) qui boucle et génère toutes les notes MIDI possibles.

// Majeur (Ionien) : 2 2 1 2 2 2 1
inline const std::vector<int> MAJOR_SCALE = {2, 2, 1, 2, 2, 2, 1};

// Gamme "empruntée" : représente les notes empruntées au mode parallèle.
// Mineur naturel (Éolien) : 2 1 2 2 1 2 2
inline const std::vector<int> BORROWED_SCALE = {2, 1, 2, 2, 1, 2, 2};

// Chromatique : 12 demi-tons.
inline const std::vector<int> CHROMATIC_SCALE = {1,1,1,1,1,1,1,1,1,1,1,1};

// ---------------------------------------------------------------------------
// Optionnel : noms lisibles (pour debug / affichage)
// ---------------------------------------------------------------------------

// Noms des classes de hauteur (C..B). Utilisé par midi_to_letter(...) dans Utilities.cpp.
// std::string_view évite des soucis d’ordre d’initialisation statique.
inline constexpr std::array<std::string_view, 12> noteNames = {
  "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

// Noms de modes : générique. Si tu as un vrai système modal ailleurs,
// étends/remplace cette table.
inline constexpr std::array<std::string_view, 8> modeNames = {
  "Inconnu", "Mode1", "Mode2", "Mode3", "Mode4", "Mode5", "Mode6", "Mode7"
};

// ---------------------------------------------------------------------------
// Coûts par défaut pour la préférence de triade en 4 voix (constraints.cpp)
// ---------------------------------------------------------------------------
// Dans constraints.cpp (H8_4v_preferHarmonicTriad), le code référence :
//   not_harmonic_triad_cost, double_fifths_cost, double_thirds_cost,
//   triad_with_octave_cost
//
// On définit ici des valeurs par défaut sûres, pour que le modèle reste
// auto-portant. Si tu injectes ces coûts via paramètres ailleurs, tu peux
// ignorer/écraser ces constantes.
constexpr int not_harmonic_triad_cost = 10;
constexpr int double_fifths_cost      = 4;
constexpr int double_thirds_cost      = 6;
constexpr int triad_with_octave_cost  = 2;

// ---------------------------------------------------------------------------
// Petites fonctions utilitaires
// ---------------------------------------------------------------------------

// Normalisation en classe de hauteur (0..11), robuste aux valeurs négatives.
constexpr int pc(int semitones) {
  const int m = semitones % PERFECT_OCTAVE;
  return (m < 0) ? (m + PERFECT_OCTAVE) : m;
}

} // namespace fuxcp

#endif // FUXCP_MODEL_FUXTYPES_H
