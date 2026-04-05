#include "PadComponent.h"
#include "KnobLookAndFeel.h"

PadComponent::PadComponent(const DrumPad& p,
                           juce::AudioProcessorValueTreeState& apvts,
                           KnobLookAndFeel& knobLnf,
                           std::function<void()> onClicked)
    : pad(p), onClicked(std::move(onClicked))
{
    volumeKnob.setLookAndFeel(&knobLnf);
    panKnob.setLookAndFeel(&knobLnf);

    addAndMakeVisible(volumeKnob);
    addAndMakeVisible(panKnob);

    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, pad.paramIdPrefix + "_volume", volumeKnob);
    panAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, pad.paramIdPrefix + "_pan", panKnob);
}

PadComponent::~PadComponent()
{
    volumeKnob.setLookAndFeel(nullptr);
    panKnob.setLookAndFeel(nullptr);
}

void PadComponent::paint(juce::Graphics& g)
{
    const auto area = getLocalBounds();

    // --- Knob labels ---
    g.setColour(juce::Colour(0xFFAAAAAA));
    g.setFont(10.0f);

    const int knobSize = juce::jmin(area.getWidth() - 8, 50);
    g.drawText("VOL", 0, 4, area.getWidth(), 12, juce::Justification::centred);
    g.drawText("PAN", 0, knobSize + 18, area.getWidth(), 12, juce::Justification::centred);

    // --- Pad button ---
    const int padTop    = (knobSize + 14) * 2 + 8;
    const int padHeight = area.getHeight() - padTop - 22;
    const int padMargin = 8;
    auto padRect = juce::Rectangle<int>(padMargin, padTop,
                                        area.getWidth() - padMargin * 2, padHeight);

    auto padColour = pad.colour;
    if (flashCounter > 0)
        padColour = padColour.brighter(0.5f);

    g.setColour(padColour);
    g.fillRoundedRectangle(padRect.toFloat(), 4.0f);

    // Pad border
    g.setColour(padColour.darker(0.3f));
    g.drawRoundedRectangle(padRect.toFloat(), 4.0f, 1.5f);

    // --- Key label on pad button ---
    {
        const float brightness = padColour.getBrightness();
        g.setColour(brightness > 0.6f ? juce::Colours::black.withAlpha(0.6f)
                                      : juce::Colours::white.withAlpha(0.6f));
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(pad.triggerKey, padRect, juce::Justification::centred);
    }

    // --- Pad name label ---
    g.setColour(juce::Colour(0xFFCCCCCC));
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText(pad.name, 0, area.getHeight() - 18, area.getWidth(), 16,
               juce::Justification::centred);
}

void PadComponent::resized()
{
    const auto area = getLocalBounds();
    const int knobSize = juce::jmin(area.getWidth() - 8, 50);
    const int knobX = (area.getWidth() - knobSize) / 2;

    volumeKnob.setBounds(knobX, 16, knobSize, knobSize);
    panKnob.setBounds(knobX, knobSize + 30, knobSize, knobSize);
}

void PadComponent::mouseDown(const juce::MouseEvent& event)
{
    // Check if click is in the pad button area
    const auto area = getLocalBounds();
    const int knobSize = juce::jmin(area.getWidth() - 8, 50);
    const int padTop = (knobSize + 14) * 2 + 8;

    if (event.y >= padTop && event.y < area.getHeight() - 22)
    {
        if (onClicked)
            onClicked();
    }
}

void PadComponent::flash()
{
    flashCounter = 4; // ~130ms at 30 Hz timer
    repaint();
}

void PadComponent::tickFlash()
{
    if (flashCounter > 0)
    {
        --flashCounter;
        if (flashCounter == 0)
            repaint();
    }
}
