#include "PluginEditor.h"

using namespace juce;
using namespace tr;

static Font mono (float h, bool bold = true)
{
    return Font (FontOptions (Font::getDefaultMonospacedFontName(), h, bold ? Font::bold : Font::plain));
}

static const Colour kCatColours[7] = {
    Colour (0xff5fa8d3), Colour (0xffe07a3f), Colour (0xffd9a441), Colour (0xffc0392b),
    Colour (0xff7d5ba6), Colour (0xff3fa37a), Colour (0xffd6457a)
};

// ======================================================= LookAndFeel
RetroLookAndFeel::RetroLookAndFeel()
{
    setColour (ComboBox::backgroundColourId, Colour (0xff2a241d));
    setColour (ComboBox::textColourId, Colour (0xffe8dcc0));
    setColour (ComboBox::outlineColourId, Colour (0xff0d0b09));
    setColour (ComboBox::arrowColourId, Colour (0xffe8a33d));
    setColour (PopupMenu::backgroundColourId, Colour (0xff2a241d));
    setColour (PopupMenu::textColourId, Colour (0xffe8dcc0));
    setColour (ListBox::backgroundColourId, Colour (0xff17130f));
    setColour (ListBox::outlineColourId, Colour (0xff0d0b09));
    setColour (BubbleComponent::backgroundColourId, Colour (0xff2a241d));
    setColour (BubbleComponent::outlineColourId, Colour (0xffe8a33d));
    setColour (TooltipWindow::textColourId, Colour (0xffe8dcc0));
}

void RetroLookAndFeel::drawButtonBackground (Graphics& g, Button& b, const Colour&, bool, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.5f);
    const bool on = b.getToggleState() || down;
    auto face = on ? r.translated (0.f, 2.f) : r;

    g.setColour (Colour (0xff0d0b09));
    g.fillRoundedRectangle (r.translated (0.f, 3.f), 5.f);

    ColourGradient grad (on ? Colour (0xff4a4036) : Colour (0xff6b5f50), r.getX(), r.getY(),
                         on ? Colour (0xff2e2820) : Colour (0xff3d352c), r.getX(), r.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (face, 5.f);
    g.setColour (Colour (0xff15120f));
    g.drawRoundedRectangle (face, 5.f, 1.2f);

    if (on)
    {
        g.setColour (Colour (0x55ff9a2e));
        g.fillEllipse (face.getX() + 5.f, face.getCentreY() - 6.f, 12.f, 12.f);
    }
    g.setColour (on ? Colour (0xffff9a2e) : Colour (0xff3a2a16));
    g.fillEllipse (face.getX() + 8.f, face.getCentreY() - 3.f, 6.f, 6.f);
}

void RetroLookAndFeel::drawButtonText (Graphics& g, TextButton& b, bool, bool down)
{
    const bool on = b.getToggleState() || down;
    g.setColour (on ? Colour (0xffffe9b8) : Colour (0xffa89c84));
    g.setFont (mono (12.f));
    auto area = b.getLocalBounds().withTrimmedLeft (16);
    if (on) area = area.translated (0, 2);
    g.drawText (b.getButtonText(), area, Justification::centred);
}

void RetroLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos,
                                         float startAngle, float endAngle, Slider&)
{
    auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.f);
    const auto c = b.getCentre();
    const float r = jmin (b.getWidth(), b.getHeight()) * 0.5f;

    g.setColour (Colour (0xff0d0b09));
    g.fillEllipse (b.translated (0.f, 2.f));
    g.setColour (Colour (0xffb8a678));
    g.fillEllipse (b);
    g.setColour (Colour (0xff26211c));
    g.fillEllipse (b.reduced (5.f));

    const float ang = startAngle + pos * (endAngle - startAngle);
    Path p;
    p.addRoundedRectangle (-1.5f, -r + 7.f, 3.f, r * 0.42f, 1.5f);
    g.setColour (Colour (0xffe8dcc0));
    g.fillPath (p, AffineTransform::rotation (ang).translated (c.x, c.y));
}

