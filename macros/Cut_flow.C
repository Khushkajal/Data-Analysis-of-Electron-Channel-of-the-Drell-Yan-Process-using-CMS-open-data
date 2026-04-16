/*
Author: Khushpreet Kaur

Description:
This ROOT macro performs a complete cut-flow analysis for Drell–Yan (Z → e⁺e⁻)
and background processes using CMS NanoAOD datasets. It applies a sequence of
physics-motivated selection criteria to isolate a clean di-electron signal and
quantify background rejection.

The analysis includes:
- Selection of exactly two tight electrons (WP90 ID)
- Opposite charge requirement
- Kinematic cuts on pT and pseudorapidity (η)
- Invariant mass window (60–120 GeV) for Z boson
- MET (or projected MET) cut for background suppression
- Dilepton transverse momentum (pT_ll) cut
- Angular separation (Δφ) between leptons

For each stage, the macro counts surviving events and computes efficiencies.
It processes multiple datasets (signal + backgrounds) in one run and saves
results into a text file for easy comparison.
Output:
- Printed cut-flow table in terminal
- Saved summary in "cutflow.txt"

How to Use:
1. Open ROOT: root -l
2. Load the macro: .L Electron_Cut_Flow_Complete.C
3. Run full analysis: Electron_Cut_Flow_Complete()
4. Input:- Place all ROOT files in the same directory.
5. Output:
   - Terminal: cut-flow for each file
   - File: "cutflow.txt" containing all results
6. Notes:
   - Modify file list inside 'Electron_Cut_Flow_Complete()' if needed.
   - Uses projected_MET if available, otherwise MET_pt.
   - This is the final step of the analysis workflow.
 */

#include <TSystem.h>
R__LOAD_LIBRARY(libPhysics)
#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <iostream>
#include <cmath>
#include <TLorentzVector.h>
#include <iomanip>
#include <vector>
#include <fstream>

using namespace std;

