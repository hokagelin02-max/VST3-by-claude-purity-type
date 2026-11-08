#include "PluginEditor.h"
#include <cmath>

using namespace juce;
using namespace tr;

static Font mono (float h, bool bold = true)
{
    return Font (FontOptions (Font::getDefaultMonospacedFontName(), h, bold ? Font::bold : Font::plain));
}

static const char* kRateNames[4] = { "1/4", "1/8", "1/16", "1/32" };

// ======================================================= LookAndFeel
RetroLookAndFeel::RetroLookAndFeel()
{
    setColour (PopupMenu::backgroundColourId, Colour (0xff222224));
    setColour (PopupMenu::textColourId, Colour (0xffe6e6e8));
    setColour (PopupMenu::highlightedBackgroundColourId, Colour (0xffe8a33d));
    setColour (PopupMenu::highlightedTextColourId, Colour (0xff1a1206));
    setColour (BubbleComponent::backgroundColourId, Colour (0xff111113));
    setColour (BubbleComponent::outlineColourId, Colour (0xffffa31a));
    setColour (TooltipWindow::textColourId, Colour (0xffffa31a));
}

void RetroLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos,
                                         float a0, float a1, Slider& s)
{
    auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (3.f);
    const auto c = b.getCentre();
    const float r = jmin (b.getWidth(), b.getHeight()) * 0.5f;
    const float ang = a0 + pos * (a1 - a0);
    const float twoPi = MathConstants<float>::twoPi;

    if (s.getName() == "chrome")
    {
        // ---- хромированная ручка громкости
        g.setColour (Colour (0x99000000));
        g.fillEllipse (b.translated (0.f, 4.f).expanded (2.f));
        g.setColour (Colour (0xff0c0c0d));
        g.fillEllipse (b);

        auto ring = b.reduced (3.f);
        ColourGradient cg (Colour (0xfff4f4f6), ring.getX(), ring.getY(),
                           Colour (0xff4d4d52), ring.getRight(), ring.getBottom(), false);
        cg.addColour (0.35, Colour (0xffd0d0d4));
        cg.addColour (0.50, Colour (0xff7b7b80));
        cg.addColour (0.70, Colour (0xffc4c4c8));
        g.setGradientFill (cg);
        g.fillEllipse (ring);

        g.setColour (Colour (0x30000000));
        for (int i = 0; i < 72; ++i)
        {
            const float a = (float) i * twoPi / 72.f;
            g.drawLine (c.x + std::cos (a) * r * 0.86f, c.y + std::sin (a) * r * 0.86f,
                        c.x + std::cos (a) * r * 0.95f, c.y + std::sin (a) * r * 0.95f, 1.f);
        }

        auto face = b.reduced (r * 0.28f);
        ColourGradient fg (Colour (0xffeeeef0), face.getX(), face.getY(),
                           Colour (0xff6a6a70), face.getRight(), face.getBottom(), false);
        fg.addColour (0.5, Colour (0xffb9b9be));
        g.setGradientFill (fg);
        g.fillEllipse (face);

        g.setColour (Colour (0x16000000));
        for (int k = 1; k <= 6; ++k)
            g.drawEllipse (face.reduced ((float) k * face.getWidth() * 0.07f), 0.7f);
        g.setColour (Colour (0x66000000));
        g.drawEllipse (face, 1.2f);

        Path p;
        p.addRoundedRectangle (-2.f, -r * 0.93f, 4.f, r * 0.17f, 1.5f);
        g.setColour (Colour (0xff111111));
        g.fillPath (p, AffineTransform::rotation (ang).translated (c.x, c.y));
        return;
    }

    // ---- пластиковая ручка торпедо
    const float kr = r * 0.62f;

    g.setColour (Colour (0xffd9d9d0));
    for (int i = 0; i <= 10; ++i)
    {
        const float a = a0 + (float) i / 10.f * (a1 - a0);
        const float R = r * 0.93f;
        g.fillEllipse (c.x + R * std::sin (a) - 1.3f, c.y - R * std::cos (a) - 1.3f, 2.6f, 2.6f);
    }

    g.setColour (Colour (0x88000000));
    g.fillEllipse (c.x - kr, c.y - kr + 3.f, kr * 2.f, kr * 2.f);

    ColourGradient body (Colour (0xff3d3d40), c.x - kr, c.y - kr, Colour (0xff0e0e0f), c.x + kr, c.y + kr, false);
    g.setGradientFill (body);
    g.fillEllipse (c.x - kr, c.y - kr, kr * 2.f, kr * 2.f);

    g.setColour (Colour (0xff060606));
    for (int i = 0; i < 28; ++i)
    {
        const float a = (float) i * twoPi / 28.f;
        g.drawLine (c.x + std::cos (a) * kr * 0.86f, c.y + std::sin (a) * kr * 0.86f,
                    c.x + std::cos (a) * kr * 0.99f, c.y + std::sin (a) * kr * 0.99f, 1.4f);
    }

    const float fr = kr * 0.78f;
    ColourGradient top (Colour (0xff4a4a4e), c.x - fr, c.y - fr, Colour (0xff18181a), c.x + fr, c.y + fr, false);
    g.setGradientFill (top);
    g.fillEllipse (c.x - fr, c.y - fr, fr * 2.f, fr * 2.f);

    Path p;
    p.addRoundedRectangle (-1.6f, -fr + 2.f, 3.2f, fr * 0.6f, 1.5f);
    g.setColour (Colour (0xfff2f2ea));
    g.fillPath (p, AffineTransform::rotation (ang).translated (c.x, c.y));
}

