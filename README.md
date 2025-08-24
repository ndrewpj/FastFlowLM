<p align="center">
  <a href="https://www.fastflowlm.com" target="_blank">
    <img src="assets/logo.png" alt="FastFlowLM Logo" width="200"/>
  </a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/NPU-Optimized-red" />
</p>

Run large language models on AMD Ryzen™ AI NPUs — in minutes.

FastFlowLM is a lightweight runtime for deploying LLMs like LLaMA3.1/3.2, Qwen3, DeepSeek-R1 and Gemma3 (Vision is now supported) directly on AMD’s NPU — no GPU needed, faster and over 11x more power efficient than the iGPU or hybrid (iGPU+NPU) solutions.

FastFlowLM supports full context lengths — up to 128K tokens with LLaMA 3.1/3.2 and Gemma3:4B.

**Just like Ollama — but purpose-built and deeply optimized for the Ryzen™ AI NPU**

> FastFlowLM supports all Ryzen™ AI Series chips with XDNA2 NPUs (Strix, Strix Halo, and Kraken).

🔽 **Download:** [flm-setup.exe](https://github.com/FastFlowLM/FastFlowLM/releases/latest/download/flm-setup.exe)  
📦 **Supported Models:** [docs.fastflowlm.com/models/](https://https://docs.fastflowlm.com/models/)  
📖 **Documentation:** [docs.fastflowlm.com](https://docs.fastflowlm.com)  
💬 **Discord Server:** [discord.gg/z24t23HsHF](https://discord.gg/z24t23HsHF)  
📺 **YouTube Demos:** [youtube.com/@FastFlowLM-YT](https://www.youtube.com/@FastFlowLM-YT)  
🧪 **Test Drive (Remote Machine):** [open-webui.testdrive-fastflowlm.com](https://open-webui.testdrive-fastflowlm.com/)  

---

## 📺 Demo Videos

<!-- **🎥 Check out our demo videos!**   -->
From the new **Gemma3:4b vision (first NPU-only VLM)** model to the **think/no_think Qwen3**, **4-power-mode operation**, and head-to-head comparisons with **Ollama, LM Studio, Lemonade**, and more — it’s all [up on YouTube](https://youtu.be/9QipiMg5Yz8?list=PLf87s9UUZrJp4r3JM4NliPEsYuJNNqFAJ)!


<!-- FastFlowLM vs AMD’s official stack (Ryzen™ AI software 1.4) — **real-time speedup and power efficiency**: 

- Same prompt (length: 1835 tokens), same model (LLaMA 3.2 1B model; weights int4; activation bf16), running on the same machine (AMD Ryzen™ AI 5 340 NPU with 32 GB SO-DIMM DDR5 5600 MHz memory)
- Real-time CPU, iGPU, NPU usage, and power consumption shown (Windows task manager + HWINFO)

<table>
  <tr>
    <td valign="top">
      <h4>🔹 FastFlowLM vs Ryzen™ AI SW 1.4 (NPU-only)</h4>
      <a href="https://youtu.be/kv31FZ_q0_I?list=PLf87s9UUZrJp4r3JM4NliPEsYuJNNqFAJ">
        <img src="https://img.youtube.com/vi/kv31FZ_q0_I/0.jpg" alt="Demo: FastFlowLM vs OGA" width="320">
      </a>
    </td>
    <td valign="top">
      <h4>🔹 FastFlowLM vs Ryzen™ AI SW 1.4 (Hybrid)</h4>
      <a href="https://youtu.be/PFjH-L_Kr0w?list=PLf87s9UUZrJp4r3JM4NliPEsYuJNNqFAJ">
        <img src="https://img.youtube.com/vi/PFjH-L_Kr0w/0.jpg" alt="Demo: FastFlowLM vs GAIA" width="320">
      </a>
    </td>
  </tr>
</table> -->

<!-- 🎥 Demo videos (e.g., the new gemma3:4b visoin, think/no_think qwen3 model, 4 power mode operation, etc.) and comparisons with Ollama, LM Studio, Lemonade, etc. can be found [here on YouTube](https://www.youtube.com/watch?v=OZuLQcmFe9A&list=PLf87s9UUZrJp4r3JM4NliPEsYuJNNqFAJ). -->

---

## 🧪 Test Drive (Remote Demo)

🚀 Don’t have a Ryzen™ AI PC? Instantly try FastFlowLM on a live AMD Ryzen™ AI 5 340 NPU with 32 GB memory ([spec](https://www.amazon.com/4X4-BOX-AI340-Display-Support-Copilot/dp/B0F2GFLF67/ref=sr_1_5?crid=1X16RDUCQ2497&dib=eyJ2IjoiMSJ9.C5GS4xMl_kkJ7Yr6dNFi6g.Dfj_l9Dk1yuIBjppqmKSqNAAPQc1F4Mu3zJ9-MDlszw&dib_tag=se&keywords=4x4+box+ai340&qid=1752010554&sprefix=www.amazon.com%2F4X4-BOX-AI340%2Caps%2C176&sr=8-5)) — no setup needed.  

✨ Now with **Gemma3:4b (the first NPU-only VLM!)** supported here.  

🌐 **Launch Now**: [https://open-webui.testdrive-fastflowlm.com/](https://open-webui.testdrive-fastflowlm.com/)  
🔐 **Login**: `guest@flm.npu`  
🔑 **Password**: `0000`

📺 [Watch this short video](https://www.youtube.com/watch?v=0AhkX2ZLu7Y&list=PLf87s9UUZrJp4r3JM4NliPEsYuJNNqFAJ&index=2) to see how to try the remote demo in just a few clicks.

> Alternatively, **sign up** with your own credentials instead of using the shared guest account.
> ⚠️ Some universities or companies may **block access** to the test drive site. If it doesn't load over Wi-Fi, try switching to a **cellular network**.  
> Real-time demo powered by **FastFlowLM + Open WebUI** — no downloads, no installs.  
> Try optimized LLM models: `gemma3:4b`, `qwen3:4b`, etc. — all accelerated on NPU.

⚠️ **Please note**:  
- FastFlowLM is designed for **single-user local use**. This remote demo machine may experience short wait times when **multiple users** access it concurrently — please be patient.
- When switching models, it may take longer time to replace the model in memory.
- Large prompts (30k+ tokens) and VLM (gemma3:4b) may take longer — but it works! 🙂

---

## ⚡ Quick Start

A packaged Windows installer is available here: [**flm-setup.exe**](https://github.com/FastFlowLM/FastFlowLM/releases/latest/download/flm-setup.exe). For more details, see the [release notes](https://github.com/FastFlowLM/FastFlowLM/releases/).

📺 [Watch the quick start video](https://www.youtube.com/watch?v=YkwFQ9-cc3I&list=PLf87s9UUZrJp4r3JM4NliPEsYuJNNqFAJ)

> ⚠️ Ensure NPU driver is **32.0.203.258** or later (check via Task Manager→Performance→NPU or Device Manager) — [Driver Download](https://www.amd.com/en/support).

After installation, open **PowerShell**. To run a model in terminal (CLI or Interactive Mode):
```
flm run llama3.2:1b
```
> Requires internet access to HuggingFace to pull (download) the optimized model kernel. The model will be downloaded to the folder: ``C:\Users\<USER>\Documents\flm\models\``. ⚠️ If HuggingFace is not directly accessible in your region, you can manually download the model and place it in this directory.

To start the local REST API server (Server Mode):
```
flm serve llama3.2:1b
```
> The model tag (e.g., `llama3.2:1b`) sets the initial model, which is optional. If another model is requested, FastFlowLM will automatically switch to it. Local server is on port 11434 (default).

By default, **FLM runs in `performance` NPU power mode**. You can switch to other NPU power modes (`powersaver`, `balanced`, or `turbo`) using the `--pmode` flag:

**CLI mode:**
```Powershell
flm run gemma3:4b --pmode balanced
```

**Server mode:**
```Powershell
flm serve gemma3:4b --pmode balanced
```

> ⚠️ Note: Using powersaver or balanced will lower NPU clock speeds and cause a significant drop in speed. For more details about NPU power mode, refer to the [AMD XRT SMI Documentation](https://ryzenai.docs.amd.com/en/latest/xrt_smi.html).

For detailed instructions, click [Documentation](https://docs.fastflowlm.com/).

---

## 🖼️ Vision Support for Gemma3:4b (VLM)  

FastFlowLM now supports **vision-language inference** with **Gemma3:4b**. ⚡ Quick start:  

After installation, open **PowerShell** and run:
```
flm run gemma3:4b
```

In CLI, attach an image:
```
/input "path/to/image.png" What's in this image?
```

> Supports **.png** and **.jpg** formats  

---

## 📚 Supported Models by FastFlowLM (FLM)

Check the full list here:
👉 https://docs.fastflowlm.com/models/

---

## 🧠 Local AI on Your NPU

FastFlowLM makes it easy to run cutting-edge **LLMs** (and now **VLMs** too ) locally with:
- ⚡ Fast and low power
- 🧰 Simple CLI and API
- 🔐 Fully private and offline

No model rewrites, no tuning — it just works.

---

## ✅ Features

- **Runs fully on AMD Ryzen™ AI NPU** — no GPU or CPU load  
- **Developer-first flow** — like Ollama, but optimized for NPU  
- **Support for long context windows** — up to 128k tokens (e.g., LLaMA 3.1/3.2, Gemma3:4B)  
- **No low-level tuning required** — You focus on your app, we handle the rest

---

## ⚡ Performance

<!-- Compared to AMD Ryzen™ AI Software 1.4 (GAIA or Lemonade):

### LLM Decoding Speed (TPS: Tokens per Second)
- 🚀 Up to **14.2× faster** vs NPU-only baseline  
- 🚀 Up to **16.2× faster** vs hybrid iGPU+NPU baseline

### Power Efficiency
- 🔋 Up to **2.66× more efficient in decoding** vs NPU-only  
- 🔋 Up to **11.38× more efficient in decoding** vs hybrid  
- 🔋 Up to **3.4× more efficient in prefill** vs NPU-only or hybrid

### Latency
- ⏱️ **Matches or exceeds** TTFT (Time to First Token) of NPU-only or hybrid mode -->

<!-- ### Benchmarks -->
<p style="font-size:85%; margin:0;">
📊 View the detailed results here:
<a href="https://docs.fastflowlm.com/benchmarks/" style="text-decoration:none;">
<strong>[Benchmark results]</strong>
</a>
</p>

---

## 🛠️ Instructions

[Documentation and examples](https://docs.fastflowlm.com/). Like Ollama, you can:
- Load and run models locally via CLI (Interactive Mode)
- Integrate into your app via a simple REST API via a local server (Server Mode)
> Compatible with tools like **Microsoft AI Toolkit**, **Open WebUI**, and more.

---

## 📄 License

- All orchestration code and CLI tools are open-source under the [MIT License](./LICENSE_RUNTIME.txt).
- NPU-accelerated kernels are **proprietary binaries**, free for **non-commercial use** only — see [LICENSE_BINARY.txt](./LICENSE_BINARY.txt) and [TERMS.md](./TERMS.md) for details.
- **Non-commercial users need to acknowledge FastFlowLM** by adding this line to your README or project page:  
  ```
  Powered by [FastFlowLM](https://github.com/FastFlowLM/FastFlowLM)
  ```
For commercial use or licensing inquiries, email us: info@fastflowlm.com

---

💬 **Have feedback or want early access to our new releases? [Open an issue](https://github.com/fastflowlm/fastflowlm/issues/new) or [Join our Discord community](https://discord.gg/z24t23HsHF)

---

## 🙏 Acknowledgements

- Powered by the advanced **AMD Ryzen™ AI NPU architecture**
- Inspired by the widely adopted [Ollama](https://github.com/ollama/ollama)
- Tokenization accelerated with [MLC-ai/tokenizers-cpp](https://github.com/mlc-ai/tokenizers-cpp)
- Chat formatting via [Google/minja](https://github.com/google/minja)
- Low-level kernels optimized using the powerful [IRON](https://github.com/Xilinx/mlir-aie/tree/main/programming_guide)+[AIE-MLIR](https://github.com/Xilinx/mlir-aie/tree/main/mlir_tutorials)
