#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ParameterRow.h"

/*
//==============================================================================
   ParameterColumn

   Contient et place verticalement plusieurs paramètres.
//==============================================================================
*/
class ParameterColumn
{
public:
    ParameterRow* addParameter(juce::Component& parent,
                               const juce::String& label,
                               std::unique_ptr<juce::Component> control)
    {
        auto row = std::make_unique<ParameterRow>(label, std::move(control));
        auto* rowPtr = row.get();

        parent.addAndMakeVisible(*row);
        rows.push_back(std::move(row));

        return rowPtr;
    }

    void layout(juce::Rectangle<int> bounds)
    {
        auto inner = bounds.reduced(12);

        constexpr int rowHeight = 28;
        constexpr int spacingY = 10;

        for (auto& row : rows)
        {
            row->setBounds(inner.removeFromTop(rowHeight));
            inner.removeFromTop(spacingY);
        }
    }

private:
    std::vector<std::unique_ptr<ParameterRow>> rows;
};