// ======================================================= DeckButton
DeckButton::DeckButton (const String& text, Icon ic, float fontSize)
    : Button (text), icon (ic), fs (fontSize)
{
    setClickingTogglesState (false);
}

void DeckButton::paintButton (Graphics& g, bool over, bool down)
{
    auto r = getLocalBounds().toFloat().reduced (1.f);
    if (down) r = r.translated (0.f, 1.f);

    g.setColour (Colours::black);
    g.fillRoundedRectangle (r.translated (0.f, 1.5f), 4.f);

    ColourGradient gr (down ? Colour (0xff1c1c1e) : (over ? Colour (0xff4b4b4f) : Colour (0xff3d3d41)), r.getX(), r.getY(),
                       down ? Colour (0xff141416) : Colour (0xff1b1b1d), r.getX(), r.getBottom(), false);
    g.setGradientFill (gr);
    g.fillRoundedRectangle (r, 4.f);

    g.setColour (Colour (0x26ffffff));
    g.drawLine (r.getX() + 4.f, r.getY() + 1.5f, r.getRight() - 4.f, r.getY() + 1.5f, 1.f);
    g.setColour (Colour (0xff050506));
    g.drawRoundedRectangle (r, 4.f, 1.f);

    const bool lit = getToggleState() && amberWhenOn;
    g.setColour (lit ? Colour (0xffffb347) : Colour (0xffc4c4c8));
    const auto c = r.getCentre();

    auto tri = [&] (float cx, float cy, bool left)
    {
        Path t;
        if (left) t.addTriangle (cx + 4.f, cy - 5.f, cx + 4.f, cy + 5.f, cx - 4.f, cy);
        else      t.addTriangle (cx - 4.f, cy - 5.f, cx - 4.f, cy + 5.f, cx + 4.f, cy);
        g.fillPath (t);
    };

    switch (icon)
    {
        case Prev:   tri (c.x - 5.f, c.y, true);  tri (c.x + 4.f, c.y, true);  break;
        case Next:   tri (c.x - 4.f, c.y, false); tri (c.x + 5.f, c.y, false); break;
        case Back:
        {
            Path a;
            a.startNewSubPath (c.x + 7.f, c.y + 6.f);
            a.lineTo (c.x + 7.f, c.y - 1.f);
            a.quadraticTo (c.x + 7.f, c.y - 5.f, c.x + 2.f, c.y - 5.f);
            a.lineTo (c.x - 4.f, c.y - 5.f);
            g.strokePath (a, PathStrokeType (2.2f));
            Path head;
            head.addTriangle (c.x - 3.f, c.y - 10.f, c.x - 3.f, c.y, c.x - 9.f, c.y - 5.f);
            g.fillPath (head);
            break;
        }
        case Search:
            g.drawEllipse (c.x - 7.f, c.y - 7.f, 10.f, 10.f, 2.f);
            g.drawLine (c.x + 1.f, c.y + 1.f, c.x + 7.f, c.y + 7.f, 2.4f);
            break;
        default:
            g.setFont (mono (fs));
            g.drawText (getButtonText(), r.toNearestInt(), Justification::centred);
            break;
    }
}

