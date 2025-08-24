---
title: Gemma
nav_order: 4
parent: Models
---

```
flm run gemma3:1b
```

---

```
flm run gemma3:4b
```

📝 **Note:**
> – Vision-enabled version (supports both text and images)
> – In CLI mode, use this command to attach an image:

```
/input "file/to/image.jpg" what is inside this image?
```

---

```
flm run gemma3:270m
```

⚠️ **Warning:** 
> – Experimental in FLM  
> – Limited accuracy; may produce errors  
> – Can loop on long outputs (quirk from Unsloth weights, also seen in LM Studio)  
> – We’re working on improved quantization + hyperparameters  

---