#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

struct DrumPad
{
    juce::String name;
    juce::String paramIdPrefix;
    int midiNote;
    juce::Colour colour;
    juce::String triggerKey;
};