// ======================================================= LCD
void LcdDisplay::paint (Graphics& g)
{
    const Colour amber (0xffffa31a);
    auto r = getLocalBounds().toFloat();

    g.setColour (Colour (0xff030303));
    g.fillRoundedRectangle (r, 6.f);

    auto glass = r.reduced (3.f);
    g.setGradientFill (ColourGradient (Colour (0xff150e05), glass.getX(), glass.getY(),
                                       Colour (0xff0a0602), glass.getX(), glass.getBottom(), false));
    g.fillRoundedRectangle (glass, 4.f);
    g.setColour (Colour (0x14ff9a00));
    g.fillRoundedRectangle (glass, 4.f);

    auto glow = [&] (const String& text, Rectangle<int> area, Justification j, const Font& f, Colour col)
    {
        g.setFont (f);
        g.setColour (col.withAlpha (0.16f));
        for (auto o : { Point<int> (-1, 0), Point<int> (1, 0), Point<int> (0, -1), Point<int> (0, 1) })
            g.drawText (text, area.translated (o.x, o.y), j);
        g.setColour (col);
        g.drawText (text, area, j);
    };

    const auto gi = glass.toNearestInt();

    glow ("ST", gi.withTrimmedLeft (10).withHeight (20), Justification::centredLeft, mono (11.f), amber);
    if (arpOn)
        glow ("ARP " + rate, gi.withTrimmedRight (14).withHeight (20), Justification::centredRight, mono (11.f), amber);

    // главная строка: КАТЕГОРИЯ  NN  ИМЯ
    auto mainArea = gi.reduced (10, 0).withTrimmedTop (20).withTrimmedBottom (22).withTrimmedRight (46);
    const String line = cat + "  " + num + "  " + name;
    g.setFont (mono (20.f));
    g.setColour (amber.withAlpha (0.16f));
    for (auto o : { Point<int> (-1, 0), Point<int> (1, 0) })
        g.drawFittedText (line, mainArea.translated (o.x, o.y), Justification::centredLeft, 1, 0.6f);
    g.setColour (amber);
    g.drawFittedText (line, mainArea, Justification::centredLeft, 1, 0.6f);

    glow ("KEMATIK TAPES", gi.withTrimmedLeft (10).withTop (gi.getBottom() - 20), Justification::centredLeft,
          mono (9.f, false), amber.withAlpha (0.7f));

    // мини-эквалайзер
    const float bx = glass.getRight() - 40.f;
    const float by = glass.getBottom() - 10.f;
    g.setColour (amber);
    for (int i = 0; i < 5; ++i)
    {
        const float hgt = 3.f + bars[i] * 30.f;
        g.fillRect (bx + (float) i * 7.f, by - hgt, 5.f, hgt);
    }

    // блики на стекле
    g.setGradientFill (ColourGradient (Colour (0x16ffffff), glass.getX(), glass.getY(),
                                       Colour (0x00ffffff), glass.getX(), glass.getCentreY(), false));
    g.fillRoundedRectangle (glass.withHeight (glass.getHeight() * 0.5f), 4.f);
    g.setColour (Colour (0xff000000));
    g.drawRoundedRectangle (glass, 4.f, 1.5f);
}

