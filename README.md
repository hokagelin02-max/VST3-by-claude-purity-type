# Kematik Tapes — лёгкий VST3-синт для plugg / darkplugg / trap / futuristic / glo

72 пресета в 7 вкладках: **BELLS, ARPS, E-PIANO, BRASS, STRINGS, LEADS, FAST** + вкладка **EDIT**.
Все звуки синтезируются (без сэмплов), поэтому плагин лёгкий для процессора.

## Управление
- Вкладки сверху — категории. Список справа — пресеты категории.
- **EDIT** — настройки текущего звука: ATTACK, RELEASE, TONE, RESO, DRIVE, LO-FI, CHORUS, REVERB, SIZE, DLY MIX, DLY TIME, DLY FDBK.
  При выборе другого пресета всё сбрасывается на стандартные значения этого пресета.
- **VOLUME**, **ARP** (авто-включается на пресетах ARPS) и скорость арпеджио 1/4 .. 1/32 внизу.

## Сборка .exe через GitHub
Вкладка **Actions** -> последний запуск -> внизу **Artifacts** -> **TapeRomp-Setup** (zip с `TapeRomp-Setup.exe`).
Установщик ставит плагин в `C:\Program Files\Common Files\VST3\Kematik Tapes.vst3` и удаляет старый `TapeRomp.vst3`.
Windows может показать предупреждение SmartScreen: «Подробнее» -> «Выполнить в любом случае».

## Где что менять
- `Source/Presets.h` — таблица пресетов
- `Source/SynthEngine.h` — звуковой движок
- `Source/Fx.h` — эффекты EDIT
- `Source/PluginEditor.cpp` — внешний вид

JUCE бесплатен для некоммерческого использования; для продажи плагина нужна коммерческая лицензия JUCE.
