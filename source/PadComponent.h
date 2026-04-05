#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "DrumPad.h"

class KnobLookAndFeel;

class PadComponent : public juce::Component
{
public:
    PadComponent(const DrumPad& pad,
                 juce::AudioProcessorValueTreeState& apvts,
                 KnobLookAndFeel& knobLnf,
                 std::function<void()> onClicked);
    ~PadComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

    void flash();
    void tickFlash();

private:
    const DrumPad& pad;
    std::function<void()> onClicked;

    juce::Slider volumeKnob { juce::Slider::RotaryVerticalDrag,
                               juce::Slider::NoTextBox };
    juce::Slider panKnob    { juce::Slider::RotaryVerticalDrag,
                               juce::Slider::NoTextBox };

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;

    int flashCounter = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PadComponent)
};
