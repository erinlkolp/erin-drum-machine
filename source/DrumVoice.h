#pragma once
#include <juce_audio_utils/juce_audio_utils.h>

class DrumVoice : public juce::SamplerVoice
{
public:
    void stopNote(float velocity, bool allowTailOff) override
    {
        if (!allowTailOff)
        {
            // Force stop: choke group or end-of-sample
            juce::SamplerVoice::stopNote(velocity, false);
        }
        // Ignore normal note-off (allowTailOff == true) so the
        // sample plays through to completion regardless of MIDI note length
    }
};
