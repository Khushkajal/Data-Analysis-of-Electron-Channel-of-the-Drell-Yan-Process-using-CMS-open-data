/*
Author: Khushpreet Kaur
Macro: superimposed_plots()

Description:
This ROOT macro performs a comparative analysis of signal and background processes 
by generating normalized, superimposed histograms for key kinematic variables in 
the di-electron channel.

The macro processes multiple ROOT files corresponding to different physics processes 
(Drell-Yan signal and backgrounds like TTbar, WZ, ZZ, WW, SingleTop) and applies 
basic event selection:
    - Exactly two tight electrons (MVA WP90)
    - Opposite charge requirement

For each selected event, the following observables are reconstructed:
    - Invariant mass of dilepton system (m_ll)
    - Transverse momentum of dilepton system (pT_ll)
    - Projected Missing Transverse Energy (projected_MET)
    - Azimuthal separation between electrons (Δφ_ll)
    - Leading and subleading electron pT
    - Leading and subleading electron pseudorapidity (η)

Each distribution is normalized to unity and plotted together for all processes, 
allowing direct shape comparison between signal and backgrounds.

The output consists of high-quality overlay plots saved as PNG files.

Purpose:
This macro is useful for:
    - Understanding kinematic differences between signal and background
    - Designing selection cuts
    - Validating physics intuition for Drell-Yan analyses
*/

#include <TFile.h>
#include <TTree.h>
#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TLorentzVector.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <TColor.h>
#include <TStyle.h>
#include <TLatex.h>

