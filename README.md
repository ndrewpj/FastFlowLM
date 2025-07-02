<p align="center">
  <a href="https://www.fastflowlm.com" target="_blank">
    <img src="assets/logo.png" alt="FastFlowLM Logo" width="200"/>
  </a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/license-MIT-blue.svg" />
  <img src="https://img.shields.io/badge/NPU-Optimized-red" />
</p>

Run large language models on AMD Ryzen™ AI NPUs — in minutes.

FastFlowLM is a lightweight runtime for deploying LLMs like LLaMA and DeepSeek directly on AMD’s NPU — no GPU needed, and over 11x more power efficient than the integrated GPU.

FastFlowLM supports full context lengths — up to 128K tokens with LLaMA 3.1 and 3.2.

**Just like Ollama — but purpose-built and deeply optimized for the Ryzen™ NPU**

> FastFlowLM supports all Ryzen™ AI 300 Series chips with XDNA2 NPUs (Strix, Strix Halo, and Kraken).

---

## 📺 Demo Videos

FastFlowLM vs AMD’s official stack — **real-time speedup and power efficiency**: 

- Same prompt (length: 1835 tokens), same model (LLaMA 3.2 1B model; weights int4; activation bf16), running on the same machine (AMD Ryzen AI 5 340 NPU with 16 GB SO-DIMM DDR5 5600 MHz memory)
- Real-time CPU, iGPU, NPU usage, and power consumption shown (Windows task manager + HWINFO)

<table>
  <tr>
    <td valign="top">
      <h3>🔹 FastFlowLM vs Ryzen AI SW 1.4 (NPU-only)</h3>
      <a href="https://www.youtube.com/watch?v=kv31FZ_q0_I">
        <img src="https://img.youtube.com/vi/kv31FZ_q0_I/0.jpg" alt="Demo: FastFlowLM vs OGA">
      </a>
    </td>
    <td valign="top">
      <h3>🔹 FastFlowLM vs Ryzen AI SW 1.4 (Hybrid)</h3>
      <a href="https://www.youtube.com/watch?v=PFjH-L_Kr0w">
        <img src="https://img.youtube.com/vi/PFjH-L_Kr0w/0.jpg" alt="Demo: FastFlowLM vs GAIA">
      </a>
    </td>
  </tr>
</table>

---

## ⚡ Quick Start

A packaged Windows installer is available here: [**flm-setup.exe**](https://github.com/FastFlowLM/FastFlowLM/releases/download/v0.1.2/flm-setup-v0.1.2.exe). For more details (available models, etc.), see the [release notes](https://github.com/FastFlowLM/FastFlowLM/releases/).

After installation, open **PowerShell**.

To run a model in terminal (Interactive Mode):
```
flm run llama3.2:1B
```
> Requires internet access to HuggingFace to pull (download) the optimized model kernel if not already present. The model will be downloaded to the folder: ``C:\Users\<USER>\Documents\flm\models\``. ⚠️ If HuggingFace is not directly accessible in your region, you can manually download the model and place it in this directory.

To start the local REST API server (Server Mode):
```
flm serve llama3.2:1B
```
> The model tag (e.g., `llama3.2:1B`) sets the initial model, which is optional. If another model is requested, FastFlowLM will automatically switch to it. Local server is on port 11434 (default).

---

## 🧠 Local AI on Your NPU

FastFlowLM makes it easy to run modern LLMs locally with:
- ⚡ High performance and low power
- 🧰 Simple CLI and API
- 🔐 Fully private and offline

No model rewrites, no tuning — it just works.

---

## ✅ Features

- **Runs fully on AMD Ryzen™ NPU** — no GPU or CPU load  
- **CLI-first developer flow** — like Ollama, but optimized for NPU  
- **Support for long context windows** — up to 128k tokens (e.g., LLaMA 3.1/3.2)  
- **No low-level tuning required** — You focus on your app, we handle the rest

---

## ⚡ Performance

Compared to AMD Ryzen™ AI Software 1.4 (GAIA or Lemonade):

### LLM Decoding Speed (TPS: Tokens per Second)
- 🚀 Up to **14.2× faster** vs NPU-only baseline  
- 🚀 Up to **16.2× faster** vs hybrid iGPU+NPU baseline

### Power Efficiency
- 🔋 Up to **2.66× more efficient in decoding** vs NPU-only  
- 🔋 Up to **11.38× more efficient in decoding** vs hybrid  
- 🔋 Up to **3.4× more efficient in prefill** vs NPU-only or hybrid

### Latency
- ⏱️ **Matches or exceeds** TTFT (Time to First Token) of NPU-only or hybrid mode

### Benchmarks
<p style="font-size:85%; margin:0;">
📊 View the detailed results here:
<a href="benchmarks/llama3_results.md" style="text-decoration:none;">
<strong>[Benchmark results]</strong>
</a>
</p>

---

## 🧪 Model Support

```
flm run llama3.1:8B
```
```
flm run llama3.2:1B
```
```
flm run llama3.2:3B
```
```
flm run deepseek
```
*...with more models coming soon.*

---

## 🛠️ Instructions

Documentation and example workflows coming soon. Like Ollama, you can:
- Load and run models locally via CLI (Interactive Mode)
- Integrate into your app via a simple REST API via a local server (Server Mode)
> Compatible with tools like **Microsoft AI Toolkit**, **Open WebUI**, and more.

---

## 🔒 Proprietary Kernel Optimizations

FastFlowLM uses **proprietary low-level kernel code** optimized for AMD Ryzen™ NPUs.  
> These kernels are **not open source**, but are included as binaries for seamless integration.

The rest of the stack — CLI, model runner, orchestration — is open and developer-friendly.

---

## 📝 Licensing & Contact

- 🆓 **Deep-optimized FastFlowLM kernels** are **free for non-commercial use**  
- 💼 **Interested in commercial use?** Email us at [info@fastflowlm.edu](mailto:info@fastflowlm.edu)  
- 📦 **Want to bring your own model?** We can optimize it for FastFlowLM — just reach out!

---

## License

This repository contains two types of components:

- **Open-source components** (e.g., CLI, orchestration code) are released under the **MIT License**.
- **Proprietary binaries** (used for low-level NPU acceleration) are **not included** in this repository and are covered by **separate licensing terms**.

---

💬 **Have feedback or want early access? [Open an issue](https://github.com/fastflowlm/fastflowlm/issues/new) or reach out!**

---

## 🙏 Acknowledgements

- Powered by the advanced **AMD Ryzen™ AI NPU architecture**
- Inspired by the widely adopted [Ollama](https://github.com/ollama/ollama)
- Low-level kernels optimized using the powerful [IRON](https://github.com/Xilinx/mlir-aie/tree/main/programming_guide)+[Riallto](https://riallto.ai/)+[AIE-MLIR](https://github.com/Xilinx/mlir-aie/tree/main/mlir_tutorials)