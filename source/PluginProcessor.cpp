#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DrumVoice.h"
#include "BinaryData.h"

juce::AudioProcessorValueTreeState::ParameterLayout
DrumMachineProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    const std::array<juce::String, numPads> prefixes = {
        "kick", "snare", "clap", "closed_hh",
        "open_hh", "rimshot", "cowbell", "tom"
    };
    const std::array<juce::String, numPads> names = {
        "Kick", "Snare", "Clap", "Closed HH",
        "Open HH", "Rim Shot", "Cowbell", "Tom"
    };

    for (int i = 0; i < numPads; ++i)
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(prefixes[i] + "_volume", 1),
            names[i] + " Volume",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.8f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(prefixes[i] + "_pan", 1),
            names[i] + " Pan",
            juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f),
            0.0f));
    }

    return { params.begin(), params.end() };
}

DrumMachineProcessor::DrumMachineProcessor()
    : AudioProcessor(BusesProperties()
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    formatManager.registerBasicFormats();
    initPads();
    loadSamples();
}

DrumMachineProcessor::~DrumMachineProcessor() = default;

void DrumMachineProcessor::initPads()
{
    pads[0] = { "Kick",    "kick",      36, juce::Colour(0xFFFF6600), "A" };
    pads[1] = { "Snare",   "snare",     38, juce::Colour(0xFFFFFFFF), "S" };
    pads[2] = { "Clap",    "clap",      39, juce::Colour(0xFFDD88AA), "D" };
    pads[3] = { "C.HH",    "closed_hh", 42, juce::Colour(0xFFFFDD00), "F" };
    pads[4] = { "O.HH",    "open_hh",   46, juce::Colour(0xFFFFDD00), "G" };
    pads[5] = { "Rim",     "rimshot",   37, juce::Colour(0xFFFFFFFF), "H" };
    pads[6] = { "Cowbell", "cowbell",   56, juce::Colour(0xFFDD2200), "J" };
    pads[7] = { "Tom",     "tom",       43, juce::Colour(0xFFFF6600), "K" };
}

void DrumMachineProcessor::loadSamples()
{
    struct SampleRef
    {
        const char* data;
        int size;
    };

    const std::array<SampleRef, numPads> sampleData = {{
        { BinaryData::kick_wav,      BinaryData::kick_wavSize },
        { BinaryData::snare_wav,     BinaryData::snare_wavSize },
        { BinaryData::clap_wav,      BinaryData::clap_wavSize },
        { BinaryData::closed_hh_wav, BinaryData::closed_hh_wavSize },
        { BinaryData::open_hh_wav,   BinaryData::open_hh_wavSize },
        { BinaryData::rimshot_wav,   BinaryData::rimshot_wavSize },
        { BinaryData::cowbell_wav,   BinaryData::cowbell_wavSize },
        { BinaryData::tom_wav,       BinaryData::tom_wavSize },
    }};

    for (int i = 0; i < numPads; ++i)
    {
        auto stream = std::make_unique<juce::MemoryInputStream>(
            sampleData[i].data, static_cast<size_t>(sampleData[i].size), false);

        std::unique_ptr<juce::AudioFormatReader> reader(
            formatManager.createReaderFor(std::move(stream)));

        if (reader != nullptr)
        {
            juce::BigInteger midiNotes;
            midiNotes.setBit(pads[i].midiNote);

            synthesisers[i].addSound(new juce::SamplerSound(
                pads[i].name, *reader, midiNotes,
                pads[i].midiNote,   // midiNoteForNormalPitch
                0.0,                // attackTimeSecs
                0.0,                // releaseTimeSecs
                10.0));             // maxSampleLengthSeconds

            for (int v = 0; v < voicesPerPad; ++v)
                synthesisers[i].addVoice(new DrumVoice());
        }
    }
}

void DrumMachineProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    padBuffer.setSize(2, samplesPerBlock);

    for (auto& synth : synthesisers)
        synth.setCurrentPlaybackSampleRate(sampleRate);
}

void DrumMachineProcessor::releaseResources() {}

void DrumMachineProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // --- Distribute incoming MIDI to per-pad buffers ---
    std::array<juce::MidiBuffer, numPads> padMidi;

    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn() || msg.isNoteOff())
        {
            for (int i = 0; i < numPads; ++i)
            {
                if (pads[i].midiNote == msg.getNoteNumber())
                {
                    padMidi[i].addEvent(msg, metadata.samplePosition);

                    if (msg.isNoteOn())
                        padTriggered[i].store(true, std::memory_order_relaxed);

                    break;
                }
            }
        }
    }

    // --- Handle GUI pad audition clicks ---
    for (int i = 0; i < numPads; ++i)
    {
        if (pendingTrigger[i].exchange(false, std::memory_order_relaxed))
        {
            padMidi[i].addEvent(
                juce::MidiMessage::noteOn(1, pads[i].midiNote, (juce::uint8) 127), 0);
            padTriggered[i].store(true, std::memory_order_relaxed);
        }
    }

    // --- Hi-hat choke group ---
    auto chokeVoices = [this](int synthIndex)
    {
        for (int v = 0; v < synthesisers[synthIndex].getNumVoices(); ++v)
        {
            auto* voice = synthesisers[synthIndex].getVoice(v);
            if (voice->isVoiceActive())
                voice->stopNote(0.0f, false);
        }
    };

    for (const auto metadata : padMidi[closedHHIndex])
    {
        if (metadata.getMessage().isNoteOn())
        {
            chokeVoices(openHHIndex);
            break;
        }
    }

    for (const auto metadata : padMidi[openHHIndex])
    {
        if (metadata.getMessage().isNoteOn())
        {
            chokeVoices(closedHHIndex);
            break;
        }
    }

    // --- Render each pad and mix to output ---
    for (int i = 0; i < numPads; ++i)
    {
        padBuffer.clear();
        synthesisers[i].renderNextBlock(padBuffer, padMidi[i], 0, buffer.getNumSamples());

        const float vol = apvts.getRawParameterValue(pads[i].paramIdPrefix + "_volume")->load();
        const float pan = apvts.getRawParameterValue(pads[i].paramIdPrefix + "_pan")->load();

        // Linear panning: full volume on both channels at center
        const float leftGain  = vol * juce::jmin(1.0f, 1.0f - pan);
        const float rightGain = vol * juce::jmin(1.0f, 1.0f + pan);

        buffer.addFrom(0, 0, padBuffer, 0, 0, buffer.getNumSamples(), leftGain);
        buffer.addFrom(1, 0, padBuffer, 1, 0, buffer.getNumSamples(), rightGain);
    }
}

juce::AudioProcessorEditor* DrumMachineProcessor::createEditor()
{
    return new DrumMachineEditor(*this);
}

void DrumMachineProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DrumMachineProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DrumMachineProcessor();
}