// ======================================================= Editor
TapeRompEditor::TapeRompEditor (TapeRompProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&laf);
    setSize (760, 440);

    presetParam = proc.apvts.getParameter ("preset");

    for (int i = 0; i < kTabs; ++i)
    {
        tabs[i].setButtonText (i < kNumCats ? kCategories[i] : "EDIT");
        tabs[i].setClickingTogglesState (true);
        tabs[i].setRadioGroupId (1001);
        tabs[i].onClick = [this, i]
        {
            if (i < kNumCats) showCategory (i);
            else              showEdit();
        };
        addAndMakeVisible (tabs[i]);
    }

    list.setRowHeight (32);
    list.setOutlineThickness (2);
    addAndMakeVisible (list);

    static const char* editNames[kEditKnobs] = { "ATTACK", "RELEASE", "TONE", "RESO", "DRIVE", "LO-FI",
                                                 "CHORUS", "REVERB", "SIZE", "DLY MIX", "DLY TIME", "DLY FDBK" };
    static const char* editIds[kEditKnobs]   = { "attack", "release", "cutoff", "reso", "drive", "lofi",
                                                 "chorus", "reverb", "rsize", "dlymix", "dlytime", "dlyfb" };
    for (int i = 0; i < kEditKnobs; ++i)
        setupKnob (editKnobs[i], editNames[i], editIds[i]);

    setupKnob (volumeKnob, "VOLUME", "gain");

    arpBtn.setClickingTogglesState (true);
    addAndMakeVisible (arpBtn);
    arpAtt = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "arp", arpBtn);

    rateBox.addItemList ({ "1/4", "1/8", "1/16", "1/32" }, 1);
    addAndMakeVisible (rateBox);
    rateAtt = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "arprate", rateBox);

    const int idx = jlimit (0, kNumPresets - 1, (int) proc.apvts.getRawParameterValue ("preset")->load());
    tabs[kPresets[idx].cat].setToggleState (true, dontSendNotification);
    syncUI (idx);
    updateVisibility();

    startTimerHz (30);
}

TapeRompEditor::~TapeRompEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void TapeRompEditor::setupKnob (Knob& k, const char* text, const char* id)
{
    k.s.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    k.s.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    k.s.setRotaryParameters (MathConstants<float>::pi * 1.2f, MathConstants<float>::pi * 2.8f, true);
    addAndMakeVisible (k.s);

    k.l.setText (text, dontSendNotification);
    k.l.setJustificationType (Justification::centred);
    k.l.setColour (Label::textColourId, Colour (0xffd9cba3));
    k.l.setFont (mono (12.f));
    addAndMakeVisible (k.l);

    k.att = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, k.s);

    // подсказка со значением при вращении
    if (String (id) == "dlytime")
        k.s.textFromValueFunction = [] (double v) -> String
        {
            static const char* names[5] = { "1/16", "1/16 dot", "1/8", "1/8 dot", "1/4" };
            return names[jlimit (0, 4, (int) std::round (v))];
        };
    else
        k.s.textFromValueFunction = [] (double v) -> String { return String (roundToInt (v * 100.0)) + "%"; };

    k.s.setPopupDisplayEnabled (true, false, this);
}

void TapeRompEditor::rebuildList (int cat)
{
    currentCat = cat;
    catPresets.clear();
    for (int i = 0; i < kNumPresets; ++i)
        if (kPresets[i].cat == cat) catPresets.push_back (i);
    list.updateContent();

    syncing = true;
    int row = -1;
    for (size_t r = 0; r < catPresets.size(); ++r)
        if (catPresets[r] == lastPreset) { row = (int) r; break; }
    if (row >= 0) list.selectRow (row);
    else          list.deselectAllRows();
    syncing = false;
    list.repaint();
}

void TapeRompEditor::showCategory (int cat)
{
    editMode = false;
    rebuildList (cat);
    updateVisibility();
    repaint();
}

