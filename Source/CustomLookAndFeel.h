#pragma once

#include <JuceHeader.h>  // Include JUCE headers

// Declare the CustomLookAndFeel class
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();   // Constructor declaration

    // Override the method for drawing a rotary slider
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;
};
