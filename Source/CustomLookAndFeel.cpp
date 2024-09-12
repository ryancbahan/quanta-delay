#include "CustomLookAndFeel.h"

CustomLookAndFeel::CustomLookAndFeel()
{
    generateNoiseTexture();
}

CustomLookAndFeel::~CustomLookAndFeel() = default;

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

void CustomLookAndFeel::drawBackground(juce::Graphics& g, int width, int height)
{
    auto bounds = juce::Rectangle<float>(0, 0, width, height);
    
    // Create a gradient for the background
    juce::ColourGradient gradient(juce::Colour(45, 45, 50), bounds.getTopLeft(),
                                  juce::Colour(30, 30, 35), bounds.getBottomRight(),
                                  false);
    g.setGradientFill(gradient);
    g.fillAll();

    // Apply the noise texture
    g.setTiledImageFill(noiseTexture, 0, 0, 0.2f);
    g.fillAll();

    // Add a subtle vignette effect
    juce::ColourGradient vignette(juce::Colours::black.withAlpha(0.3f), bounds.getCentre(),
                                  juce::Colours::transparentBlack, bounds.getTopLeft(),
                                  true);
    g.setGradientFill(vignette);
    g.fillAll();
}

void CustomLookAndFeel::generateNoiseTexture()
{
    const int size = 256;
    noiseTexture = juce::Image(juce::Image::ARGB, size, size, true);

    juce::Random random;
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            float noise = random.nextFloat() * 0.2f;
            noiseTexture.setPixelAt(x, y, juce::Colour(noise, noise, noise, 1.0f));
        }
    }
}