// ======================================================= Editor
TapeRompEditor::TapeRompEditor (TapeRompProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&laf);
    setSize (900, 590);

    presetParam = proc.apvts.getParameter ("preset");

    // первый индекс и количество пресетов в каждой категории
    for (int c = 0; c < kNumCats; ++c) { catFirst[c] = -1; catCount[c] = 0; }
    for (int i = 0; i < kNumPresets; ++i)
    {
        const int c = kPresets[i].cat;
        if (catFirst[c] < 0) catFirst[c] = i;
        ++catCount[c];
    }

    // текстура пластика торпедо
    noise = Image (Image::ARGB, 256, 256, true);
    for (int y = 0; y < 256; ++y)
        for (int x = 0; x < 256; ++x)
        {
            const float v = rng.nextFloat();
            noise.setPixelAt (x, y, Colour::fromFloatRGBA (v > 0.5f ? 1.f : 0.f, v > 0.5f ? 1.f : 0.f, v > 0.5f ? 1.f : 0.f, 0.045f * v));
        }

    for (auto* b : { &offBtn, &seekPrev, &seekNext, &backBtn, &searchBtn, &modeBtn, &zapBtn, &arpBtn, &rateBtn })
        addAndMakeVisible (*b);
    for (auto& b : numBtn) addAndMakeVisible (b);
    addAndMakeVisible (lcd);

    arpBtn.setClickingTogglesState (true);
    arpBtn.amberWhenOn = true;
    arpAtt = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "arp", arpBtn);

    offBtn.onClick     = [this] { proc.panic = true; };
    seekPrev.onClick   = [this] { stepPreset (-1); };
    seekNext.onClick   = [this] { stepPreset (+1); };
    backBtn.onClick    = [this] { stepCategory (-1); };
    modeBtn.onClick    = [this] { stepCategory (+1); };
    searchBtn.onClick  = [this] { showPresetMenu(); };
    lcd.onClick        = [this] { showPresetMenu(); };
    zapBtn.onClick     = [this] { setPreset (Random::getSystemRandom().nextInt (kNumPresets)); };
    rateBtn.onClick    = [this] { cycleRate(); };

    for (int i = 0; i < 6; ++i)
        numBtn[i].onClick = [this, i]
        {
            const int cat = kPresets[jmax (0, lastPreset)].cat;
            if (i < catCount[cat]) setPreset (catFirst[cat] + i);
        };

    setupKnob (volume, "", "gain", true);

    static const char* names[kEditKnobs] = { "ATTACK", "RELEASE", "TONE", "RESO", "DRIVE", "LO-FI",
                                             "CHORUS", "REVERB", "SIZE", "DLY MIX", "DLY TIME", "DLY FDBK" };
    static const char* ids[kEditKnobs]   = { "attack", "release", "cutoff", "reso", "drive", "lofi",
                                             "chorus", "reverb", "rsize", "dlymix", "dlytime", "dlyfb" };
    for (int i = 0; i < kEditKnobs; ++i)
        setupKnob (editKnobs[i], names[i], ids[i], false);

    lastPreset = jlimit (0, kNumPresets - 1, (int) proc.apvts.getRawParameterValue ("preset")->load());
    refreshLcd();
    startTimerHz (30);
}

