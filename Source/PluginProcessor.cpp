#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace tr;

TapeRompProcessor::TapeRompProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    presetP  = apvts.getRawParameterValue ("preset");
    cutoffP  = apvts.getRawParameterValue ("cutoff");
    releaseP = apvts.getRawParameterValue ("release");
    reverbP  = apvts.getRawParameterValue ("reverb");
    gainP    = apvts.getRawParameterValue ("gain");
    arpP     = apvts.getRawParameterValue ("arp");
    arpRateP = apvts.getRawParameterValue ("arprate");

    apvts.addParameterListener ("preset", this);
}

TapeRompProcessor::~TapeRompProcessor()
{
    apvts.removeParameterListener ("preset", this);
}

juce::AudioProcessorValueTreeState::ParameterLayout TapeRompProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;
    l.add (std::make_unique<AudioParameterInt>   (ParameterID { "preset", 1 },  "Preset", 0, kNumPresets - 1, 0));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "cutoff", 1 },  "Tone",    0.f, 1.f, 0.5f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "release", 1 }, "Release", 0.f, 1.f, 0.5f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "reverb", 1 },  "Reverb",  0.f, 1.f, 0.20f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "gain", 1 },    "Volume",  0.f, 1.f, 0.70f));
    l.add (std::make_unique<AudioParameterBool>  (ParameterID { "arp", 1 },     "Arp", false));
    l.add (std::make_unique<AudioParameterChoice>(ParameterID { "arprate", 1 }, "Arp Rate",
                                                  StringArray { "1/4", "1/8", "1/16", "1/32" }, 2));
    return l;
}

// При смене пресета: категория ARPS включает арпеджиатор, остальные выключают
void TapeRompProcessor::parameterChanged (const juce::String& id, float v)
{
    if (id != "preset" || restoring.load()) return;
    const int i = juce::jlimit (0, kNumPresets - 1, (int) v);
    if (auto* a = apvts.getParameter ("arp"))
        a->setValueNotifyingHost (kPresets[i].arp ? 1.f : 0.f);
}

void TapeRompProcessor::prepareToPlay (double sampleRate, int)
{
    engine.prepare (sampleRate);
    reverb.setSampleRate (sampleRate);
    reverb.reset();
    arpClear();
    arpNote = -1;
    wasArp = false;
}

bool TapeRompProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

// ---------------------------------------------------------------- арпеджиатор
void TapeRompProcessor::arpNoteOn (int n, float v)
{
    if (! heldFlag[n])
    {
        heldFlag[n] = true;
        if (heldCount++ == 0) retrigger = true;   // первая нота -> стартуем сразу
    }
    heldVel = v;
}

void TapeRompProcessor::arpNoteOff (int n)
{
    if (heldFlag[n]) { heldFlag[n] = false; --heldCount; }
}

void TapeRompProcessor::arpClear()
{
    for (auto& f : heldFlag) f = false;
    heldCount = 0;
    arpIndex = 0;
}

int TapeRompProcessor::nextArpNote (int pattern)
{
    int seq[256];
    int len = 0;
    for (int n = 0; n < 128; ++n)
        if (heldFlag[n]) seq[len++] = n;
    if (len == 0) return -1;

    if (pattern == 3)
    {
        const int base = len;
        for (int i = 0; i < base; ++i) seq[len++] = seq[i] + 12;
    }

    int pos;
    switch (pattern)
    {
        case 1:  pos = len - 1 - (arpIndex % len); break;
        case 2:
        {
            const int cycle = len > 1 ? 2 * len - 2 : 1;
            const int k = arpIndex % cycle;
            pos = k < len ? k : cycle - k;
            break;
        }
        default: pos = arpIndex % len; break;
    }
    arpIndex = (arpIndex + 1) % 100000;
    return juce::jlimit (0, 127, seq[pos]);
}

