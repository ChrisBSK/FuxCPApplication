#include "LeftPanel.h"
#include "../optionsWindow/OptionsPanel.h"
#include "../../controller/AppController.h"

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


LeftPanel::LeftPanel(AppController& controller)
    : appController(controller)  {
    label.setText("Cantus Firmus (MIDI)",juce::sendNotification);
    addAndMakeVisible(text);
    //addAndMakeVisible(label);


    voices.addItem(("2"), 2);
    voices.addItem(("3"), 3);
    voices.addItem(("4"), 4);

    labelVoices.setText("Number of voices",juce::sendNotification);
    addAndMakeVisible(voices);
    //addAndMakeVisible(labelVoices);


    moreOptions.setButtonText("More Options");
    addAndMakeVisible(moreOptions);


    moreOptions.onClick = [this]()
    {

        auto* content  = new OptionsPanel();

        int numVoices = voices.getSelectedId();

        content->setNumVoices(numVoices);

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
        juce::String error;

        auto rawText = text.getText().trim();

        if (rawText.isEmpty())
        {
            showAlert(juce::AlertWindow::WarningIcon, Messages::titleError, Messages::cfEmpty);
            return;
        }

        if (!rawText.containsOnly("0123456789 ,;"))
        {
            showAlert(juce::AlertWindow::WarningIcon, Messages::titleError, Messages::cfNotNumbers);
            return;
        }

        auto cf = parseCantusFirmus(rawText);

        if (cf.empty())
        {
            showAlert(juce::AlertWindow::WarningIcon, Messages::titleError, Messages::cfInvalid);
            return;
        }

        if (voices.getSelectedItemIndex() == -1)
        {
            showAlert(juce::AlertWindow::WarningIcon, Messages::titleError, Messages::noVoices);
            return;
        }

        int numVoices = voices.getSelectedId();

        // =========================
        // 🔥 MODEL (IMPORTANT)
        // =========================

        auto& problem = appController.getProblem();

        problem.setCantusFirmus(cf);

        std::vector<CantusProblem::Voice> voicesVec;


        int numGeneratedVoices = numVoices - 1;

        for (int i = 0; i < speciesBoxes.size(); ++i)
        {
            CantusProblem::Voice v;
            v.id = i;


            v.species = speciesBoxes[i]->getSelectedId();

            if (i < typeBoxes.size()) {
                int id = typeBoxes[i]->getSelectedId();
                v.type = id - 4;
            }
            else
                v.type = 1;

            DBG("Voice " << i
                << " species=" << v.species
                << " type=" << v.type);

            voicesVec.push_back(v);
        }

        problem.setVoices(voicesVec);

        // =========================
        // LANCEMENT
        // =========================

        appController.startGeneration("dummy.mid");
    };


    voices.onChange = [this]()
    {
        int numVoices = voices.getSelectedId();
        updateVoiceSpeciesUI(numVoices);
    };

    updateVoiceSpeciesUI(0);

    speciesHeader.setText(
        juce::String::fromUTF8(u8"Espèce"),
        juce::dontSendNotification
    );
    typeHeader.setText("Type", juce::dontSendNotification);

    addAndMakeVisible(speciesHeader);
    addAndMakeVisible(typeHeader);
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
    g.fillAll(juce::Colours::darkgrey);

    auto area = getLocalBounds();

    const int spacing = 40;
    const int numSections = 3;

    int totalSpacing = spacing * (numSections - 1);
    int sectionHeight = (area.getHeight() - totalSpacing) / numSections;

    const int titleHeight = 24;

    juce::Colour darkGreen = juce::Colour(0xff2f4f4f);

    juce::String titles[3] =
    {
        "Cantus Firmus",
        "Number of voices",
        "Buttons"
    };

    for (int i = 0; i < numSections; ++i)
    {
        auto section = area.removeFromTop(sectionHeight);

        // 🔥 uniquement la barre de titre
        auto titleArea = section.removeFromTop(titleHeight);

        g.setColour(darkGreen);
        g.fillRect(titleArea);

        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(titles[i], titleArea.reduced(10, 0),
                   juce::Justification::centredLeft);

        // (optionnel) petite ligne séparatrice
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.drawLine((float)section.getX(),
                   (float)section.getY(),
                   (float)section.getRight(),
                   (float)section.getY(),
                   1.0f);

        if (i < numSections - 1)
            area.removeFromTop(spacing);
    }
}

