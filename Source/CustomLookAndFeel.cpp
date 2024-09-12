#include "CustomLookAndFeel.h"

// Constructor definition (you can add custom initialization if needed)
CustomLookAndFeel::CustomLookAndFeel()
{
    // Custom initialization code, e.g., set colors if needed
}

// Implement the overridden drawRotarySlider method
void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPosProportional, float rotaryStartAngle,
                                         float rotaryEndAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2);
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    const auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    const auto centreX = bounds.getCentreX();
    const auto centreY = bounds.getCentreY();

    // Cyberpunk pink color
    const juce::Colour cyberpunkPink(255, 20, 147);

    // Main body of the knob
    g.setColour(juce::Colour(40, 40, 40));
    g.fillEllipse(bounds);

    // Outer bevel
    const float outerBevelThickness = radius * 0.08f;
    juce::ColourGradient outerBevelGradient(juce::Colours::white.withAlpha(0.2f), centreX, centreY - radius,
                                            juce::Colours::black.withAlpha(0.3f), centreX, centreY + radius,
                                            false);
    g.setGradientFill(outerBevelGradient);
    g.drawEllipse(bounds.reduced(outerBevelThickness / 2), outerBevelThickness);

    // Inner circle
    const auto innerBounds = bounds.reduced(radius * 0.2f);
    g.setColour(juce::Colour(30, 30, 30));
    g.fillEllipse(innerBounds);

    // Inner bevel
    const float innerBevelThickness = radius * 0.1f;
    juce::ColourGradient innerBevelGradient(juce::Colours::white.withAlpha(0.2f), centreX, innerBounds.getY(),
                                            juce::Colours::black.withAlpha(0.3f), centreX, innerBounds.getBottom(),
                                            false);
    g.setGradientFill(innerBevelGradient);
    g.drawEllipse(innerBounds.reduced(innerBevelThickness / 2), innerBevelThickness);

    // Glow arc (thinner and shaded)
    const float arcThickness = radius * 0.12f;
    juce::Path arcPath;
    arcPath.addCentredArc(centreX, centreY, radius - arcThickness / 2, radius - arcThickness / 2,
                          0.0f, rotaryStartAngle, toAngle, true);

    juce::ColourGradient arcGradient(cyberpunkPink.brighter(0.2f), centreX, centreY - radius,
                                     cyberpunkPink.darker(0.2f), centreX, centreY + radius,
                                     false);
    g.setGradientFill(arcGradient);
    g.strokePath(arcPath, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Position indicator dot (inside the knob and shaded)
    const float dotRadius = radius * 0.08f;
    const float dotDistance = radius * 0.6f;
    float dotX = centreX + dotDistance * std::cos(toAngle - juce::MathConstants<float>::halfPi);
    float dotY = centreY + dotDistance * std::sin(toAngle - juce::MathConstants<float>::halfPi);
    
    juce::ColourGradient dotGradient(cyberpunkPink.brighter(0.3f), dotX - dotRadius, dotY - dotRadius,
                                     cyberpunkPink.darker(0.3f), dotX + dotRadius, dotY + dotRadius,
                                     true);
    g.setGradientFill(dotGradient);
    g.fillEllipse(dotX - dotRadius, dotY - dotRadius, dotRadius * 2, dotRadius * 2);

    // Subtle highlight
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.fillEllipse(bounds.reduced(radius * 0.35f).translated(0, -radius * 0.1f));
}

