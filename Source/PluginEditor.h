/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
using namespace juce;

//==============================================================================
/**
*/
struct OtherLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OtherLookAndFeel()
    {
        setColour (juce::Slider::thumbColourId, juce::Colours::darkblue);
    }
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider, const String& label)
    {
        auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
        auto centreX = (float) x + (float) width  * 0.5f;
        auto centreY = (float) y + (float) height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();
 
        // fill
        g.setColour (juce::Colours::skyblue);
        g.fillEllipse (rx, ry, rw, rw);
        
        // outline
        g.setColour (juce::Colours::darkblue);
        g.drawEllipse (rx, ry, rw, rw, 1.0f);
        
        if (slider.isEnabled())
            g.setColour (slider.findColour (Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 1.0f : 0.7f));
        else
            g.setColour (Colour (0x80808080));
        
        juce::Path p;
        auto pointerLength = radius * 0.33f;
        auto pointerThickness = 2.0f;
        p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
        // pointer
        g.setColour (juce::Colours::white);
        g.fillPath (p);
        // textbox
        g.setFont(24.0f);
        g.drawFittedText(label , x, slider.getBottom() - 3, width, 25, Justification::centred, 3);
        g.setColour(Colours::whitesmoke);
    }
};

class EZDistortionAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer, public OtherLookAndFeel
{
public:
    EZDistortionAudioProcessorEditor (EZDistortionAudioProcessor&);
    ~EZDistortionAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void setBoxParameters (ComboBox& box)
    {
        box.addItem(String("Soft Clipping"), 1);
        box.addItem(String("Hard Clipping"), 2);
        box.addItem(String("Foldback"), 3);
        box.addItem(String("ScoopFold"), 4);
        box.setJustificationType(Justification::centred);
        addAndMakeVisible(box);
        box.setSelectedId(1);
    }
    
    void setSliderParametersDial (Slider& slider, bool hasTextBox = false)
    {
        slider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
        if (hasTextBox)
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 25);
        else
            slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 50, 25);
        addAndMakeVisible(slider);
        slider.setAlpha(0);
        slider.setLookAndFeel(&otherLookAndFeel);
    }
    void drawGroupRectangle (Slider& slider1, Slider& slider2, const String& label, Graphics& g, bool isVertical=true, bool hasLabel=false)
    {
        
        int hPad = 10;
        int vPad = 10;
        int xpos = slider1.getX() - hPad/2;
        int ypos = slider1.getY() - vPad/2;
        int width = 0;
        int height = 0;
        if (isVertical)
        {
            width = slider1.getWidth() + (hPad);
            height = (slider1.getHeight() * 2) + distanceBetweenSlidersVertical + (vPad * 3 );
        }
        else
        {
            width = (slider1.getWidth() * 2) + horizontalDistance + (hPad *2 );
            height = slider1.getHeight() + (vPad * 2);
        }
        g.setColour(Colours::white);
        g.drawRoundedRectangle(xpos, ypos, width, height, 20, 2);
        g.setColour(Colours::grey);
        auto fillRect = Rectangle<float>(xpos, ypos, width, height);
        g.setOpacity(.5);
        g.fillRoundedRectangle(fillRect, 20);
           
        if (hasLabel)
        {
            int textPosX = 0;
            if (isVertical)
                textPosX = 0;
            else
                textPosX = 0;
            g.drawFittedText(label, textPosX, slider1.getY() - (vPad*2), width, 20, Justification::centredTop, 3);
        }
       
       }
       void drawParamText(Graphics& g);
       void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    EZDistortionAudioProcessor& audioProcessor;
    OtherLookAndFeel otherLookAndFeel;
    
    Slider gainSlider;
    Slider mixSlider;
    Slider thresholdSlider;
    ComboBox distortionType;
    
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;

    int sliderWidthAndHeight = 100;
    int distanceBetweenSlidersVertical = 20;
    int horizontalDistance = 40;
    int row1X = 50;
    int column1Y = 100;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EZDistortionAudioProcessorEditor)
};
