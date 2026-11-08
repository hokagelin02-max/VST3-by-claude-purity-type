[Setup]
AppName=Kematik Tapes
AppVersion=2.0.0
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\dist
OutputBaseFilename=TapeRomp-Setup
Compression=lzma
SolidCompression=yes

[InstallDelete]
; убираем старую версию плагина (TapeRomp), чтобы в DAW не было двух плагинов
Type: filesandordirs; Name: "{commoncf64}\VST3\TapeRomp.vst3"

[Files]
Source: "..\build\TapeRomp_artefacts\Release\VST3\Kematik Tapes.vst3\*"; DestDir: "{commoncf64}\VST3\Kematik Tapes.vst3"; Flags: recursesubdirs createallsubdirs ignoreversion
