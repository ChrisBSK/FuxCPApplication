#pragma once

#include <optional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ParameterHelpText.h"
#include "OnOffSwitchButton.h"
#include "StyledLabel.h"

/*
//==============================================================================
   BoxParameter

   Paramètre réutilisable pour les colonnes d'options.

   - Sans repères d'extrémité : label à gauche, contrôle à droite.
   - Avec repères d'extrémité : boîte verticale avec label, slider et repères.
//==============================================================================
*/
class BoxParameter : public juce::Component,
                     private juce::Slider::Listener
{
public:
    struct EndpointLabels
    {
        juce::String left;
        juce::String right;
    };

    BoxParameter(const juce::String& labelText,
                 std::unique_ptr<juce::Component> controlToUse,
                 std::optional<EndpointLabels> endpointLabelsToUse = std::nullopt)
        : control(std::move(controlToUse)),
          endpointLabels(std::move(endpointLabelsToUse))
    {
        setupLabel(labelText);
        setupEndpointLabels();
        setupSliderLookIfNeeded();

        addAndMakeVisible(*control);
    }

    ~BoxParameter() override
    {
        if (boxSlider != nullptr)
            boxSlider->removeListener(this);
    }

    template <typename ControlType>
    ControlType* getControlAs()
    {
        return dynamic_cast<ControlType*>(control.get());
    }

    int getPreferredHeight() const
    {
        if (isSwitchControl())
            return 70;

        return usesBoxLayout() ? 84 : 38;
    }

    // Indique que ce paramètre appartient à la voix sélectionnée.
    void setLinkedToSelectedVoice(bool linked)
    {
        if (isLinkedToSelectedVoice != linked)
        {
            isLinkedToSelectedVoice = linked;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override
    {
        if (! usesBoxLayout())
            return;

        g.setColour(isLinkedToSelectedVoice
                    ? juce::Colour(0xff4b3b67)
                    : juce::Colour(0xff2f5f57));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 7.0f);
    }

    void resized() override
    {
        auto area = getLocalBounds();

        if (usesBoxLayout())
        {
            layoutBoxParameter(area.reduced(12, 8));
            return;
        }

        layoutInlineParameter(area);
    }

private:
    bool isLinkedToSelectedVoice = false;

    bool usesBoxLayout() const
    {
        return endpointLabels.has_value() || isSwitchControl();
    }

    bool isSwitchControl() const
    {
        return dynamic_cast<OnOffSwitchButton*>(control.get()) != nullptr;
    }

    void setupLabel(const juce::String& labelText)
    {
        const auto text = formatLabelForDisplay(labelText);
        const auto tooltip = ParameterHelpText::getForLabel(labelText);

        if (usesBoxLayout())
        {
            boxLabel.setText(text, juce::dontSendNotification);
            boxLabel.setColour(juce::Label::textColourId, juce::Colours::white);
            boxLabel.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
            boxLabel.setJustificationType(isSwitchControl()
                                          ? juce::Justification::centred
                                          : juce::Justification::centredLeft);
            boxLabel.setTooltip(tooltip);
            addAndMakeVisible(boxLabel);
            return;
        }

        inlineLabel.setText(text, juce::dontSendNotification);
        inlineLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        inlineLabel.setJustificationType(juce::Justification::centred);
        inlineLabel.setTooltip(tooltip);
        addAndMakeVisible(inlineLabel);
    }

    void setupEndpointLabels()
    {
        if (! endpointLabels.has_value())
            return;

        leftEndpointLabel.setText(endpointLabels->left, juce::dontSendNotification);
        leftEndpointLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        leftEndpointLabel.setJustificationType(juce::Justification::centredLeft);

        rightEndpointLabel.setText(endpointLabels->right, juce::dontSendNotification);
        rightEndpointLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        rightEndpointLabel.setJustificationType(juce::Justification::centredRight);

        addAndMakeVisible(leftEndpointLabel);
        addAndMakeVisible(rightEndpointLabel);
    }

    void setupSliderLookIfNeeded()
    {
        if (! endpointLabels.has_value())
            return;

        if (auto* slider = dynamic_cast<juce::Slider*>(control.get()))
        {
            boxSlider = slider;
            slider->setSliderStyle(juce::Slider::LinearHorizontal);
            slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            slider->setColour(juce::Slider::trackColourId, juce::Colour(0xff28564f));
            slider->setColour(juce::Slider::thumbColourId, juce::Colours::white);
            slider->addListener(this);

            sliderValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
            sliderValueLabel.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
            sliderValueLabel.setJustificationType(juce::Justification::centred);
            sliderValueLabel.setInterceptsMouseClicks(false, false);
            addAndMakeVisible(sliderValueLabel);

            updateSliderValueLabel();
        }
    }

    void layoutBoxParameter(juce::Rectangle<int> area)
    {
        if (isSwitchControl())
        {
            layoutSwitchBoxParameter(area);
            return;
        }

        boxLabel.setBounds(area.removeFromTop(18));

        area.removeFromTop(2);

        const auto valueArea = area.removeFromTop(16);
        control->setBounds(area.removeFromTop(18));
        updateSliderValueLabel();
        positionSliderValueLabel(valueArea);

        auto labelsArea = area.removeFromTop(16);
        leftEndpointLabel.setBounds(labelsArea.removeFromLeft(labelsArea.getWidth() / 2));
        rightEndpointLabel.setBounds(labelsArea);
    }

    void layoutSwitchBoxParameter(juce::Rectangle<int> area)
    {
        boxLabel.setBounds(area.removeFromTop(18));
        area.removeFromTop(3);

        const int switchWidth = juce::jmin(area.getWidth(), juce::jlimit(74, 90, area.getWidth() / 2));
        const int switchHeight = juce::jmin(area.getHeight(), juce::jlimit(23, 28, area.getHeight()));
        const int x = area.getX() + (area.getWidth() - switchWidth) / 2;
        const int y = area.getY() + (area.getHeight() - switchHeight) / 2;

        control->setBounds(x, y, switchWidth, switchHeight);
    }

    void layoutInlineParameter(juce::Rectangle<int> area)
    {
        const int gap = juce::jlimit(5, 10, area.getWidth() / 28);
        const int minimumControlWidth = juce::jlimit(70, 120, area.getWidth() / 2);
        const int maximumLabelWidth = juce::jlimit(82, 120, area.getWidth() / 2);
        const int availableLabelWidth = area.getWidth() - gap - minimumControlWidth;
        const int labelWidth = juce::jlimit(70, maximumLabelWidth, availableLabelWidth);

        inlineLabel.setBounds(area.removeFromLeft(labelWidth));
        area.removeFromLeft(gap);

        control->setBounds(area.reduced(0, 2));
    }

    static juce::String formatLabelForDisplay(const juce::String& labelText)
    {
        if (labelText == "Avoid Repeated Notes")
            return "Avoid Repeated\nNotes";

        return labelText;
    }

    void sliderValueChanged(juce::Slider*) override
    {
        updateSliderValueLabel();
    }

    void updateSliderValueLabel()
    {
        if (boxSlider == nullptr)
            return;

        const int decimals = getValueDecimalPlaces(*boxSlider);
        sliderValueLabel.setText(juce::String(boxSlider->getValue(), decimals),
                                 juce::dontSendNotification);
        positionSliderValueLabel(sliderValueLabel.getBounds());
    }

    void positionSliderValueLabel(juce::Rectangle<int> valueArea)
    {
        if (boxSlider == nullptr || valueArea.isEmpty())
            return;

        constexpr int valueWidth = 44;
        const auto sliderBounds = control->getBounds();
        const double proportion = boxSlider->valueToProportionOfLength(boxSlider->getValue());
        const int x = sliderBounds.getX()
                    + juce::roundToInt(proportion * sliderBounds.getWidth())
                    - valueWidth / 2;

        sliderValueLabel.setBounds(x,
                                   valueArea.getY(),
                                   valueWidth,
                                   valueArea.getHeight());
    }

    static int getValueDecimalPlaces(const juce::Slider& slider)
    {
        const double interval = slider.getInterval();

        if (interval >= 1.0)
            return 0;

        if (interval >= 0.1)
            return 1;

        return 2;
    }

    StyledLabel inlineLabel;
    juce::Label boxLabel;
    juce::Label sliderValueLabel;
    juce::Label leftEndpointLabel;
    juce::Label rightEndpointLabel;
    std::unique_ptr<juce::Component> control;
    std::optional<EndpointLabels> endpointLabels;
    juce::Slider* boxSlider = nullptr;
};
