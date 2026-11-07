#pragma once
#include <iterator>

// Все звуки синтезируются в реальном времени (без сэмплов) -> маленький размер и низкая нагрузка на CPU.
namespace tr
{
enum Wave { Sine = 0, Saw, Square, Tri };

struct Preset
{
    const char* name;
    int cat;                  // индекс категории (вкладки)
    int w1, w2;               // формы волн осцилляторов
    float semi2, detune;      // osc2: сдвиг в полутонах, расстройка в центах
    float fmRatio, fmAmt;     // FM (колокольчики / e-piano): отношение частот, глубина
    float a, d, s, r;         // огибающая (сек / уровень)
    float cutoff, res, fenv;  // фильтр: Гц, резонанс 0..1, глубина огибающей в октавах
    bool arp;                 // включать арпеджиатор при выборе пресета
    int arpPattern;           // 0 вверх, 1 вниз, 2 вверх-вниз, 3 вверх + октава
    float gain;
};

inline const char* const kCategories[7] = { "BELLS", "ARPS", "E-PIANO", "BRASS", "STRINGS", "LEADS", "FAST" };

inline const Preset kPresets[] =
{
    // ---------------- BELLS ----------------
    { "Ice Bell",       0, Sine, Sine,    12,  4, 3.5f, 2.2f, .001f, 1.6f, 0.f, 1.4f, 14000, .10f, 0.f, false, 0, .80f },
    { "Dark Chime",     0, Sine, Tri,      7,  6, 1.41f,3.0f, .001f, 2.2f, 0.f, 1.8f,  5000, .15f, 0.f, false, 0, .80f },
    { "Music Box",      0, Sine, Sine,    12,  2, 5.0f, 1.2f, .001f, 0.9f, 0.f, 0.8f, 12000, .10f, 0.f, false, 0, .80f },
    { "Glo Glock",      0, Sine, Sine,    24,  0, 7.0f, 1.6f, .001f, 0.7f, 0.f, 0.9f, 16000, .10f, 0.f, false, 0, .75f },
    { "Haunted Bell",   0, Sine, Sine,     0,  8, 2.76f,2.8f, .002f, 3.0f, 0.f, 3.0f,  4000, .20f, 0.f, false, 0, .80f },

    // ---------------- ARPS (арпеджиатор включается сам) ----------------
    { "Pluck Arp",      1, Saw,  Square,   0,  8, 0.f,  0.f,  .001f, .18f, 0.f, .15f,  1800, .25f, 3.0f, true, 0, .70f },
    { "Dark Arp",       1, Saw,  Saw,    -12, 12, 0.f,  0.f,  .001f, .22f, 0.f, .20f,   900, .30f, 2.5f, true, 2, .75f },
    { "Future Arp",     1, Square, Saw,   12,  6, 0.f,  0.f,  .001f, .15f, 0.f, .12f,  2500, .20f, 2.0f, true, 3, .70f },
    { "Glass Arp",      1, Tri,  Sine,    12,  3, 2.0f, .8f,  .001f, .30f, 0.f, .40f,  8000, .10f, 1.0f, true, 0, .75f },
    { "Trap Arp",       1, Saw,  Square,   0, 10, 0.f,  0.f,  .001f, .12f, 0.f, .10f,  1400, .35f, 3.5f, true, 1, .70f },

    // ---------------- E-PIANO ----------------
    { "Rhodes",         2, Sine, Sine,    12,  2, 1.0f, 1.4f, .002f, 1.2f, .25f, .50f,  9000, .05f, 0.f, false, 0, .80f },
    { "Wurly",          2, Tri,  Sine,     0,  4, 2.0f, .9f,  .002f, 0.9f, .30f, .40f,  6000, .10f, 0.f, false, 0, .80f },
    { "Dark Keys",      2, Sine, Tri,    -12,  5, 1.0f, 1.0f, .003f, 1.5f, .30f, .70f,  2500, .10f, .5f, false, 0, .85f },
    { "Soft EP",        2, Sine, Sine,    12,  3, 1.0f, .6f,  .005f, 1.0f, .40f, .60f,  5000, .05f, 0.f, false, 0, .80f },
    { "Plugg Keys",     2, Sine, Sine,    12,  6, 3.0f, 1.6f, .001f, 0.5f, .10f, .35f,  7000, .10f, .5f, false, 0, .80f },

    // ---------------- BRASS ----------------
    { "Trap Brass",     3, Saw,  Saw,      0, 10, 0.f,  0.f,  .040f, .30f, .70f, .25f,   900, .15f, 2.8f, false, 0, .70f },
    { "Dark Horns",     3, Saw,  Saw,    -12, 12, 0.f,  0.f,  .060f, .40f, .70f, .30f,   700, .20f, 2.5f, false, 0, .70f },
    { "Royal Brass",    3, Saw,  Square,   0,  8, 0.f,  0.f,  .050f, .35f, .75f, .30f,  1200, .10f, 2.5f, false, 0, .70f },
    { "Stab Brass",     3, Saw,  Saw,      0, 14, 0.f,  0.f,  .010f, .20f, .30f, .15f,  1000, .20f, 3.0f, false, 0, .70f },
    { "Soft Brass",     3, Saw,  Tri,      0,  6, 0.f,  0.f,  .090f, .40f, .80f, .40f,   800, .10f, 2.0f, false, 0, .70f },

    // ---------------- STRINGS ----------------
    { "Dark Strings",   4, Saw,  Saw,      0, 14, 0.f,  0.f,  .600f, .50f, .80f, 1.0f,  2200, .10f, .5f, false, 0, .65f },
    { "Glo Strings",    4, Saw,  Saw,     12, 18, 0.f,  0.f,  .500f, .50f, .85f, 1.2f,  4500, .05f, .3f, false, 0, .65f },
    { "Cinematic Str",  4, Saw,  Saw,    -12, 12, 0.f,  0.f,  .800f, .60f, .90f, 1.5f,  1800, .10f, .7f, false, 0, .65f },
    { "Pizz Strings",   4, Saw,  Square,   0, 10, 0.f,  0.f,  .003f, .25f, 0.f, .20f,  2500, .15f, 1.5f, false, 0, .75f },
    { "Soft Pad Str",   4, Tri,  Saw,      0, 12, 0.f,  0.f, 1.000f, .60f, .90f, 1.8f,  2000, .05f, .3f, false, 0, .65f },

    // ---------------- LEADS ----------------
    { "Future Lead",    5, Saw,  Square,   0, 12, 0.f,  0.f,  .005f, .20f, .90f, .25f,  4500, .20f, 1.0f, false, 0, .65f },
    { "Glo Lead",       5, Square, Saw,   12,  8, 0.f,  0.f,  .003f, .15f, .85f, .30f,  5000, .15f, .8f, false, 0, .65f },
    { "Dark Lead",      5, Saw,  Saw,    -12, 10, 0.f,  0.f,  .005f, .20f, .80f, .30f,  2500, .30f, 1.5f, false, 0, .70f },
    { "Sine Lead",      5, Sine, Tri,      0,  5, 0.f,  0.f,  .010f, .10f, 1.0f, .30f,  8000, .05f, 0.f, false, 0, .80f },
    { "Plugg Lead",     5, Square, Tri,   12,  6, 0.f,  0.f,  .002f, .25f, .30f, .20f,  3500, .25f, 2.0f, false, 0, .70f },

    // ---------------- FAST ----------------
    { "Fast Pluck",     6, Saw,  Square,   0,  7, 0.f,  0.f,  .001f, .09f, 0.f, .08f,  2500, .20f, 3.0f, false, 0, .75f },
    { "Rage Stab",      6, Saw,  Saw,      0, 15, 0.f,  0.f,  .001f, .12f, 0.f, .10f,  1800, .30f, 3.5f, false, 0, .75f },
    { "Blip",           6, Square, Sine,  12,  0, 0.f,  0.f,  .001f, .07f, 0.f, .06f,  6000, .10f, 1.0f, false, 0, .75f },
    { "Zap",            6, Saw,  Saw,     12, 20, 0.f,  0.f,  .001f, .10f, 0.f, .10f,  1200, .40f, 4.0f, false, 0, .70f },
    { "Quick Saw",      6, Saw,  Saw,      0, 10, 0.f,  0.f,  .001f, .14f, .20f, .12f,  3000, .15f, 2.5f, false, 0, .70f },
};

inline const int kNumPresets = (int) std::size (kPresets);
} // namespace tr
