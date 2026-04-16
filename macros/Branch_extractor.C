/*
Author: Khushpreet Kaur

Description:
This ROOT macro performs branch reduction (slimming) on CMS NanoAOD files for
Drell–Yan (Z → e⁺e⁻) analysis. It selects only relevant branches related to
electrons, MET, jets, and pileup information to significantly reduce file size
and improve processing speed. 

The script loops over multiple input ROOT files (signal and background samples),
creates a TChain for each, disables all branches, and selectively enables only
the required ones. A new slimmed ROOT file is then produced for each dataset.

This is the first step in the analysis workflow, preparing efficient datasets
for further steps such as MET addition, event selection, and final plotting.
How to Use:
1. Open ROOT:
2. Load the macro:
   .L branch_extractor.C
3. Run the function:
   branch_extractor()
4. Input:
   - Place all input ROOT files (DY, WZ, ZZ, WW, TT, SingleTop) 
     in the same directory as this script.
5. Output:
   - For each input file, a slimmed file will be created:
     e.g., DYtoLL_M50.root → DYtoLL_M50_slim.root
6. Notes:
   - Ensure the TTree name is "Events" (NanoAOD format).
   - Modify 'branches_to_keep' if you want to include more variables.
   - This step reduces file size and speeds up further analysis.
*/

#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <iostream>
#include <vector>
#include <string>
R__LOAD_LIBRARY(libPhysics)
#include "TLorentzVector.h"

void branch_extractor() {
    // Define branches to keep
    std::vector<std::string> branches_to_keep = {
        "run", "luminosityBlock", "event",
        "Electron_mvaFall17V2Iso_WP80", "Electron_mvaFall17V2Iso_WP90", "Electron_mvaFall17V2Iso_WPL",
        "Electron_mvaFall17V2noIso_WP80", "Electron_mvaFall17V2noIso_WP90", "Electron_mvaFall17V2noIso_WPL",
        "Electron_charge", "Electron_cutBased", "Electron_jetIdx", "Electron_pdgId", "Electron_photonIdx",
        "Electron_tightCharge", "Electron_phi", "Electron_pt", "Electron_r9", "Electron_scEtOverPt",
        "Electron_mass", "Electron_dxy", "Electron_dxyErr", "Electron_dz", "Electron_dzErr", "Electron_eCorr",
        "Electron_eInvMinusPInv", "Electron_energyErr", "Electron_eta", "Electron_hoe", "nElectron",
        "Electron_dEscaleDown", "Electron_dEscaleUp", "Electron_dEsigmaDown", "Electron_dEsigmaUp", "Electron_deltaEtaSC",
        "Electron_dr03EcalRecHitSumEt", "Electron_dr03HcalDepth1TowerSumEt", "Electron_dr03TkSumPt", "Electron_dr03TkSumPtHEEP",
        "CaloMET_phi", "CaloMET_pt", "GenMET_phi", "GenMET_pt", "MET_MetUnclustEnUpDeltaX", "MET_MetUnclustEnUpDeltaY",
        "MET_covXX", "MET_covXY", "MET_covYY", "MET_phi", "MET_pt", "MET_significance", "MET_sumEt", "MET_sumPtUnclustered",
        "Pileup_sumLOOT", "PuppiMET_phi", "PuppiMET_phiJERDown", "PuppiMET_phiJERUp", "PuppiMET_phiJESDown", "PuppiMET_phiJESUp",
        "PuppiMET_phiUnclusteredDown", "PuppiMET_phiUnclusteredUp", "PuppiMET_pt", "PuppiMET_ptJERDown", "PuppiMET_ptJERUp",
        "PuppiMET_ptJESDown", "PuppiMET_ptJESUp", "PuppiMET_ptUnclusteredDown", "PuppiMET_ptUnclusteredUp", "PuppiMET_sumEt",
        "Electron_genPartIdx", "Flag_goodVertices", "Pileup_nTrueInt", "Pileup_pudensity", "Pileup_gpudensity", "Pileup_nPU",
        "Pileup_sumEOOT", "Pileup_sumLOOT", "fixedGridRhoFastjetCentralChargedPileUp",
        "Jet_area", "Jet_btagCSVV2", "Jet_btagDeepB", "Jet_btagDeepCvB", "Jet_btagDeepCvL",
        "Jet_btagDeepFlavB", "Jet_btagDeepFlavCvB", "Jet_btagDeepFlavCvL", "Jet_btagDeepFlavQG",
        "Jet_eta", "Jet_phi", "Jet_pt", "Jet_mass", "Jet_electronIdx1", "Jet_electronIdx2", "Jet_jetId"
    };

    // Input files (one slimmed output per input file)
    std::vector<std::string> input_files = {
        "DYtoLL_M50.root",
        "WZ.root",
        "ZZ.root",
        "WWTo2L2Nu.root",
        "TTTo2L2Nu.root",
        "SingleTop.root"
    };

    for (const auto &inputName : input_files) {
        std::cout << "Processing " << inputName << std::endl;

        // Build TChain for this single file
        TChain *chain = new TChain("Events");
        chain->Add(inputName.c_str());

        if (chain->GetNtrees() == 0) {
            std::cerr << "Error: No valid trees in " << inputName << std::endl;
            delete chain;
            continue;
        }

        // Disable all branches
        chain->SetBranchStatus("*", 0);

        // Enable only the branches in the list
        for (const auto &branch : branches_to_keep) {
            chain->SetBranchStatus(branch.c_str(), 1);
        }

        // Set small cache size (for memory)
        chain->SetCacheSize(512 * 1024); // 512 kB

        // Build output file name
        std::string outputName = inputName;
        size_t pos = outputName.find(".root");
        if (pos != std::string::npos) {
            outputName.replace(pos, 5, "_slim.root");
        } else {
            outputName += "_slim.root";
        }

        // Create output file
        TFile *fout = TFile::Open(outputName.c_str(), "RECREATE");
        if (!fout || fout->IsZombie()) {
            std::cerr << "Error: Cannot create " << outputName << std::endl;
            delete chain;
            continue;
        }

        fout->cd();

        // Clone tree (only enabled branches)
        TTree *outTree = chain->CloneTree(-1, "fast");
        if (!outTree) {
            std::cerr << "Error: Failed to clone tree from " << inputName << std::endl;
            delete chain;
            fout->Close();
            delete fout;
            continue;
        }

        // Small basket size for memory efficiency
        outTree->SetBasketSize("*", 8000);

        // Write and close
        outTree->Write("", TObject::kOverwrite);
        fout->Close();

        std::cout << "  -> Saved slimmed file: " << outputName << std::endl;

        // Clean up
        delete chain;
        delete fout;
    }
}
