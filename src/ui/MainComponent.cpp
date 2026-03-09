#include "MainComponent.h"

MainComponent::MainComponent()
{

    // ===== Cantus Firmus =====
    cfLabel.setText("Cantus Firmus (MIDI)", juce::dontSendNotification);
    addAndMakeVisible(cfLabel);

    cfEditor.setText("Enter a Cantus Firmus");
    addAndMakeVisible(cfEditor);

    // ===== Species =====
    speciesLabel.setText("Species", juce::dontSendNotification);
    addAndMakeVisible(speciesLabel);

    speciesBox.addItem("1", 1);
    speciesBox.addItem("2", 2);
    speciesBox.addItem("3", 3);
    speciesBox.addItem("4", 4);
    speciesBox.addItem("5", 5);

    speciesBox.setSelectedId(1);
    addAndMakeVisible(speciesBox);

    // ===== Voices =====
    voicesLabel.setText("Voices", juce::dontSendNotification);
    addAndMakeVisible(voicesLabel);

    voicesBox.addItem("2", 2);
    voicesBox.addItem("3", 3);
    voicesBox.addItem("4", 4);

    voicesBox.setSelectedId(2);
    addAndMakeVisible(voicesBox);

    // ===== Status Zone =====
    statusTitleLabel.setText("Statut:", juce::dontSendNotification);
    statusTitleLabel.setJustificationType(juce::Justification::centredLeft);
    statusTitleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(statusTitleLabel);

    statusMessageLabel.setText("Pret", juce::dontSendNotification);
    statusMessageLabel.setJustificationType(juce::Justification::centredLeft);
    statusMessageLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(statusMessageLabel);

    // ===== Generate button =====
    generateButton.setButtonText("Generate");
    addAndMakeVisible(generateButton);

    /*
    generateButton.onClick = [this]()
    {
        auto midi = parseCantusFirmus(cfEditor.getText());
        const std::vector<int> cf(midi);

        generationService.startGeneration(
            cf,
            speciesBox.getSelectedId(),
            voicesBox.getSelectedId(),
            ""
        );
    };*/
    generateButton.onClick = [this]()
{
    auto midi = parseCantusFirmus(cfEditor.getText());

    if (midi.empty())
    {
        statusMessageLabel.setText(
            "Erreur: Cantus Firmus invalide (0-127)",
            juce::dontSendNotification);

        statusMessageLabel.setColour(
            juce::Label::textColourId,
            juce::Colours::red);

        return;
    }

    juce::File desktop =
        juce::File::getSpecialLocation(juce::File::userDesktopDirectory);

    midiOutFileToGenerate =
        desktop.getChildFile(
            "FuxCP_Solution_" +
            juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S") +
            ".mid");

    generateButton.setEnabled(false);

    statusMessageLabel.setText(
        "Recherche en cours...",
        juce::dontSendNotification);

    statusMessageLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::yellow);


    // 🔐 SAFE POINTER
    juce::Component::SafePointer<MainComponent> safeThis(this);

    generationService.setOnFinishedCallback([safeThis]()
    {
        if (safeThis == nullptr)
            return;

        bool success = safeThis->generationService.getLastGenerationSuccess();
        juce::String error = safeThis->generationService.getLastError();

        safeThis->generateButton.setEnabled(true);

        if (success)
        {
            safeThis->statusMessageLabel.setText(
                "Solution trouvee: " + safeThis->midiOutFileToGenerate.getFileName(),
                juce::dontSendNotification);

            safeThis->statusMessageLabel.setColour(
                juce::Label::textColourId,
                juce::Colours::green);

            safeThis->midiOutFileToGenerate.revealToUser();
        }
        else
        {
            juce::String message = "Aucune solution trouvee";

            if (!error.isEmpty())
                message += ": " + error;

            safeThis->statusMessageLabel.setText(
                message,
                juce::dontSendNotification);

            safeThis->statusMessageLabel.setColour(
                juce::Label::textColourId,
                juce::Colours::orange);
        }
    });

    generationService.startGeneration(
        midi,
        speciesBox.getSelectedId(),
        voicesBox.getSelectedId(),
        midiOutFileToGenerate.getFullPathName());
};

    setSize (2000, 2000);
}





void MainComponent::paint (juce::Graphics& g)
{
    // Fond simple
    g.fillAll (juce::Colours::darkgrey);

    // Titre centré
    g.setColour (juce::Colours::white);
    g.setFont (20.0f);

}

void MainComponent::resized()
{
    int x = 20;
    int w = getWidth() - 40;
    int y = 20;
    int h = 24;
    int gap = 10;

    // Cantus Firmus
    cfLabel.setBounds(x, y, w, h);
    y += h + gap;
    cfEditor.setBounds(x, y, w, 30);
    y += 40;

    // Species
    speciesLabel.setBounds(x, y, 100, h);
    speciesBox.setBounds(x + 100, y, 120, h);
    y += 40;

    // Voices
    // Voices
    voicesLabel.setBounds(x, y, 100, h);
    voicesBox.setBounds(x + 100, y, 120, h);
    y += 40;

    // Status zone (avant le bouton)
    statusTitleLabel.setBounds(x, y, 80, h);
    statusMessageLabel.setBounds(x + 80, y, w - 80, h);
    y += 40 + gap;

    // Generate button
    generateButton.setBounds(x, y, 120, 30);
}



std::vector<int> MainComponent::parseCantusFirmus(const juce::String& text)
{
    std::vector<int> result;

    if (text.isEmpty()) {
        return result;
    }

    auto tokens = juce::StringArray::fromTokens(text, " ,;", "\"");

    for (auto& t : tokens)
    {
        int value = t.getIntValue();
        if (value >= 0 && value <= 127) {
            result.push_back(value);
        }
    }

    return result;
}
