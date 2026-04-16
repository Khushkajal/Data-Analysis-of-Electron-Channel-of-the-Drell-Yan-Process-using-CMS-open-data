# Precision Measurement Analysis of the Drell-Yan $e^+e^-$ Channel at $\sqrt{s} = 13$ TeV

[![CERN-ROOT](https://img.shields.io/badge/CERN-ROOT-blue.svg)](https://root.cern/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Physics: CMS-Open-Data](https://img.shields.io/badge/Physics-CMS--Open--Data-orange.svg)](https://opendata.cern.ch/)

## 🔬 Executive Summary
This repository contains a high-performance analysis framework for the study of the Drell-Yan process ($q\bar{q} \to Z/\gamma^* \to e^+e^-$) using **CMS Open Data**. The pipeline executes a systematic event selection (Cut-Flow) to isolate the $Z$ resonance, achieving a final signal purity of **92.63%**. 

A key technical highlight of this repository is the implementation of **Projected MET** and **MVA-based Electron Identification** to suppress irreducible diboson and leptonic $t\bar{t}$ backgrounds.

## 🏗 Repository Architecture
The codebase is structured to facilitate a modular physics workflow, from data slimming to final visualization:

* `branch_extractor_k.C`: A data-slimming utility that extracts essential kinematic branches from NanoAOD-like structures to reduce I/O overhead.
* `branch_type.C`: A diagnostic tool for validating leaf types and tree structures across multi-sample datasets.
* `projected_MET1.C`: **Core Physics Logic.** Implements the Projected MET algorithm to mitigate mismeasured MET in high-recoil environments.
* `Electron_Cut_Flow_Complete.C`: The primary analysis engine. Executes sequential selection cuts and computes absolute and relative efficiencies.
* `Stacked.C` & `superimposed_plots.C`: High-level visualization macros for CMS-style stacked backgrounds and signal comparison plots.

## 🧬 Physics Selection Logic
The analysis targets the intermediate-mass region ($60 < m_{ee} < 120$ GeV). The selection sequence is optimized as follows:

1.  **Object Definition:** Exactly two electrons passing `mvaFall17V2Iso_WP90` (Tight) with opposite charge requirements.
2.  **Acceptance:** $p_T(\text{lead}) > 25$ GeV, $p_T(\text{sub}) > 20$ GeV, $|\eta| < 2.5$.
3.  **Neutrino Rejection:** Application of a $MET < 25$ GeV veto.
4.  **Topology:** Azimuthal separation $\Delta\phi_{ee} > 2.5$ rad and recoil suppression via $p_T^{\ell\ell} < 40$ GeV.

## 📊 Performance Metrics
The effectiveness of the selection strategy is quantified by the rejection of **99.75%** of simulated Standard Model backgrounds.

| Stage | DY Signal Yield | Total Background | Purity (%) |
| :--- | :--- | :--- | :--- |
| **Pre-selection** | 162,046 | 65,818 | 71.12% |
| **Final Selection** | 67,124 | 5,340 | **92.63%** |

## 🚀 Execution Guide
Ensure you have **CERN ROOT 6.28/02** or later installed.

```bash
# 1. Slim the raw data
root -l branch_extractor_k.C

# 2. Add Projected MET branch
root -l 'projected_MET1.C("DYtoLL_M50.root")'

# 3. Run the analysis and generate Yields
root -l Electron_Cut_Flow_Complete.C

# 4. Generate Stacked Plots
root -l Stacked.C