void TapeRompEditor::showEdit()
{
    editMode = true;
    updateVisibility();
    repaint();
}

void TapeRompEditor::updateVisibility()
{
    list.setVisible (! editMode);
    for (auto& k : editKnobs)
    {
        k.s.setVisible (editMode);
        k.l.setVisible (editMode);
    }
}

void TapeRompEditor::setPreset (int idx)
{
    presetParam->beginChangeGesture();
    presetParam->setValueNotifyingHost (presetParam->convertTo0to1 ((float) idx));
    presetParam->endChangeGesture();

    proc.applyPresetDefaults (idx);   // сброс вкладки EDIT на стандартные значения пресета
    syncUI (idx);
}

void TapeRompEditor::syncUI (int idx)
{
    lastPreset = idx;
    const int cat = kPresets[idx].cat;

    // если пользователь сейчас смотрит другую категорию, список не переключаем,
    // кроме случая, когда пресет изменил хост
    if (currentCat < 0 || cat == currentCat)
        rebuildList (cat);
    else if (! editMode)
    {
        tabs[cat].setToggleState (true, dontSendNotification);
        rebuildList (cat);
    }
    else
        currentCat = cat;

    repaint();
}

void TapeRompEditor::selectedRowsChanged (int row)
{
    if (syncing || row < 0 || row >= (int) catPresets.size()) return;
    setPreset (catPresets[(size_t) row]);
}

void TapeRompEditor::paintListBoxItem (int row, Graphics& g, int w, int h, bool selected)
{
    if (row < 0 || row >= (int) catPresets.size()) return;

    if (selected)
    {
        g.setColour (Colour (0xffe8a33d));
        g.fillRect (0, 0, w, h);
    }
    else if (row % 2)
    {
        g.setColour (Colour (0x0fffffff));
        g.fillRect (0, 0, w, h);
    }
    g.setColour (selected ? Colour (0xff1b140c) : Colour (0xffd9cba3));
    g.setFont (mono (15.f, false));
    g.drawText (String (row + 1).paddedLeft ('0', 2) + "  " + kPresets[catPresets[(size_t) row]].name,
                12, 0, w - 16, h, Justification::centredLeft);
}

void TapeRompEditor::timerCallback()
{
    // синхронизация с автоматизацией хоста
    const int idx = jlimit (0, kNumPresets - 1, (int) std::round (presetParam->convertFrom0to1 (presetParam->getValue())));
    if (idx != lastPreset)
    {
        lastPreset = idx;
        if (! editMode) tabs[kPresets[idx].cat].setToggleState (true, dontSendNotification);
        rebuildList (kPresets[idx].cat);
        repaint();
    }

    // катушки крутятся, пока звучат ноты
    const bool playing = proc.activeVoices.load() > 0;
    if (playing) reelSpeed = 0.14f;
    else         reelSpeed *= 0.90f;

    if (reelSpeed > 0.003f)
    {
        reel += reelSpeed;
        if (! editMode) repaint (cassetteArea.toNearestInt());
    }
    else if (reelSpeed != 0.f)
    {
        reelSpeed = 0.f;
        if (! editMode) repaint (cassetteArea.toNearestInt());
    }
}

