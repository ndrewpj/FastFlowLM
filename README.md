<p align="center">
  <a href="https://www.fastflowlm.com" target="_blank">
    <img src="assets/logo.png" alt="FastFlowLM Logo" width="200"/>
  </a>
</p>


Run large language models on AMD Ryzen™ AI NPUs — in minutes.

FastFlowLM is a lightweight runtime for deploying LLMs like LLaMA and DeepSeek directly on AMD’s integrated NPU — no GPU or CPU needed.

**Just like Ollama — but built for the great Ryzen™ NPU.**

---

## 📺 Demo Videos

FastFlowLM vs AMD’s official stack — **real-time speedup and power efficiency**: 

- Same prompt (length: 1835 tokens), same model (LLaMA 3.2 1B model), running on the same machine (AMD Ryzen AI 5 340 NPU with 16GB SO-DIMM DDR5 5600 MHz memory)
- Real-time CPU, iGPU, NPU usage, and power consumption shown (Windows task manager + HWINFO)

### 🔹 FastFlowLM vs AMD Ryzen AI Software 1.4 (NPU-only via OGA)

[![Demo: FastFlowLM vs OGA](https://img.youtube.com/vi/kv31FZ_q0_I/0.jpg)](https://www.youtube.com/watch?v=kv31FZ_q0_I)

### 🔹 FastFlowLM vs AMD Ryzen AI Software 1.4 (Hybrid iGPU+NPU via GAIA)

[![Demo: FastFlowLM vs GAIA](https://img.youtube.com/vi/PFjH-L_Kr0w/0.jpg)](https://www.youtube.com/watch?v=PFjH-L_Kr0w)

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
- **No low-level tuning required** — Worry about your app, we handle the rest

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

### ⚡ Performance and Efficiency Benchmarks

#### 🚀 Decoding Speed (Tokens per Second; three model sizes and various context lengths)

| **Model**        | **Hardware**       | **1k** | **2k** | **4k** | **8k** | **16k** | **32k** | **64k** | **128k** |
|------------------|--------------------|--------|--------|--------|--------|---------|---------|---------|----------|
| **LLaMA 3.2 1B**  | NPU (FastFlowLM)    | 36.7   | 35.8   | 33.2   | 29.6   | 24.0    | 17.7    | 11.5    | 6.8      |
|                  | iGPU                | 28.7   | 19.0   | 10.9   | 6.0    | 3.2     | 1.6     | 0.8     | 0.4      |
|                  | CPU                 | 54.6   | 52.6   | 42.3   | 34.1   | 24.4    | 14.8    | 8.4     | 4.5      |
| **LLaMA 3.2 3B**  | NPU (FastFlowLM)    | 16.1   | 15.4   | 14.3   | 12.4   | 9.9     | 7.0     | 4.4     | 2.6      |
|                  | iGPU                | 23.2   | 18.8   | 14.0   | 9.2    | 5.5     | 3.0     | 1.6     | 0.8      |
|                  | CPU                 | 22.6   | 21.3   | 17.5   | 14.1   | 9.4     | 6.1     | 3.5     | 1.9      |
| **LLaMA 3.1 8B**  | NPU (FastFlowLM)    | 7.6    | 7.4    | 7.1    | 6.5    | 5.7     | 4.4     | 3.1     | 2.0      |
|                  | iGPU                | 11.3   | 9.9    | 7.7    | 5.4    | 3.4     | 1.9     | 1.0     | 0.5      |
|                  | CPU                 | 10.3   | 7.7    | 7.6    | 6.7    | 5.8     | 3.3     | 2.0     | 1.1      |

> 📝 *Note: All results above were obtained using LM Studio for CPU and iGPU backends. Official AMD Ryzen AI Software 1.4 is excluded as it supports a maximum context length of only 2048 tokens.*

#### 🔋 Power Consumption (Watts) During Decoding

| **Model**        | **Method**       | **CPU** | **NPU** | **iGPU** | **Total Power (W)** | **Efficiency Gain** |
|------------------|------------------|--------:|--------:|--------:|---------------------:|----------------------:|
| **LLaMA 3.2 1B**  | NPU (FastFlowLM)  | 0.07    | 1.57    | 0       | **1.64**             | –                    |
|                  | NPU (Ryzen AI SW) | 0.85    | 2.05    | 0       | 2.90                 | 1.77×                |
|                  | iGPU              | 0.12    | 0       | 14      | 14.12                | 8.61×                |
|                  | CPU               | 4.90    | 0       | 0       | 4.90                 | 2.99×                |
| **LLaMA 3.2 3B**  | NPU (FastFlowLM)  | 0.06    | 1.33    | 0       | **1.39**             | –                    |
|                  | NPU (Ryzen AI SW) | 0.95    | 2.05    | 0       | 3.00                 | 2.16×                |
|                  | iGPU              | 0.11    | 0       | 13      | 13.11                | 9.43×                |
|                  | CPU               | 4.50    | 0       | 0       | 4.50                 | 3.24×                |
| **LLaMA 3.1 8B**  | NPU (FastFlowLM)  | 0.07    | 1.17    | 0       | **1.24**             | –                    |
|                  | NPU (Ryzen AI SW) | 0.80    | 2.50    | 0       | 3.30                 | 2.66×                |
|                  | iGPU              | 0.11    | 0       | 14      | 14.11                | 11.38×               |
|                  | CPU               | 4.50    | 0       | 0       | 4.50                 | 3.63×                |

#### ⚙️ Power Consumption (Watts) During Prefill

| **Model**        | **Method**         | **CPU** | **NPU** | **iGPU** | **Total Power (W)** | **Efficiency Gain** |
|------------------|--------------------|--------:|--------:|--------:|---------------------:|----------------------:|
| **LLaMA 3.2 1B**  | NPU (FastFlowLM)   | 0.31    | 0.90    | 0.00    | **1.21**             | –                    |
|                  | NPU (Ryzen AI SW)  | 0.96    | 2.05    | 0.00    | 3.01                 | 2.49×                |
|                  | iGPU               | 2.70    | 0.00    | 10.00   | 12.70                | 10.50×               |
| **LLaMA 3.2 3B**  | NPU (FastFlowLM)   | 0.20    | 0.90    | 0.00    | **1.10**             | –                    |
|                  | NPU (Ryzen AI SW)  | 1.06    | 2.10    | 0.00    | 3.16                 | 2.87×                |
|                  | iGPU               | 2.10    | 0.00    | 11.00   | 13.10                | 11.91×               |
| **LLaMA 3.1 8B**  | NPU (FastFlowLM)   | 0.23    | 0.86    | 0.00    | **1.09**             | –                    |
|                  | NPU (Ryzen AI SW)  | 1.20    | 2.50    | 0.00    | 3.70                 | 3.39×                |
|                  | iGPU               | 1.40    | 0.00    | 14.00   | 15.40                | 14.13×               |

---

## 🧪 Model Support

FastFlowLM supports many of today’s best open models:
- LLaMA 3.1 / 3.2  
- DeepSeek R1  
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

## 📝 Licensing & Contact

- 🆓 **Deep-optimized FastFlowLM models** are **free for non-commercial use**  
- 💼 **Interested in commercial licensing?** Email us at [info@fastflowlm.edu](mailto:info@fastflowlm.edu)  
- 📦 **Want to bring your own model?** We can optimize it for FastFlowLM — just reach out!

---

## License

Open components are released under the **MIT License**. Proprietary binaries are subject to separate terms.

---

💬 **Have feedback or want early access? [Open an issue](#) or reach out!**