void TapeRompProcessor::generateArp (const juce::MidiBuffer& in, int numSamples, const Preset& pr)
{
    arpEvents.clear();

    for (const auto meta : in)
    {
        const auto m = meta.getMessage();
        if (m.isNoteOn())                              arpNoteOn (m.getNoteNumber(), m.getFloatVelocity());
        else if (m.isNoteOff())                        arpNoteOff (m.getNoteNumber());
        else if (m.isAllNotesOff() || m.isAllSoundOff()) arpClear();
    }

    double bpm = 120.0;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm())
                bpm = juce::jmax (20.0, *b);

    static const double beats[4] = { 1.0, 0.5, 0.25, 0.125 };
    const int rate = juce::jlimit (0, 3, (int) arpRateP->load());
    const double stepLen = getSampleRate() * 60.0 / bpm * beats[rate];
    const double gateLen = stepLen * 0.7;

    for (int i = 0; i < numSamples; ++i)
    {
        if (retrigger || stepPos >= stepLen)
        {
            stepPos = retrigger ? 0.0 : stepPos - stepLen;
            retrigger = false;

            if (arpNote >= 0)
            {
                arpEvents.addEvent (juce::MidiMessage::noteOff (1, arpNote), i);
                arpNote = -1;
            }

            const int note = nextArpNote (pr.arpPattern);
            if (note >= 0)
            {
                arpEvents.addEvent (juce::MidiMessage::noteOn (1, note, heldVel), i);
                arpNote = note;
            }
        }
        else if (arpNote >= 0 && stepPos >= gateLen)
        {
            arpEvents.addEvent (juce::MidiMessage::noteOff (1, arpNote), i);
            arpNote = -1;
        }

        stepPos += 1.0;
    }
}

// ---------------------------------------------------------------- аудио
void TapeRompProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();

    const int idx = juce::jlimit (0, kNumPresets - 1, (int) presetP->load());
    const Preset& pr = kPresets[idx];
    const float cutMul = std::exp2 ((cutoffP->load() - 0.5f) * 4.f);
    const float relMul = std::exp2 ((releaseP->load() - 0.5f) * 4.f);
    engine.setPreset (pr, cutMul, relMul);

    const bool arpOn = arpP->load() > 0.5f;
    const juce::MidiBuffer* src = &midi;

    if (arpOn)
    {
        generateArp (midi, n, pr);
        src = &arpEvents;
        wasArp = true;
    }
    else if (wasArp)
    {
        engine.allNotesOff();
        arpClear();
        arpNote = -1;
        wasArp = false;
    }

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getWritePointer (1);

    int pos = 0;
    for (const auto meta : *src)
    {
        const auto m = meta.getMessage();
        const int t = juce::jlimit (pos, n, meta.samplePosition);
        if (t > pos) { engine.render (L + pos, R + pos, t - pos); pos = t; }

        if (m.isNoteOn())                                   engine.noteOn (m.getNoteNumber(), m.getFloatVelocity());
        else if (m.isNoteOff())                             engine.noteOff (m.getNoteNumber());
        else if (m.isAllNotesOff() || m.isAllSoundOff())    engine.allNotesOff();
    }
    if (pos < n) engine.render (L + pos, R + pos, n - pos);

    // мастер-громкость + мягкий клиппер
    float g = gainP->load();
    g = g * g * 1.6f;
    for (int i = 0; i < n; ++i)
    {
        L[i] = std::tanh (L[i] * g);
        R[i] = std::tanh (R[i] * g);
    }

    // реверберация (пропускается, если выключена -> экономия CPU)
    const float rev = reverbP->load();
    if (rev > 0.005f)
    {
        juce::Reverb::Parameters rp;
        rp.roomSize = 0.6f;
        rp.damping = 0.5f;
        rp.wetLevel = rev * 0.4f;
        rp.dryLevel = 1.0f - 0.2f * rev;
        rp.width = 1.0f;
        reverb.setParameters (rp);
        reverb.processStereo (L, R, n);
    }

    activeVoices.store (engine.numActive());
}

// ---------------------------------------------------------------- состояние
void TapeRompProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void TapeRompProcessor::setStateInformation (const void* data, int size)
{
    restoring = true;
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
    restoring = false;
}

juce::AudioProcessorEditor* TapeRompProcessor::createEditor()
{
    return new TapeRompEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapeRompProcessor();
}
