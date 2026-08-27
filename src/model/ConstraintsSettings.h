//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   ConstraintSettings.h

   Représente l'état des réglages choisis dans l'interface : valeurs des
   sliders, shapes, priorités du solveur, options globales (borrow mode,
   méthode de recherche/minimisation).

    GenerationService lit ces réglages et les transforme en CostModel
   ou en vecteurs de coûts compréhensibles par FuxCP.
   =====================================================================
*/

#pragma once

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

struct ConstraintSettings
{

    // =====================================================================
    //   Part1. INDICES DES COÛTS FUXCP
    //
    //   Ces enums servent à lire les vecteurs de coûts dans le bon ordre.
    //   L'ordre doit rester aligné avec FuxCP / Dorian.
    //
    // =====================================================================

    /*
        Indices du vecteur mélodique utilisé par FuxCP.

        Exemple :
        - secondCost  -> coût associé à l'intervalle de seconde
        - fifthCost   -> coût associé à l'intervalle de quinte
        - octaveCost  -> coût associé à l'octave

        Ces noms évitent d'écrire melodicCosts[0], melodicCosts[4], etc.
    */
    enum MelodicCostIndex
    {
        secondCost = 0,
        thirdCost,
        fourthCost,
        tritoneCost,
        fifthCost,
        sixthCost,
        seventhCost,
        octaveCost
    };

    /*
        Indices du vecteur des coûts spécifiques.

        Ces coûts ne sont pas les coûts mélodiques principaux.
        Ils décrivent plutôt des règles particulières de contrepoint :
        cambiata, syncopation, triad3, m2, etc.

        Ces noms évitent d'écrire specificCosts[1], specificCosts[5], etc.
    */
    enum SpecificCostIndex
    {
        penultSixthCost = 0,
        cambiataCost,
        mSkipCost,
        triad3rdCost,
        m2ZeroCost,
        syncopationCost,
        prefSlider
    };

    /*
        Indices du vecteur des coûts généraux.

        Ces coûts concernent les grandes relations harmoniques ou structurelles :
        borrow, fifth, octave, variety, triad, direct motion, etc.

        Ces noms évitent d'écrire generalCosts[0], generalCosts[2], etc.
    */
    enum GeneralCostIndex
    {
        borrowCost = 0,
        harmonicFifthCost,
        harmonicOctaveCost,
        successiveCost,
        varietyCost,
        triadCost,
        directMotionCost,
        penultCost
    };




    // =====================================================================
    //   Part2. TYPES CHOISIS PAR L'INTERFACE
    //
    //   Ces enums décrivent les choix faits par l'utilisateur dans l'application.
    //   Ils ne sont pas encore les coûts FuxCP eux-mêmes.
    //   Ils servent à traduire l'interface vers le modèle.
    //
    // =====================================================================

    /*
        Fonction de coût contrôlée par un slider ou une shape.

        Exemple :
        - melodyMovement   -> le slider pilote steps1(s)
        - intervalColour   -> le slider pilote steps2(s)
        - perfectIntervals -> le slider pilote harmo(s)

        Ce type permet de dire :
        "cette shape s'applique à quelle fonction de coût ?"
    */
    enum class ShapeCostTarget
    {
        melodyMovement = 0,
        intervalColour,
        perfectIntervals
    };

    /*
        Shape choisie dans une ComboBox de l'interface.

        Chaque valeur correspond à une fonction build_*_shape définie par Dorian.

        Exemple :
        - linear     -> s monte progressivement de 0 à 1
        - invertedV  -> s monte puis redescend
        - m          -> s suit une forme de M
    */
    enum class ShapeType
    {
        fixedZero = 0,
        fixedOne,
        linear,
        linearDescending,
        invertedV,
        v,
        m,
        step,
        stepDescending
    };

    /*
        Méthode de recherche choisie dans la colonne Search.

        Exemple :
        - dfs -> Depth First Search
        - bab -> Branch and Bound

        --> (Pourrait être étendu à d'autres méthodes de recherche)

        Par défaut, l'application utilise bab.
    */
    enum class SearchMethod
    {
        dfs = 0,
        bab
    };

