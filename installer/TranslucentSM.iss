#define MyAppName "TranslucentSM"
#define MyAppPublisher "Yzy15"

#if GetEnv("APP_VERSION") == ""
  #define MyAppVersion "1.0.0"
#else
  #define MyAppVersion GetEnv("APP_VERSION")
#endif

[Setup]
AppId={{96E49E9A-3D7C-4C61-B390-AB2B6D90185A}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={localappdata}\TranslucentSM
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
Compression=lzma
SolidCompression=yes
WizardStyle=modern
OutputDir=output
OutputBaseFilename=TranslucentSM-Setup-{#MyAppVersion}
UninstallDisplayIcon={app}\TranslucentSM.exe

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "chinesesimp"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "startup"; Description: "Launch at Windows startup"; GroupDescription: "Additional options"

[Files]
Source: "..\package\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\TranslucentSM"; Filename: "{app}\TranslucentSM.exe"
Name: "{autodesktop}\TranslucentSM"; Filename: "{app}\TranslucentSM.exe"; Tasks: desktopicon

[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "TranslucentSM"; ValueData: """{app}\TranslucentSM.exe"""; Flags: uninsdeletevalue; Tasks: startup
Root: HKCU; Subkey: "Software\TranslucentSM"; ValueType: string; ValueName: "InstallPath"; ValueData: "{app}"

[Run]
Filename: "{app}\TranslucentSM.exe"; Description: "{cm:LaunchProgram,TranslucentSM}"; Flags: nowait postinstall skipifsilent
