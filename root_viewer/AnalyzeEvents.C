#include <iostream>

#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH2.h"
#include "TH1.h"
#include "THStack.h"
#include "TProfile.h"
#include "TLegend.h"
#include "TLine.h"

#include "AnalyzeEvents.h"

using namespace std;

void AnalyzeEvents(const char* input_filename_char = "simulation_data.root") {

  std::string input_filename = input_filename_char;

  std::cout << "Input file:  " << input_filename << std::endl;
  
  TFile *f = TFile::Open(input_filename.c_str());
  if (!f || f->IsZombie()) {
    std::cerr << "ERROR: file ROOT is unopenalble: " << input_filename.c_str() << std::endl;
    return;
  }

  TTree *tree = (TTree*)f->Get("Events");
  if (!tree) {
    std::cerr << "ERROR: TTree 'Events' not found." << std::endl;
    f->Close();
    return;
  }

  RunInfo *runInfo = 0;
  tree->SetBranchAddress("RunInfo", &runInfo);
  tree->GetEntry(0);
  Double_t BeamEnergy = runInfo->SpectralEnergy;
  std::cout << "Beam Energy: " << BeamEnergy << " keV" <<  std::endl;

  const int e_bins = (Int_t) (BeamEnergy * 3.1); 
  const float e_min = 0.0;
  const float e_max = BeamEnergy * 1.1;

  EventData *event = 0; 
  tree->SetBranchAddress("Event", &event); 

  Long64_t nEntries = tree->GetEntries();
  std::cout << "Starting Analysis " << nEntries << " events..." << std::endl;

  TH1F *hTotEdep_vs_Nev = new TH1F("hTotEdep_vs_Nev", "Total Deposited Energy per Event;Sequential Event Num.;Total Deposited Energy (keV)", nEntries, 0, nEntries);
  TH1F *hTotEesc_vs_Nev = new TH1F("hTotEesc_vs_Nev", "Total Escaped Energy per Event;Sequential Event Num.;Total Escaped Energy (keV)", nEntries, 0, nEntries);
  TH1F *hTotEnsm_vs_Nev = new TH1F("hTotEnsm_vs_Nev", "Total Energy dep. NonSensitive Material  per Event;Sequential Event Num.;Total Energy dep. NonSensitive Material (keV)", nEntries, 0, nEntries);

  TH2F *h2Etot = new TH2F("h2Etot", "Energy;Energy Category; Energy (keV)", 3, 0.5, 3 + 0.5, e_bins, e_min, e_max);
  h2Etot->GetZaxis()->SetTitle("Events");
  h2Etot->GetXaxis()->SetBinLabel(1, "E_{deposited}"); 
  h2Etot->GetXaxis()->SetBinLabel(3, "E_{escaped}");  
  h2Etot->GetXaxis()->SetBinLabel(2, "E_{NonSensitiveMat}"); 

  TH2F *h2EdepTrackerVsZ = new TH2F("h2EdepTrackerVsZ", "Energy Deposited in the tracker hits vs. Z-Position;Z-Position (cm);Energy Deposited (keV); Hits", 60, -15, 15, e_bins, e_min, e_max);
  TH2F *Ph2EdepTrackerVsZ = new TH2F("Ph2EdepTrackerVsZ", "Energy Deposited in the tracker vs. Z-Position;Z-Position (cm);Energy Deposit (keV); Hits", 60, -15, 15, e_bins, e_min, e_max);
  TH2F *h2TrackerXvsY = new TH2F("h2TrackerXvsY", "Hits in the tracker;X-Position (cm);Y-Position (cm); Hits", 100, -25, 25, 100, -25, 25);

  // istogrammi: tutti i colpi e con varie esclusioni di PhysicsModuleType
  TH1F *hEdepTrackerLayerALL = new TH1F("hEdepTrackerLayerALL",
    "Energy Deposited per Hit (all); Deposited Energy (keV); Hits",
    e_bins, e_min, e_max);

  TH1F *hEdepTrackerLayerNoCopper = new TH1F("hEdepTrackerLayerNoCopper",
    "Energy Deposited per Hit (PhysicsModuleType != Copper); Deposited Energy (keV); Hits",
    e_bins, e_min, e_max);

  TH1F *hEdepTrackerLayerNoCopperMJ55 = new TH1F("hEdepTrackerLayerNoMJ",
    "Energy Deposited per Hit (PhysicsModuleType != Copper or MJ55); Deposited Energy (keV); Hits",
    e_bins, e_min, e_max);

  TH1F *hEdepTrackerLayerNoCopperMJ55FEE = new TH1F("hEdepTrackerLayerNoMJFEE",
    "Energy Deposited per Hit (PhysicsModuleType != Copper or MJ55 or FEE); Deposited Energy (keV); Hits",
    e_bins, e_min, e_max);

  TH2F *hLayers_vs_Nev = new TH2F("hLayers_vs_Nev", "Hits per Layer  per Event;Sequential Event Num.; Layer Number", nEntries, 0, nEntries,10, 0.5, 10.5);
  TH2F *hEdepTrackerLayerMultiplicity = new TH2F("hEdepTrackerLayerMultiplicity", "Total Deposited Energy per Layer Multiplicity; Layer Multiplicity; Deposited Energy (keV)", 50, 0.5, 50.5, e_bins, e_min, e_max);
  TH1F *hLayerMultiplicity = new TH1F("hLayerMultiplicity", "Layer Multiplicity;Layer Multiplicity; Events", 50, 0.5, 50);

  for (Long64_t i = 0; i < nEntries; i++) {
    tree->GetEntry(i);
    
    hTotEdep_vs_Nev->Fill(event->TriggerID, event->TotDepositedEnergy);
    hTotEesc_vs_Nev->Fill(event->TriggerID, event->EscapedEnergy);
    hTotEnsm_vs_Nev->Fill(event->TriggerID, event->NSMaterialEnergy);

    h2Etot->Fill(1, event->TotDepositedEnergy);
    h2Etot->Fill(3, event->EscapedEnergy);    
    h2Etot->Fill(2, event->NSMaterialEnergy);

    int NlayersHit = 0;
    
    for (const auto& hit : event->Hits) {
      if (hit.Index == 1) {
        for (const auto& particleID : hit.PrimaryParticleIDs) {
          for (const auto& interaction : event->Interactions) {

            h2EdepTrackerVsZ->Fill(hit.Z, hit.EnergyDeposit);
            Ph2EdepTrackerVsZ->Fill(hit.Z, hit.EnergyDeposit);
            h2TrackerXvsY->Fill(hit.X, hit.Y);

            // tutti i colpi
            hEdepTrackerLayerALL->Fill(hit.EnergyDeposit);

            // tagli sui PhysicsModuleType
            if (event->PhysicsModuleType != "Copper") {
              hEdepTrackerLayerNoCopper->Fill(hit.EnergyDeposit);
              if (event->PhysicsModuleType != "M55J"){
                hEdepTrackerLayerNoCopperMJ55->Fill(hit.EnergyDeposit);
                if (event->PhysicsModuleType != "ComPairFEEBoard"){
                  hEdepTrackerLayerNoCopperMJ55FEE->Fill(hit.EnergyDeposit);
                }
              }
            }

            int layerhit = 0;
            float current_Z_limit = 11.5;
            for (int layer = 1; layer <= 10; ++layer) {
              if (hit.Z > current_Z_limit) {
                layerhit = layer;
                NlayersHit++;
                break;
              }
              current_Z_limit -= 1.5;
            }
            
            hEdepTrackerLayerMultiplicity->Fill(NlayersHit, hit.EnergyDeposit);
            hLayers_vs_Nev->Fill(event->TriggerID, layerhit);
          }        
        }
      }
    }
    hLayerMultiplicity->Fill(NlayersHit);
  }
    
  hTotEdep_vs_Nev->SetFillColor(kGreen+2);
  hTotEnsm_vs_Nev->SetFillColor(kBlue);
  hTotEesc_vs_Nev->SetFillColor(kRed);

  hTotEdep_vs_Nev->SetLineColor(kGreen+2);
  hTotEnsm_vs_Nev->SetLineColor(kBlue);
  hTotEesc_vs_Nev->SetLineColor(kRed);
  
  THStack *hs_EnergyPerEvent = new THStack("hs_EnergyPerEvent", "Energy per Event; Event Num.;Total Energy (keV)");
  hs_EnergyPerEvent->Add(hTotEdep_vs_Nev);
  hs_EnergyPerEvent->Add(hTotEnsm_vs_Nev);
  hs_EnergyPerEvent->Add(hTotEesc_vs_Nev);

  TProfile *pEdepTrackerVsZ = Ph2EdepTrackerVsZ->ProfileX("pEdepVsZ", 1, -1);
  pEdepTrackerVsZ->GetYaxis()->SetTitle("Energy Deposited (keV)");
  
  // ---------- COMMON CANVAS FOR HISTOGRAMS + COMPTON EDGE ----------
  TCanvas *cEdep = new TCanvas("cEdep", "Edep per Hit: All vs Non-Copper", 800, 600);

  hEdepTrackerLayerALL->SetLineColor(kBlack);
  hEdepTrackerLayerALL->SetLineWidth(2);
  hEdepTrackerLayerNoCopper->SetLineColor(kRed);
  hEdepTrackerLayerNoCopper->SetLineWidth(2);
  hEdepTrackerLayerNoCopperMJ55->SetLineColor(kBlue);
  hEdepTrackerLayerNoCopperMJ55->SetLineWidth(2);
  hEdepTrackerLayerNoCopperMJ55FEE->SetLineColor(kOrange);
  hEdepTrackerLayerNoCopperMJ55FEE->SetLineWidth(2);

  hEdepTrackerLayerALL->Draw("HIST");
  hEdepTrackerLayerNoCopper->Draw("HIST SAME");
  hEdepTrackerLayerNoCopperMJ55->Draw("HIST SAME");
  hEdepTrackerLayerNoCopperMJ55FEE->Draw("HIST SAME");

  TLegend *leg = new TLegend(0.6, 0.7, 0.88, 0.88);
  leg->AddEntry(hEdepTrackerLayerALL, "All hits", "l");
  leg->AddEntry(hEdepTrackerLayerNoCopper, "PhysicsModuleType != Copper", "l");
  leg->AddEntry(hEdepTrackerLayerNoCopperMJ55, "PhysicsModuleType != Copper or MJ55", "l");
  leg->AddEntry(hEdepTrackerLayerNoCopperMJ55FEE, "PhysicsModuleType != Copper or MJ55 or FEE", "l");
  leg->Draw();

  // ====== LINEA VERTICALE ALLA COMPTON EDGE SULLO STESSO PLOT ======
  const double me_c2_keV = 511.0;
  double Ecompton = BeamEnergy * (1.0 - 1.0 / (1.0 + 2.0 * BeamEnergy / me_c2_keV));
  // Se vuoi un valore fisso, sostituisci con:
  // double Ecompton = 80.0; // keV, ad esempio

  double ymax = hEdepTrackerLayerALL->GetMaximum();
  TLine *lineCE = new TLine(Ecompton, 0.0, Ecompton, ymax);
  lineCE->SetLineColor(kMagenta+2);
  lineCE->SetLineWidth(2);
  lineCE->SetLineStyle(2); // tratteggiata
  lineCE->Draw("SAME");
  // ================================================================

  // ------------------------------------------------------

  std::string output_filename;
  output_filename = input_filename;

  const std::string SIM_DIR = "/sim_root/";
  const std::string ANA_DIR = "/sim_ana/";

  size_t pos = input_filename.rfind(".sim.root");
  if (pos != std::string::npos) {
    output_filename.replace(pos, 9, ".ana.root");
  } else {
    output_filename = input_filename.substr(0, input_filename.rfind(".root")) + ".ana.root";
    std::cout << "ATTENTION: '.sim.root' not found, output name is: " << output_filename << std::endl;
  }
    
  size_t dir_pos = output_filename.find(SIM_DIR);
  if (dir_pos != std::string::npos) {
    output_filename.replace(dir_pos, SIM_DIR.length(), ANA_DIR);
  } else {
    std::cerr << "WARNING: Path '" << SIM_DIR << "' not found!" << std::endl;
  }

  TFile *outputFile = new TFile(output_filename.c_str(), "RECREATE");
  hTotEdep_vs_Nev->Write();
  hTotEnsm_vs_Nev->Write();
  hTotEesc_vs_Nev->Write();
  hs_EnergyPerEvent->Write();
  h2Etot->Write();
  h2EdepTrackerVsZ->Write();
  pEdepTrackerVsZ->Write();
  h2TrackerXvsY->Write();
  hEdepTrackerLayerALL->Write();
  hEdepTrackerLayerNoCopper->Write();
  hEdepTrackerLayerNoCopperMJ55->Write();
  hEdepTrackerLayerNoCopperMJ55FEE->Write();
  hEdepTrackerLayerMultiplicity->Write();
  hLayers_vs_Nev->Write();
  hLayerMultiplicity->Write();
  cEdep->Write();   // salva anche la canvas con la linea di Compton
  outputFile->Close();

  std::cout << "\n Analyses completed. File analyzed: " << input_filename.c_str() << std::endl;
  std::cout << "Output file: " << output_filename << std::endl;
  f->Close();
  delete f;
}