    /*
        Méthode de minimisation choisie dans la colonne Search.

        Exemple :
        - lexicographic -> FuxCP minimise finalCosts dans l'ordre des priorités
        - weightedSum   -> FuxCP minimise la somme coût x poids

        Par défaut, l'application reste en lexicographic.
    */
    enum class MinimizationMethod
    {
        lexicographic = 0,
        weightedSum
    };




    // =====================================================================
    // Part3. STRUCTURES DE DONNÉES
    //
    // Ces structures regroupent les réglages de l'interface.
    //
    // Elles permettent de stocker :
    //   - une shape appliquée à une voix
    //   - les valeurs de sliders d'un contrepoint
    //   - les réglages globaux du solveur (ex: BAB/Borrow Mode)
    //
    // =====================================================================

    /*
        Décrit une shape appliquée à une fonction de coût pour une voix.

        Exemple :
        - voiceIndex = 0               -> Contrepoint 1
        - target = melodyMovement      -> on pilote steps1(s)
        - shape = invertedV            -> on part d'une forme en V inversé
        - controlValues = {0, 0.5, 1, 0.5, 0} -> les 5 points visibles dans l'interface

        Remarque: - une shape ne s'applique pas au cantus firmus.
                  - elle s'applique à un contrepoint et à une fonction de coût précise.
    */
    struct ShapeAssignment
    {
        int voiceIndex = 0;
        ShapeCostTarget target = ShapeCostTarget::melodyMovement;
        ShapeType shape = ShapeType::invertedV;
        std::vector<double> controlValues;
    };

    /*
        Valeurs des sliders pour un contrepoint.

        Chaque contrepoint possède ses propres valeurs de fonctions de coûts.
        Modifier CP1 ne modifie donc pas CP2 ou CP3.

        Exemple :
        - melodyMovement   -> valeur envoyée à steps1(s)
        - intervalColour   -> valeur envoyée à steps2(s)
        - perfectIntervals -> valeur envoyée à harmo(s)
    */
    struct CounterpointCostParameters
    {
        double melodyMovement = 0.0;
        double intervalColour = 0.0;
        double perfectIntervals = 0.0;
    };

    /*
        Paramètres mélodiques globaux.

        Ces valeurs pilotent les deux fonctions mélodiques définies par Dorian :

        avoidLargeLeap pilote steps1(s) :
        - 0.0 -> favorise les mouvements conjoints
        - 1.0 -> favorise les sauts

        melodicIntervalColor pilote steps2(s) :
        - 0.0 -> pénalise fortement les dissonances mélodiques
        - 1.0 -> pénalise fortement les consonances parfaites

        Remarque :
        l'interface principale utilise maintenant surtout les valeurs par
        contrepoint dans CounterpointCostParameters.
    */
    struct Melodic
    {
        double avoidLargeLeap = 0.0;
        double melodicIntervalColor = 0.0;
    };

    /*
        Emplacement prévu pour les paramètres généraux.
        Pour l'instant, il est vide.
        On le garde seulement si on veut ajouter plus tard des réglages généraux
    */
    struct General
    {
        //TODO
    };


    /*
        Paramètre harmonique global.

        perfectIntervalBalance pilote harmo(s) :
        - 0.0 -> on pénalise davantage les octaves
        - 1.0 -> on pénalise davantage les quintes
    */
    struct Harmonic
    {
        double perfectIntervalBalance = 0.0;
    };

    /*
        Emplacement prévu pour les paramètres spécifiques.

        Pour l'instant, il est vide.
        On le garde seulement si on veut ajouter plus tard des réglages comme
        cambiata, syncopation, triad3, etc.
    */
    struct Specific
    {
        //TODO
    };

    /*
        Paramètres globaux du solveur.

        Ces réglages ne dépendent pas d'un contrepoint précis.
        Ils concernent toute la génération.
    */
    struct Global
    {
        int borrowMode = 1;
        SearchMethod searchMethod = SearchMethod::bab;
        MinimizationMethod minimizationMethod = MinimizationMethod::lexicographic;
    };

    /*
        Ordre de priorité utilisé par l'optimisation lexicographique.

        1 = priorité la plus forte.
        14 = priorité la plus faible.

        L'ordre du vecteur doit rester cohérent avec l'ordre des coûts FuxCP.
    */
    struct Importance
    {
        std::vector<int> costs {
            14, // borrow
            6,  // fifth
            5,  // octave
            2,  // succ
            9,  // variety
            3,  // triad
            8,  // direct
            10, // motion
            12, // penult
            11, // cambiata
            4,  // triad3
            13, // m2
            1,  // syncopation
            7   // melodic
        };
    };




