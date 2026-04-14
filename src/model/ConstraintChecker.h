#pragma once

#include <vector>
#include <cmath>
#include "../model/CantusProblem.h"

/*
==============================================================================
    ConstraintChecker

    Rôle :
    - Vérifier qu'une solution respecte les contraintes définies dans
      ConstraintSettings

    Avantage :
    - Centralise toutes les règles
    - Facile à étendre (MaxLeap, StepBias, Direction, etc.)
==============================================================================
*/

class ConstraintChecker
{
public:

    // =============================
    // Méthode principale
    // =============================
    static bool isSolutionValid(const std::vector<int>& solution,
                                const CantusProblem& problem)
    {
        return checkMaxLeap(solution, problem);
            //&& checkRepetition(solution, problem)
            //&& checkDirection(solution, problem);
    }

private:

    // =============================
    // 1. Max Leap
    // =============================
    static bool checkMaxLeap(const std::vector<int>& solution,
                             const CantusProblem& problem)
    {
        int maxLeap = problem.getSettings().rules.maxLeap;

        for (size_t i = 1; i < solution.size(); ++i)
        {
            int interval = std::abs(solution[i] - solution[i - 1]);

            if (interval > maxLeap)
                return false;
        }

        return true;
    }

    // =============================
    // 2. Repetition
    // =============================
    static bool checkRepetition(const std::vector<int>& solution,
                                const CantusProblem& problem)
    {
        bool allow = problem.getSettings().rules.allowRepetition;

        if (allow)
            return true;

        for (size_t i = 1; i < solution.size(); ++i)
        {
            if (solution[i] == solution[i - 1])
                return false;
        }

        return true;
    }

    // =============================
    // 3. Direction
    // =============================
    static bool checkDirection(const std::vector<int>& solution,
                               const CantusProblem& problem)
    {
        int direction = problem.getSettings().rules.direction;

        if (direction == 0)
            return true;

        int up = 0;
        int down = 0;

        for (size_t i = 1; i < solution.size(); ++i)
        {
            if (solution[i] > solution[i - 1]) up++;
            if (solution[i] < solution[i - 1]) down++;
        }

        if (direction > 0)
            return up >= down;

        if (direction < 0)
            return down >= up;

        return true;
    }
};