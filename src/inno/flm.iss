[Setup]

; Basic installer configuration for flm

AppName=flm

AppVersion=0.9.6

AppPublisher=FastFlowLM

AppPublisherURL=www.fastflowlm.com

DefaultDirName={pf64}\flm

DefaultGroupName=flm

DisableProgramGroupPage=no

OutputBaseFilename=flm-setup

Compression=lzma

SolidCompression=yes

LicenseFile=terms.txt

; Icon configuration to preserve original background
SetupIconFile=logo.ico

UninstallDisplayIcon={app}\logo.ico

; Force icon usage without transparency effects
; UsePreviousAppDir=no

; SignTool=FLM_INC


[Files]

; Main executable

Source: "flm.exe"; DestDir: "{app}"; Flags: ignoreversion

; Required DLL

Source: "libcurl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "llama_npu.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "gemma_npu.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "gemma_text_npu.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "qwen_npu.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "lm_head.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "dequant.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "gemm.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "npu_utils.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "q4_npu_eXpress.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "vcruntime140_1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "abseil_dll.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "avcodec-61.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "avdevice-61.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "avfilter-10.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "avformat-61.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "avutil-59.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "libprotobuf-lite.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "libprotobuf.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "libprotoc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "swresample-5.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "swscale-8.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion

; Application icon (used for shortcuts)

Source: "logo.ico"; DestDir: "{app}"; Flags: ignoreversion

Source: "model_list.json"; DestDir: "{app}"; Flags: ignoreversion


[Icons]

Name: "{group}\flm"; \
    Filename: "{app}\flm.exe"; \
    WorkingDir: "{app}"; \
    IconFilename: "{app}\logo.ico"; \
    IconIndex: 0; \
    Comment: "Launch flm"

; Desktop shortcut (conditional based on user choice)
Name: "{commondesktop}\flm run"; \
    Filename: "{sys}\cmd.exe"; \
    Parameters: "/K ""{app}\flm.exe"" run llama3.2:1b"; \
    WorkingDir: "{app}"; \
    IconFilename: "{app}\logo.ico"; \
    IconIndex: 0; \
    Comment: "Launch flm with llama3.2:1b model"; \
    Tasks: desktopicon
    
    
; Desktop shortcut (conditional based on user choice)
Name: "{commondesktop}\flm serve"; \
    Filename: "{sys}\cmd.exe"; \
    Parameters: "/K ""{app}\flm.exe"" serve"; \
    WorkingDir: "{app}"; \
    IconFilename: "{app}\logo.ico"; \
    IconIndex: 0; \
    Comment: "Launch flm in serve mode"; \
    Tasks: desktopicon

[Registry]
; Add application directory to system PATH
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; \
    ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}"

; FLM_MODEL_PATH and FLM_SERVE_PORT will be set to user's choice during installation

[Tasks]
; Optional desktop icon task

Name: "desktopicon"; Description: "Create a desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked

[Code]
var
  ModelPathPage: TInputDirWizardPage;
  PortPage: TInputQueryWizardPage;
  CustomModelPath: string;
  CustomPort: string;

function GetExistingModelPath: string;
var
  ExistingPath: string;
begin
  // Check if FLM_MODEL_PATH environment variable already exists
  if RegQueryStringValue(HKEY_LOCAL_MACHINE,
    'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
    'FLM_MODEL_PATH', ExistingPath)
  then begin
    // FLM_MODEL_PATH exists, use it as default
    Result := ExistingPath;
  end
  else begin
    // FLM_MODEL_PATH doesn't exist, use default userdocs location
    Result := ExpandConstant('{userdocs}\flm');
  end;
end;

function GetExistingPort: string;
var
  ExistingPort: string;
begin
  // Check if FLM_SERVE_PORT environment variable already exists
  if RegQueryStringValue(HKEY_LOCAL_MACHINE,
    'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
    'FLM_SERVE_PORT', ExistingPort)
  then begin
    // FLM_SERVE_PORT exists, use it as default
    Result := ExistingPort;
  end
  else begin
    // FLM_SERVE_PORT doesn't exist, use default port
    Result := '11434';
  end;
end;

procedure InitializeWizard;
begin
  ModelPathPage := CreateInputDirPage(wpSelectDir,
    'Model Storage Location', 'Where should FLM store downloaded models?',
    'Select the folder where FLM will store downloaded models, then click Next.' + #13#10 + #13#10 + 
    'Existing FLM users -- restart PC after path change, then move flm/model/.',
    True, '');
  ModelPathPage.Add('Model storage location:');
  ModelPathPage.Values[0] := GetExistingModelPath;
  
  PortPage := CreateInputQueryPage(wpSelectDir,
    'Server Port Configuration', 'What port should FLM use for the server?',
    'Enter the port number for FLM server (default: 11434).' + #13#10 + #13#10 +
    'Existing FLM users -- the current port will be used as default.');
  PortPage.Add('Server port:', False);
  PortPage.Values[0] := GetExistingPort;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  if CurPageID = ModelPathPage.ID then
  begin
    CustomModelPath := ModelPathPage.Values[0];
  end
  else if CurPageID = PortPage.ID then
  begin
    CustomPort := PortPage.Values[0];
  end;
  Result := True;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    
    // Always set the FLM_MODEL_PATH environment variable to user's choice
    RegWriteStringValue(HKEY_LOCAL_MACHINE,
      'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
      'FLM_MODEL_PATH', CustomModelPath);
      
    // Always set the FLM_SERVE_PORT environment variable to user's choice
    RegWriteStringValue(HKEY_LOCAL_MACHINE,
      'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
      'FLM_SERVE_PORT', CustomPort);
  end;
end;


