[Setup]

; Basic installer configuration for flm

AppName=flm

AppVersion=1.0

DefaultDirName={pf64}\flm

DefaultGroupName=flm

DisableProgramGroupPage=no

OutputBaseFilename=flm-setup

Compression=lzma

SolidCompression=yes





[Files]

; Main executable

Source: "flm.exe"; DestDir: "{app}"; Flags: ignoreversion

; Required DLL

Source: "libcurl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "llama_npu.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "q4_npu_eXpress.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "vcruntime140_1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion

; Application icon (used for shortcuts)

Source: "logo.ico"; DestDir: "{app}"; Flags: ignoreversion

Source: "model_list.json"; DestDir: "{app}"; Flags: ignoreversion


[Icons]

Name: "{group}\flm"; \
    Filename: "{app}\flm.exe"; \
    WorkingDir: "{app}"; \
    IconFilename: "{app}\logo.ico"; \
    Comment: "Launch flm"

; Desktop shortcut (conditional based on user choice)
Name: "{commondesktop}\flm"; \
    Filename: "{sys}\cmd.exe"; \
    Parameters: "/K ""{app}\flm.exe"" run llama3.2:1B"; \
    WorkingDir: "{app}"; \
    IconFilename: "{app}\logo.ico"; \
    Comment: "Launch flm with llama3.2:1B model"; \
    Tasks: desktopicon

[Registry]
; Add application directory to system PATH
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; \
    ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}"; \
    Check: NeedsAddPath('{app}')

[Tasks]

; Optional desktop icon task

Name: "desktopicon"; Description: "Create a desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked

[Code]
function NeedsAddPath(Param: string): boolean;
var
  OrigPath: string;
begin
  if not RegQueryStringValue(HKEY_LOCAL_MACHINE,
    'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
    'Path', OrigPath)
  then begin
    Result := True;
    exit;
  end;
  Result := Pos(';' + Param + ';', ';' + OrigPath + ';') = 0;
end;
