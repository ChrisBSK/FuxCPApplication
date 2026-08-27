//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   SavedConfigurationsPage.cpp

   Construit et gère la liste des configurations sauvegardées (ListBox).


   AppController::getSavedConfigurations()/loadConfiguration()/
   deleteConfiguration().
//==============================================================================
*/

#include "SavedConfigurationsPage.h"


SavedConfigurationsPage::SavedConfigurationsPage()
{
    titleLabel.setText(juce::String::fromUTF8("Saved configurations"),
                       juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    // Affiché uniquement quand la liste est vide (voir refresh()).
    emptyStateLabel.setText(
        juce::String::fromUTF8("Aucune configuration enregistrée pour l'instant.\n"
                               "Utilisez le bouton \"Save configuration\" depuis l'écran principal."),
        juce::dontSendNotification);
    emptyStateLabel.setJustificationType(juce::Justification::centred);
    emptyStateLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.6f));
    emptyStateLabel.setFont(juce::Font(14.0f));
    addChildComponent(emptyStateLabel);

    listBox.setRowHeight(42);
    listBox.setColour(juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    listBox.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(listBox);
}

//==============================================================================
// Rendu et layout
//==============================================================================

void SavedConfigurationsPage::paint(juce::Graphics& g)
{
    // Même cadre arrondi que les autres pages secondaires.
    auto area = getLocalBounds().reduced(22);

    g.setColour(juce::Colour(0xff3f3f3f));
    g.fillRoundedRectangle(area.toFloat(), 8.0f);

    g.setColour(juce::Colours::white.withAlpha(0.25f));
    g.drawRoundedRectangle(area.toFloat(), 8.0f, 1.0f);
}

void SavedConfigurationsPage::resized()
{
    auto area = getLocalBounds().reduced(42, 32);

    titleLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(12);

    // La liste et le message "aucune configuration" occupent la même zone :
    // un seul des deux est visible à la fois (voir refresh()).
    listBox.setBounds(area);
    emptyStateLabel.setBounds(area);
}

//==============================================================================
// Connexion et rafraîchissement
//==============================================================================

void SavedConfigurationsPage::setAppController(AppController* controller)
{
    appController = controller;
}

/*
    Relit la liste des configurations sauvegardées et rafraîchit l'affichage.

    Appelée à chaque ouverture de la page : une sauvegarde a pu être faite
    depuis le dernier passage sur cet onglet.
*/
void SavedConfigurationsPage::refresh()
{
    entries = appController != nullptr
        ? appController->getSavedConfigurations()
        : std::vector<AppController::SavedConfigurationInfo>{};

    const bool hasEntries = ! entries.empty();
    emptyStateLabel.setVisible(! hasEntries);
    listBox.setVisible(hasEntries);

    listBox.updateContent();
    listBox.repaint();
}

//==============================================================================
// juce::ListBoxModel
//==============================================================================

int SavedConfigurationsPage::getNumRows()
{
    return (int) entries.size();
}

/*
    Dessine une ligne : le nom choisi par l'utilisateur en haut,
    la date de sauvegarde en dessous, en plus petit et plus discret.
*/
void SavedConfigurationsPage::paintListBoxItem(int rowNumber,
                                               juce::Graphics& g,
                                               int width,
                                               int height,
                                               bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= (int) entries.size())
        return;

    // Mémorisé pour que listBoxItemClicked sache où se trouve la zone "×"
    // (les lignes n'ont pas toutes exactement la même largeur si une
    // scrollbar apparaît).
    lastRowWidth = width;

    auto bounds = juce::Rectangle<int>(0, 0, width, height).reduced(4, 3);

    if (rowIsSelected)
    {
        g.setColour(juce::Colour(0xff2f4f4f));
        g.fillRoundedRectangle(bounds.toFloat(), 5.0f);
    }

    // La zone "×" est retirée en premier, à droite : le texte (nom + date)
    // se partage ensuite tout l'espace restant.
    auto deleteZone = bounds.removeFromRight(deleteZoneWidth);

    g.setColour(juce::Colours::white.withAlpha(0.45f));
    g.setFont(juce::Font(15.0f, juce::Font::bold));
    g.drawText(juce::String::fromUTF8("×"), deleteZone, juce::Justification::centred);

    auto textArea = bounds.reduced(10, 0);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText(entries[(size_t) rowNumber].name,
              textArea.removeFromTop(height / 2),
              juce::Justification::centredLeft,
              true);

    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(juce::Font(11.5f));
    g.drawText(entries[(size_t) rowNumber].dateDisplay,
              textArea,
              juce::Justification::centredLeft,
              true);
}

/*
    Un clic dans la zone "×" supprime la configuration (après confirmation).
    Un clic ailleurs sur la ligne la charge directement : pas de bouton
    "Charger" séparé, pour rester le plus simple possible côté interface.
*/
void SavedConfigurationsPage::listBoxItemClicked(int row, const juce::MouseEvent& event)
{
    if (row < 0 || row >= (int) entries.size())
        return;

    // La zone "×" occupe les derniers deleteZoneWidth pixels de la ligne.
    const int rowWidth = lastRowWidth > 0 ? lastRowWidth : listBox.getWidth();

    if (event.getPosition().x >= rowWidth - deleteZoneWidth)
    {
        confirmAndDelete(row);
        return;
    }

    if (onConfigurationSelected)
        onConfigurationSelected(entries[(size_t) row].file);
}

/*
    Demande confirmation avant de supprimer une configuration : une
    suppression est définitive, contrairement à un chargement.
*/
void SavedConfigurationsPage::confirmAndDelete(int row)
{
    if (row < 0 || row >= (int) entries.size())
        return;

    auto file = entries[(size_t) row].file;
    auto name = entries[(size_t) row].name;

    juce::AlertWindow::showOkCancelBox(
        juce::AlertWindow::WarningIcon,
        juce::String::fromUTF8("Supprimer la configuration"),
        juce::String::fromUTF8("Supprimer définitivement « ") + name + juce::String::fromUTF8(" » ?"),
        juce::String::fromUTF8("Supprimer"),
        juce::String::fromUTF8("Annuler"),
        nullptr,
        juce::ModalCallbackFunction::create([this, file](int result)
        {
            // result vaut 1 pour le premier bouton ("Supprimer"), 0 sinon.
            if (result != 1)
                return;

            if (appController != nullptr)
                appController->deleteConfiguration(file);

            // La ligne supprimée doit disparaître immédiatement de la liste.
            refresh();
        }));
}
