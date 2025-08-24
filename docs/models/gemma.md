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

> Vision-enabled version (supports both text and images)

> In CLI mode, use this command to attach an image:

```
/input "file/to/image.jpg" what is inside this image?
```

---

```
flm run gemma3:270m
```

> ⚠️ **Note:** This model is still experimental in FLM.  
> – Accuracy is limited and responses may contain errors.  
> – Gemma3:270m can occasionally loop on long outputs (a quirk from the Unsloth weights, also seen in LM Studio).  
> – Exploring better quantization and hyperparameters to improve it.  

---