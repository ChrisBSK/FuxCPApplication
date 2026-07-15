#pragma once

#include <functional>
#include <utility>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../model/ConstraintsSettings.h"

/*
//==============================================================================
   ShapeColumnControls

   Petit bloc visuel pour ajouter des shapes.
   Chaque shape cree une consigne simple : voix + fonction de cout + forme + mesures.
//==============================================================================
*/
class ShapeColumnControls : public juce::Component,
                            private juce::Timer
{
public:
    std::function<void(const std::vector<ConstraintSettings::ShapeAssignment>&)> onShapeAssignmentsChanged;

    ShapeColumnControls()
    {
        setWantsKeyboardFocus(true);

        setupLabel(costFunctionLabel, "Cost function");
        setupLabel(shapeLabel, "Shape");
        setupLabel(measureLabel, "Measures");
        setupLabel(fromLabel, "M");
        setupLabel(toLabel, "to M");

        setupComboBox(costFunctionBox);
        costFunctionBox.addItem("Melody movement", static_cast<int>(ConstraintSettings::ShapeCostTarget::melodyMovement) + 1);
        costFunctionBox.addItem("Interval colour", static_cast<int>(ConstraintSettings::ShapeCostTarget::intervalColour) + 1);
        costFunctionBox.addItem("Perfect intervals", static_cast<int>(ConstraintSettings::ShapeCostTarget::perfectIntervals) + 1);
        costFunctionBox.setSelectedId(1, juce::dontSendNotification);

        setupComboBox(shapeBox);
        shapeBox.addItem("Fixed 0", static_cast<int>(ConstraintSettings::ShapeType::fixedZero) + 1);
        shapeBox.addItem("Fixed 1", static_cast<int>(ConstraintSettings::ShapeType::fixedOne) + 1);
        shapeBox.addItem("Linear", static_cast<int>(ConstraintSettings::ShapeType::linear) + 1);
        shapeBox.addItem("Linear desc.", static_cast<int>(ConstraintSettings::ShapeType::linearDescending) + 1);
        shapeBox.addItem("Inverted V", static_cast<int>(ConstraintSettings::ShapeType::invertedV) + 1);
        shapeBox.addItem("V", static_cast<int>(ConstraintSettings::ShapeType::v) + 1);
        shapeBox.addItem("M", static_cast<int>(ConstraintSettings::ShapeType::m) + 1);
        shapeBox.addItem("Step", static_cast<int>(ConstraintSettings::ShapeType::step) + 1);
        shapeBox.addItem("Step desc.", static_cast<int>(ConstraintSettings::ShapeType::stepDescending) + 1);
        shapeBox.setSelectedId(static_cast<int>(ConstraintSettings::ShapeType::invertedV) + 1,
                               juce::dontSendNotification);

        setupMeasureEditor(startMeasureEditor, "1");
        setupMeasureEditor(endMeasureEditor, "1");

        addButton.setButtonText("Add");
        addButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2f5f57));
        addButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff4b3b67));
        addButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        addAndMakeVisible(addButton);

        removeButton.setButtonText("Remove");
        removeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff24363b));
        removeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        addAndMakeVisible(removeButton);

        addButton.onClick = [this]()
        {
            addShapePreview();
        };

        removeButton.onClick = [this]()
        {
            removeSelectedShapePreview();
        };
    }

    int getFormHeight() const
    {
        return 138;
    }

    void setSelectedCounterpointIndex(int index)
    {
        selectedCounterpointIndex = index;
    }

    void paint(juce::Graphics& g) override
    {
        drawFormBackground(g);
        drawShapeList(g);
        drawWarning(g);
    }

    void resized() override
    {
        auto area = getFormBounds().reduced(10, 6);

        layoutLabelAndControl(area, costFunctionLabel, costFunctionBox);
        area.removeFromTop(3);

        layoutLabelAndControl(area, shapeLabel, shapeBox);
        area.removeFromTop(3);

        measureLabel.setBounds(area.removeFromTop(13));
        area.removeFromTop(1);

        auto measureArea = area.removeFromTop(16);
        const int labelWidth = 18;
        const int editorWidth = juce::jlimit(18, 26, measureArea.getWidth() / 6);
        const int gap = 3;

        fromLabel.setBounds(measureArea.removeFromLeft(labelWidth));
        startMeasureEditor.setBounds(measureArea.removeFromLeft(editorWidth));
        measureArea.removeFromLeft(gap);

        toLabel.setBounds(measureArea.removeFromLeft(labelWidth + 7));
        endMeasureEditor.setBounds(measureArea.removeFromLeft(editorWidth));

        area.removeFromTop(5);

        const int buttonWidth = juce::jmin(58, area.getWidth());
        addButton.setBounds(area.getX() + (area.getWidth() - buttonWidth) / 2,
                            area.getY(),
                            buttonWidth,
                            18);

        layoutRemoveButton();
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        grabKeyboardFocus();

        for (const auto& visibleCard : visibleCardBounds)
        {
            if (visibleCard.bounds.contains(event.getPosition()))
            {
                selectedShapeIndex = visibleCard.shapeIndex;
                repaint();
                return;
            }
        }

        selectedShapeIndex = -1;
        repaint();
    }

    bool keyPressed(const juce::KeyPress& key) override
    {
        if (key == juce::KeyPress::downKey)
            return scrollShapeList(1);

        if (key == juce::KeyPress::upKey)
            return scrollShapeList(-1);

        return false;
    }

    void mouseWheelMove(const juce::MouseEvent& event,
                        const juce::MouseWheelDetails& wheel) override
    {
        if (! getListBounds().contains(event.getPosition()) || shapePreviews.size() <= maxVisibleShapes)
            return;

        const int direction = wheel.deltaY < 0.0f ? 1 : -1;
        scrollShapeList(direction);
    }