    // =====================================================================
    // Part4. ÉTAT ACTUEL DES RÉGLAGES
    //
    // Ces variables contiennent les choix actuels de l'utilisateur.
    // L'interface modifie ces valeurs, puis GenerationService les lit pour
    // construire le problème FuxCP.
    //
    // =====================================================================

    // Paramètres mélodiques globaux encore utilisés pour certains resets/defaults.
    Melodic melodic;

    // Paramètre harmonique global encore utilisé pour certains resets/defaults.
    Harmonic harmonic;

    // Paramètres globaux du solveur : borrow mode, méthode de recherche, etc.
    Global global;

    // Ordre de priorité des contraintes pour l'optimisation lexicographique.
    Importance importance;

    // Shapes choisies dans l'interface.
    // Chaque élément dit : voix + fonction de coût + shape + 5 valeurs.
    std::vector<ShapeAssignment> shapeAssignments;

    /*
        Valeurs des sliders propres à chaque contrepoint.

        Chaque élément du vecteur correspond à un contrepoint :
        - counterpointCostParameters[0] -> paramètres de Contrepoint 1
        - counterpointCostParameters[1] -> paramètres de Contrepoint 2
        - counterpointCostParameters[2] -> paramètres de Contrepoint 3

        Exemple :
        si l'utilisateur bouge "Melody moves" dans la colonne Contrepoint 1,
        on modifie counterpointCostParameters[0].melodyMovement.

        Si l'utilisateur bouge "Melody moves" dans la colonne Contrepoint 2,
        on modifie counterpointCostParameters[1].melodyMovement.

        Remarque :
        - Les contrepoints sont indépendants.
        - Modifier un slider dans CP1 ne modifie jamais les valeurs de CP2 ou CP3.
    */
    std::vector<CounterpointCostParameters> counterpointCostParameters;

    // =====================================================================
    // Part5. CONSTRUCTION DES VECTEURS ENVOYÉS À FUXCP
    //
    // Ces méthodes traduisent les réglages stockés dans ConstraintSettings
    // en vecteurs de coûts compréhensibles par FuxCP.
    //
    // Elles préparent les données que GenerationService utilisera.
    //
    // =====================================================================

    /*
    Construit le vecteur des coûts mélodiques globaux.

    Ce vecteur correspond à m_costs dans FuxCP.

        Ordre du vecteur :
        - seconde
        - tierce
        - quarte
        - triton
        - quinte
        - sixte
        - septième
        - octave

        Exemple:
         -> les coûts mélodiques peuvent être modifiés par steps1(s) et steps2(s), pour l'instant.

        steps1(s) agit sur le mouvement mélodique :
        - s = 0.0 -> favorise les mouvements conjoints
        - s = 1.0 -> favorise les sauts

        Donc si s augmente :
        - le coût des secondes augmente
        - le coût des grands intervalles diminue
        - le contrepoint est encouragé à faire plus de sauts

        -----

        steps2(s) agit sur la couleur des intervalles :
        - s = 0.0 -> pénalise fortement les dissonances mélodiques
        - s = 1.0 -> pénalise fortement les consonances parfaites

        Donc si s augmente :
        - les secondes, quartes et septièmes deviennent moins coûteuses
        - les quintes et octaves deviennent plus coûteuses
        - le contrepoint est encouragé à utiliser une couleur plus dissonante

    */
    std::vector<int> buildMelodicCosts(int cantusFirmusLength) const;

    /*
        Construit le vecteur des coûts généraux.

        Ce vecteur correspond à g_costs dans FuxCP.

        Ordre du vecteur :
        - borrow
        - fifth
        - octave
        - succ
        - variety
        - triad
        - direct motion
        - penult

        Exemple :
         -> les coûts fifth et octave peuvent être modifiés par harmo(s), pour l'instant.

        harmo(s) agit sur l'équilibre entre quintes et octaves :
        - s = 0.0 -> pénalise davantage les octaves
        - s = 1.0 -> pénalise davantage les quintes

        Donc si s augmente :
        - le coût des quintes augmente
        - le coût des octaves diminue
    */
    std::vector<int> buildGeneralCosts() const;

