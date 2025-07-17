---
title: Local Server (Server Mode)
parent: Instructions
nav_order: 2
has_children: true
---

# Local Server (Server Mode)

To activate "server mode," simply open PowerShell and enter:

```bash
flm serve llama3.2:1b
```

You can choose to change the server port (default is 11434) by going to **System Properties** → **Environment Variables**, then modifying the value of `FLM_SERVE_PORT`.

> ⚠️ **Be cautious**: If you update this value, be sure to change any higher-level port settings in your application as well to ensure everything works correctly.
