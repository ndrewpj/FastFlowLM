---
title: CLI (Interactive Mode)
nav_order: 1
parent: Instructions
---

# ⚡ CLI (Interactive Mode)

FastFlowLM offers a terminal-based interactive experience, similar to Ollama, but fully offline and accelerated excusively on AMD NPUs.

---

## 🔧 Pre-Run PowerShell Commands

### 🆘 Show Help

```powershell
flm help
```

---

### 🚀 Run a Model

Run a model interactively from the terminal:

```powershell
flm run llama3.2:1b
```

> `flm` is short for FastFlowLM. If the model isn't available locally, it will be downloaded automatically. This launches FastFlowLM in CLI mode.

---

### ⬇️ Pull a Model (Download Only)

Download a model from Hugging Face without launching it:

```powershell
flm pull llama3.2:3b
```

This code forces a re-download of the model, overwriting the current version.

```powershell
flm pull llama3.2:3b --force
```

> "force" is only needed when a major `flm` update is released and installed. Proceed with Caution.

---

### 📦 List Supported and Downloaded Models

Display all available models and locally downloaded models:

```powershell
flm list
```

---

### ❌ Remove a Downloaded Model

Delete a model from local storage:

```powershell
flm remove llama3.2:3b
```

---

### 📄 Run with a Text File

Load input from a local text file:

```powershell
flm run llama3.2:1b "C:\Users\Public\Desktop\alice_in_wonderland.txt"
```

👉 [Download the sample prompt](https://github.com/FastFlowLM/FastFlowLM/blob/main/assets/alice_in_wonderland.txt)  

> ⚠️ **Note:**: a model’s supported context length is limited by available DRAM capacity. For example, with **32 GB** of DRAM, **LLaMA 3.1:8B** cannot run beyond a **32K** context length. For the full **128K** context, we recommend larger memory system.

If DRAM is heavily used by other programs while running **FastFlowLM**, you may encounter errors due to insufficient memory, such as:

```error
[XRT] ERROR: Failed to submit the command to the hw queue (0xc01e0200):
Even after the video memory manager split the DMA buffer, the video memory manager
could not page-in all of the required allocations into video memory at the same time.
The device is unable to continue.
```

> 🤔 Interested in checking the DRAM usage?

**Method 1 – Task Manager (Quick View)**  
1. Press **Ctrl + Shift + Esc** (or **Ctrl + Alt + Del** and select **Task Manager**).  
2. Go to the **Performance** tab.  
3. Click **Memory** to see total, used, and available DRAM, as well as usage percentage.  

**Method 2 – Resource Monitor (Detailed View)**  
1. Press **Windows + R**.  
2. Type:  

   \`\`\`
   resmon
   \`\`\`

3. Press **Enter**.  
4. Go to the **Memory** tab to view detailed DRAM usage and a per-process breakdown.

---

Sure! Here's a **simple and clear** version:

---

### 📂 Load a Local Text File in CLI Mode

Use any file that can be opened in Notepad (like `.txt`, `.json`, `.csv`, etc.).

#### ✅ Format (in CLI mode):

```powershell
/input "<file_path>" <prompt>
```

#### 🧪 Example:

```powershell
/input "C:\Users\Public\Desktop\alice_in_wonderland.txt" Can you rewrite the story in 2 sentences?
```

#### 🔎 Notes:

* Use quotes **only around the file path**
* **No quotes** around the prompt
* File must be plain text (readable in Notepad)

---

### 🌐 Start Server Mode

Launch FastFlowLM as a local REST API server (also support OpenAI API):

```powershell
flm serve llama3.2:1b
```

---

## 🧠 Commands Inside Interactive Mode

Once inside the CLI, use the following commands:

---

### 🆘 Help

```text
/?
```

> Displays all available interactive commands. Highly recommended for first-time users.

---

### 🪪 Model Info

```text
/show
```

> View model architecture, size, cache path, and more.

---

### 🔄 Change Model

```text
/load [model_name]
```

> Unload the current model and load a new one. KV cache will be cleared.

---

### 💾 Save Conversation

```text
/save
```

> Save the current conversation history to disk.

---

### 🧹 Clear Memory

```text
/clear
```

> Clear the KV cache (model memory) for a fresh start.

---

### 📊 Show Runtime Stats

```text
/status
```

> Display runtime statistics like token count, throughput, etc.

---

### 🕰️ Show History

```text
/history
```

> Review the current session's conversation history.

---

### 🔍 Toggle Verbose Mode

```text
/verbose
```

> Enable detailed performance metrics per turn. Run again to disable.

---

### 📦 List Models

Display all available models and locally downloaded models:

```text
/list
```

---

### 👋 Quit Interactive Mode

```text
/bye
```

> Exit the CLI.

---

### 🧠 Think Mode Toggle

Type `/think` to toggle Think Mode on or off interactively in the CLI.

> 💡 **Note**: This feature is only supported on certain models, such as **Qwen3**.

---

### ⚙️ Set Variables

```text
/set
```

> Customize decoding parameters like `top_k`, `top_p`, `temperature`, `context length`, `generate_limit`, etc.

> ⚠️ **Note:** Providing invalid or extreme hyperparameter values may cause inference errors.
> `generate_limit` sets an upper limit on the number of tokens that can be generated for each response.

---

### 🛠 Change Default Context Length

You can find more information about available models here:  

```
C:\Program Files\flm\model_list.json
```

You can also change the `default_context_length` setting.

> ⚠️ **Note:** Be cautious! The system reserves DRAM space based on the context length you set.  
> Setting a longer default context length may cause errors on systems with smaller DRAM.
> Also, each model has its own context length limit (examples below).  
> - **Qwen3** → up to **32k** tokens  
> - **LLaMA 3.1 / 3.2** → up to **128k** tokens