TapeRompEditor::~TapeRompEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void TapeRompEditor::setupKnob (Knob& k, const char* text, const char* id, bool chrome)
{
    if (chrome) k.s.setName ("chrome");
    k.s.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    k.s.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    k.s.setRotaryParameters (MathConstants<float>::pi * 1.2f, MathConstants<float>::pi * 2.8f, true);
    addAndMakeVisible (k.s);

    k.l.setText (text, dontSendNotification);
    k.l.setJustificationType (Justification::centred);
    k.l.setColour (Label::textColourId, Colour (0xffe2e2da));
    k.l.setFont (mono (11.f));
    addAndMakeVisible (k.l);

    k.att = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, k.s);

    if (String (id) == "dlytime")
        k.s.textFromValueFunction = [] (double v) -> String
        {
            static const char* n[5] = { "1/16", "1/16 dot", "1/8", "1/8 dot", "1/4" };
            return n[jlimit (0, 4, (int) std::round (v))];
        };
    else
        k.s.textFromValueFunction = [] (double v) -> String { return String (roundToInt (v * 100.0)) + "%"; };

    k.s.setPopupDisplayEnabled (true, false, this);
}

// ---------------------------------------------------------------- логика выбора звука
void TapeRompEditor::setPreset (int idx)
{
    idx = jlimit (0, kNumPresets - 1, idx);
    presetParam->beginChangeGesture();
    presetParam->setValueNotifyingHost (presetParam->convertTo0to1 ((float) idx));
    presetParam->endChangeGesture();

    proc.applyPresetDefaults (idx);   // сброс EDIT на стандартные значения пресета
    lastPreset = idx;
    refreshLcd();
    repaint (panelRect);
}

void TapeRompEditor::stepPreset (int d)
{
    setPreset ((jmax (0, lastPreset) + d + kNumPresets) % kNumPresets);
}

void TapeRompEditor::stepCategory (int d)
{
    const int cat = (kPresets[jmax (0, lastPreset)].cat + d + kNumCats) % kNumCats;
    setPreset (catFirst[cat]);
}

void TapeRompEditor::cycleRate()
{
    if (auto* p = proc.apvts.getParameter ("arprate"))
    {
        const int cur = (int) proc.apvts.getRawParameterValue ("arprate")->load();
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 ((float) ((cur + 1) % 4)));
        p->endChangeGesture();
    }
}

void TapeRompEditor::showPresetMenu()
{
    PopupMenu menu;
    for (int c = 0; c < kNumCats; ++c)
    {
        PopupMenu sub;
        for (int i = 0; i < kNumPresets; ++i)
            if (kPresets[i].cat == c)
                sub.addItem (i + 1, kPresets[i].name, true, i == lastPreset);
        menu.addSubMenu (kCategories[c], sub, true, nullptr, c == kPresets[jmax (0, lastPreset)].cat);
    }

    Component::SafePointer<TapeRompEditor> safe (this);
    menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&lcd),
                        [safe] (int result)
                        {
                            if (safe != nullptr && result > 0) safe->setPreset (result - 1);
                        });
}

void TapeRompEditor::refreshLcd()
{
    const auto& pr = kPresets[jmax (0, lastPreset)];
    lcd.cat  = kCategories[pr.cat];
    lcd.num  = String (jmax (0, lastPreset) - catFirst[pr.cat] + 1).paddedLeft ('0', 2);
    lcd.name = String (pr.name).toUpperCase();

    lastArp  = proc.apvts.getRawParameterValue ("arp")->load() > 0.5f ? 1 : 0;
    lastRate = jlimit (0, 3, (int) proc.apvts.getRawParameterValue ("arprate")->load());
    lcd.arpOn = lastArp == 1;
    lcd.rate = kRateNames[lastRate];
    lcd.repaint();
}

