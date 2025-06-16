# FastFlowLM

Run large language models on AMD Ryzen™ AI NPUs — in minutes.

FastFlowLM is a lightweight runtime for deploying LLMs like LLaMA and DeepSeek directly on AMD’s integrated NPU — no GPU or CPU needed.

**Just like Ollama — but built for Ryzen™.**

---

## 🧠 Local AI on Your NPU

FastFlowLM makes it easy to run modern LLMs locally with:
- ⚡ High performance and low power
- 🧰 Simple CLI and API
- 🔐 Fully private and offline

No drivers, no model rewrites, no tuning — it just works.

---

## ✅ Features

- **Runs fully on AMD Ryzen™ NPU** — no GPU or CPU load  
- **CLI-first developer flow** — like Ollama, but optimized for NPU  
- **Support for long context windows** — up to 128k tokens (e.g., LLaMA 3.1/3.2)  
- **No low-level tuning required** — *Worry about your app, we handle the rest*

---

## ⚡ Performance

Compared to AMD Ryzen™ AI Software 1.4 (GAIA or Lemonade):

### LLM Decoding Speed (TPS: Tokens per Second)
- 🚀 Up to **14.2× faster** vs NPU-only baseline  
- 🚀 Up to **16.2× faster** vs hybrid iGPU+NPU baseline

### Power Efficiency
- 🔋 Up to **2.66× more efficient** vs NPU-only  
- 🔋 Up to **11.38× more efficient** vs hybrid  
- 🔋 Up to **3.4× more efficient in prefill** vs NPU-only or hybrid

### Latency
- ⏱️ **Matches or exceeds** TTFT of NPU-only or hybrid configurations

---

## 🧪 Model Support

FastFlowLM supports many of today’s best open models:
- LLaMA 3.1 / 3.2  
- DeepSeek R1  
- Phi-2 / Phi-3  
*...with more models coming soon.*

---

## 🛠️ Getting Started

Documentation, install guides, and example workflows coming soon.  
You’ll be able to:
- Load and run models locally via CLI
- Integrate into your app via a simple HTTP API

---

## 🔒 Proprietary Kernel Optimizations

FastFlowLM uses **proprietary low-level kernel code** optimized for AMD Ryzen™ NPUs.  
> These kernels are **not open source**, but are included as binaries for seamless integration.

The rest of the stack — CLI, model runner, orchestration — is open and developer-friendly.

---

## License

Open components are released under the **MIT License**. Proprietary binaries are subject to separate terms.

---

💬 **Have feedback or want early access? [Open an issue](#) or reach out!**
