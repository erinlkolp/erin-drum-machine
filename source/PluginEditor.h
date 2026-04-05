#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "PadComponent.h"
#include "KnobLookAndFeel.h"

class DrumMachineEditor : public juce::AudioProcessorEditor,
                          private juce::Timer
{
public:
    explicit DrumMachineEditor(DrumMachineProcessor&);
    ~DrumMachineEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

private:
    void timerCallback() override;

    DrumMachineProcessor& processor;
    KnobLookAndFeel knobLookAndFeel;
    std::array<std::unique_ptr<PadComponent>, DrumMachineProcessor::numPads> padComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumMachineEditor)
};
