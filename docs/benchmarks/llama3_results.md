---
title: LLaMA 3 Benchmark Results
parent: Benchmarks
nav_order: 1
---

## ⚡ Performance and Efficiency Benchmarks

This section reports the decoding speed and power usage of LLMs on different hardware: NPU (FastFlowLM), NPU (Ryzen AI Software), iGPU (LM Studio), and CPU (LM Studio).

> **Note:** Results are based on FastFlowLM v0.1.3. Newer versions may deliver up to 20% improved performance.

---

### 🚀 Decoding Speed (Tokens per Second, or TPS, at different sequence lengths)

| **Model**        | **Hardware**       | **1k** | **2k** | **4k** | **8k** | **16k** | **32k** | **64k** | **128k** |
|------------------|--------------------|--------|--------|--------|--------|---------|---------|---------|----------|
| **LLaMA 3.2 1B**  | NPU (FastFlowLM)    | 36.7   | 35.8   | 33.2   | 29.6   | 24.0    | 17.7    | 11.5    | 6.8      |
|                  | NPU (Ryzen AI SW)   | 18.6   | 14.9   | *NA*   | *NA*    | *NA*     | *NA*     | *NA*     | *NA*      |
|                  | iGPU                | 28.7   | 19.0   | 10.9   | 6.0    | 3.2     | 1.6     | 0.8     | 0.4      |
|                  | CPU                 | 54.6   | 52.6   | 42.3   | 34.1   | 24.4    | 14.8    | 8.4     | 4.5      |
| **LLaMA 3.2 3B**  | NPU (FastFlowLM)    | 16.1   | 15.4   | 14.3   | 12.4   | 9.9     | 7.0     | 4.4     | 2.6      |
|                  | NPU (Ryzen AI SW)   | 9.0   | 6.1   | *NA*   | *NA*    | *NA*     | *NA*     | *NA*     | *NA*      |
|                  | iGPU                | 23.2   | 18.8   | 14.0   | 9.2    | 5.5     | 3.0     | 1.6     | 0.8      |
|                  | CPU                 | 22.6   | 21.3   | 17.5   | 14.1   | 9.4     | 6.1     | 3.5     | 1.9      |
| **LLaMA 3.1 8B**  | NPU (FastFlowLM)    | 7.6    | 7.4    | 7.1    | 6.5    | 5.7     | 4.4     | 3.1     | 2.0      |
|                  | NPU (Ryzen AI SW)   | 6.3   | 4.6    | *NA*   | *NA*    | *NA*     | *NA*     | *NA*     | *NA*      |
|                  | iGPU                | 11.3   | 9.9    | 7.7    | 5.4    | 3.4     | 1.9     | 1.0     | 0.5      |
|                  | CPU                 | 10.3   | 7.7    | 7.6    | 6.7    | 5.8     | 3.3     | 2.0     | 1.1      |

> 🔎 Note: The official release of Ryzen AI Software 1.4 limits context length to 2,048 tokens, thus "*NA*" is used in the table (NPU-only mode). The hybrid mode of Ryzen AI Software 1.4 uses iGPU for decoding. Its performance is simliar to iGPU (LM Studio). Also, it limits context length to 2,048, thus, we did not include hybrid mode for comparison.

---

### 🔋 Power Consumption (Watts) During Decoding

| **Model**        | **Method**         | **CPU** | **NPU** | **iGPU** | **Total Power (W)** | **Efficiency Gain** |
|------------------|--------------------|--------:|--------:|--------:|---------------------:|----------------------:|
| **LLaMA 3.2 1B**  | NPU (FastFlowLM)   | 0.07    | 1.57    | 0       | **1.64**             | –                    |
|                  | NPU (Ryzen AI SW)  | 0.85    | 2.05    | 0       | 2.90                 | 1.77×                |
|                  | iGPU               | 0.12    | 0       | 14.00   | 14.12                | 8.61×                |
|                  | CPU                | 4.90    | 0       | 0       | 4.90                 | 2.99×                |
| **LLaMA 3.2 3B**  | NPU (FastFlowLM)   | 0.06    | 1.33    | 0       | **1.39**             | –                    |
|                  | NPU (Ryzen AI SW)  | 0.95    | 2.05    | 0       | 3.00                 | 2.16×                |
|                  | iGPU               | 0.11    | 0       | 13.00   | 13.11                | 9.43×                |
|                  | CPU                | 4.50    | 0       | 0       | 4.50                 | 3.24×                |
| **LLaMA 3.1 8B**  | NPU (FastFlowLM)   | 0.07    | 1.17    | 0       | **1.24**             | –                    |
|                  | NPU (Ryzen AI SW)  | 0.80    | 2.50    | 0       | 3.30                 | 2.66×                |
|                  | iGPU               | 0.11    | 0       | 14.00   | 14.11                | 11.38×               |
|                  | CPU                | 4.50    | 0       | 0       | 4.50                 | 3.63×                |

---

### 🔋 Power Consumption (Watts) During Prefill

| **Model**        | **Method**         | **CPU** | **NPU** | **iGPU** | **Total Power (W)** | **Efficiency Gain** |
|------------------|--------------------|--------:|--------:|--------:|---------------------:|----------------------:|
| **LLaMA 3.2 1B**  | NPU (FastFlowLM)   | 0.31    | 0.90    | 0    | **1.21**             | –                    |
|                  | NPU (Ryzen AI SW)  | 0.96    | 2.05    | 0    | 3.01                 | 2.49×                |
|                  | iGPU               | 2.70    | 0    | 10.00   | 12.70                | 10.50×               |
| **LLaMA 3.2 3B**  | NPU (FastFlowLM)   | 0.20    | 0.90    | 0    | **1.10**             | –                    |
|                  | NPU (Ryzen AI SW)  | 1.06    | 2.10    | 0    | 3.16                 | 2.87×                |
|                  | iGPU               | 2.10    | 0    | 11.00   | 13.10                | 11.91×               |
| **LLaMA 3.1 8B**  | NPU (FastFlowLM)   | 0.23    | 0.86    | 0    | **1.09**             | –                    |
|                  | NPU (Ryzen AI SW)  | 1.20    | 2.50    | 0    | 3.70                 | 3.39×                |
|                  | iGPU               | 1.40    | 0    | 14.00   | 15.40                | 14.13×               |

> 🔎 Note: CPU is not commonly used for prefill and is excluded from this table.
