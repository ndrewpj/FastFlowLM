# 🚀 FastFlowLM

**Deploy large language models (LLMs) on AMD Ryzen™ AI NPUs—in minutes.**  
Think **Ollama**, purpose-built for the AMD Ryzen™ NPU architecture.

---

## Overview

**FastFlowLM** is a high-performance runtime for deploying state-of-the-art LLMs—such as **LLaMA**, **DeepSeek**, and others—directly on AMD Ryzen™ NPUs. It is engineered for **low-latency**, **low-power**, and **always-on** AI, leveraging the NPU silicon already embedded in next-gen AI PCs.

---

## Key Features

### 👨‍💻 Developer-Centric Design  
Purpose-built for local AI agent development.  
No low-level NPU knowledge required—FastFlowLM abstracts the hardware layer for you.

### 🧰 Ollama-Like Simplicity  
Offers a streamlined **CLI** and **API**, mirroring the ease of use of Ollama, with the added benefit of **native NPU acceleration**.

### 💻 Zero GPU/CPU Dependency  
All model inference runs entirely on the **AMD Ryzen™ NPU**, freeing CPU and GPU resources for other workloads.

### 📏 Extended Context Support  
Supports long context windows—**up to 128k tokens** on models like **LLaMA 3.1/3.2**—enabling long-form reasoning, multi-turn memory, and RAG workflows without compromise.

---

## ⚡ Performance That Speaks for Itself

Compared to AMD Ryzen™ AI Software 1.4 (GAIA or Lemonade):

### 🚀 LLM Decoding Throughput *(TPS: Tokens per Second)*
- Up to **14.2× faster** vs NPU-only baseline  
- Up to **16.2× faster** vs hybrid iGPU+NPU baseline

### 🔋 Power Efficiency
- Up to **2.66× more efficient** in LLM decoding vs NPU-only  
- Up to **11.38× more efficient** in LLM decoding vs hybrid iGPU+NPU  
- Up to **3.4× more efficient** in LLM prefill vs NPU-only or hybrid

### ⏱️ Latency *(LLM Prefill Speed)*
- **Matches or exceeds** the **Time to First Token (TTFT)** performance of NPU-only and hybrid configurations

---

## Model Support

- ✅ Meta LLaMA 3.1 / 3.2  
- ✅ DeepSeek R1  
- ✅ And more...

---

## Quick Start (Coming Soon)

Full installation guide, API reference, and deployment examples will be published shortly.

---

## License

This project is released under the **MIT License**.  
Proprietary components are distributed in binary form and subject to separate licensing terms.

---

## 🔒 Proprietary Kernel Optimizations

FastFlowLM leverages **proprietary, low-level kernel code** optimized specifically for AMD Ryzen™ NPUs.  
> These performance-critical components are **not open source**, but seamlessly integrated into the runtime for maximum efficiency and security.

The open-source layers include the CLI, model orchestration, and runtime logic—enabling developers to integrate and deploy models without concern for NPU internals.

---

## Acknowledgments

Special thanks to **AMD Ryzen™ AI** engineering teams and the broader open-source community for driving innovation in efficient edge AI.

---