void LeftPanel::resized()
{
    auto area = getLocalBounds();

    const int spacing = 40;
    const int numSections = 3;
    const float widthRatio = 0.6f;

    int totalSpacing = spacing * (numSections - 1);
    int sectionHeight = (area.getHeight() - totalSpacing) / numSections;

    // =========================
    // SECTION 1 : Cantus Firmus
    // =========================
    auto section1 = area.removeFromTop(sectionHeight);
    auto content1 = section1.reduced(10);

    content1.removeFromTop(30);

    {
        auto row = content1.removeFromTop(22);
        int width = static_cast<int>(row.getWidth() * widthRatio);
        int x = row.getX() + 10; // léger décalage à gauche
        text.setBounds(x, row.getY(), width, row.getHeight());
    }

    area.removeFromTop(spacing);

    int dynamicRows = speciesLabels.size();
    int rowHeight = 18 + 2 + 22 + 2 + 22 + 4; // label + species + type

    int dynamicHeight = dynamicRows * rowHeight;
    int baseHeight = 80; // voix combobox + marges

    int section2Height = baseHeight + dynamicHeight;

    // =========================
    // SECTION 2 : Number of voices
    // =========================
    auto section2 = area.removeFromTop(section2Height);
    auto content2 = section2.reduced(10);

    content2.removeFromTop(20);

    int dynamicIndent = 20;
    int dynamicLabelHeight = 18;
    int boxHeight = 22;
    int dynamicGap = 2;

    {
        auto row = content2.removeFromTop(22);
        int width = static_cast<int>(row.getWidth() * widthRatio);
        int x = row.getX() + 10; // un peu sur la gauche
        voices.setBounds(x, row.getY(), width, row.getHeight());
    }

    content2.removeFromTop(12); // espace sous la combobox "Number of voices"

    {
        int width = content2.getWidth() * 0.35f;
        int x = content2.getX() + dynamicIndent;
        int y = content2.getY();

        for (int i = 0; i < speciesLabels.size(); ++i)
        {
            int y = content2.getY();

            const int labelHeight = 18;
            const int boxHeight = 22;

            // 🔥 colonnes
            int leftX = content2.getX() + 10;
            int colSpacing = 10;

            int labelWidth = 80;
            int boxWidth = 60;

            // =========================
            // HEADER (Espèce / Type)
            // =========================
            int headerY = y;

            int speciesX = leftX + labelWidth + colSpacing;
            int typeX    = speciesX + boxWidth + colSpacing;

            speciesHeader.setBounds(speciesX, headerY, boxWidth, labelHeight);
            typeHeader.setBounds(typeX, headerY, boxWidth, labelHeight);

            y += labelHeight + 5;

            // =========================
            // LIGNES
            // =========================
            for (int i = 0; i < speciesLabels.size(); ++i)
            {
                // 🔽 label "Voix X"
                speciesLabels[i]->setBounds(leftX, y, labelWidth, labelHeight);

                // 🔽 species
                speciesBoxes[i]->setBounds(speciesX, y, boxWidth, boxHeight);

                // 🔽 type
                typeBoxes[i]->setBounds(typeX, y, boxWidth, boxHeight);

                y += boxHeight + 8;
            }
        }
    }

    area.removeFromTop(spacing);

    // =========================
    // SECTION 3 : Buttons
    // =========================
    int section3Height = 170; // taille fixe pour les boutons
    auto section3 = getLocalBounds().removeFromBottom(section3Height);
    auto content3 = section3.reduced(15);

    int buttonWidth = static_cast<int>(content3.getWidth() * widthRatio);
    int buttonX = content3.getX() + (content3.getWidth() - buttonWidth) / 2;
    int y = content3.getY() + 35;

    moreOptions.setBounds(buttonX, y, buttonWidth, boxHeight);
    y += boxHeight + 5;

    generateButton.setBounds(buttonX, y, buttonWidth, boxHeight);
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

void LeftPanel::updateVoiceSpeciesUI(int numVoices)
{
    for (auto* box : speciesBoxes)
        removeChildComponent(box);

    for (auto* label : speciesLabels)
        removeChildComponent(label);

    speciesBoxes.clear();
    speciesLabels.clear();

    for (auto* box : typeBoxes)
        removeChildComponent(box);

    typeBoxes.clear();

    for (int i = 0; i < numVoices; ++i)
    {
        auto* rowLabel = new juce::Label();

        if (i == 0)
            rowLabel->setText(juce::String::fromUTF8(u8"Cantus Firmus"),
                              juce::dontSendNotification);
        else
            rowLabel->setText("Contrepoint " + juce::String(i) + "",
                              juce::dontSendNotification);

        addAndMakeVisible(rowLabel);
        speciesLabels.add(rowLabel);

        auto* speciesBox = new juce::ComboBox();
        speciesBox->addItem("1", 1);
        speciesBox->addItem("2", 2);
        speciesBox->addItem("3", 3);
        speciesBox->addItem("4", 4);
        speciesBox->addItem("5", 5);
        speciesBox->setSelectedId(1);

        addAndMakeVisible(speciesBox);
        speciesBoxes.add(speciesBox);

        auto* typeBox = new juce::ComboBox();

        // Exemple simple (à adapter selon ton modèle)
        typeBox->addItem("-3", 1);
        typeBox->addItem("-2", 2);
        typeBox->addItem("-1", 3);
        typeBox->addItem("0", 4);
        typeBox->addItem("1", 5);
        typeBox->addItem("2", 6);
        typeBox->addItem("3", 7);


        typeBox->setSelectedId(1);

        addAndMakeVisible(typeBox);
        typeBoxes.add(typeBox);


    }


    resized();
}



void LeftPanel::showAlert(juce::AlertWindow::AlertIconType icon,
                         const juce::String& title,
                         const juce::String& message)
{
    juce::AlertWindow::showMessageBoxAsync(icon, title, message);
}

juce::String LeftPanel::getCantusText() const
{
    return text.getText();
}

void LeftPanel::setCantusText(const juce::String& newText)
{
    text.setText(newText, juce::dontSendNotification);
}