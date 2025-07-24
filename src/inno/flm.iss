[Setup]

; Basic installer configuration for flm

AppName=flm

AppVersion=0.1.8

AppPublisher=FastFlowLM

AppPublisherURL=www.fastflowlm.com

DefaultDirName={pf64}\flm

DefaultGroupName=flm

DisableProgramGroupPage=no

OutputBaseFilename=flm-setup

Compression=lzma

SolidCompression=yes

LicenseFile=terms.txt

SignTool=FastFlowLM_url $f


[Files]

; Main executable

Source: "flm.exe"; DestDir: "{app}"; Flags: ignoreversion

; Required DLL

Source: "libcurl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "llama_npu.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "qwen_npu.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "lm_head.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "dequant.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "gemm.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "npu_utils.dll"; DestDir: "{app}"; Flags: ignoreversion
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
Name: "{commondesktop}\flm run"; \
    Filename: "{sys}\cmd.exe"; \
    Parameters: "/K ""{app}\flm.exe"" run llama3.2:1b"; \
    WorkingDir: "{app}"; \
    IconFilename: "{app}\logo.ico"; \
    Comment: "Launch flm with llama3.2:1b model"; \
    Tasks: desktopicon
    
    
; Desktop shortcut (conditional based on user choice)
Name: "{commondesktop}\flm serve"; \
    Filename: "{sys}\cmd.exe"; \
    Parameters: "/K ""{app}\flm.exe"" serve"; \
    WorkingDir: "{app}"; \
    IconFilename: "{app}\logo.ico"; \
    Comment: "Launch flm in serve mode"; \
    Tasks: desktopicon

[Registry]
; Add application directory to system PATH (only if not already present)
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; \
    ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}"; \
    Check: NeedsAddPath('{app}')

; Set FLM_SERVE_PORT environment variable (only if not already defined)
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; \
    ValueType: string; ValueName: "FLM_SERVE_PORT"; ValueData: "11434"; \
    Check: NeedsSetFlmPort

[Tasks]

; Optional desktop icon task

Name: "desktopicon"; Description: "Create a desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked

[Code]
function NeedsAddPath(Param: string): boolean;
var
  OrigPath: string;
begin
  // Check if PATH environment variable exists
  if not RegQueryStringValue(HKEY_LOCAL_MACHINE,
    'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
    'Path', OrigPath)
  then begin
    // PATH doesn't exist, so we need to add it
    Result := True;
    exit;
  end;
  
  // Check if the path is already in the PATH variable
  // We add semicolons around both the original path and our parameter
  // to ensure we match complete path entries, not partial matches
  Result := Pos(';' + Param + ';', ';' + OrigPath + ';') = 0;
end;

function NeedsSetFlmPort: boolean;
var
  FlmPort: string;
begin
  // Check if FLM_SERVE_PORT environment variable already exists
  if not RegQueryStringValue(HKEY_LOCAL_MACHINE,
    'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
    'FLM_SERVE_PORT', FlmPort)
  then begin
    // FLM_SERVE_PORT doesn't exist, so we need to set it
    Result := True;
    exit;
  end;
  // FLM_SERVE_PORT already exists, don't overwrite it
  Result := False;
end;
