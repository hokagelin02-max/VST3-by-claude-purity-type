#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

// Look & feel в стиле старого кассетного магнитофона
class RetroLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RetroLookAndFeel();
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool, bool) override;
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
};

class TapeRompEditor : public juce::AudioProcessorEditor,
                       private juce::Timer,
                       private juce::ListBoxModel
{
public:
    explicit TapeRompEditor (TapeRompProcessor&);
    ~TapeRompEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    static constexpr int kTabs = 8;     // 7 категорий + EDIT
    static constexpr int kEditKnobs = 12;

    void timerCallback() override;
    int getNumRows() override { return (int) catPresets.size(); }
    void paintListBoxItem (int row, juce::Graphics&, int w, int h, bool selected) override;
    void selectedRowsChanged (int lastRowSelected) override;

    void showCategory (int cat);
    void showEdit();
    void rebuildList (int cat);
    void updateVisibility();
    void setPreset (int globalIndex);
    void syncUI (int globalIndex);
    void drawCassette (juce::Graphics&);
    static void drawReel (juce::Graphics&, juce::Point<float> c, float tapeR, float hubR, float angle);

    struct Knob
    {
        juce::Slider s;
        juce::Label l;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
    };
    void setupKnob (Knob&, const char* text, const char* paramId);

    TapeRompProcessor& proc;
    RetroLookAndFeel laf;

    juce::TextButton tabs[kTabs];
    juce::ListBox list { "presets", this };
    Knob editKnobs[kEditKnobs];
    Knob volumeKnob;
    juce::TextButton arpBtn { "ARP" };
    juce::ComboBox rateBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> arpAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rateAtt;

    juce::RangedAudioParameter* presetParam = nullptr;
    std::vector<int> catPresets;
    int currentCat = -1;
    int lastPreset = -1;
    bool editMode = false;
    float reel = 0.f, reelSpeed = 0.f;
    bool syncing = false;
    juce::Rectangle<float> cassetteArea;
    juce::Rectangle<int> midArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeRompEditor)
};
