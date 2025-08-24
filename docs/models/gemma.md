---
title: Gemma
nav_order: 4
parent: Models
---

**Google/gemma-3-1b-it**

```
flm run gemma3:1b
```

---

**Google/gemma-3-4b-it**

```
flm run gemma3:4b
```

📝 **Note:**
> – Vision-enabled (supports text + images); in CLI mode, attach an image with:

```
/input "file/to/image.jpg" describe this image?
```

---

**Google/gemma-3-270m**

```
flm run gemma3:270m
```

⚠️ **Warning:** 
> – `gemma3:270m` is Experimental in FLM  
> – Limited accuracy; may produce errors  
> – Can loop on long outputs (quirk from Unsloth weights, also seen in LM Studio)  
> – Experimenting with different quantization + hyperparameters  

---