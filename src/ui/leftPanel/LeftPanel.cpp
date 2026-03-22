#include "LeftPanel.h"
#include "../optionsWindow/OptionsPanel.h"

static std::vector<int> parseCantusFirmus(const juce::String& text);

namespace Messages
{
    static const juce::String titleError   = juce::String::fromUTF8(u8"Erreur");
    static const juce::String titleSuccess = juce::String::fromUTF8(u8"Succès");

    static const juce::String cfEmpty      = juce::String::fromUTF8(u8"Le Cantus Firmus est vide.");
    static const juce::String cfNotNumbers = juce::String::fromUTF8(u8"Le Cantus Firmus ne doit contenir que des nombres (0-127).");
    static const juce::String cfInvalid    = juce::String::fromUTF8(u8"Le Cantus Firmus est invalide (0-127).");

    static const juce::String noVoices     = juce::String::fromUTF8(u8"Veuillez sélectionner un nombre de voix.");
    static const juce::String noSpecies    = juce::String::fromUTF8(u8"Veuillez sélectionner une espèce.");

    static const juce::String noSolution   = juce::String::fromUTF8(u8"Aucune solution trouvée");
}

LeftPanel::LeftPanel() {
    label.setText("Cantus Firmus (MIDI)",juce::sendNotification);
    addAndMakeVisible(text);
    addAndMakeVisible(label);


    voices.addItem(("2"), 2);
    voices.addItem(("3"), 3);
    voices.addItem(("4"), 4);

    labelVoices.setText("Number of voices",juce::sendNotification);
    addAndMakeVisible(voices);
    addAndMakeVisible(labelVoices);

    species.addItem(("1"), 1);
    species.addItem(("2"), 2);
    species.addItem(("3"), 3);
    species.addItem(("4"), 4);
    species.addItem(("5"), 5);

    labelSpecies.setText("Species",juce::sendNotification);
    addAndMakeVisible(species);
    addAndMakeVisible(labelSpecies);

    moreOptions.setButtonText("More Options");
    addAndMakeVisible(moreOptions);


    moreOptions.onClick = [this]()
    {

        auto* content = new OptionsPanel();

        juce::DialogWindow::LaunchOptions dialog;

        dialog.content.setOwned(content);
        dialog.dialogTitle = "More Options";
        dialog.dialogBackgroundColour = juce::Colours::darkgrey;

        dialog.escapeKeyTriggersCloseButton = true;
        dialog.useNativeTitleBar = true;
        dialog.resizable = true;

        dialog.launchAsync();
    };

    generateButton.setButtonText("Generate");
    addAndMakeVisible(generateButton);

    generateButton.onClick = [this]()
    {
        GenerationInput input;
        juce::String error;

        if (!collectAndValidateInput(input, error))
        {
            showAlert(juce::AlertWindow::WarningIcon,
                      Messages::titleError,
                      error);
            return;
        }

        prepareOutputFile();

        bool started = generationService.startGeneration(
            input.cantusFirmus,
            input.species,
            input.numberOfVoices,
            midiOutFileToGenerate.getFullPathName()
        );

        if (!started)
        {
            showAlert(juce::AlertWindow::WarningIcon,
                      Messages::titleError,
                      generationService.getLastError());
        }
    };

}


bool LeftPanel::collectAndValidateInput(GenerationInput& input, juce::String& error)
{
    auto rawText = text.getText().trim();

    if (rawText.isEmpty())
    {
        error = Messages::cfEmpty;
        return false;
    }

    if (!rawText.containsOnly("0123456789 ,;"))
    {
        error = Messages::cfNotNumbers;
        return false;
    }

    input.cantusFirmus = parseCantusFirmus(rawText);

    if (input.cantusFirmus.empty())
    {
        error = Messages::cfInvalid;
        return false;
    }

    if (voices.getSelectedItemIndex() == -1)
    {
        error = Messages::noVoices;
        return false;
    }
    input.numberOfVoices = voices.getSelectedId();

    if (species.getSelectedItemIndex() == -1)
    {
        error = Messages::noSpecies;
        return false;
    }
    input.species = species.getSelectedId();

    return true;
}

void LeftPanel::prepareOutputFile()
{
    auto desktop = juce::File::getSpecialLocation(
        juce::File::userDesktopDirectory);

    midiOutFileToGenerate = desktop.getChildFile(
        "FuxCP_Solution_" +
        juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S") +
        ".mid");
}




LeftPanel::~LeftPanel()
{
    //le thread solver est terminé avant destruction
    //generationService.stopThread(-1);
    generationService.stopThread(2000);


}

void LeftPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgreen);

    g.setColour(juce::Colours::white);

}

void LeftPanel::resized()
{
    int margin = 20;
    int width = 150;
    int labelHeight = 25;
    int boxHeight = 30;
    int spacing = 10;

    int y = margin;

    // Cantus Firmus
    label.setBounds(margin, y, width, labelHeight);
    y += labelHeight + spacing;

    text.setBounds(margin, y, width, boxHeight);
    y += boxHeight + 30;


    // Number of voices
    labelVoices.setBounds(margin, y, width, labelHeight);
    y += labelHeight + spacing;

    voices.setBounds(margin, y, width, boxHeight);
    y += boxHeight + 30;


    // Species
    labelSpecies.setBounds(margin, y, width, labelHeight);
    y += labelHeight + spacing;

    species.setBounds(margin, y, width, boxHeight);

    y += boxHeight + 70; // espace après species

    moreOptions.setBounds(margin, y, width, boxHeight);
    y += boxHeight + spacing;

    generateButton.setBounds(margin, y, width, boxHeight);
    y += boxHeight + spacing;


}

static std::vector<int> parseCantusFirmus(const juce::String& text)
{
    std::vector<int> result;

    if (text.isEmpty())
        return result;

    auto tokens = juce::StringArray::fromTokens(text, " ,;", "\"");

    for (const auto& t : tokens)
    {
        if (t.isEmpty())
            return {}; // évite cas "60,,64"

        if (!t.containsOnly("0123456789"))
            return {}; // invalide → on rejette tout

        int value = t.getIntValue();

        if (value < 0 || value > 127)
            return {}; // hors MIDI

        result.push_back(value);
    }

    return result;
}



void LeftPanel::showAlert(juce::AlertWindow::AlertIconType icon,
                         const juce::String& title,
                         const juce::String& message)
{
    juce::AlertWindow::showMessageBoxAsync(icon, title, message);
}