void TapeRompEditor::resized()
{
    auto b = getLocalBounds().reduced (20);
    b.removeFromTop (34);

    auto tabRow = b.removeFromTop (40);
    const int tw = tabRow.getWidth() / kTabs;
    for (int i = 0; i < kTabs; ++i)
        tabs[i].setBounds (tabRow.removeFromLeft (tw).reduced (3, 0));

    b.removeFromTop (12);
    midArea = b.removeFromTop (236);

    auto mid = midArea;
    cassetteArea = mid.removeFromLeft (450).toFloat();
    mid.removeFromLeft (20);
    list.setBounds (mid);

    // сетка ручек вкладки EDIT: 6 x 2
    auto g = midArea.reduced (10, 6);
    g.removeFromTop (26);
    const int cw = g.getWidth() / 6;
    const int ch = g.getHeight() / 2;
    for (int i = 0; i < kEditKnobs; ++i)
    {
        Rectangle<int> cell (g.getX() + (i % 6) * cw, g.getY() + (i / 6) * ch, cw, ch);
        editKnobs[i].l.setBounds (cell.removeFromBottom (18));
        editKnobs[i].s.setBounds (cell.withSizeKeepingCentre (58, 58));
    }

    b.removeFromTop (10);
    auto bottom = b;

    auto vcell = bottom.removeFromLeft (110);
    volumeKnob.l.setBounds (vcell.removeFromBottom (18));
    volumeKnob.s.setBounds (vcell.withSizeKeepingCentre (50, 50));

    bottom.removeFromLeft (30);
    arpBtn.setBounds (bottom.removeFromLeft (100).withSizeKeepingCentre (100, 38));
    bottom.removeFromLeft (20);
    rateBox.setBounds (bottom.removeFromLeft (90).withSizeKeepingCentre (90, 30));
}

void TapeRompEditor::drawReel (Graphics& g, Point<float> c, float tapeR, float hubR, float angle)
{
    g.setColour (Colour (0xff3a2a1c));
    g.fillEllipse (c.x - tapeR, c.y - tapeR, tapeR * 2.f, tapeR * 2.f);
    g.setColour (Colour (0xffe6dfd0));
    g.fillEllipse (c.x - hubR, c.y - hubR, hubR * 2.f, hubR * 2.f);
    g.setColour (Colour (0xff121110));
    g.fillEllipse (c.x - hubR * 0.62f, c.y - hubR * 0.62f, hubR * 1.24f, hubR * 1.24f);

    g.setColour (Colour (0xffe6dfd0));
    for (int i = 0; i < 6; ++i)
    {
        const float a = angle + (float) i * MathConstants<float>::twoPi / 6.f;
        g.drawLine (c.x + std::cos (a) * hubR * 0.25f, c.y + std::sin (a) * hubR * 0.25f,
                    c.x + std::cos (a) * hubR * 0.85f, c.y + std::sin (a) * hubR * 0.85f, 2.f);
    }
}

