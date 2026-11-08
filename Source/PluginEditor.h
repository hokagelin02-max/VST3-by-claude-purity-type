#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

// ---------------------------------------------------------------- Look & feel (ручки)
class RetroLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RetroLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
};

// ---------------------------------------------------------------- резиновая кнопка магнитолы
class DeckButton : public juce::Button
{
public:
    enum Icon { None, Prev, Next, Back, Search };

    DeckButton (const juce::String& text, Icon ic = None, float fontSize = 11.f);
    void paintButton (juce::Graphics&, bool over, bool down) override;

    bool amberWhenOn = false;   // подсвечивать янтарным, когда включена (ARP)

private:
    Icon icon;
    float fs;
};

// ---------------------------------------------------------------- янтарный LCD-дисплей
class LcdDisplay : public juce::Component
{
public:
    LcdDisplay() { setMouseCursor (juce::MouseCursor::PointingHandCursor); }

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override { if (onClick) onClick(); }

    std::function<void()> onClick;
    juce::String cat, num, name, rate;
    bool arpOn = false;
    float bars[5] { 0.f, 0.f, 0.f, 0.f, 0.f };
};

// ---------------------------------------------------------------- редактор
class TapeRompEditor : public juce::AudioProcessorEditor,
                       private juce::Timer
{
public:
    explicit TapeRompEditor (TapeRompProcessor&);
    ~TapeRompEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    static constexpr int kEditKnobs = 12;

    struct Knob
    {
        juce::Slider s;
        juce::Label l;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
    };

    void setupKnob (Knob&, const char* text, const char* paramId, bool chrome);
    void timerCallback() override;

    void setPreset (int globalIndex);
    void stepPreset (int delta);
    void stepCategory (int delta);
    void showPresetMenu();
    void refreshLcd();
    void cycleRate();

    void drawDash (juce::Graphics&);
    void drawFaceplate (juce::Graphics&);
    void drawCable (juce::Graphics&);

    TapeRompProcessor& proc;
    RetroLookAndFeel laf;

    // кнопки / дисплей
    DeckButton offBtn  { "OFF", DeckButton::None, 11.f };
    DeckButton seekPrev { "", DeckButton::Prev };
    DeckButton seekNext { "", DeckButton::Next };
    DeckButton backBtn  { "", DeckButton::Back };
    DeckButton searchBtn { "", DeckButton::Search };
    DeckButton modeBtn  { "MODE", DeckButton::None, 12.f };
    DeckButton zapBtn   { "ZAP", DeckButton::None, 10.f };
    DeckButton arpBtn   { "ARP", DeckButton::None, 10.f };
    DeckButton rateBtn  { "RATE", DeckButton::None, 10.f };
    DeckButton numBtn[6] { { "1" }, { "2" }, { "3" }, { "4" }, { "5" }, { "6" } };
    LcdDisplay lcd;

    Knob volume;
    Knob editKnobs[kEditKnobs];

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> arpAtt;

    juce::RangedAudioParameter* presetParam = nullptr;
    int catFirst[tr::kNumCats] {};
    int catCount[tr::kNumCats] {};
    int lastPreset = -1;
    int lastArp = -1, lastRate = -1;

    juce::Image noise;
    juce::Random rng;
    juce::Rectangle<int> faceRect, panelRect;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeRompEditor)
};
