#include <iostream>
#include <string>

#include "TFile.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"

using namespace std;

// Macro compilabile ROOT:
// .L CompareEdep.C+
// CompareEdep("file1.ana.root", "file2.ana.root", true);

void CompareEdep(const char* file1_name = "file1.ana.root",
                 const char* file2_name = "file2.ana.root",
                 bool normalize = true)
{
  cout << "File 1: " << file1_name << endl;
  cout << "File 2: " << file2_name << endl;

  // Apri i file ROOT
  TFile* f1 = TFile::Open(file1_name);
  if (!f1 || f1->IsZombie()) {
    cerr << "ERROR: impossibile aprire " << file1_name << endl;
    return;
  }

  TFile* f2 = TFile::Open(file2_name);
  if (!f2 || f2->IsZombie()) {
    cerr << "ERROR: impossibile aprire " << file2_name << endl;
    return;
  }

  // Recupera gli istogrammi
  TH1* h1 = dynamic_cast<TH1*>(f1->Get("hEdepTrackerLayerALL"));
  TH1* h2 = dynamic_cast<TH1*>(f2->Get("hEdepTrackerLayerALL"));

  if (!h1) {
    cerr << "ERROR: hEdepTrackerLayerALL non trovato in " << file1_name << endl;
    return;
  }
  if (!h2) {
    cerr << "ERROR: hEdepTrackerLayerALL non trovato in " << file2_name << endl;
    return;
  }

  // Clona gli istogrammi per non toccare gli originali nei file
  TH1* h1c = dynamic_cast<TH1*>(h1->Clone("hEdepTrackerLayerALL_file1"));
  TH1* h2c = dynamic_cast<TH1*>(h2->Clone("hEdepTrackerLayerALL_file2"));
  TH1F *hRateEDep = new TH1F("RateEnergyDepSpec",
    "Rate of Deposited Energy spectrums;Deposited Energy (keV);Rate",
    h1c->GetNbinsX(),
    h1c->GetXaxis()->GetXmin(),
    h1c->GetXaxis()->GetXmax());
  hRateEDep->Divide(h1c, h2c, 1.0, 1.0, "");

  // Opzionale: normalizza ad area unitaria per confrontare la forma
  if (normalize) {
    if (h1c->Integral() > 0) h1c->Scale(1.0 / h1c->Integral());
    if (h2c->Integral() > 0) h2c->Scale(1.0 / h2c->Integral());
  }

  // Stile
  gStyle->SetOptStat(0);

  h1c->SetLineColor(kBlue + 1);
  h1c->SetLineWidth(2);
  h2c->SetLineColor(kRed + 1);
  h2c->SetLineWidth(2);

  // Sistema un titolo generico
  h1c->SetTitle("Confronto hEdepTrackerLayerALL; Deposited Energy (keV);Arbitrary units");

  // Canvas
  TCanvas* c = new TCanvas("cCompareEdep", "Compare hEdepTrackerLayerALL", 900, 700);

  // Decidi automaticamente il range Y
  double maxy = std::max(h1c->GetMaximum(), h2c->GetMaximum());
  h1c->SetMaximum(1.2 * maxy);

  // Disegna
  h1c->Draw("HIST");
  h2c->Draw("HIST SAME");

  c->Update();

  TCanvas *cRate = new TCanvas("cRate", "Ratio of spectra", 900, 700);
  hRateEDep->SetLineColor(kMagenta+2);
  hRateEDep->SetLineWidth(2);
  hRateEDep->Draw("HIST");

  cout << "Confronto completato. Canvas: cCompareEdep" << endl;
}
