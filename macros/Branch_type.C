/*
Author: Khushpreet Kaur

Description:
This ROOT macro is used to inspect and verify the data types of selected
branches in slimmed CMS NanoAOD ROOT files. It is particularly useful for
understanding the structure of jet-related variables before performing
further analysis such as event selection or plotting.

The script loops over multiple input ROOT files, accesses the "Events"
TTree, and checks whether specific branches exist. For each branch, it
retrieves and prints the data type (e.g., Float_t, Int_t, etc.) using
TLeaf information. This helps ensure correct variable handling in later
analysis steps.

How to Use:

1. Open ROOT:
   root -l

2. Load the macro:
   .L branch_type.C

3. Run the function:
   branch_type()

4. Input:
   - Place all slimmed ROOT files in the same directory.

5. Output:
   - Prints branch names and their data types in the terminal.

6. Notes:
   - Ensure the TTree name is "Events".
   - Modify 'requiredBranches' to check additional variables.
*/
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TLeaf.h>
#include <iostream>
#include <vector>

void branch_type() {
  
  std::vector<std::string> filenames = {
        "DYtoLL_M50_slim.root",
        "WZ_slim.root",
        "ZZ_slim.root",
        "WWTo2L2Nu_slim.root",
        "TTTo2L2Nu_slim.root",
        "SingleTop_slim.root"
    };


    std::vector<std::string> requiredBranches = {"nJet",
"Jet_area",
"Jet_btagCSVV2",
"Jet_btagDeepB",
"Jet_btagDeepCvB",
"Jet_btagDeepCvL",
"Jet_btagDeepFlavB",
"Jet_btagDeepFlavCvB"
,"Jet_btagDeepFlavCvL"
,"Jet_btagDeepFlavQG"
    };

for (const auto &fname : filenames) {
        std::cout << "\n=== Checking file: " << fname << " ===" << std::endl;

        // Open the ROOT file
        TFile *file = TFile::Open(fname.c_str());
        if (!file || file->IsZombie()) {
            std::cerr << "Error: Cannot open ROOT file " << fname << std::endl;
            continue;
        }

        // Get the TTree
        TTree *tree = (TTree*)file->Get("Events");
        if (!tree) {
            std::cerr << "Error: Tree 'Events' not found in " << fname << std::endl;
            file->Close();
            delete file;
            continue;
        }
    std::cout << "Checking Branch Types...\n";
    
    // Loop over required branches
    for (const auto& branchName : requiredBranches) {
        TBranch *branch = tree->GetBranch(branchName.c_str());
        if (!branch) {
            std::cout << "Branch: " << branchName << " not found!" << std::endl;
            continue;
        }

        // Get the leaf associated with the branch
        TLeaf *leaf = branch->GetLeaf(branchName.c_str());
        if (leaf) {
            std::cout << "Branch: " << branchName << ", Type: " << leaf->GetTypeName() << std::endl;
        } else {
            std::cout << "Branch: " << branchName << ", Type: Unknown" << std::endl;
        }
    }

    // Close the file
    file->Close();
}
