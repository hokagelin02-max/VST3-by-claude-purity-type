#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace tr;

TapeRompProcessor::TapeRompProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    presetP  = apvts.getRawParameterValue ("preset");
    attackP  = apvts.getRawParameterValue ("attack");
    releaseP = apvts.getRawParameterValue ("release");
    cutoffP  = apvts.getRawParameterValue ("cutoff");
    resoP    = apvts.getRawParameterValue ("reso");
    driveP   = apvts.getRawParameterValue ("drive");
    lofiP    = apvts.getRawParameterValue ("lofi");
    chorusP  = apvts.getRawParameterValue ("chorus");
    reverbP  = apvts.getRawParameterValue ("reverb");
    rsizeP   = apvts.getRawParameterValue ("rsize");
    dlyMixP  = apvts.getRawParameterValue ("dlymix");
    dlyTimeP = apvts.getRawParameterValue ("dlytime");
    dlyFbP   = apvts.getRawParameterValue ("dlyfb");
    gainP    = apvts.getRawParameterValue ("gain");
    arpP     = apvts.getRawParameterValue ("arp");
    arpRateP = apvts.getRawParameterValue ("arprate");
}

juce::AudioProcessorValueTreeState::ParameterLayout TapeRompProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;
    auto f = [&l] (const char* id, const char* name, float def)
    {
        l.add (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, 0.f, 1.f, def));
    };

    l.add (std::make_unique<AudioParameterInt> (ParameterID { "preset", 1 }, "Preset", 0, kNumPresets - 1, 0));

    // ---- EDIT (сбрасываются при смене пресета)
    f ("attack",  "Attack",       0.5f);
    f ("release", "Release",      0.5f);
    f ("cutoff",  "Tone",         0.5f);
    f ("reso",    "Resonance",    0.5f);
    f ("drive",   "Drive",        0.0f);
    f ("lofi",    "Lo-Fi",        0.0f);
    f ("chorus",  "Chorus",       0.0f);
    f ("reverb",  "Reverb",       0.2f);
    f ("rsize",   "Reverb Size",  0.5f);
    f ("dlymix",  "Delay Mix",    0.0f);
    l.add (std::make_unique<AudioParameterInt> (ParameterID { "dlytime", 1 }, "Delay Time", 0, 4, 2));
    f ("dlyfb",   "Delay Feedback", 0.35f);

    // ---- общие
    f ("gain", "Volume", 0.7f);
    l.add (std::make_unique<AudioParameterBool>   (ParameterID { "arp", 1 }, "Arp", false));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "arprate", 1 }, "Arp Rate",
                                                   StringArray { "1/4", "1/8", "1/16", "1/32" }, 2));
    return l;
}

void TapeRompProcessor::applyPresetDefaults (int idx)
{
    idx = juce::jlimit (0, kNumPresets - 1, idx);
    const Preset& pr = kPresets[idx];

    auto set = [this] (const char* id, float denormalised)
    {
        if (auto* p = apvts.getParameter (id))
        {
            p->beginChangeGesture();
            p->setValueNotifyingHost (p->convertTo0to1 (denormalised));
            p->endChangeGesture();
        }
    };

    set ("attack", 0.5f);   set ("release", 0.5f);  set ("cutoff", 0.5f);  set ("reso", 0.5f);
    set ("drive", 0.f);     set ("lofi", 0.f);      set ("chorus", 0.f);
    set ("reverb", pr.reverb);  set ("rsize", 0.5f);
    set ("dlymix", pr.delay);   set ("dlytime", 2.f);  set ("dlyfb", 0.35f);
    set ("arp", pr.arp ? 1.f : 0.f);
}

void TapeRompProcessor::prepareToPlay (double sampleRate, int)
{
    engine.prepare (sampleRate);
    fx.prepare (sampleRate);
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
        if (heldCount++ == 0) retrigger = true;
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

void TapeRompProcessor::generateArp (const juce::MidiBuffer& in, int numSamples, const Preset& pr, double bpm)
{
    arpEvents.clear();

    for (const auto meta : in)
    {
        const auto m = meta.getMessage();
        if (m.isNoteOn())                                arpNoteOn (m.getNoteNumber(), m.getFloatVelocity());
        else if (m.isNoteOff())                          arpNoteOff (m.getNoteNumber());
        else if (m.isAllNotesOff() || m.isAllSoundOff()) arpClear();
    }

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

    double bpm = 120.0;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm())
                bpm = juce::jmax (20.0, *b);

    // макросы из вкладки EDIT
    const float cutMul = std::exp2 ((cutoffP->load() - 0.5f) * 4.f);
    const float attMul = std::exp2 ((attackP->load() - 0.5f) * 5.f);
    const float relMul = std::exp2 ((releaseP->load() - 0.5f) * 4.f);
    const float resOff = (resoP->load() - 0.5f) * 0.8f;
    engine.setMacros (pr, cutMul, attMul, relMul, resOff);

    const bool arpOn = arpP->load() > 0.5f;
    const juce::MidiBuffer* src = &midi;

    if (arpOn)
    {
        generateArp (midi, n, pr, bpm);
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

    // эффекты
    static const double delayBeats[5] = { 0.25, 0.375, 0.5, 0.75, 1.0 };
    FxParams fp;
    fp.drive = driveP->load();
    fp.lofi = lofiP->load();
    fp.chorus = chorusP->load();
    fp.delayMix = dlyMixP->load() * 0.7f;
    fp.delayFb = dlyFbP->load();
    fp.delaySamples = getSampleRate() * 60.0 / bpm * delayBeats[juce::jlimit (0, 4, (int) dlyTimeP->load())];
    fp.reverbAmt = reverbP->load();
    fp.reverbSize = rsizeP->load();
    fx.process (L, R, n, fp);

    // мастер-громкость + страховочный мягкий клиппер
    const float vol = gainP->load();
    const float g = vol * vol * 1.5f;
    for (int i = 0; i < n; ++i)
    {
        L[i] = std::tanh (L[i] * g);
        R[i] = std::tanh (R[i] * g);
    }

    activeVoices.store (engine.numActive());
}

void TapeRompProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void TapeRompProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* TapeRompProcessor::createEditor()
{
    return new TapeRompEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapeRompProcessor();
}
