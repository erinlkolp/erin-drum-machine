#include "PluginEditor.h"

DrumMachineEditor::DrumMachineEditor(DrumMachineProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setResizable(false, false);
    setWantsKeyboardFocus(true);

    const auto& pads = processor.getPads();
    auto& apvts = processor.getAPVTS();

    for (int i = 0; i < DrumMachineProcessor::numPads; ++i)
    {
        padComponents[i] = std::make_unique<PadComponent>(
            pads[i], apvts, knobLookAndFeel,
            [this, i]()
            {
                processor.pendingTrigger[i].store(true, std::memory_order_relaxed);
            });
        addAndMakeVisible(*padComponents[i]);
    }

    setSize(800, 400);
    startTimerHz(30);
}

DrumMachineEditor::~DrumMachineEditor()
{
    stopTimer();
}

void DrumMachineEditor::paint(juce::Graphics& g)
{
    // --- Dark background with subtle horizontal striping ---
    g.fillAll(juce::Colour(0xFF1A1A1A));

    g.setColour(juce::Colour(0xFF222222));
    for (int y = 0; y < getHeight(); y += 4)
        g.fillRect(0, y, getWidth(), 2);

    // --- Header bar ---
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.fillRect(0, 0, getWidth(), 60);

    // Title
    g.setColour(juce::Colour(0xFFFF6600));
    g.setFont(juce::Font(28.0f, juce::Font::bold));
    g.drawText("TR-808 DRUM MACHINE", 20, 10, 500, 40,
               juce::Justification::centredLeft);

    // Branding
    g.setColour(juce::Colour(0xFF888888));
    g.setFont(juce::Font(14.0f));
    g.drawText("ERIN", getWidth() - 80, 10, 60, 40,
               juce::Justification::centredRight);

    // Orange divider
    g.setColour(juce::Colour(0xFFFF6600));
    g.fillRect(0, 58, getWidth(), 2);
}

void DrumMachineEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(70); // header space
    area.reduce(10, 10);

    const int padWidth = area.getWidth() / DrumMachineProcessor::numPads;

    for (int i = 0; i < DrumMachineProcessor::numPads; ++i)
        padComponents[i]->setBounds(area.removeFromLeft(padWidth));
}

bool DrumMachineEditor::keyPressed(const juce::KeyPress& key)
{
    const char keyMap[] = { 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k' };

    const auto keyChar = static_cast<char>(key.getTextCharacter());

    for (int i = 0; i < DrumMachineProcessor::numPads; ++i)
    {
        if (keyChar == keyMap[i])
        {
            processor.pendingTrigger[i].store(true, std::memory_order_relaxed);
            return true;
        }
    }

    return false;
}

void DrumMachineEditor::timerCallback()
{
    for (int i = 0; i < DrumMachineProcessor::numPads; ++i)
    {
        if (processor.padTriggered[i].exchange(false, std::memory_order_relaxed))
            padComponents[i]->flash();

        padComponents[i]->tickFlash();
    }
}
