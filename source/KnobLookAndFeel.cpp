#include "KnobLookAndFeel.h"

KnobLookAndFeel::KnobLookAndFeel() = default;

void KnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y,
                                        int width, int height,
                                        float sliderPos,
                                        float rotaryStartAngle,
                                        float rotaryEndAngle,
                                        juce::Slider&)
{
    const auto bounds = juce::Rectangle<float>(
        static_cast<float>(x), static_cast<float>(y),
        static_cast<float>(width), static_cast<float>(height)).reduced(4.0f);

    const auto radius   = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    const auto centreX  = bounds.getCentreX();
    const auto centreY  = bounds.getCentreY();
    const auto angle    = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Knob body
    g.setColour(juce::Colour(0xFF333333));
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    // Outer ring
    g.setColour(juce::Colour(0xFF555555));
    g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 1.5f);

    // Value arc (orange)
    {
        juce::Path arc;
        arc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f,
                          0.0f, rotaryStartAngle, angle, true);
        g.setColour(juce::Colour(0xFFFF6600));
        g.strokePath(arc, juce::PathStrokeType(2.5f));
    }

    // Pointer line (white)
    {
        const auto pointerLength = radius * 0.6f;
        const auto pointerAngle  = angle - juce::MathConstants<float>::halfPi;

        juce::Path pointer;
        pointer.startNewSubPath(centreX, centreY);
        pointer.lineTo(centreX + pointerLength * std::cos(pointerAngle),
                       centreY + pointerLength * std::sin(pointerAngle));
        g.setColour(juce::Colours::white);
        g.strokePath(pointer, juce::PathStrokeType(2.0f));
    }
}