using namespace std;
void processTree(TTree* tree, TH1F* hist_mll, TH1F* hist_ptll, TH1F* hist_met, TH1F* hist_dphill,TH1F* hist_pt_lead, TH1F* hist_pt_sub, TH1F* hist_eta_lead, TH1F* hist_eta_sub,float scale)
 {
    UInt_t nElectron;
    Float_t Electron_pt[100], Electron_eta[100], Electron_phi[100];
    Int_t Electron_pdgId[100];
    Bool_t Electron_mvaFall17V2Iso_WP90[100];
    Float_t projected_MET;

    tree->SetBranchAddress("nElectron", &nElectron);
    tree->SetBranchAddress("Electron_pt", Electron_pt);
    tree->SetBranchAddress("Electron_eta", Electron_eta);
    tree->SetBranchAddress("Electron_phi", Electron_phi);
    tree->SetBranchAddress("Electron_pdgId", Electron_pdgId);
    tree->SetBranchAddress("Electron_mvaFall17V2Iso_WP90", Electron_mvaFall17V2Iso_WP90);
    tree->SetBranchAddress("projected_MET", &projected_MET);

    int passedEvents = 0;

    Long64_t nEntries = tree->GetEntries();
   for (Long64_t i = 0; i < nEntries; i++) {
    tree->GetEntry(i);

    // Select tight electrons
    std::vector<int> tight_electrons;
    for (UInt_t j = 0; j < nElectron; j++) {
        if (Electron_mvaFall17V2Iso_WP90[j]) {
            tight_electrons.push_back(j);
        }
    }

    // Require exactly two tight electrons
    if (tight_electrons.size() != 2) continue;

    int i1 = tight_electrons[0];
    int i2 = tight_electrons[1];

    // Opposite charge
    if (Electron_pdgId[i1] * Electron_pdgId[i2] >= 0) continue;

    passedEvents++;

    TLorentzVector e1, e2;
    e1.SetPtEtaPhiM(Electron_pt[i1], Electron_eta[i1], Electron_phi[i1], 0.000511);
    e2.SetPtEtaPhiM(Electron_pt[i2], Electron_eta[i2], Electron_phi[i2], 0.000511);
    TLorentzVector dilepton = e1 + e2;

    double mll = dilepton.M();
    double ptll = dilepton.Pt();
    double dphill = fabs(e1.DeltaPhi(e2));

    hist_mll->Fill(mll, scale);
    hist_ptll->Fill(ptll, scale);
    hist_met->Fill(projected_MET, scale);
    hist_dphill->Fill(dphill, scale);

    // Determine leading/subleading by pT
    int lead = (Electron_pt[i1] > Electron_pt[i2]) ? i1 : i2;
    int sub  = (lead == i1) ? i2 : i1;

    hist_pt_lead->Fill(Electron_pt[lead], scale);
    hist_pt_sub->Fill(Electron_pt[sub], scale);
    hist_eta_lead->Fill(Electron_eta[lead], scale);
    hist_eta_sub->Fill(Electron_eta[sub], scale);
}

   // std::cout << "Events passing 2 tight electrons with opposite charge: " << passedEvents << std::endl;
}
void superimposed_plots() {
   vector<string> filenames = {
    "DYtoLL_M50_slim.root" ,
    "TTTo2L2Nu_slim.root",
    "WZ_slim.root",
    "ZZ_slim.root",
    "WWTo2L2Nu.root",
    "SingleTop_slim.root"
};

    vector<string> labels = {
        "DYtoLL_M50",
        "TTTo2L2Nu",
        "WZ",
        "ZZ",
        "WWTo2L2Nu",
        "SingleTop"
    };

    vector<int> colors = {
        TColor::GetColor("#e82e2e"), 
        TColor::GetColor("#2d49ad"), 
        TColor::GetColor("#ffcc00"), 
        TColor::GetColor("#e046d3"), 
        TColor::GetColor("#30ba1e"), 
        TColor::GetColor("#b08c51")  
    };

    vector<TH1F*> h_pt_lead_all, h_pt_sub_all, h_eta_lead_all, h_eta_sub_all, h_mll_all, h_ptll_all, h_met_all, h_dphill_all;


    TLegend *legend = new TLegend(0.75, 0.65, 0.95, 0.88);
    legend->SetTextSize(0.03);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);

    for (size_t i = 0; i < filenames.size(); i++) {
        TFile *file = TFile::Open(filenames[i].c_str());
        if (!file || file->IsZombie()) {
            cerr << "Cannot open file: " << filenames[i] << endl;
            continue;
        }

        TTree *tree = (TTree*)file->Get("Events");
        if (!tree) {
            cerr << "No tree found in file: " << filenames[i] << endl;
            continue;
        }

        // Create histograms 
        TH1F *h_mll     = new TH1F(Form("h_mll_%zu", i), "m_{ll} of the electron-positron pair; m_{ll} [GeV]; Normalized Events", 50, 50, 150);
        TH1F *h_ptll    = new TH1F(Form("h_ptll_%zu", i), "p_{T}^{ll} of the electron-positron pair; p_{T}^{ll} [GeV]; Normalized Events", 25, 0, 100);
        TH1F *h_met     = new TH1F(Form("h_met_%zu", i), "Projected MET for the electron-positron pair; Projected MET [GeV]; Normalized Events", 25, 0, 100);
        TH1F *h_dphill  = new TH1F(Form("h_dphill_%zu", i), "#Delta#phi_{ll} between the electron-positron pair; #Delta#phi_{ll}; Normalized Events", 30, 0, 3.14);
        TH1F *h_pt_lead  = new TH1F(Form("h_pt_lead_%zu", i), "Leading Electron p_{T}; p_{T}^{lead} [GeV]; Normalized Events", 25, 0, 100);
        TH1F *h_pt_sub   = new TH1F(Form("h_pt_sub_%zu", i), "Subleading Electron p_{T}; p_{T}^{sub} [GeV]; Normalized Events", 25, 0, 100);
        TH1F *h_eta_lead = new TH1F(Form("h_eta_lead_%zu", i), "Leading Electron #eta; #eta^{lead}; Normalized Events", 30, -3, 3);
        TH1F *h_eta_sub  = new TH1F(Form("h_eta_sub_%zu", i), "Subleading Electron #eta; #eta^{sub}; Normalized Events", 30, -3, 3);

        gStyle->SetTitleFontSize(0.035);  // Smaller than default (~0.05)

        h_mll->SetLineColor(colors[i]);     h_mll->SetLineWidth(5);
        h_ptll->SetLineColor(colors[i]);    h_ptll->SetLineWidth(5);
        h_met->SetLineColor(colors[i]);     h_met->SetLineWidth(5);
        h_dphill->SetLineColor(colors[i]);  h_dphill->SetLineWidth(5);
        h_pt_lead->SetLineColor(colors[i]);  h_pt_lead->SetLineWidth(5);
        h_pt_sub->SetLineColor(colors[i]);   h_pt_sub->SetLineWidth(5);
        h_eta_lead->SetLineColor(colors[i]); h_eta_lead->SetLineWidth(5);
        h_eta_sub->SetLineColor(colors[i]);  h_eta_sub->SetLineWidth(5);

        processTree(tree, h_mll, h_ptll, h_met, h_dphill,
            h_pt_lead, h_pt_sub, h_eta_lead, h_eta_sub, 1.0);  // scale = 1

        // Normalize
        if (h_mll->Integral() > 0) h_mll->Scale(1.0 / h_mll->Integral());
        if (h_ptll->Integral() > 0) h_ptll->Scale(1.0 / h_ptll->Integral());
        if (h_met->Integral() > 0) h_met->Scale(1.0 / h_met->Integral());
        if (h_dphill->Integral() > 0) h_dphill->Scale(1.0 / h_dphill->Integral());
        if (h_pt_lead->Integral() > 0) h_pt_lead->Scale(1.0 / h_pt_lead->Integral());
        if (h_pt_sub->Integral() > 0) h_pt_sub->Scale(1.0 / h_pt_sub->Integral());
        if (h_eta_lead->Integral() > 0) h_eta_lead->Scale(1.0 / h_eta_lead->Integral());
        if (h_eta_sub->Integral() > 0) h_eta_sub->Scale(1.0 / h_eta_sub->Integral());

        h_mll_all.push_back(h_mll);
        h_ptll_all.push_back(h_ptll);
        h_met_all.push_back(h_met);
        h_dphill_all.push_back(h_dphill);
        h_pt_lead_all.push_back(h_pt_lead);
        h_pt_sub_all.push_back(h_pt_sub);
        h_eta_lead_all.push_back(h_eta_lead);
        h_eta_sub_all.push_back(h_eta_sub);

        legend->AddEntry(h_mll, labels[i].c_str(), "l");
    }

    // Drawing function
    auto drawOverlay = [&](vector<TH1F*>& hists, string filename) {
    TCanvas *c = new TCanvas("c", "canvas", 2500, 1800);
    gStyle->SetOptStat(0);
    gPad->SetRightMargin(0.25);
    gPad->SetTopMargin(0.1);
    gPad->SetLeftMargin(0.1);

  

    // Determine ymax
    float yMax = 1.0;
    if (filename.find("mll") != std::string::npos) {
        yMax = 0.25;
    } else if (filename.find("eta") != std::string::npos) {
        yMax = 0.08;
    }

    // Set yMax for all histograms
    for (auto& h : hists) {
        h->SetMaximum(yMax);
    }

    // Draw histograms
    for (size_t i = 0; i < hists.size(); i++) {
        if (i == 0) hists[i]->Draw("HIST");
        else hists[i]->Draw("HIST SAME");
    }

    legend->Draw();

    TLatex cmsText;
    cmsText.SetNDC();
    cmsText.SetTextFont(42);
    cmsText.SetTextSize(0.025);
    cmsText.SetTextAlign(33);
    cmsText.DrawLatex(0.75, 0.93, "CMS Open Data");

    c->SetFrameLineColor(kBlack);
    c->SetFrameLineWidth(2);

    c->SaveAs((filename + ".png").c_str());

    delete c;
};


    drawOverlay(h_mll_all, "mll_overlay_random_events");
    drawOverlay(h_ptll_all, "ptll_overlay_random_events");
    drawOverlay(h_met_all, "met_overlay_random_events");
    drawOverlay(h_dphill_all, "dphill_overlay_random_events");
    drawOverlay(h_pt_lead_all, "pt_leading_electron");
    drawOverlay(h_pt_sub_all, "pt_subleading_electron");
    drawOverlay(h_eta_lead_all, "eta_leading_electron");
    drawOverlay(h_eta_sub_all, "eta_subleading_electron");
}