void TapeRompEditor::drawCassette (Graphics& g)
{
    const auto c = cassetteArea;
    const int idx = jmax (0, lastPreset);
    const auto& pr = kPresets[idx];

    g.setColour (Colour (0xff0e0c0a));
    g.fillRoundedRectangle (c.translated (0.f, 4.f), 14.f);
    g.setColour (Colour (0xff2a2723));
    g.fillRoundedRectangle (c, 14.f);
    g.setColour (Colour (0xff4a443c));
    g.drawRoundedRectangle (c.reduced (1.f), 14.f, 2.f);

    Rectangle<float> label (c.getX() + 24.f, c.getY() + 16.f, c.getWidth() - 48.f, 100.f);
    g.setColour (Colour (0xffe6dbbd));
    g.fillRoundedRectangle (label, 6.f);

    auto stripe = label.withHeight (24.f);
    g.setColour (kCatColours[pr.cat]);
    g.fillRoundedRectangle (stripe, 6.f);
    g.fillRect (stripe.withTop (stripe.getCentreY()));

    g.setColour (Colour (0xff1d1710));
    g.setFont (mono (13.f));
    g.drawText ("SIDE A  -  " + String (kCategories[pr.cat]), stripe.reduced (10.f, 0.f), Justification::centredLeft);
    g.drawText ("No." + String (idx + 1).paddedLeft ('0', 2), stripe.reduced (10.f, 0.f), Justification::centredRight);

    g.setColour (Colour (0x55000000));
    g.drawLine (label.getX() + 12.f, label.getY() + 84.f, label.getRight() - 12.f, label.getY() + 84.f, 1.f);

    g.setColour (Colour (0xff2b2118));
    g.setFont (mono (24.f));
    g.drawText (pr.name, label.withTrimmedTop (28.f).withTrimmedBottom (18.f).reduced (14.f, 0.f), Justification::centredLeft);

    Rectangle<float> win (c.getX() + 60.f, c.getY() + 128.f, c.getWidth() - 120.f, 64.f);
    g.setColour (Colour (0xff121110));
    g.fillRoundedRectangle (win, 32.f);
    g.setColour (Colour (0xff3a342c));
    g.drawRoundedRectangle (win, 32.f, 2.f);

    drawReel (g, { win.getX() + 36.f, win.getCentreY() }, 28.f, 14.f, reel);
    drawReel (g, { win.getRight() - 36.f, win.getCentreY() }, 22.f, 14.f, reel * 1.3f);

    Path tr;
    tr.startNewSubPath (c.getX() + 80.f, c.getBottom());
    tr.lineTo (c.getX() + 105.f, c.getBottom() - 30.f);
    tr.lineTo (c.getRight() - 105.f, c.getBottom() - 30.f);
    tr.lineTo (c.getRight() - 80.f, c.getBottom());
    tr.closeSubPath();
    g.setColour (Colour (0xff1b1916));
    g.fillPath (tr);
    g.setColour (Colour (0xff0a0908));
    for (int i = 0; i < 2; ++i)
        g.fillEllipse (c.getCentreX() - 40.f + (float) i * 70.f, c.getBottom() - 20.f, 10.f, 10.f);

    g.setColour (Colour (0xff6b645a));
    for (auto p : { Point<float> (c.getX() + 12.f, c.getY() + 12.f), Point<float> (c.getRight() - 12.f, c.getY() + 12.f),
                    Point<float> (c.getX() + 12.f, c.getBottom() - 12.f), Point<float> (c.getRight() - 12.f, c.getBottom() - 12.f) })
        g.fillEllipse (p.x - 4.f, p.y - 4.f, 8.f, 8.f);
}

void TapeRompEditor::paint (Graphics& g)
{
    ColourGradient bg (Colour (0xff3b3329), 0.f, 0.f, Colour (0xff1b1713), 0.f, (float) getHeight(), false);
    g.setGradientFill (bg);
    g.fillAll();

    g.setColour (Colour (0x07ffffff));
    for (int y = 0; y < getHeight(); y += 3)
        g.drawHorizontalLine (y, 0.f, (float) getWidth());

    g.setColour (Colour (0xffe8dcc0));
    g.setFont (mono (20.f));
    g.drawText ("KEMATIK TAPES", 24, 10, 300, 26, Justification::centredLeft);

    g.setColour (Colour (0xff9c917a));
    g.setFont (mono (11.f, false));
    g.drawText ("PLUGG / DARK / TRAP / FUTURISTIC / GLO", getWidth() - 400, 14, 376, 18, Justification::centredRight);

    if (editMode)
    {
        const auto r = midArea.toFloat();
        g.setColour (Colour (0xff17130f));
        g.fillRoundedRectangle (r, 10.f);
        g.setColour (Colour (0xff0d0b09));
        g.drawRoundedRectangle (r.reduced (1.f), 10.f, 2.f);

        const int idx = jmax (0, lastPreset);
        g.setColour (kCatColours[kPresets[idx].cat]);
        g.fillRoundedRectangle (r.getX() + 14.f, r.getY() + 12.f, 8.f, 14.f, 2.f);

        g.setColour (Colour (0xffe8dcc0));
        g.setFont (mono (15.f));
        g.drawText ("EDIT  -  " + String (kPresets[idx].name), (int) r.getX() + 30, (int) r.getY() + 8, 330, 22, Justification::centredLeft);

        g.setColour (Colour (0xff8a806c));
        g.setFont (mono (11.f, false));
        g.drawText ("RESETS WHEN PRESET CHANGES", (int) r.getRight() - 290, (int) r.getY() + 10, 276, 18, Justification::centredRight);
    }
    else
    {
        drawCassette (g);
    }
}