void TapeRompEditor::timerCallback()
{
    // синхронизация с автоматизацией хоста
    const int idx = jlimit (0, kNumPresets - 1, (int) std::round (presetParam->convertFrom0to1 (presetParam->getValue())));
    const int arp = proc.apvts.getRawParameterValue ("arp")->load() > 0.5f ? 1 : 0;
    const int rate = jlimit (0, 3, (int) proc.apvts.getRawParameterValue ("arprate")->load());

    if (idx != lastPreset || arp != lastArp || rate != lastRate)
    {
        const bool presetChanged = idx != lastPreset;
        lastPreset = idx;
        refreshLcd();
        if (presetChanged) repaint (panelRect);
    }

    // анимация мини-эквалайзера
    const bool playing = proc.activeVoices.load() > 0;
    bool any = false;
    for (auto& b : lcd.bars)
    {
        const float target = playing ? rng.nextFloat() * 0.9f : 0.f;
        b += (target - b) * 0.35f;
        if (b < 0.01f && ! playing) b = 0.f;
        any = any || b > 0.f;
    }
    if (playing || any) lcd.repaint();
}

// ---------------------------------------------------------------- раскладка
void TapeRompEditor::resized()
{
    faceRect  = Rectangle<int> (40, 36, 820, 232);
    panelRect = Rectangle<int> (40, 296, 820, 262);

    auto at = [this] (int x, int y, int w, int h)
    {
        return Rectangle<int> (faceRect.getX() + x, faceRect.getY() + y, w, h);
    };

    offBtn.setBounds   (at (18, 14, 54, 22));
    seekPrev.setBounds (at (18, 52, 104, 26));
    seekNext.setBounds (at (18, 82, 104, 26));
    backBtn.setBounds  (at (18, 128, 50, 32));
    searchBtn.setBounds (at (72, 128, 50, 32));
    modeBtn.setBounds  (at (18, 170, 104, 30));

    volume.s.setBounds (at (143, 66, 124, 124));

    lcd.setBounds (at (292, 58, 420, 88));

    // ряд кнопок под дисплеем: ZAP ARP 1-6 RATE
    DeckButton* row[9] = { &zapBtn, &arpBtn, &numBtn[0], &numBtn[1], &numBtn[2], &numBtn[3], &numBtn[4], &numBtn[5], &rateBtn };
    for (int i = 0; i < 9; ++i)
        row[i]->setBounds (at (292 + i * 47, 162, 44, 28));

    // сетка ручек EDIT: 6 x 2
    auto g = panelRect.reduced (14, 0);
    g.removeFromTop (40);
    g.removeFromBottom (8);
    const int cw = g.getWidth() / 6;
    const int ch = g.getHeight() / 2;
    for (int i = 0; i < kEditKnobs; ++i)
    {
        Rectangle<int> cell (g.getX() + (i % 6) * cw, g.getY() + (i / 6) * ch, cw, ch);
        editKnobs[i].l.setBounds (cell.removeFromBottom (16));
        editKnobs[i].s.setBounds (cell.withSizeKeepingCentre (66, 66));
    }
}

// ---------------------------------------------------------------- рисование
void TapeRompEditor::drawDash (Graphics& g)
{
    g.setGradientFill (ColourGradient (Colour (0xff666a62), 0.f, 0.f, Colour (0xff3a3d38), 0.f, (float) getHeight(), false));
    g.fillAll();

    g.setTiledImageFill (noise, 0, 0, 1.0f);
    g.fillAll();

    // углубление под магнитолу
    auto rec = faceRect.expanded (12).toFloat();
    g.setColour (Colour (0xff0d0d0d));
    g.fillRoundedRectangle (rec, 10.f);
    g.setColour (Colour (0x55000000));
    g.drawRoundedRectangle (rec.expanded (2.f), 12.f, 3.f);
    g.setColour (Colour (0x22ffffff));
    g.drawLine (rec.getX() + 8.f, rec.getBottom() + 3.f, rec.getRight() - 8.f, rec.getBottom() + 3.f, 1.f);
}