private:
    void timerCallback() override
    {
        warningText.clear();
        stopTimer();
        repaint();
    }

    struct ShapePreview
    {
        ConstraintSettings::ShapeAssignment assignment;
        juce::String costFunction;
        juce::String shape;
        juce::String startMeasure;
        juce::String endMeasure;
    };

    struct VisibleShapeCard
    {
        juce::Rectangle<int> bounds;
        int shapeIndex = -1;
    };

    juce::Rectangle<int> getFormBounds() const
    {
        auto bounds = getLocalBounds();
        bounds.setHeight(juce::jmin(bounds.getHeight(), getFormHeight()));
        return bounds;
    }

    juce::Rectangle<int> getListBounds() const
    {
        auto bounds = getLocalBounds();
        bounds.removeFromTop(getFormBounds().getHeight() + warningAreaHeight + 8);
        return bounds;
    }

    juce::Rectangle<int> getWarningBounds() const
    {
        auto bounds = getLocalBounds();
        bounds.removeFromTop(getFormBounds().getHeight() + 4);
        bounds.setHeight(warningAreaHeight - 6);
        return bounds.reduced(10, 0);
    }

    void drawFormBackground(juce::Graphics& g)
    {
        g.setColour(juce::Colour(0xff2f5f57));
        g.fillRoundedRectangle(getFormBounds().toFloat(), 7.0f);
    }

    void drawShapeList(juce::Graphics& g)
    {
        auto listArea = getListBounds();
        visibleCardBounds.clear();

        if (shapePreviews.empty())
            return;

        g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
        g.setColour(juce::Colours::white.withAlpha(0.85f));
        auto headerArea = listArea.removeFromTop(20);
        headerArea.removeFromRight(62);
        g.drawText("Shape list", headerArea, juce::Justification::centredLeft);

        listArea.removeFromTop(5);

        constexpr int cardHeight = 62;
        constexpr int gap = 6;
        const int maxFirstIndex = juce::jmax(0, static_cast<int>(shapePreviews.size()) - maxVisibleShapes);
        firstVisibleShapeIndex = juce::jlimit(0, maxFirstIndex, firstVisibleShapeIndex);

        for (int visibleIndex = 0; visibleIndex < maxVisibleShapes; ++visibleIndex)
        {
            const int shapeIndex = firstVisibleShapeIndex + visibleIndex;

            if (shapeIndex >= static_cast<int>(shapePreviews.size()))
                break;

            if (listArea.getHeight() < cardHeight)
                break;

            auto card = listArea.removeFromTop(cardHeight);
            drawShapeCard(g, card, shapeIndex, shapePreviews[static_cast<size_t>(shapeIndex)]);
            visibleCardBounds.push_back({ card, shapeIndex });
            listArea.removeFromTop(gap);
        }

        if (shapePreviews.size() > maxVisibleShapes && listArea.getHeight() >= 16)
        {
            const int hiddenCount = static_cast<int>(shapePreviews.size())
                                  - firstVisibleShapeIndex
                                  - maxVisibleShapes;
            const juce::String label = hiddenCount > 0
                ? "+ " + juce::String(hiddenCount) + " more"
                : "scroll up";

            g.setColour(juce::Colours::white.withAlpha(0.75f));
            g.drawText(label,
                       listArea.removeFromTop(16),
                       juce::Justification::centred);
        }
    }

    void drawShapeCard(juce::Graphics& g,
                       juce::Rectangle<int> card,
                       int index,
                       const ShapePreview& preview)
    {
        const bool isSelected = selectedShapeIndex == index;

        g.setColour(isSelected
                    ? juce::Colour(0xff4b3b67)
                    : juce::Colour(0xff2f5f57).withAlpha(0.9f));
        g.fillRoundedRectangle(card.toFloat(), 6.0f);

        if (isSelected)
        {
            g.setColour(juce::Colours::white.withAlpha(0.65f));
            g.drawRoundedRectangle(card.toFloat().reduced(0.5f), 6.0f, 1.2f);
        }

        auto content = card.reduced(8, 5);

        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
        g.drawText("Shape " + juce::String(index + 1),
                   content.removeFromTop(16),
                   juce::Justification::centredLeft);

        g.setFont(juce::Font(juce::FontOptions(10.5f)));
        g.setColour(juce::Colours::white.withAlpha(0.86f));
        g.drawText("CP " + juce::String(preview.assignment.voiceIndex + 1)
                   + " - " + preview.costFunction,
                   content.removeFromTop(14),
                   juce::Justification::centredLeft);
        g.drawText(preview.shape,
                   content.removeFromTop(14),
                   juce::Justification::centredLeft);
        g.drawText("M" + preview.startMeasure + " to M" + preview.endMeasure,
                   content.removeFromTop(14),
                   juce::Justification::centredLeft);
    }

    void addShapePreview()
    {
        // Une shape est toujours liée à une voix de contrepoint sélectionnée.
        if (selectedCounterpointIndex < 0)
        {
            showWarning("Select a CP");
            return;
        }

        ShapePreview preview;
        preview.assignment.voiceIndex = selectedCounterpointIndex;
        preview.assignment.target = getSelectedCostTarget();
        preview.assignment.shape = getSelectedShapeType();
        preview.assignment.startMeasure = getMeasureValue(startMeasureEditor, 1);
        preview.assignment.endMeasure = getMeasureValue(endMeasureEditor,
                                                        preview.assignment.startMeasure);

        if (preview.assignment.endMeasure < preview.assignment.startMeasure)
            std::swap(preview.assignment.startMeasure, preview.assignment.endMeasure);

        // Meme voix + meme fonction + mesures qui se croisent = conflit.
        if (hasShapeConflict(preview.assignment))
        {
            showWarning("Conflict");
            return;
        }

        preview.costFunction = costFunctionBox.getText();
        preview.shape = shapeBox.getText();
        preview.startMeasure = juce::String(preview.assignment.startMeasure);
        preview.endMeasure = juce::String(preview.assignment.endMeasure);

        shapePreviews.push_back(std::move(preview));
        selectedShapeIndex = -1;
        clampListScroll();
        notifyShapeAssignmentsChanged();
        resized();
        repaint();
    }

    void removeSelectedShapePreview()
    {
        if (shapePreviews.empty() || selectedShapeIndex < 0)
            return;

        shapePreviews.erase(shapePreviews.begin() + selectedShapeIndex);
        selectedShapeIndex = -1;
        clampListScroll();
        notifyShapeAssignmentsChanged();

        resized();
        repaint();
    }

    void notifyShapeAssignmentsChanged()
    {
        // Envoie uniquement les donnees utiles au modele, sans les textes d'affichage.
        if (! onShapeAssignmentsChanged)
            return;

        std::vector<ConstraintSettings::ShapeAssignment> assignments;
        assignments.reserve(shapePreviews.size());

        for (const auto& preview : shapePreviews)
            assignments.push_back(preview.assignment);

        onShapeAssignmentsChanged(assignments);
    }

    bool hasShapeConflict(const ConstraintSettings::ShapeAssignment& candidate) const
    {
        for (const auto& preview : shapePreviews)
        {
            const auto& existing = preview.assignment;

            if (existing.voiceIndex != candidate.voiceIndex)
                continue;

            if (existing.target != candidate.target)
                continue;

            if (measuresOverlap(existing, candidate))
                return true;
        }

        return false;
    }

    static bool measuresOverlap(const ConstraintSettings::ShapeAssignment& first,
                                const ConstraintSettings::ShapeAssignment& second)
    {
        return first.startMeasure <= second.endMeasure
            && second.startMeasure <= first.endMeasure;
    }

    void showWarning(const juce::String& message)
    {
        warningText = message;
        startTimer(1200);
        repaint();
    }

    void drawWarning(juce::Graphics& g)
    {
        if (warningText.isEmpty())
            return;

        auto warningArea = getWarningBounds();

        g.setColour(juce::Colour(0xff4b3b67));
        g.fillRoundedRectangle(warningArea.toFloat(), 5.0f);

        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
        g.drawText(warningText, warningArea, juce::Justification::centred);
    }

    ConstraintSettings::ShapeCostTarget getSelectedCostTarget() const
    {
        return static_cast<ConstraintSettings::ShapeCostTarget>(
            juce::jmax(1, costFunctionBox.getSelectedId()) - 1
        );
    }

    ConstraintSettings::ShapeType getSelectedShapeType() const
    {
        return static_cast<ConstraintSettings::ShapeType>(
            juce::jmax(1, shapeBox.getSelectedId()) - 1
        );
    }

    static int getMeasureValue(const juce::TextEditor& editor, int fallback)
    {
        const int value = editor.getText().getIntValue();
        return value > 0 ? value : fallback;
    }

    void clampListScroll()
    {
        const int maxFirstIndex = juce::jmax(0, static_cast<int>(shapePreviews.size()) - maxVisibleShapes);
        firstVisibleShapeIndex = juce::jlimit(0, maxFirstIndex, firstVisibleShapeIndex);
    }

    bool scrollShapeList(int direction)
    {
        if (shapePreviews.size() <= maxVisibleShapes)
            return false;

        const int previousFirstIndex = firstVisibleShapeIndex;
        const int maxFirstIndex = juce::jmax(0, static_cast<int>(shapePreviews.size()) - maxVisibleShapes);

        firstVisibleShapeIndex = juce::jlimit(0,
                                              maxFirstIndex,
                                              firstVisibleShapeIndex + direction);

        if (firstVisibleShapeIndex == previousFirstIndex)
            return true;

        repaint();
        return true;
    }

    void layoutRemoveButton()
    {
        auto listArea = getListBounds();

        if (shapePreviews.empty())
        {
            removeButton.setVisible(false);
            removeButton.setBounds({});
            return;
        }

        removeButton.setVisible(true);

        auto headerArea = listArea.removeFromTop(20);
        const int buttonWidth = juce::jmin(58, headerArea.getWidth() / 2);

        removeButton.setBounds(headerArea.getRight() - buttonWidth,
                               headerArea.getY(),
                               buttonWidth,
                               19);
    }

    void setupLabel(juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        label.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
        label.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(label);
    }

    void setupComboBox(juce::ComboBox& comboBox)
    {
        comboBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff24363b));
        comboBox.setColour(juce::ComboBox::textColourId, juce::Colours::white);
        comboBox.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff94adb0));
        comboBox.setColour(juce::ComboBox::arrowColourId, juce::Colours::white);
        addAndMakeVisible(comboBox);
    }

    void setupMeasureEditor(juce::TextEditor& editor, const juce::String& text)
    {
        editor.setText(text, false);
        editor.setInputRestrictions(2, "0123456789");
        editor.setJustification(juce::Justification::centred);
        editor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff24363b));
        editor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
        editor.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff94adb0));
        editor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::white);
        addAndMakeVisible(editor);
    }

    void layoutLabelAndControl(juce::Rectangle<int>& area,
                               juce::Label& label,
                               juce::ComboBox& comboBox)
    {
        label.setBounds(area.removeFromTop(13));
        area.removeFromTop(1);
        comboBox.setBounds(area.removeFromTop(16));
    }

    juce::Label costFunctionLabel;
    juce::Label shapeLabel;
    juce::Label measureLabel;
    juce::Label fromLabel;
    juce::Label toLabel;

    juce::ComboBox costFunctionBox;
    juce::ComboBox shapeBox;
    juce::TextEditor startMeasureEditor;
    juce::TextEditor endMeasureEditor;
    juce::TextButton addButton;
    juce::TextButton removeButton;
    std::vector<ShapePreview> shapePreviews;
    std::vector<VisibleShapeCard> visibleCardBounds;
    int selectedShapeIndex = -1;
    int firstVisibleShapeIndex = 0;
    int selectedCounterpointIndex = -1;
    juce::String warningText;

    static constexpr int maxVisibleShapes = 3;
    static constexpr int warningAreaHeight = 28;
};