    /*
        Construit le vecteur des coûts spécifiques.

        Ce vecteur correspond à s_costs dans FuxCP.

        Ordre du vecteur :
        - penultSixth
        - cambiata
        - mSkip
        - triad3
        - m2
        - syncopation
        - prefSlider

        Pour l'instant, ces coûts restent surtout des valeurs par défaut.
    */
    std::vector<int> buildSpecificCosts() const;

    /*
        Construit le vecteur d'importance du solveur.

        Ce vecteur définit l'ordre des priorités utilisé en optimisation
        lexicographique.

        1 = coût le plus prioritaire.
        14 = coût le moins prioritaire.

        Exemple :
        si syncopation vaut 1, alors la syncopation est traitée comme une priorité
        très forte par le solveur.

        Donc en pratique :
        - plus le nombre est petit, plus le coût est important
        - plus le nombre est grand, plus le coût est secondaire
        - changer l'ordre modifie la manière dont le solveur choisit une solution
    */
    std::vector<int> buildImportanceCosts() const;



    // =====================================================================
    // Part6. PARAMÈTRES GLOBAUX
    //
    // Ces méthodes modifient les réglages qui concernent toute la génération.
    // Elles ne ciblent pas un contrepoint précis.
    //
    // =====================================================================

    /* Modifie la valeur globale envoyée à steps1(s). */
    void setLargeLeapPenalty(double value)
    {
        melodic.avoidLargeLeap = value;
    }

    /* Retourne la valeur globale utilisée pour steps1(s). */
    double getLargeLeapPenalty() const
    {
        return melodic.avoidLargeLeap;
    }

    /* Modifie la valeur globale envoyée à steps2(s). */
    void setMelodicIntervalColor(double value)
    {
        melodic.melodicIntervalColor = value;
    }

    /* Retourne la valeur globale utilisée pour steps2(s). */
    double getMelodicIntervalColor() const
    {
        return melodic.melodicIntervalColor;
    }

    /* Modifie la valeur globale envoyée à harmo(s). */
    void setPerfectIntervalBalance(double value)
    {
        harmonic.perfectIntervalBalance = value;
    }

    /* Retourne la valeur globale utilisée pour harmo(s). */
    double getPerfectIntervalBalance() const
    {
        return harmonic.perfectIntervalBalance;
    }

    /*
        Active ou désactive Borrow Mode.

        - value = 0 -> Borrow Mode désactivé
        - value != 0 -> Borrow Mode activé

        Borrow Mode indique si le solveur peut utiliser des notes empruntées
        hors du cadre strict de la gamme.
    */
    void setBorrowMode(int value)
    {
        global.borrowMode = (value == 0 ? 0 : 1);
    }

    /* Retourne l'état actuel de Borrow Mode. */
    int getBorrowMode() const
    {
        return global.borrowMode;
    }

    /*
        Choisit la méthode de recherche du solveur.

        Exemples :
        - SearchMethod::dfs -> Depth First Search
        - SearchMethod::bab -> Branch and Bound
    */
    void setSearchMethod(SearchMethod method)
    {
        global.searchMethod = method;
    }

    /* Retourne la méthode de recherche actuellement choisie. */
    SearchMethod getSearchMethod() const
    {
        return global.searchMethod;
    }

    /*
        Choisit la méthode de minimisation du solveur.

        Ce réglage est lu par GenerationService au moment de créer le problème FuxCP.
    */
    void setMinimizationMethod(MinimizationMethod method)
    {
        global.minimizationMethod = method;
    }

    /* Retourne la méthode de minimisation actuellement choisie. */
    MinimizationMethod getMinimizationMethod() const
    {
        return global.minimizationMethod;
    }

    /*
        Remplace l'ordre des priorités du solveur.

        On vérifie d'abord que le nouveau vecteur a la bonne taille.
        Cela évite de donner au solveur un ordre incomplet ou invalide.
    */
    void setImportanceCosts(const std::vector<int>& values)
    {
        if (values.size() == importance.costs.size())
            importance.costs = values;
    }

    /*
        Retourne l'ordre actuel des priorités.

        Utilisé par l'interface pour afficher la liste Solver Priorities,
        et par le solveur pour construire le vecteur d'importance.
    */
    const std::vector<int>& getImportanceCosts() const
    {
        return importance.costs;
    }


