# MasterArbeit Game Prototype - "LongLiveMadness"

## Overview

A turn-based role-playing game prototype built in Unreal Engine 5.5, featuring LLM-driven NPC companions running
entirely on local hardware via [llama.cpp](https://github.com/ggml-org/llama.cpp). The LLM integration is handled by the [LlamaRunner](https://github.com/eh-dozo/LlamaRunner-vMasterArbeit) plugin, included as a git submodule.

## Academic Context

This project was developed as part of a Master's thesis at **[HTW  Berlin](https://www.htw-berlin.de/) (Hochschule fur Technik und Wirtschaft Berlin)**, Faculty 4.

| | |
| --- | --- |
| **Thesis** | *Beyond Scripted-Intelligence: Case Study of Transformer Models and Emergent Behaviors in Interactive Virtual Environments* |
| **Programme** | International Media Informatics (M.Sc.) |
| **Author** | Eliot Hoff |
| **1st Supervisor** | Prof. Dr. Tobias Lenz |
| **2nd Supervisor** | Prof. Gefei Zhang |
| **Date** | February 2026 |

The thesis investigates whether LLM-Based Game Agents can deploy on consumer hardware and produce believable
human NPC behavior. Five instruction-tuned models (1-4B  parameters) were evaluated with multiple quantization configurations, and an exploratory participant study (N=18) assessed technology acceptance, immersion, and
behavioral patterns.

## Requirements

- **OS:** Windows
- **Engine:** Unreal Engine 5.5
- **GPU:** CUDA-capable (NVIDIA) -- the project **cannot** be built or run without CUDA support
- **IDE:** Visual Studio 2022 (with C++ Game Development workload and Windows 11 SDK)
- **Model:** A `.gguf` model file is required but not included in this repository due to git LFS limitations.
  Contact the author to obtain the model files used in the thesis.

## Setup

Clone the repository **with  submodules**:
```bash
git clone --recurse-submodules https://github.com/eh-dozo/MasterArbeit.git
```

If you already cloned without the plugin git submodule:
```bash
git submodule update --init --recursive
```

Project files generation and build depends on your IDE (Visual Studio, Rider, ...).

## Running

### Unreal Editor settings at: `Project Settings > Plugins > Llama Runner > ...`

Open `MasterArbeit.uproject` in Unreal Editor and Play-In-Editor (Alt+P). The LlamaRunner plugin requires:

- `.gguf` model file configured in **... > General > Path to model file** 
- `.gbnf` grammar file (can be found in `./Plugins/LlamaRunner/Resources/Grammar/*.gbnf`) (latest is `turn_behavior_04.gbnf`) configured in **... > Sampler > Path to grammar file**

before the [ggml](https://github.com/ggml-org/ggml) (i.e., [llama.cpp](https://github.com/ggml-org/llama.cpp)) backend works.

## License

This project is licensed under the [GNU General Public License v3.0](LICENSE).

The [LlamaRunner](https://github.com/eh-dozo/LlamaRunner-vMasterArbeit) submodule carry its own (same) license terms.
