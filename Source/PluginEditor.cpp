/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EZDistortionAudioProcessorEditor::EZDistortionAudioProcessorEditor (EZDistortionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 360);
    Timer::startTimerHz(20);

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    mixAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "MIX", mixSlider);
    gainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "GAIN", gainSlider);
    thresholdAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "THRESHOLD", thresholdSlider);
    typeAttachment = std::make_unique<BoxAttachment>(audioProcessor.apvts, "TYPE", distortionType);
    
    setSliderParametersDial(mixSlider, true);
    setSliderParametersDial(gainSlider, true);
    setSliderParametersDial(thresholdSlider, true);
    
    setBoxParameters(distortionType);

    mixSlider.setRotaryParameters(4 * M_PI / 3, 8 * M_PI /3, true);
    gainSlider.setRotaryParameters(4 * M_PI / 3, 8 * M_PI /3, true);
    thresholdSlider.setRotaryParameters(4 * M_PI / 3, 8 * M_PI /3, true);
}

EZDistortionAudioProcessorEditor::~EZDistortionAudioProcessorEditor()
{
}

//==============================================================================
void EZDistortionAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto sliderPosGain =  Decibels::decibelsToGain(gainSlider.getValue()) / Decibels::decibelsToGain(gainSlider.getMaximum());
    auto sliderPosThreshold = Decibels::decibelsToGain(thresholdSlider.getValue()) / Decibels::decibelsToGain(thresholdSlider.getMaximum());
    auto sliderPosMix = mixSlider.getValue() / mixSlider.getMaximum();
    auto titleFont = Font("Euphemia UCAS", 60.0f, Font::plain);
    auto fillRect2 = Rectangle<float>(mixSlider.getX()-5, mixSlider.getY() - 5, mixSlider.getWidth()+10, mixSlider.getHeight() + 25);
    g.fillAll (Colours::black);
    g.setFont(titleFont);
    g.setColour(Colours::skyblue);
    g.setOpacity(1);
    g.drawFittedText(String("EZ Distortion"), 160, 0, 200, 60, Justification::centredTop, 1);
    g.setColour(Colours::grey);
    g.setOpacity(.50);
    g.fillRoundedRectangle(fillRect2, 10);
    g.setColour(Colours::white);
    g.drawRoundedRectangle(fillRect2, 10, 2);
    
    drawGroupRectangle(gainSlider, thresholdSlider, String("Stuff"), g);
    drawRotarySlider(g, mixSlider.getX(), mixSlider.getY(), sliderWidthAndHeight, sliderWidthAndHeight, sliderPosMix, 4 * M_PI / 3, 8*M_PI/3, mixSlider, String("Mix"));
    drawRotarySlider(g, gainSlider.getX(), gainSlider.getY(), sliderWidthAndHeight, sliderWidthAndHeight, sliderPosGain, 4 * M_PI / 3, 8 * M_PI /3, gainSlider, String("Gain"));
    drawRotarySlider(g, thresholdSlider.getX(), thresholdSlider.getY(), sliderWidthAndHeight, sliderWidthAndHeight, sliderPosThreshold , 4 * M_PI / 3, 8 * M_PI /3, thresholdSlider, String("Threshold"));
    drawParamText(g);

}

void EZDistortionAudioProcessorEditor::resized()
{
    distortionType.setBounds(row1X-25, column1Y, 150, 25);
    mixSlider.setBounds(row1X, distortionType.getBottom() + distanceBetweenSlidersVertical, sliderWidthAndHeight, sliderWidthAndHeight);
    gainSlider.setBounds(mixSlider.getRight()+horizontalDistance, column1Y, sliderWidthAndHeight, sliderWidthAndHeight);
    thresholdSlider.setBounds(gainSlider.getX(), gainSlider.getBottom()+distanceBetweenSlidersVertical, sliderWidthAndHeight, sliderWidthAndHeight);
}
void EZDistortionAudioProcessorEditor::drawParamText(Graphics &g)
{
    auto text = String("");
    if (gainSlider.isMouseOverOrDragging())
    {
        auto& gain = *audioProcessor.apvts.getRawParameterValue("GAIN");
        text = String("Gain:         " + std::to_string(gain) + "dB");
    }
    if (mixSlider.isMouseOverOrDragging())
    {
        auto& mix = *audioProcessor.apvts.getRawParameterValue("MIX");
        text = String("Mix:           " + std::to_string(mix));
    }
    if (thresholdSlider.isMouseOverOrDragging())
    {
        auto& threshold = *audioProcessor.apvts.getRawParameterValue("THRESHOLD");
        text = String("Threshold:      " + std::to_string(threshold) + "dB");
    }
    if (distortionType.isMouseOverOrDragging())
    {
        text = String("Type:           " + distortionType.getText());
    }
    auto textRect = Rectangle<float>(2, 10, 150, 50);
    g.setColour(Colours::white);
    g.drawFittedText(text, 5, 10, 140, 50, Justification::centredTop, 2);
    g.drawRoundedRectangle(textRect, 10, 2);
    g.setColour(Colours::grey);
    g.setOpacity(.5);
    g.drawRoundedRectangle(textRect, 10, 2);
}
void EZDistortionAudioProcessorEditor::timerCallback()
{
    repaint();
}
