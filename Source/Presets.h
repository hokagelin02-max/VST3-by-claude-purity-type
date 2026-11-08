#pragma once
#include <iterator>

// Kematik Tapes - таблица пресетов (все звуки синтезируются, сэмплов нет -> лёгкий плагин).
// Файл сгенерирован; порядок полей ВАЖЕН (см. struct Preset).
namespace tr
{
enum Wave { Sine = 0, Saw, Square, Tri, Pulse };

struct Preset
{
    const char* name;
    int cat;                   // индекс категории (вкладки)
    int w1, w2;                // формы волн
    float semi2, detune;       // osc2: полутона, расстройка (центы)
    float mix2;                // уровень osc2
    float sub;                 // саб-синус (-1 октава)
    float noise;               // шум (удар / дыхание)
    float fmRatio, fmAmt;      // FM для osc1
    float tine;                // FM "тайн" для osc2 (e-piano)
    float a, d, s, r;          // огибающая
    float cutoff, res, fenv;   // фильтр: Гц, резонанс, глубина огибающей (октавы)
    float pitchEnv;            // питч-огибающая (полутона, затухает ~60 мс)
    float lfoRate, lfoDepth;   // вибрато (Гц, полутона)
    float reverb, delay;       // стандартные значения EDIT для пресета
    bool arp;                  // включать арпеджиатор
    int arpPattern;            // 0 вверх, 1 вниз, 2 вверх-вниз, 3 вверх+октава
    float gain;
};

inline const int kNumCats = 7;
inline const char* const kCategories[kNumCats] = { "BELLS", "ARPS", "E-PIANO", "BRASS", "STRINGS", "LEADS", "FAST" };

inline const Preset kPresets[] =
{
    { "Ice Bell", 0, Sine, Sine, 12.0f, 3.0f, 0.35f, 0.0f, 0.0f, 3.5f, 1.2f, 0.0f, 0.001f, 1.6f, 0.0f, 1.4f, 9000.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.35f, 0.15f, false, 0, 0.75f },
    { "Dark Chime", 0, Sine, Tri, 7.0f, 6.0f, 0.3f, 0.0f, 0.0f, 1.41f, 1.5f, 0.0f, 0.001f, 2.2f, 0.0f, 1.8f, 4500.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.2f, false, 0, 0.75f },
    { "Music Box", 0, Sine, Sine, 12.0f, 2.0f, 0.25f, 0.0f, 0.0f, 5.0f, 0.8f, 0.0f, 0.001f, 0.9f, 0.0f, 0.8f, 9000.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.25f, false, 0, 0.75f },
    { "Glo Glock", 0, Sine, Sine, 24.0f, 0.0f, 0.2f, 0.0f, 0.0f, 7.0f, 0.9f, 0.0f, 0.001f, 0.7f, 0.0f, 0.9f, 10000.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.2f, false, 0, 0.75f },
    { "Haunted Bell", 0, Sine, Sine, 0.0f, 8.0f, 0.4f, 0.0f, 0.0f, 2.76f, 1.6f, 0.0f, 0.002f, 3.0f, 0.0f, 3.0f, 3500.0f, 0.05f, 0.0f, 0.0f, 4.5f, 0.08f, 0.5f, 0.3f, false, 0, 0.75f },
    { "Tubular", 0, Sine, Sine, 12.0f, 3.0f, 0.3f, 0.0f, 0.0f, 2.0f, 1.3f, 0.0f, 0.001f, 2.5f, 0.0f, 2.0f, 6000.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.0f, false, 0, 0.75f },
    { "Kalimba", 0, Sine, Tri, 12.0f, 3.0f, 0.3f, 0.0f, 0.06f, 4.0f, 0.5f, 0.0f, 0.001f, 0.5f, 0.0f, 0.5f, 6000.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.2f, false, 0, 0.75f },
    { "Crystal", 0, Sine, Sine, 19.0f, 5.0f, 0.3f, 0.0f, 0.0f, 6.0f, 1.0f, 0.0f, 0.001f, 1.2f, 0.0f, 1.5f, 10000.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.45f, 0.25f, false, 0, 0.75f },
    { "Dream Bell", 0, Sine, Tri, 12.0f, 6.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.8f, 0.0f, 0.01f, 2.0f, 0.0f, 2.5f, 5000.0f, 0.05f, 0.0f, 0.0f, 5.0f, 0.05f, 0.5f, 0.35f, false, 0, 0.75f },
    { "Toy Bell", 0, Tri, Sine, 24.0f, 3.0f, 0.25f, 0.0f, 0.0f, 3.0f, 0.6f, 0.0f, 0.001f, 0.45f, 0.0f, 0.4f, 8000.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.1f, false, 0, 0.75f },
    { "Pluck Arp", 1, Saw, Square, 0.0f, 8.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.18f, 0.0f, 0.15f, 1500.0f, 0.25f, 2.5f, 0.0f, 0.0f, 0.0f, 0.15f, 0.2f, true, 0, 0.7f },
    { "Dark Arp", 1, Saw, Saw, -12.0f, 12.0f, 0.6f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.22f, 0.0f, 0.2f, 800.0f, 0.3f, 2.5f, 0.0f, 0.0f, 0.0f, 0.2f, 0.3f, true, 2, 0.7f },
    { "Future Arp", 1, Square, Saw, 12.0f, 6.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.15f, 0.0f, 0.12f, 2200.0f, 0.2f, 2.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.25f, true, 3, 0.7f },
    { "Glass Arp", 1, Tri, Sine, 12.0f, 3.0f, 0.8f, 0.0f, 0.0f, 2.0f, 0.7f, 0.0f, 0.001f, 0.3f, 0.0f, 0.4f, 7000.0f, 0.05f, 1.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, true, 0, 0.7f },
    { "Trap Arp", 1, Saw, Square, 0.0f, 10.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.12f, 0.0f, 0.1f, 1200.0f, 0.3f, 3.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.2f, true, 1, 0.7f },
    { "Bell Arp", 1, Sine, Sine, 12.0f, 8.0f, 0.3f, 0.0f, 0.0f, 3.5f, 1.0f, 0.0f, 0.001f, 0.5f, 0.0f, 0.5f, 9000.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.35f, 0.3f, true, 0, 0.7f },
    { "Pulse Arp", 1, Pulse, Pulse, 12.0f, 10.0f, 0.6f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.14f, 0.0f, 0.1f, 2500.0f, 0.15f, 2.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.25f, true, 2, 0.7f },
    { "Sub Arp", 1, Saw, Sine, -12.0f, 8.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.2f, 0.0f, 0.15f, 900.0f, 0.15f, 2.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.15f, true, 0, 0.7f },
    { "Wide Arp", 1, Saw, Saw, 0.0f, 20.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.25f, 0.0f, 0.3f, 2000.0f, 0.15f, 2.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.35f, true, 3, 0.7f },
    { "Pitch Arp", 1, Saw, Square, 0.0f, 6.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.1f, 0.0f, 0.1f, 1800.0f, 0.15f, 2.0f, 4.0f, 0.0f, 0.0f, 0.2f, 0.2f, true, 0, 0.7f },
    { "Rhodes Classic", 2, Sine, Sine, 0.0f, 3.0f, 0.35f, 0.0f, 0.0f, 1.0f, 1.1f, 1.1f, 0.002f, 2.5f, 0.05f, 0.4f, 6000.0f, 0.05f, 0.3f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.8f },
    { "Rhodes Dark", 2, Sine, Sine, 0.0f, 3.0f, 0.3f, 0.12f, 0.0f, 1.0f, 0.8f, 0.6f, 0.002f, 3.0f, 0.05f, 0.5f, 2800.0f, 0.05f, 0.3f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, false, 0, 0.8f },
    { "Wurly", 2, Sine, Tri, 0.0f, 3.0f, 0.3f, 0.0f, 0.015f, 2.0f, 1.3f, 0.0f, 0.002f, 1.3f, 0.1f, 0.3f, 4500.0f, 0.05f, 0.3f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.8f },
    { "Soft EP", 2, Sine, Sine, 0.0f, 3.0f, 0.3f, 0.0f, 0.0f, 1.0f, 0.5f, 0.4f, 0.002f, 3.0f, 0.1f, 0.6f, 4000.0f, 0.05f, 0.3f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, false, 0, 0.8f },
    { "Plugg Keys", 2, Sine, Sine, 0.0f, 3.0f, 0.35f, 0.0f, 0.0f, 3.0f, 1.2f, 1.4f, 0.002f, 0.8f, 0.0f, 0.35f, 5500.0f, 0.05f, 0.5f, 0.0f, 0.0f, 0.0f, 0.25f, 0.2f, false, 0, 0.8f },
    { "Bright Tine", 2, Sine, Sine, 0.0f, 3.0f, 0.4f, 0.0f, 0.0f, 1.0f, 1.5f, 2.0f, 0.002f, 1.5f, 0.05f, 0.4f, 8000.0f, 0.05f, 0.3f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.8f },
    { "Dream Keys", 2, Sine, Sine, 0.0f, 3.0f, 0.3f, 0.0f, 0.0f, 1.0f, 0.6f, 0.3f, 0.01f, 3.5f, 0.3f, 1.2f, 3500.0f, 0.05f, 0.3f, 0.0f, 5.0f, 0.06f, 0.45f, 0.3f, false, 0, 0.8f },
    { "Glo Keys", 2, Sine, Sine, 12.0f, 3.0f, 0.3f, 0.0f, 0.0f, 2.0f, 0.8f, 1.0f, 0.002f, 1.4f, 0.05f, 0.5f, 6500.0f, 0.05f, 0.3f, 0.0f, 0.0f, 0.0f, 0.35f, 0.25f, false, 0, 0.8f },
    { "Lofi Keys", 2, Sine, Sine, 0.0f, 3.0f, 0.2f, 0.0f, 0.03f, 1.0f, 0.5f, 0.2f, 0.002f, 2.0f, 0.05f, 0.5f, 2200.0f, 0.05f, 0.3f, 0.0f, 0.8f, 0.15f, 0.3f, 0.0f, false, 0, 0.8f },
    { "Trap Keys", 2, Sine, Sine, 0.0f, 3.0f, 0.3f, 0.25f, 0.0f, 1.0f, 0.9f, 0.8f, 0.002f, 1.0f, 0.0f, 0.3f, 3000.0f, 0.05f, 0.3f, 0.0f, 0.0f, 0.0f, 0.2f, 0.15f, false, 0, 0.8f },
    { "Trap Brass", 3, Saw, Saw, 0.0f, 10.0f, 0.9f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.04f, 0.3f, 0.7f, 0.25f, 900.0f, 0.15f, 2.5f, 0.0f, 5.0f, 0.05f, 0.2f, 0.0f, false, 0, 0.7f },
    { "Dark Horns", 3, Saw, Saw, -12.0f, 12.0f, 0.7f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.06f, 0.4f, 0.7f, 0.3f, 650.0f, 0.2f, 2.5f, 0.0f, 0.0f, 0.0f, 0.25f, 0.0f, false, 0, 0.7f },
    { "Royal Brass", 3, Saw, Square, 0.0f, 8.0f, 0.8f, 0.0f, 0.04f, 0.0f, 0.0f, 0.0f, 0.05f, 0.35f, 0.75f, 0.3f, 1100.0f, 0.1f, 2.2f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.7f },
    { "Stab Brass", 3, Saw, Saw, 0.0f, 14.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.008f, 0.2f, 0.3f, 0.15f, 1000.0f, 0.2f, 3.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.1f, false, 0, 0.7f },
    { "Soft Brass", 3, Saw, Tri, 0.0f, 6.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.09f, 0.4f, 0.8f, 0.4f, 800.0f, 0.1f, 2.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.7f },
    { "Brass Swell", 3, Saw, Saw, 0.0f, 8.0f, 0.8f, 0.15f, 0.0f, 0.0f, 0.0f, 0.0f, 0.35f, 0.5f, 0.85f, 0.5f, 600.0f, 0.15f, 3.0f, 0.0f, 0.0f, 0.0f, 0.35f, 0.0f, false, 0, 0.7f },
    { "Horn Scoop", 3, Saw, Saw, 0.0f, 8.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.03f, 0.4f, 0.7f, 0.3f, 900.0f, 0.15f, 2.5f, -2.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.7f },
    { "Synth Brass 80s", 3, Saw, Saw, 0.0f, 18.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.02f, 0.25f, 0.8f, 0.25f, 1500.0f, 0.2f, 2.5f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, false, 0, 0.7f },
    { "Muted Brass", 3, Square, Saw, 0.0f, 6.0f, 0.6f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.03f, 0.3f, 0.6f, 0.2f, 700.0f, 0.35f, 2.2f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.7f },
    { "Low Brass", 3, Saw, Saw, -12.0f, 8.0f, 0.8f, 0.35f, 0.0f, 0.0f, 0.0f, 0.0f, 0.06f, 0.4f, 0.75f, 0.3f, 500.0f, 0.1f, 2.5f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.7f },
    { "Dark Strings", 4, Saw, Saw, 0.0f, 14.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.6f, 0.5f, 0.8f, 1.0f, 1800.0f, 0.08f, 0.5f, 0.0f, 5.2f, 0.04f, 0.4f, 0.0f, false, 0, 0.65f },
    { "Glo Strings", 4, Saw, Saw, 12.0f, 18.0f, 0.6f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.85f, 1.2f, 3500.0f, 0.08f, 0.3f, 0.0f, 0.0f, 0.0f, 0.4f, 0.0f, false, 0, 0.65f },
    { "Cinematic", 4, Saw, Saw, -12.0f, 12.0f, 0.8f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.8f, 0.6f, 0.9f, 1.5f, 1500.0f, 0.08f, 0.7f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, false, 0, 0.65f },
    { "Pizz Strings", 4, Saw, Square, 0.0f, 10.0f, 0.8f, 0.0f, 0.03f, 0.0f, 0.0f, 0.0f, 0.003f, 0.25f, 0.0f, 0.2f, 2000.0f, 0.15f, 1.5f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.65f },
    { "Soft Pad", 4, Tri, Saw, 0.0f, 12.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.6f, 0.9f, 1.8f, 1800.0f, 0.05f, 0.3f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, false, 0, 0.65f },
    { "Staccato Str", 4, Saw, Saw, 0.0f, 10.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.02f, 0.15f, 0.2f, 0.15f, 2200.0f, 0.08f, 1.2f, 0.0f, 0.0f, 0.0f, 0.25f, 0.0f, false, 0, 0.65f },
    { "Violin Lead", 4, Saw, Saw, 0.0f, 5.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.12f, 0.3f, 0.9f, 0.3f, 2800.0f, 0.08f, 0.5f, 0.0f, 5.5f, 0.12f, 0.3f, 0.0f, false, 0, 0.65f },
    { "Choir Pad", 4, Tri, Sine, 12.0f, 6.0f, 0.5f, 0.0f, 0.02f, 0.0f, 0.0f, 0.0f, 0.7f, 0.6f, 0.9f, 1.5f, 2500.0f, 0.05f, 0.2f, 0.0f, 4.8f, 0.05f, 0.55f, 0.0f, false, 0, 0.65f },
    { "Dark Pad", 4, Saw, Saw, -12.0f, 16.0f, 0.8f, 0.25f, 0.0f, 0.0f, 0.0f, 0.0f, 1.2f, 0.7f, 0.85f, 2.0f, 900.0f, 0.08f, 0.8f, 0.0f, 0.0f, 0.0f, 0.55f, 0.0f, false, 0, 0.65f },
    { "Tape Strings", 4, Saw, Saw, 0.0f, 20.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.5f, 0.8f, 0.8f, 1400.0f, 0.08f, 0.5f, 0.0f, 0.9f, 0.15f, 0.35f, 0.2f, false, 0, 0.65f },
    { "Future Lead", 5, Saw, Square, 0.0f, 12.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.005f, 0.2f, 0.9f, 0.25f, 3500.0f, 0.2f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.25f, false, 0, 0.65f },
    { "Glo Lead", 5, Square, Saw, 12.0f, 8.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.003f, 0.15f, 0.85f, 0.3f, 4000.0f, 0.15f, 0.8f, 0.0f, 5.5f, 0.1f, 0.2f, 0.3f, false, 0, 0.65f },
    { "Dark Lead", 5, Saw, Saw, -12.0f, 10.0f, 0.8f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.005f, 0.2f, 0.8f, 0.3f, 1800.0f, 0.3f, 1.5f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.65f },
    { "Sine Lead", 5, Sine, Tri, 0.0f, 5.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.01f, 0.1f, 1.0f, 0.3f, 7000.0f, 0.05f, 0.0f, 0.0f, 5.5f, 0.12f, 0.3f, 0.0f, false, 0, 0.65f },
    { "Plugg Lead", 5, Square, Tri, 12.0f, 6.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.002f, 0.25f, 0.3f, 0.2f, 3000.0f, 0.25f, 2.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.25f, false, 0, 0.65f },
    { "Pulse Lead", 5, Pulse, Pulse, 0.0f, 12.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.004f, 0.2f, 0.8f, 0.25f, 3500.0f, 0.15f, 1.2f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.65f },
    { "Scoop Lead", 5, Saw, Square, 0.0f, 8.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.005f, 0.25f, 0.8f, 0.3f, 3000.0f, 0.15f, 1.5f, -2.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.65f },
    { "Soft Lead", 5, Tri, Saw, 0.0f, 6.0f, 0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.02f, 0.2f, 0.9f, 0.4f, 2500.0f, 0.05f, 0.3f, 0.0f, 0.0f, 0.0f, 0.35f, 0.0f, false, 0, 0.65f },
    { "Hyper Lead", 5, Saw, Saw, 0.0f, 18.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.002f, 0.15f, 0.9f, 0.2f, 4500.0f, 0.25f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.3f, false, 0, 0.65f },
    { "Bit Lead", 5, Square, Pulse, 0.0f, 3.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.002f, 0.1f, 0.8f, 0.1f, 5000.0f, 0.15f, 0.0f, 0.0f, 7.0f, 0.2f, 0.1f, 0.0f, false, 0, 0.65f },
    { "Fast Pluck", 6, Saw, Square, 0.0f, 7.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.09f, 0.0f, 0.08f, 1500.0f, 0.2f, 2.5f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.72f },
    { "Rage Stab", 6, Saw, Saw, 0.0f, 15.0f, 0.8f, 0.0f, 0.03f, 0.0f, 0.0f, 0.0f, 0.001f, 0.12f, 0.0f, 0.1f, 1400.0f, 0.3f, 2.5f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.72f },
    { "Blip", 6, Square, Sine, 12.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.07f, 0.0f, 0.06f, 4000.0f, 0.1f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.72f },
    { "Zap", 6, Saw, Saw, 12.0f, 20.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.1f, 0.0f, 0.1f, 1200.0f, 0.35f, 2.0f, 18.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.72f },
    { "Quick Saw", 6, Saw, Saw, 0.0f, 10.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.14f, 0.2f, 0.12f, 2200.0f, 0.15f, 2.5f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.72f },
    { "Laser", 6, Sine, Saw, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.18f, 0.0f, 0.1f, 5000.0f, 0.05f, 0.0f, 30.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.72f },
    { "Bubble", 6, Sine, Sine, 12.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.12f, 0.0f, 0.1f, 6000.0f, 0.05f, 0.0f, -7.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.72f },
    { "Noise Tick", 6, Sine, Saw, 0.0f, 8.0f, 0.0f, 0.0f, 0.6f, 0.0f, 0.0f, 0.0f, 0.001f, 0.05f, 0.0f, 0.05f, 3500.0f, 0.4f, 1.5f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.72f },
    { "Trill", 6, Square, Tri, 12.0f, 8.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.2f, 0.5f, 0.1f, 3000.0f, 0.15f, 1.0f, 0.0f, 11.0f, 0.6f, 0.2f, 0.0f, false, 0, 0.72f },
    { "Sub Pluck", 6, Sine, Sine, 0.0f, 0.0f, 0.0f, 0.7f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.15f, 0.0f, 0.1f, 1000.0f, 0.15f, 1.5f, 5.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.72f },
    { "Chip Run", 6, Pulse, Square, 12.0f, 3.0f, 0.6f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.08f, 0.0f, 0.05f, 4500.0f, 0.15f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.72f },
    { "FM Snap", 6, Sine, Sine, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f, 2.0f, 1.3f, 0.0f, 0.001f, 0.1f, 0.0f, 0.08f, 6000.0f, 0.15f, 0.0f, 3.0f, 0.0f, 0.0f, 0.2f, 0.0f, false, 0, 0.72f },
};

inline const int kNumPresets = (int) std::size (kPresets);
} // namespace tr
