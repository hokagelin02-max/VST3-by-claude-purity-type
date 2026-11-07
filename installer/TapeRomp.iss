[Setup]
AppName=TapeRomp
AppVersion=1.0.0
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\dist
OutputBaseFilename=TapeRomp-Setup
Compression=lzma
SolidCompression=yes

[Files]
Source: "..\build\TapeRomp_artefacts\Release\VST3\TapeRomp.vst3\*"; DestDir: "{commoncf64}\VST3\TapeRomp.vst3"; Flags: recursesubdirs createallsubdirs ignoreversion
