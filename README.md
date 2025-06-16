# 🚀 FastFlowLM

**Deploy large language models (LLMs) on AMD Ryzen™ AI NPUs—in minutes.**

Think **Ollama**, but purpose-built for the great **AMD Ryzen™ NPUs**.

---

## 🔧 What is FastFlowLM?

**FastFlowLM** is a high-performance runtime for deploying popular LLMs like **LLaMA** and **DeepSeek-R1** directly on AMD Ryzen™ NPUs. It's hardware-optimized for lightning-fast, low-power, private, always-on AI—running entirely on the silicon already in your AI PC.

---

## 👨‍💻 Built for Developers Building Local AI Agents

### 🧠 Zero Low-Level Tuning
You don't need to know anything about NPU internals—just run your model. FastFlowLM takes care of all the hardware optimization for you.

### 🧰 Familiar Workflow
FastFlowLM offers the **CLI and API simplicity** developers love from tools like Ollama, but optimized for **AMD Ryzen™ NPU** execution.

### 💻 No GPU or CPU Required
Models run **entirely on the Ryzen™ NPU**, leaving your CPU and GPU free for other tasks.

### 📏 Full Context Window Support  
Supports full-context windows—**up to 128k tokens** on models like **LLaMA 3.1/3.2**—ideal for long-form reasoning and RAG applications.

---

## ⚡ Performance That Speaks for Itself

Compared to AMD Ryzen™ AI Software 1.4 (GAIA or Lemonade):

### 🚀 LLM Decoding Speed *(TPS: Tokens per Second)*
- Up to **14.2× faster** in LLM decoding *(vs NPU-only baseline)*
- Up to **16.2× faster** in LLM decoding *(vs hybrid iGPU+NPU baseline)*

### 🔋 Power Efficiency
- Up to **2.66× more efficient** in LLM decoding *(vs NPU-only baseline)*
- Up to **11.38× more efficient** in LLM decoding *(vs hybrid iGPU+NPU baseline)*
- Up to **3.4× more efficient** in LLM prefill *(vs NPU-only or hybrid baseline)*

### ⏱️ Latency *(LLM Prefill Speed)*
- **Matches or exceeds** the **Time to First Token (TTFT)** of **NPU-only** or **hybrid** baselines

---

## 🚀 Get Started

Coming soon: setup instructions, model loading guide, and API examples.

---

## 🧪 Supported Models

- Meta LLaMA 2 / 3
- DeepSeek-V2 / R1
- Phi-2 / Phi-3
- Command-R / Zephyr
- And more...

---

## 📄 License

MIT License

---

## 🙌 Acknowledgments

Thanks to AMD for Ryzen™ AI hardware innovation, and the open-source community for continued support.

---
