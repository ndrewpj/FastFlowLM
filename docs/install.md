---
title: Installation
nav_order: 1
has_children: false
---

A packaged Windows installer is available here:  
[**flm-setup.exe**](https://github.com/FastFlowLM/FastFlowLM/releases/download/v0.1.3/flm-setup-v0.1.3.exe)

For version history and changelog, see the [release notes](https://github.com/FastFlowLM/FastFlowLM/releases/).

---

> ⚠️ **Driver requirement:** Ensure your AMD NPU driver is **32.0.203.258 or later**.  
> Check via **Task Manager → Performance → NPU** or **Device Manager**.  
> [Download driver](https://www.amd.com/en/support)

---

### 🔧 Set NPU Power Mode to Performance

For optimal performance, set the NPU power mode to **performance** or **turbo**.  
Open **PowerShell** and run:

```powershell
xrt-smi configure --pmode performance
```
> For more details about NPU power mode, refer to the [AMD XRT SMI Documentation](https://ryzenai.docs.amd.com/en/latest/xrt_smi.html).

After installation, do a quick test to see if FastFlowLM is properly installed. Open **PowerShell**, and run a model in terminal (CLI or Interactive Mode):

```
flm run llama3.2:1B
```
> Requires internet access to HuggingFace to pull (download) the optimized model kernel. The model will be automatically downloaded to the folder: ``C:\Users\<USER>\Documents\flm\models\``. 
>⚠️ If HuggingFace is not directly accessible in your region, you can manually download the model (e.g., [hf-mirror]https://hf-mirror.com/models?search=fastflowlm) and place it in this directory.