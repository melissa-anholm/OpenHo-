# AI Development Considerations

**Document Purpose:** Outline options for implementing AI opponents in OpenHo, including libraries, tools, and architectural approaches.

**Status:** Planning/Design phase

---

## AI Libraries & Tools Overview

### For Training (Offline)

**Python-based (Recommended for Training):**

#### 1. PyTorch
- **Pros:** Excellent for GANs, good documentation, flexible, industry standard
- **Cons:** Large dependency, Python-only for training
- **Best for:** GAN training, behavioral cloning
- **Export:** Can export trained models to formats that C++ can load

#### 2. TensorFlow/Keras
- **Pros:** Good for neural networks, production-ready, can export to C++
- **Cons:** Heavier than PyTorch, steeper learning curve
- **Best for:** General deep learning
- **Export:** TensorFlow Lite, SavedModel format

#### 3. JAX
- **Pros:** Lightweight, functional programming style, great for research
- **Cons:** Smaller ecosystem, less beginner-friendly
- **Best for:** Research and experimentation

**Recommendation for Training:** **PyTorch**
- Best GAN support
- Easiest to learn
- Models can be exported to C++
- Large community for game AI

### For Deployment (In-Game)

This is the critical part - you need something that runs fast in C++.

#### Option 1: ONNX (Open Neural Network Exchange)
- Train in PyTorch/TensorFlow
- Export to ONNX format
- Load in C++ using ONNX Runtime
- **Pros:** Format-agnostic, good C++ support, optimized inference
- **Cons:** Need to learn ONNX export process

#### Option 2: TensorFlow Lite (TFLite)
- Train in TensorFlow
- Export to TFLite format
- Load in C++ using TFLite interpreter
- **Pros:** Lightweight, optimized for inference
- **Cons:** Tied to TensorFlow, less flexible

#### Option 3: LibTorch (PyTorch C++ API)
- Train in PyTorch
- Export as TorchScript
- Load directly in C++ using LibTorch
- **Pros:** Native PyTorch support, no conversion needed
- **Cons:** Large library, adds significant dependency

#### Option 4: Custom C++ Implementation
- Train in Python, export weights
- Implement inference in C++ manually
- **Pros:** Minimal dependencies, full control
- **Cons:** More work, error-prone, hard to maintain

**Recommendation for Deployment:** **ONNX + ONNX Runtime**
- Framework-agnostic (train in PyTorch, use anywhere)
- Good C++ support
- Optimized inference
- Smaller footprint than LibTorch
- Industry standard

### For Phase 2b (RandomAI)

**No ML needed!** Just C++:
- Simple random number generation
- Action enumeration
- Basic validation
- Logging

This is pure C++ code, no external dependencies needed.

---

## Proposed Architecture

```
┌─────────────────────────────────────────────────────────┐
│                  Training Pipeline                       │
├─────────────────────────────────────────────────────────┤
│                                                           │
│  Python Scripts (Offline, before release)               │
│  ┌──────────────────────────────────────────────────┐   │
│  │ 1. Collect expert gameplay data                  │   │
│  │ 2. Train behavioral cloning model (PyTorch)      │   │
│  │ 3. Train GAN (Generator + Discriminator)         │   │
│  │ 4. Export trained models to ONNX format          │   │
│  └──────────────────────────────────────────────────┘   │
│                        │                                  │
│                        ▼                                  │
│  Trained Models (ONNX files)                            │
│  ├── behavior_cloning_model.onnx                        │
│  ├── generator_easy.onnx                                │
│  ├── generator_medium.onnx                              │
│  └── generator_hard.onnx                                │
│                                                           │
└─────────────────────────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────┐
│              Game Engine (C++)                           │
├─────────────────────────────────────────────────────────┤
│                                                           │
│  Phase 2b: RandomAI (Pure C++)                          │
│  ├── Random action selection                            │
│  ├── Action validation                                  │
│  └── Logging                                            │
│                                                           │
│  Phase 4: Sophisticated AI (C++ + ONNX Runtime)         │
│  ├── Load ONNX models at startup                        │
│  ├── Influence map calculations                         │
│  ├── Feed game state to neural network                  │
│  ├── Get move predictions from network                  │
│  └── Execute best move                                  │
│                                                           │
└─────────────────────────────────────────────────────────┘
```

---

## Recommended Stack

### Training (Offline)
- **Language:** Python 3.9+
- **Framework:** PyTorch
- **Libraries:**
  - `torch` - Neural networks
  - `numpy` - Numerical computing
  - `matplotlib` - Visualization
  - `pandas` - Data handling
  - `onnx` - Model export

### Deployment (In-Game)
- **Language:** C++17
- **Library:** ONNX Runtime
- **Installation:** Via package manager or source

### Integration
- **Trained models:** Bundled with game as ONNX files
- **Model loading:** At game startup
- **Inference:** During AI turn processing

---

## Implementation Steps

### Phase 2b (RandomAI)
1. Create `AIPlayer` base class in C++
2. Implement `RandomAI` subclass (pure C++)
3. No external dependencies needed

### Phase 4a (Influence Maps)
1. Implement influence map calculations in C++
2. Create heuristic decision-making framework
3. Still no ML needed yet

### Phase 4b (Behavioral Cloning)
1. Create Python training script
2. Collect expert gameplay data
3. Train neural network in PyTorch
4. Export to ONNX format

### Phase 4c (GAN Training)
1. Design generator and discriminator in PyTorch
2. Implement self-play training loop in Python
3. Export trained models to ONNX
4. Test models in C++ game engine

### Phase 4d (Integration)
1. Load ONNX models in C++ using ONNX Runtime
2. Implement inference in AI decision-making
3. Test different difficulty levels

---

## Key Decisions to Make

### 1. Training Environment
- Should training happen on your machine?
- Or set up cloud training (AWS, Google Cloud)?

### 2. Model Complexity
- Any constraints on model size?
- Affects inference speed and memory usage

### 3. Deployment
- Should trained models be bundled with the game?
- Or downloaded separately?

### 4. Version Control
- Should we version-control trained models?
- Or just the training scripts?

### 5. Difficulty Levels
- How many difficulty levels?
- Different models per difficulty, or same model with different parameters?

### 6. Personality
- Should different AI opponents have distinct play styles?
- Or all play the same way at different skill levels?

---

## Dependencies Summary

### Phase 2b (RandomAI)
- **C++ only** - No external dependencies
- Estimated implementation: 1-2 days

### Phase 4 (Sophisticated AI)
- **Training:** Python 3.9+, PyTorch, ONNX
- **Deployment:** C++17, ONNX Runtime
- **Estimated implementation:** High effort (significant ML/AI work)

---

## Next Steps

1. Review and refine this document based on your preferences
2. Answer the key decisions listed above
3. Create directory structure for training scripts
4. Begin Phase 2b (RandomAI) implementation
5. Gather requirements for behavioral cloning data collection

---

**Document Status:** Planning phase - awaiting decisions on key questions above