void TapeRompEditor::drawFaceplate (Graphics& g)
{
    const auto F = faceRect.toFloat();

    // корпус
    ColourGradient body (Colour (0xff2c2c2f), F.getX(), F.getY(), Colour (0xff09090a), F.getX(), F.getBottom(), false);
    body.addColour (0.10, Colour (0xff202023));
    g.setGradientFill (body);
    g.fillRoundedRectangle (F, 10.f);
    g.setColour (Colour (0xff3b3b3f));
    g.drawRoundedRectangle (F.reduced (0.5f), 10.f, 1.f);
    g.setColour (Colour (0x20ffffff));
    g.drawLine (F.getX() + 10.f, F.getY() + 2.f, F.getRight() - 10.f, F.getY() + 2.f, 1.f);

    // глянцевый блик по диагонали
    {
        Graphics::ScopedSaveState ss (g);
        Path clip;
        clip.addRoundedRectangle (F, 10.f);
        g.reduceClipRegion (clip);
        Path glare;
        glare.startNewSubPath (F.getX() + 80.f, F.getY());
        glare.lineTo (F.getX() + 300.f, F.getY());
        glare.lineTo (F.getX() + 150.f, F.getBottom());
        glare.lineTo (F.getX() - 70.f, F.getBottom());
        glare.closeSubPath();
        g.setColour (Colour (0x0bffffff));
        g.fillPath (glare);
    }

    auto at = [&F] (float x, float y, float w, float h) { return Rectangle<float> (F.getX() + x, F.getY() + y, w, h); };

    // щель CD
    auto slot = at (100.f, 20.f, 530.f, 18.f);
    g.setColour (Colour (0xff3c3c40));
    g.fillRoundedRectangle (slot.expanded (2.f), 4.f);
    g.setColour (Colour (0xff020202));
    g.fillRoundedRectangle (slot, 3.f);
    g.setColour (Colour (0x30ffffff));
    g.drawLine (slot.getX() + 3.f, slot.getBottom() - 1.f, slot.getRight() - 3.f, slot.getBottom() - 1.f, 1.f);

    // кнопка eject (декоративная)
    auto ej = at (640.f, 16.f, 56.f, 24.f);
    g.setGradientFill (ColourGradient (Colour (0xff3d3d41), ej.getX(), ej.getY(), Colour (0xff18181a), ej.getX(), ej.getBottom(), false));
    g.fillRoundedRectangle (ej, 4.f);
    g.setColour (Colour (0xff050506));
    g.drawRoundedRectangle (ej, 4.f, 1.f);
    g.setColour (Colour (0xffc4c4c8));
    Path up;
    up.addTriangle (ej.getCentreX() - 6.f, ej.getCentreY() + 1.f, ej.getCentreX() + 6.f, ej.getCentreY() + 1.f,
                    ej.getCentreX(), ej.getCentreY() - 6.f);
    g.fillPath (up);
    g.fillRect (ej.getCentreX() - 6.f, ej.getCentreY() + 4.f, 12.f, 2.f);

    // логотип
    g.setColour (Colour (0xffd8d8dc));
    g.setFont (Font (FontOptions (17.f, Font::bold).withKerningFactor (0.12f)));
    g.drawText ("KEMATIK", at (700.f, 14.f, 108.f, 26.f).toNearestInt(), Justification::centredRight);

    // мелкая печать
    g.setColour (Colour (0xff8e8e94));
    g.setFont (mono (8.f, false));
    g.drawText ("SYNTH   RDS/VKB", at (292.f, 44.f, 200.f, 12.f).toNearestInt(), Justification::centredLeft);
    g.drawText ("PUSH ENTER/SELECT", at (130.f, 204.f, 170.f, 12.f).toNearestInt(), Justification::centredLeft);
    g.drawText ("USB", at (738.f, 44.f, 44.f, 12.f).toNearestInt(), Justification::centred);
    g.drawText ("AUX", at (782.f, 108.f, 36.f, 12.f).toNearestInt(), Justification::centred);

    // USB-порт
    auto usb = at (738.f, 60.f, 44.f, 26.f);
    g.setGradientFill (ColourGradient (Colour (0xffdcdce0), usb.getX(), usb.getY(), Colour (0xff6c6c72), usb.getX(), usb.getBottom(), false));
    g.fillRoundedRectangle (usb, 3.f);
    g.setColour (Colour (0xff050506));
    g.fillRect (usb.reduced (5.f, 8.f));
    g.setColour (Colour (0xff2d4a8c));
    g.fillRect (usb.reduced (7.f, 11.f).translated (0.f, 2.f));

    // AUX-гнездо
    const auto jack = at (800.f, 128.f, 0.f, 0.f).getPosition();
    g.setColour (Colour (0xff6c6c72));
    g.fillEllipse (jack.x - 9.f, jack.y - 9.f, 18.f, 18.f);
    g.setColour (Colour (0xff050506));
    g.fillEllipse (jack.x - 5.f, jack.y - 5.f, 10.f, 10.f);
}