void Electron_Cut_Flow(const char* filename = "DYtoLL_ext1.root") {
    TFile *file = TFile::Open(filename);
    if (!file || file->IsZombie()) {
        cerr << "Error opening " << filename << endl;
        return;
    }

    TTree *tree = (TTree*)file->Get("Events");
    if (!tree) {
        cerr << "No Events tree in " << filename << endl;
        delete file;
        return;
    }

    // Declare variables
    UInt_t nElectron;
    Float_t Electron_pt[100], Electron_eta[100], Electron_phi[100];
    Int_t Electron_pdgId[100];
    Bool_t Electron_mvaFall17V2Iso_WP90[100];
    Float_t MET_pt, projected_MET = 0;
    bool has_projected_MET = false;

    // Set branch addresses
    tree->SetBranchAddress("nElectron", &nElectron);
    tree->SetBranchAddress("Electron_pt", Electron_pt);
    tree->SetBranchAddress("Electron_eta", Electron_eta);
    tree->SetBranchAddress("Electron_phi", Electron_phi);
    tree->SetBranchAddress("Electron_pdgId", Electron_pdgId);
    tree->SetBranchAddress("Electron_mvaFall17V2Iso_WP90", Electron_mvaFall17V2Iso_WP90);
    tree->SetBranchAddress("MET_pt", &MET_pt);
    
    if (tree->GetBranch("projected_MET")) {
        tree->SetBranchAddress("projected_MET", &projected_MET);
        has_projected_MET = true;
    }

    // Counters
    int stage1 = 0, stage2 = 0, stage3 = 0, stage4 = 0, stage5 = 0, stage6 = 0;
    Long64_t nEntries = tree->GetEntries();

    // Event loop
    for (Long64_t i = 0; i < nEntries; i++) {
        tree->GetEntry(i);

        vector<int> tightElectrons;
        for (UInt_t j = 0; j < nElectron; j++) {
            if (Electron_mvaFall17V2Iso_WP90[j]) tightElectrons.push_back(j);
        }

        if (tightElectrons.size() != 2) continue;

        int i1 = tightElectrons[0], i2 = tightElectrons[1];
        if (Electron_pdgId[i1] * Electron_pdgId[i2] >= 0) continue;
        stage1++;

        int lead = (Electron_pt[i1] > Electron_pt[i2]) ? i1 : i2;
        int sublead = (lead == i1) ? i2 : i1;

        if (Electron_pt[lead] > 25 && Electron_pt[sublead] > 20 &&
            fabs(Electron_eta[lead]) < 2.5 && fabs(Electron_eta[sublead]) < 2.5) {
            stage2++;

            TLorentzVector el1, el2;
            el1.SetPtEtaPhiM(Electron_pt[lead], Electron_eta[lead], Electron_phi[lead], 0.000511);
            el2.SetPtEtaPhiM(Electron_pt[sublead], Electron_eta[sublead], Electron_phi[sublead], 0.000511);
            TLorentzVector dilepton = el1 + el2;

            double mll = dilepton.M();
            double ptll = dilepton.Pt();
            double dphill = fabs(el1.DeltaPhi(el2));

            if (mll > 60 && mll < 120) {
                stage3++;

                Float_t met = has_projected_MET ? projected_MET : MET_pt;
                if (met < 25) {
                    stage4++;

                    if (ptll < 40) {
                        stage5++;

                        if (dphill > 2.5) {
                            stage6++;
                        }
                    }
                }
            }
        }
    }

    // Print results
    cout << "\nCut Flow for " << filename << ":" << endl;
    cout << "Total events: " << nEntries << endl;
    cout << "Stage 1 (2 tight e, opp. charge): " << stage1 << endl;
    cout << "Stage 2 (pT/eta cuts):           " << stage2 << endl;
    cout << "Stage 3 (60 < mll < 120):        " << stage3 << endl;
    cout << "Stage 4 (MET < 25):              " << stage4 << endl;
    cout << "Stage 5 (ptll < 40):             " << stage5 << endl;
    cout << "Stage 6 (dphi > 2.5):            " << stage6 << endl;
    cout << "Final events:                    " << stage6 << endl;

    // Efficiencies
    double eff_total = (double)stage6 / nEntries;
    double eff1 = (double)stage1 / nEntries;
    double eff2 = stage1 > 0 ? (double)stage2 / stage1 : 0;
    double eff3 = stage2 > 0 ? (double)stage3 / stage2 : 0;
    double eff4 = stage3 > 0 ? (double)stage4 / stage3 : 0;
    double eff5 = stage4 > 0 ? (double)stage5 / stage4 : 0;
    double eff6 = stage5 > 0 ? (double)stage6 / stage5 : 0;

    cout << "\nEfficiencies:" << endl;
    cout << "Total: " << fixed << setprecision(4) << eff_total << " (" << 100*eff_total << "%)" << endl;
    cout << "S1: " << 100*eff1 << "%, S2: " << 100*eff2 << "%, S3: " << 100*eff3 
         << "%, S4: " << 100*eff4 << "%, S5: " << 100*eff5 << "%, S6: " << 100*eff6 << "%" << endl;

    // Save to file
    FILE *out = fopen("cutflow.txt", "a");
    if (out) {
        fprintf(out, "%-20s %10lld %8d %8d %8d %8d %8d %8d %.6f\n",
                filename, nEntries, stage1, stage2, stage3, stage4, stage5, stage6, eff_total);
        fclose(out);
    }

    delete file;
}

void Electron_Cut_Flow_Complete() {
    // Adjust these filenames to match YOUR files
    vector<string> files = {
        "DYtoLL_M50_slim.root", "DYtoLL_ext1.root",
        "WZ_slim.root", "ZZ_slim.root", 
        "WWTo2L2Nu.root", "TTTo2L2Nu_slim.root", 
        "SingleTop_slim.root"
    };

    // Clear previous results
    remove("cutflow.txt");

    cout << "\n" << string(60, '=') << endl;
    cout << "COMPLETE CUT-FLOW ANALYSIS FOR ALL FILES" << endl;
    cout << string(60, '=') << endl;
    cout << "Filename               Total   S1     S2     S3     S4     S5     S6     Eff" << endl;
    cout << string(60, '-') << endl;

    for (const auto& f : files) {
        cout << "\n--- " << f << " ---" << endl;
        Electron_Cut_Flow(f.c_str());
    }

    cout << "\n" << string(60, '=') << endl;
    cout << "Results saved to cutflow.txt" << endl;
    cout << "Run: 'cat cutflow.txt' to see the table" << endl;
    cout << string(60, '=') << endl;
}
