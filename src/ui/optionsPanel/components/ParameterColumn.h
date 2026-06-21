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
        const int inset = juce::jlimit(6, 12, bounds.getWidth() / 18);
        auto inner = bounds.reduced(inset);

        // Lignes compactes :
        // les paramètres restent lisibles sans former de gros blocs visuels.
        const int rowCount = static_cast<int>(rows.size());
        const int spacingY = juce::jlimit(4, 7, bounds.getHeight() / 90);
        const int availableHeight = inner.getHeight()
                                  - juce::jmax(0, rowCount - 1) * spacingY;
        const int rowHeight = rowCount > 0
            ? juce::jlimit(34, 44, availableHeight / rowCount)
            : 34;

        for (auto& row : rows)
        {
            row->setBounds(inner.removeFromTop(rowHeight));
            inner.removeFromTop(spacingY);
        }
    }

private:
    std::vector<std::unique_ptr<ParameterRow>> rows;
};