    // =====================================================================
    // Part7. SHAPES PAR VOIX
    //
    // Ces méthodes gèrent les shapes choisies dans l'interface.
    //
    // Une shape est toujours liée à :
    //   - un contrepoint,
    //   - une fonction de coût,
    //   - un type de shape,
    //   - quatre valeurs de contrôle visibles dans l'interface.
    //
    // =====================================================================

    /*
        Remplace toute la liste des shapes.

        On utilise cette méthode quand on veut repartir d'un état complet.

        Exemple :
        - quand on clique sur Clear, on peut envoyer une liste vide {}
        - toutes les anciennes shapes sont alors supprimées

        Attention :
        cette méthode ne modifie pas une seule shape.
        Elle remplace toute la liste.
    */
    void setShapeAssignments(std::vector<ShapeAssignment> values)
    {
        shapeAssignments = std::move(values);
    }

    /*
        Ajoute ou remplace une shape pour une voix et une fonction de coût.

        Chaque shape est identifiée par le couple :
        - voiceIndex -> le contrepoint concerné
        - target     -> la fonction de coût concernée

        Exemple :
        voiceIndex = 0 et target = melodyMovement
        signifie :
        "shape appliquée à Melody Movement dans Contrepoint 1".

        Si une shape existe déjà pour ce couple voix/fonction,
        on la remplace.

        Si aucune shape n'existe encore,
        on l'ajoute à la liste.
    */
    void setShapeAssignment(int voiceIndex,
                            ShapeCostTarget target,
                            ShapeType shape,
                            std::vector<double> controlValues)
    {
        // On cherche d'abord si cette voix possède déjà une shape pour cette fonction.
        for (auto& assignment : shapeAssignments)
        {
            if (assignment.voiceIndex == voiceIndex && assignment.target == target)
            {
                // Si oui, on remplace son contenu.
                assignment.shape = shape;
                assignment.controlValues = std::move(controlValues);
                return;
            }
        }
        // Sinon, on crée une nouvelle shape.
        shapeAssignments.push_back({
            voiceIndex,
            target,
            shape,
            std::move(controlValues)
        });
    }

    /*
        Retire la shape d'une fonction de coût pour une voix.

        Exemple :
        si on retire la shape de Melody Movement dans CP1,
        alors CP1 revient à la valeur simple du slider Melody Movement.

        Cette méthode ne supprime pas toutes les shapes.
        Elle supprime seulement celle qui correspond au couple :
        voiceIndex + target.
    */
    void removeShapeAssignment(int voiceIndex, ShapeCostTarget target)
    {
        shapeAssignments.erase(
            std::remove_if(shapeAssignments.begin(),
                           shapeAssignments.end(),
                           [voiceIndex, target](const ShapeAssignment& assignment)
                           {
                               return assignment.voiceIndex == voiceIndex &&
                                      assignment.target == target;
                           }),
            shapeAssignments.end());
    }

    /*
        Retourne toutes les shapes actuellement enregistrées.

        GenerationService utilise cette liste pour construire le CostModel
        envoyé à FuxCP.
    */
    const std::vector<ShapeAssignment>& getShapeAssignments() const
    {
        return shapeAssignments;
    }

    // =====================================================================
    // Part8. SLIDERS PAR CONTREPOINT
    //
    // Ces méthodes stockent les valeurs des sliders pour chaque contrepoint.
    //
    // =====================================================================

    /*
        Ajuste le nombre de contrepoints stockés dans le modèle.

        Exemple :
        - 2 voix au total -> 1 contrepoint
        - 3 voix au total -> 2 contrepoints
        - 4 voix au total -> 3 contrepoints

        Cette méthode prépare le vecteur counterpointCostParameters
        pour qu'il contienne une case par contrepoint.
    */
    void setCounterpointCount(int counterpointCount)
    {
        if (counterpointCount < 0)
            counterpointCount = 0;

        counterpointCostParameters.resize(static_cast<std::size_t>(counterpointCount));
    }

    /* Modifie Melody moves pour un contrepoint précis.

       Exemple :
            counterpointIndex = 0 -> on modifie Contrepoint 1.
     */
    void setCounterpointMelodyMovement(int counterpointIndex, double value)
    {
        if (auto* parameters = getCounterpointCostParametersPointer(counterpointIndex))
            parameters->melodyMovement = value;
    }