void TapeRompEditor::drawCable (Graphics& g)
{
    const auto F = faceRect.toFloat();
    const float jx = F.getX() + 800.f;
    const float jy = F.getY() + 128.f;

    // штекер
    auto plug = Rectangle<float> (jx, jy - 5.f, 36.f, 10.f);
    g.setGradientFill (ColourGradient (Colour (0xff6a6a70), plug.getX(), plug.getY(), Colour (0xff1b1b1d), plug.getX(), plug.getBottom(), false));
    g.fillRoundedRectangle (plug, 3.f);
    g.setColour (Colour (0xff111113));
    g.fillRoundedRectangle (Rectangle<float> (jx + 22.f, jy - 7.f, 18.f, 14.f), 4.f);

    // плетёный провод
    Path p;
    const float sx = jx + 38.f;
    p.startNewSubPath (sx, jy);
    p.cubicTo (sx + 22.f, jy + 2.f, sx + 24.f, jy + 70.f, sx + 10.f, jy + 150.f);
    p.lineTo (sx + 4.f, (float) getHeight());

    g.setColour (Colour (0xff0a0a0a));
    g.strokePath (p, PathStrokeType (9.f, PathStrokeType::curved, PathStrokeType::rounded));

    Path dashed;
    const float dash[2] = { 3.f, 4.f };
    PathStrokeType (8.f).createDashedStroke (dashed, p, dash, 2);
    g.setColour (Colour (0xffd9d9d4));
    g.fillPath (dashed);
}

void TapeRompEditor::paint (Graphics& g)
{
    drawDash (g);

    // панель EDIT (как блок управления на торпедо)
    {
        const auto P = panelRect.toFloat();
        g.setColour (Colour (0x66000000));
        g.fillRoundedRectangle (P.translated (0.f, 3.f), 12.f);
        g.setGradientFill (ColourGradient (Colour (0xff2e302c), P.getX(), P.getY(), Colour (0xff1c1d1a), P.getX(), P.getBottom(), false));
        g.fillRoundedRectangle (P, 12.f);
        g.setColour (Colour (0xff0b0b0a));
        g.drawRoundedRectangle (P.reduced (1.f), 12.f, 2.f);

        g.setColour (Colour (0xffe2e2da));
        g.setFont (mono (14.f));
        const auto& pr = kPresets[jmax (0, lastPreset)];
        g.drawText ("EDIT  -  " + String (pr.name).toUpperCase(), panelRect.getX() + 20, panelRect.getY() + 8, 420, 24,
                    Justification::centredLeft);

        g.setColour (Colour (0xff8a8a82));
        g.setFont (mono (10.f, false));
        g.drawText ("RESETS WHEN PRESET CHANGES", panelRect.getRight() - 300, panelRect.getY() + 10, 280, 20,
                    Justification::centredRight);
        g.setColour (Colour (0x30ffffff));
        g.drawLine (P.getX() + 16.f, P.getY() + 36.f, P.getRight() - 16.f, P.getY() + 36.f, 1.f);
    }

    drawFaceplate (g);
    drawCable (g);
}
