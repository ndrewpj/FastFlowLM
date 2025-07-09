---
title: Open WebUI + FastFlowLM
nav_order: 2
parent: Local Server (Server Mode)
---

# 🧩 Run Open WebUI with FastFlowLM (Windows, YAML Method)

This guide walks you through using `docker-compose.yaml` to run Open WebUI connected to a local FastFlowLM instance on Windows.

---

## ✅ Prerequisites

1. [Docker Desktop for Windows](https://www.docker.com/products/docker-desktop)
   - During installation, enable **WSL2 backend**
   - Reboot if prompted

2. [FastFlowLM](../../install.md)

---

## 📁 Step 1: Create Project Folder

Open PowerShell and run:

```powershell
mkdir open-webui && cd open-webui
```

This creates a clean workspace for your Docker setup.

---

## 📝 Step 2: Create `docker-compose.yaml`

Launch Notepad:

```powershell
notepad docker-compose.yaml
```

Paste the following:

```yaml
services:
  open-webui:
    image: ghcr.io/open-webui/open-webui:main
    container_name: open-webui
    ports:
      - 3000:8080
    environment:
      - OPENAI_API_BASE_URL=http://host.docker.internal:11434
      - WEBUI_AUTH=false
      - WEBUI_SECRET_KEY=dummysecretkey
    volumes:
      - openwebui-data:/app/backend/data
    restart: unless-stopped

volumes:
  openwebui-data:
```

> `OPENAI_API_BASE_URL=http://host.docker.internal:11434` connects Open WebUI to local FastFlowLM  
> `WEBUI_AUTH=false` disables login (optional)

---

## ▶️ Step 3: Launch the Open WebUI Container (in PowerShell)

```powershell
docker compose up -d
```

This starts the container in detached mode.  
You can check logs with:

```powershell
docker logs -f open-webui
```

---

## 🌐 Step 4: Access the WebUI (in Browser)

Open browser and go to:  
**http://localhost:3000**

You should now see the Open WebUI interface.

---

## 🧪 Step 5: Serve FastFlowLM with Model

```powershell
flm serve llama3.2:1B
```

You can now use `FastFlowLM` directly in Open WebUI.

---

## 🧼 Step 6: Stop or Clean Up (in PowerShell)

```powershell
docker compose stop
```

To **remove** it completely:

```powershell
docker compose down
```

This also removes the container but keeps persistent volume data.

or 

```powershell
docker compose down -v
```

This removes the container and persistent volume data.

---

## 🧠 Notes

- Want login? Set `WEBUI_AUTH=true`
- You must keep FastFlowLM server running
- `http://host.docker.internal:11434` bridges from Docker container to your native Windows host FastFlowLM API
- For persistent chat history, the volume `openwebui-data` stores user data

---