    /*
        Modifie Interval colour pour un contrepoint précis.

        Exemple :
            counterpointIndex = 1 -> on modifie Contrepoint 2.
    */
    void setCounterpointIntervalColour(int counterpointIndex, double value)
    {
        if (auto* parameters = getCounterpointCostParametersPointer(counterpointIndex))
            parameters->intervalColour = value;
    }

    /*
        Modifie Perfect int. pour un contrepoint précis.

        Exemple :
            counterpointIndex = 2 -> on modifie Contrepoint 3.
    */
    void setCounterpointPerfectIntervals(int counterpointIndex, double value)
    {
        if (auto* parameters = getCounterpointCostParametersPointer(counterpointIndex))
            parameters->perfectIntervals = value;
    }

    /*
        Retourne les trois valeurs de sliders d'un contrepoint.

        Exemple :
        getCounterpointCostParameters(0)
        retourne les valeurs de Contrepoint 1.

        Si l'index demandé n'existe pas, on retourne des valeurs par défaut.
    */
    CounterpointCostParameters getCounterpointCostParameters(int counterpointIndex) const
    {
        if (counterpointIndex < 0 ||
            counterpointIndex >= static_cast<int>(counterpointCostParameters.size()))
        {
            return {};
        }

        return counterpointCostParameters[static_cast<std::size_t>(counterpointIndex)];
    }

    /*
        Retourne toutes les valeurs de sliders par contrepoint.

        Utile si une partie du programme veut lire toute la configuration
        des contrepoints en une seule fois.
    */
    const std::vector<CounterpointCostParameters>& getAllCounterpointCostParameters() const
    {
        return counterpointCostParameters;
    }

private:


    // =====================================================================
    // Part9. OUTIL INTERNE
    //
    // Cette méthode sert seulement à l'intérieur de ConstraintSettings.
    // Elle évitent de dupliquer la logique dans les setters publics.
    // =====================================================================

    /*
        Retourne un pointeur vers les paramètres d'un contrepoint.

        Si le contrepoint demandé n'existe pas encore, on agrandit le vecteur.
        Cela permet à l'interface de modifier CP1, CP2 ou CP3 sans devoir
        créer manuellement les cases avant.

        Exemple :
        si counterpointIndex = 2 et que le vecteur ne contient encore que CP1,
        on agrandit le vecteur pour créer CP2 et CP3.
    */
    CounterpointCostParameters* getCounterpointCostParametersPointer(int counterpointIndex)
    {
        if (counterpointIndex < 0)
            return nullptr;

        if (counterpointIndex >= static_cast<int>(counterpointCostParameters.size()))
            counterpointCostParameters.resize(static_cast<std::size_t>(counterpointIndex + 1));

        return &counterpointCostParameters[static_cast<std::size_t>(counterpointIndex)];
    }


    // =====================================================================
    // Part10. VALEURS PAR DÉFAUT FUXCP
    //
    // Ces méthodes retournent les coûts de base.
    //
    // Elles sont privées parce que l'extérieur doit utiliser buildMelodicCosts(),
    // buildGeneralCosts(), buildSpecificCosts() et buildImportanceCosts().
    //
    // =====================================================================

    /*
        Retourne les coûts mélodiques de base.

        Ces valeurs servent de point de départ avant l'application éventuelle
        de sliders ou de CostModel.

        -> correspond à m_costs initialement
    */
    std::vector<int> buildDefaultMelodicCosts(int cantusFirmusLength) const;

    /*
        Retourne les coûts généraux de base.

        Ces valeurs servent de point de départ avant l'application éventuelle
        de sliders ou de CostModel.

        -> correspond à g_costs initialement
    */
    std::vector<int> buildDefaultGeneralCosts() const;

    /*
        Retourne les coûts spécifiques de base.

        Ces valeurs servent de point de départ avant l'application éventuelle
        de sliders ou de CostModel.

        -> correspond à s_costs initialement
    */
    std::vector<int> buildDefaultSpecificCosts() const;

    /*
        Retourne l'ordre de priorité de base.

        Sert de référence pour construire le vecteur d'importance.
    */
    std::vector<int> buildDefaultImportanceCosts() const;
};
