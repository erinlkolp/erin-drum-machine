#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "DrumPad.h"

class DrumMachineProcessor : public juce::AudioProcessor
{
public:
    DrumMachineProcessor();
    ~DrumMachineProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static constexpr int numPads = 8;
    const std::array<DrumPad, numPads>& getPads() const { return pads; }
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Set by audio thread, read/reset by GUI thread for pad flash
    std::atomic<bool> padTriggered[numPads] = {};

    // Set by GUI thread, read/reset by audio thread for pad audition
    std::atomic<bool> pendingTrigger[numPads] = {};

private:
    std::array<DrumPad, numPads> pads;
    std::array<juce::Synthesiser, numPads> synthesisers;
    juce::AudioFormatManager formatManager;
    juce::AudioBuffer<float> padBuffer;

    void initPads();
    void loadSamples();

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    static constexpr int closedHHIndex = 3;
    static constexpr int openHHIndex = 4;
    static constexpr int voicesPerPad = 4;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumMachineProcessor)
};
