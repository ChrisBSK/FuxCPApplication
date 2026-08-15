#pragma once

#include <vector>
#include <juce_gui_basics/juce_gui_basics.h>

#include "../../controller/AppController.h"

/*
    SavedConfigurationsPage

    Page listant les configurations sauvegardées : nom choisi par
    l'utilisateur et date de sauvegarde, la plus récente en premier.

    Elle reprend le même habillage visuel que les autres pages secondaires
    (AboutPage, SolverExplanationPage) : un titre en haut, puis un cadre
    arrondi contenant le contenu, ici une liste au lieu d'un texte.

    Cliquer sur une ligne charge immédiatement la configuration correspondante :
    pas de bouton "Charger" séparé, pour rester le plus simple possible.

    Une petite croix "×" à droite de chaque ligne permet de supprimer la
    configuration correspondante (après confirmation).
*/
class SavedConfigurationsPage : public juce::Component,
                                private juce::ListBoxModel
{
public:
    SavedConfigurationsPage();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Connecte la page à AppController pour lire la liste des configurations.
    void setAppController(AppController* controller);

    /*
        Recharge la liste depuis le disque.

        À appeler chaque fois que la page redevient visible : une nouvelle
        configuration a pu être sauvegardée depuis la dernière visite.
    */
    void refresh();

    // Appelé quand l'utilisateur clique sur une configuration de la liste.
    std::function<void(const juce::File&)> onConfigurationSelected;

private:
    //==============================================================================
    // juce::ListBoxModel
    //==============================================================================
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& event) override;

    // Demande confirmation puis supprime la configuration de cette ligne.
    void confirmAndDelete(int row);

    // Largeur de la petite zone "×" à droite de chaque ligne, utilisée pour
    // supprimer une configuration. Les mêmes pixels servent à la dessiner
    // (paintListBoxItem) et à détecter le clic dessus (listBoxItemClicked).
    static constexpr int deleteZoneWidth = 26;

    // Largeur réelle d'une ligne, mémorisée au dernier passage de
    // paintListBoxItem : sert à retrouver la position de la zone "×" au clic.
    int lastRowWidth = 0;

    // Titre affiché en haut de la page.
    juce::Label titleLabel;

    // Message affiché à la place de la liste quand aucune configuration
    // n'a encore été sauvegardée.
    juce::Label emptyStateLabel;

    juce::ListBox listBox { "SavedConfigurationsList", this };

    AppController* appController = nullptr;

    // Copie locale de la liste affichée : le ListBoxModel a besoin d'un
    // accès synchrone pendant le rendu, refresh() la met à jour. Sa taille
    // suit simplement le nombre réel de configurations sauvegardées.
    std::vector<AppController::SavedConfigurationInfo> entries;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SavedConfigurationsPage)
};
