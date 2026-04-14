#pragma once

#include <vector>

/*
==============================================================================
    ConstraintSettings.h --> Fichier le plus important

    Ce fichier définit l’ensemble des paramètres de contrôle du solveur FuxCP.

    On distingue DEUX types de paramètres :

    1. RULE PARAMETERS  (contraintes dures) --> CONTRAINTES
       → définissent ce qui est autorisé ou interdit dans la solution

    2. COST PARAMETERS  (pondérations) --> PREFERENCES
       → influencent la qualité de la solution

    Cette séparation est essentielle pour éviter toute ambiguïté.
==============================================================================
*/


//==============================================================================
// RULE PARAMETERS (Contraintes structurelles)
//==============================================================================
//
// Ces paramètres modifient directement les règles du contrepoint.
// Ils sont utilisés pour construire les contraintes dans FuxCP.
//
struct RuleParameters
{
    // Intervalle maximal autorisé entre deux notes consécutives
    //  influence directement la structure mélodique
    int maxLeap = 5;

    // Biais en faveur des mouvements conjoints (préférence)
    //  utilisé plus tard comme coût (optionnel)
    int stepBias = 0;

    // Autorise ou non la répétition de notes consécutives
    bool allowRepetition = true;

    // Contrôle la direction globale de la mélodie
    // (0 = neutre, >0 = ascendant, <0 = descendant)
    int direction = 0;
};


//==============================================================================
// COST PARAMETERS (Pondérations des contraintes) --> PREFERENCES
//==============================================================================
//
// Ces paramètres ne modifient PAS les règles,
// mais influencent le solveur dans le choix de la meilleure solution.
//
struct CostParameters
{
    // Coûts liés aux règles mélodiques
    std::vector<int> melodic;

    // Coûts généraux du système
    std::vector<int> general;

    // Coûts spécifiques à certaines espèces
    std::vector<int> specific;

    // Importance relative des contraintes
    std::vector<int> importance;
};


//==============================================================================
// GLOBAL SETTINGS (Regroupement complet)
//==============================================================================
//
// Cette structure regroupe tous les paramètres du solveur.
//
// Elle permet de passer UN SEUL objet à FuxCP,
// ce qui simplifie énormément l’architecture.
//
struct ConstraintSettings
{
    RuleParameters rules;
    CostParameters costs;

    int borrowMode = 0; //0 = strict, 1 = autorise certaines altérations
};