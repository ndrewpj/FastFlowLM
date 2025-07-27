---
title: Installation
nav_order: 1
has_children: false
---

## ⚙️ System Requirements

- 🧠 **Memory:** 32 GB RAM or higher recommended  
- ⚡ **CPU/NPU:** AMD Ryzen AI laptop with XDNA2 NPU  
- 🖥️ **OS:** Windows 11

> While FastFlowLM can run with 16 GB RAM, complex models (e.g., 3B or 8B) may require up to 32 GB for optimal performance and longer context length (more kv cache).

---

## 🚨 CRITICAL: NPU Driver Requirpement

You must have AMD NPU driver **version 32.0.203.258 or later** installed for FastFlowLM to work correctly.

- Check via:  
  **Task Manager → Performance → NPU**  
  or  
  **Device Manager → NPU**

🔗 [Download AMD Driver](https://www.amd.com/en/support)

---

## 💾 Installation (Windows)

A packaged Windows installer is available here:  
[**flm-setup.exe**](https://github.com/FastFlowLM/FastFlowLM/releases/latest/download/flm-setup.exe)

If you see **"Windows protected your PC"**, click **More info**, then select **Run anyway**.

📺 [Watch the quick start video](https://www.youtube.com/watch?v=YkwFQ9-cc3I&list=PLf87s9UUZrJp4r3JM4NliPEsYuJNNqFAJ)

For version history and changelog, see the [release notes](https://github.com/FastFlowLM/FastFlowLM/releases/).

---

## 🚀 NPU Power Mode

For optimal performance, set the NPU power mode to **performance** or **turbo**.  
Open **PowerShell** and go to:
```powershell
cd C:\Windows\System32\AMD\
```
Then, run
```powershell
.\xrt-smi configure --pmode turbo
```
> For more details about NPU power mode, refer to the [AMD XRT SMI Documentation](https://ryzenai.docs.amd.com/en/latest/xrt_smi.html).

---

## 🧪 Quick Test (CLI Mode)

After installation, do a quick test to see if FastFlowLM is properly installed. Open **PowerShell**, and run a model in terminal (CLI or Interactive Mode):

```powershell
flm run llama3.2:1b
```

> Requires internet access to HuggingFace to pull (download) the optimized model kernel. The model will be automatically downloaded to the folder: ``C:\Users\<USER>\Documents\flm\models\``. 
>⚠️ If HuggingFace is not directly accessible in your region, you can manually download the model (e.g., [hf-mirror](https://hf-mirror.com/models?search=fastflowlm)) and place it in the directory.
