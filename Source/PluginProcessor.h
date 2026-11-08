#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <atomic>
#include "SynthEngine.h"
#include "Fx.h"

class TapeRompProcessor : public juce::AudioProcessor
{
public:
    TapeRompProcessor();
    ~TapeRompProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Kematik Tapes"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Вызывается редактором при выборе пресета пользователем:
    // сбрасывает вкладку EDIT на стандартные значения пресета и ставит арпеджиатор.
    void applyPresetDefaults (int presetIndex);

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<int> activeVoices { 0 };   // для анимации катушек

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    void generateArp (const juce::MidiBuffer& in, int numSamples, const tr::Preset& pr, double bpm);
    void arpNoteOn (int note, float vel);
    void arpNoteOff (int note);
    void arpClear();
    int nextArpNote (int pattern);

    tr::SynthEngine engine;
    tr::FxChain fx;
    juce::MidiBuffer arpEvents;

    std::atomic<float>* presetP  = nullptr;
    std::atomic<float>* attackP  = nullptr;
    std::atomic<float>* releaseP = nullptr;
    std::atomic<float>* cutoffP  = nullptr;
    std::atomic<float>* resoP    = nullptr;
    std::atomic<float>* driveP   = nullptr;
    std::atomic<float>* lofiP    = nullptr;
    std::atomic<float>* chorusP  = nullptr;
    std::atomic<float>* reverbP  = nullptr;
    std::atomic<float>* rsizeP   = nullptr;
    std::atomic<float>* dlyMixP  = nullptr;
    std::atomic<float>* dlyTimeP = nullptr;
    std::atomic<float>* dlyFbP   = nullptr;
    std::atomic<float>* gainP    = nullptr;
    std::atomic<float>* arpP     = nullptr;
    std::atomic<float>* arpRateP = nullptr;

    bool heldFlag[128] {};
    int heldCount = 0;
    float heldVel = 0.8f;
    double stepPos = 0.0;
    bool retrigger = false;
    int arpNote = -1;
    int arpIndex = 0;
    bool wasArp = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeRompProcessor)
};
