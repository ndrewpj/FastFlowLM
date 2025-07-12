---
title: Websearch with LangChain
nav_order: 5
parent: Local Server (Server Mode)
---

# 🔍 RAG with Live Web Search + FastFlowLM Summarizer

This project demonstrates how to build a lightweight Retrieval-Augmented Generation (RAG) pipeline that:
- Performs live web search using `ddgs` (DuckDuckGo)
- Summarizes the results using a local LLM via **FastFlowLM**
- Runs fully offline except for the web search

---

## ✅ Prerequisites

- Windows machine
- Python 3.9 or later
- [FastFlowLM](https://github.com/FastFlowLM/FastFlowLM) installed
- FastFlowLM model served (e.g., `llama3.2:1b`)

---

## 🛠️ Step-by-Step Setup

### 1. 🧪 Create the Project Folder

```powershell
mkdir rag_websearch_flm
cd rag_websearch_flm
```

### 2. 🐍 Create a Virtual Environment

```powershell
python -m venv rag_websearch-env
.\rag_websearch-env\Scripts\activate
```

> 💡 If PowerShell blocks script execution:
```powershell
Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
```

---

### 3. 📦 Install Required Packages

```bash
pip install langchain langchain-ollama ddgs
```

---

### 4. 🚀 Launch FastFlowLM

In another terminal:

```bash
flm serve llama3.2:1b
```

This starts your FastFlowLM API at: `http://localhost:11434`

---

### 5. 📝 Create the Script: `websearch_rag.py`

```python
# websearch_rag.py

import warnings
from ddgs import DDGS
from langchain_ollama import OllamaLLM
from langchain.prompts import PromptTemplate

warnings.filterwarnings("ignore", category=ResourceWarning)


def run_web_search(query: str, max_results: int = 5) -> str:
    """Perform a DuckDuckGo search using the ddgs package."""
    print(f"\n🔍 Running web search for: '{query}'")
    results = []

    try:
        with DDGS() as ddgs:
            for r in ddgs.text(query, max_results=max_results):
                title = r.get("title", "No title")
                body = r.get("body", "No description")
                link = r.get("href", "No link")
                results.append(f"• {title}\n  {body}\n  🔗 {link}\n")
    except Exception as e:
        print(f"❌ Error during search: {e}")
        return ""

    return "\n".join(results)


def summarize_with_fastflowlm(search_results: str, model_name="llama3.2:1b") -> str:
    """Summarize search results using FastFlowLM via OllamaLLM."""
    if not search_results.strip():
        return "⚠️ No search results to summarize."

    llm = OllamaLLM(model=model_name, base_url="http://localhost:11434")

    prompt = PromptTemplate.from_template("""
You are a factual and concise research assistant. Summarize the following web search results clearly and accurately.

Search Results:
{search_results}

Summary:
""")

    try:
        summary = (prompt | llm).invoke({"search_results": search_results})
    except Exception as e:
        return f"❌ Error generating summary: {e}"

    return summary.strip()


def main():
    """Main routine for web search + summarization using FastFlowLM."""
    query = "Recent developments in AMD Ryzen AI chips"

    search_output = run_web_search(query, max_results=5)

    print("\n🌐 Raw Web Search Output:\n")
    print(search_output or "⚠️ No results.")

    summary = summarize_with_fastflowlm(search_output)
    print("\n🧠 Summary:\n")
    print(summary)


if __name__ == "__main__":
    main()
```

---

## ▶️ Run the Script

Make sure FastFlowLM is running, then:

```bash
python websearch_rag.py
```

---

## ✅ Expected Output

```text
🔍 Running web search for: 'Recent developments in AMD Ryzen AI chips'

🌐 Raw Web Search Output:

• AMD Ryzen™ AI - Windows PCs with AI Built In
  A new era of AI PCs begins with AMD and Windows. AMD Ryzen™ AI 300 Series processor powered Copilot+ PCs deliver new, transformative AI experiences for your ...
  🔗 https://www.amd.com/en/products/processors/consumer/ryzen-ai.html

• AMD Introduces New Radeon Graphics Cards and Ryzen ...
  May 20, 2025 — AMD Introduces New Radeon Graphics Cards and Ryzen Threadripper Processors at COMPUTEX 2025 · AMD Powers Next-Gen Gaming Infused with AI · Pricing ...
  🔗 https://ir.amd.com/news-events/press-releases/detail/1253/amd-introduces-new-radeon-graphics-cards-and-ryzen-threadripper-processors-at-computex-2025

• AMD launches Ryzen AI 300 and 200 series chips for laptops
  Jan 6, 2025 — AMD has launched its Ryzen AI 300 and Ryzen 200 series of mobile processors at CES 2025 at Las Vegas, debuting a total of 15 new models.
  🔗 https://www.tomshardware.com/pc-components/cpus/amd-launches-ryzen-ai-300-and-200-series-chips-for-laptops

• CES 2025: A New Era of AI and Mobile Performance
  2025 is a year primed for next-level AI and mobile experiences in gaming, productivity, and beyond. Be one of the first to introduce these new advancements to ...
  🔗 https://www.amd.com/en/partner/browse-by-resource/partner-insights-articles/ces-2025-new-ai-era.html

• AMD Takes On Intel, Apple And Nvidia With Ryzen AI Max ...
  Jan 6, 2025 — With up to 16 Zen 5 cores, the Ryzen AI Max series increases the maximum core count by four while upping the number of XDNA 3.5 GPU cores by 16 ...
  🔗 https://www.crn.com/news/components-peripherals/2025/amd-takes-on-intel-apple-and-nvidia-with-ryzen-ai-max-chips


🧠 Summary:

Here are the key factual findings from the search results:

* AMD has introduced new AI-powered Ryzen processors, including the Ryzen 300 and 200 series, which offer improved performance and gaming capabilities.
* AMD is expanding its product line to include Ryzen AI-powered laptops, with 15 new models available at CES 2025.
* AMD is also introducing new AI-enhanced graphics cards, including the Ryzen Threadripper series, which increases core count by up to 16.5.
* The year 2025 is expected to see significant advancements in AI technology, with AMD's Ryzen AI-powered devices poised to revolutionize various aspects of computing.
* AMD is pushing the boundaries of AI performance with its Ryzen 5 and X series, increasing core count by up to 16.5 and GPU cores by up to 16.5.
* AMD is poised to take on Intel and NVIDIA in the AI market with its Ryzen AI-powered devices.
```

---

## 🧠 What’s Happening Behind the Scenes

| Step             | Description                                                  |
|------------------|--------------------------------------------------------------|
| `DDGS()`         | Searches DuckDuckGo without requiring an API key             |
| `OllamaLLM`      | Connects to FastFlowLM via local REST API                    |
| `PromptTemplate` | Defines summarization style and clarity requirement          |
| `prompt | llm`   | Uses LangChain Runnable interface to execute prompt safely   |

---
