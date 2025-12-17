#include "EventData.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <numeric>
#include <algorithm>

#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TPad.h"
#include "TGraph.h"
#include "TH2F.h"
#include "TColor.h"
#include "TLine.h"
#include "TLatex.h"
#include "TObjArray.h"
#include "TObjString.h"

using namespace std;

// ---------------- Particle colors and direction----------------
static Color_t ParticleColor(Int_t code){
  if (code == 1) return kYellow + 1;  // photon
  if (code == 2) return kGreen + 2;   // electrons
  if (code == 3) return kRed + 1;     // positron
  return kGray + 2;                   // others
}

static void Normalize3(float& x, float& y, float& z){
  const float n = std::sqrt(x*x + y*y + z*z);
  if (n > 0) { x /= n; y /= n; z /= n; }
}

#include <unordered_map>
#include <cmath>

static void DrawTracks2D(TPad* pad,
                         const std::vector<InteractionData>& inters,
                         char viewXYXZYZ,
                         bool colorByMother = true){
  pad->cd();

  // Mappa: Interaction Index -> puntatore alla InteractionData
  std::unordered_map<int, const InteractionData*> byIndex;
  byIndex.reserve(inters.size());
  for (const auto& it : inters) {
    byIndex[it.Index] = &it;
  }

  // Per ogni interazione, collega Parent -> This (se parent esiste)
  for (const auto& child : inters) {
    const int parentID = child.ParentInteractionID;

    // Se non c'è parent valido, non posso tracciare un segmento
    if (parentID <= 0) continue;

    auto itp = byIndex.find(parentID);
    if (itp == byIndex.end()) continue;

    const auto* parent = itp->second;

    // Proiezione
    double a1, b1, a2, b2;
    if (viewXYXZYZ == 'X') {        // XZ
      a1 = parent->X; b1 = parent->Z;
      a2 = child.X;  b2 = child.Z;
    } else if (viewXYXZYZ == 'Y') { // YZ
      a1 = parent->Y; b1 = parent->Z;
      a2 = child.Y;  b2 = child.Z;
    } else {                        // XY
      a1 = parent->X; b1 = parent->Y;
      a2 = child.X;  b2 = child.Y;
    }

    const Int_t pcode = colorByMother ? child.MotherParticleCode
                                      : parent->OutgoingParticleCode;

    TLine* l = new TLine(a1, b1, a2, b2);
    l->SetLineColor(ParticleColor(pcode));
    l->SetLineWidth(2);
    l->SetLineStyle(1);
    l->Draw("same");
  }
}


// ---------------- Geometry constants ----------------
const Float_t DET_HALF_X = 45.182f / 2.0f;
const Float_t DET_HALF_Y = 45.182f / 2.0f;
const Float_t DET_HALF_Z = 26.0f   / 2.0f;
const Float_t DET_UP_Z   = 13.0f;
const Float_t DET_DOWN_Z = -2.0f;
const Float_t CAL_UP_Z   = -3.0f;
const Float_t CAL_DOWN_Z = -13.0f;

const std::vector<double> z_planes = {
  -1.11320, 0.38680, 1.88680, 3.38680, 4.88680,
   6.38680, 7.88680, 9.38680, 10.88680, 12.38680
};

// Same mapping logic you used elsewhere
static int ComputeTrackerLayerFromZ(float z_cm)
{
  int   layerhit = 0;
  float current_Z_limit = DET_UP_Z - 1.5f;
  for (int layer = 1; layer <= 10; ++layer) {
    if (z_cm > current_Z_limit) { layerhit = layer; break; }
    current_Z_limit -= 1.5f;
  }
  return layerhit;
}

static void DrawTextPanel(TPad* pad, const TString& full_output){
  pad->cd();

  TObjArray* lines = full_output.Tokenize('\n');
  if (!lines || lines->GetEntries() == 0) { delete lines; return; }

  const Double_t x_start = 0.05;
  const Double_t y_start = 0.95;
  const Double_t line_spacing = 0.035;

  Double_t current_y = y_start;

  TIter next(lines);
  TObjString* obj_line = nullptr;
  while ((obj_line = (TObjString*) next())) {
    const char* text = obj_line->GetString().Data();
    TLatex* text_line = new TLatex(x_start, current_y, text);
    text_line->SetTextSize(0.03);
    text_line->SetNDC();
    text_line->Draw("same");

    current_y -= line_spacing;
    if (current_y < 0.15) break;
  }

  // ---------- LEGENDA DIREZIONI PARTICELLE ----------
  const Double_t y_leg = 0.10;

  TLatex* leg = new TLatex(
    0.05, y_leg,
    "Directions:  #gamma (yellow),  e^{-} (green),  e^{+} (red),  other (gray)"
  );
  leg->SetNDC();
  leg->SetTextSize(0.03);
  leg->SetTextColor(kBlack);
  leg->Draw("same");
  // --------------------------------------------------

  lines->Delete();
  delete lines;
}


static void DrawHitCloud2D(TPad* pad,
                           const char* hname,
                           const char* htitle,
                           float AXIS_LIMIT,
                           float max_e,
                           const std::vector<size_t>& hitOrder,
                           const std::vector<HitData>& hits,
                           char viewXYXZYZ,
                           bool drawTrackerPlanes,
                           bool drawDetBox,
                           bool drawCalBox){
  pad->cd();
  pad->SetGrid();
  pad->SetRightMargin(0.15);

  // Axes
  TH2F* h = nullptr;
  if (viewXYXZYZ == 'X') {
    // XZ
    h = new TH2F(hname, htitle, 100, -AXIS_LIMIT, AXIS_LIMIT, 100, -AXIS_LIMIT, AXIS_LIMIT);
    h->GetXaxis()->SetTitle("X (cm)");
    h->GetYaxis()->SetTitle("Z (cm)");
  } else if (viewXYXZYZ == 'Y') {
    // YZ
    h = new TH2F(hname, htitle, 100, -AXIS_LIMIT, AXIS_LIMIT, 100, -AXIS_LIMIT, AXIS_LIMIT);
    h->GetXaxis()->SetTitle("Y (cm)");
    h->GetYaxis()->SetTitle("Z (cm)");
  } else {
    // XY
    h = new TH2F(hname, htitle, 100, -AXIS_LIMIT, AXIS_LIMIT, 100, -AXIS_LIMIT, AXIS_LIMIT);
    h->GetXaxis()->SetTitle("X (cm)");
    h->GetYaxis()->SetTitle("Y (cm)");
  }

  h->SetMinimum(0);
  h->SetMaximum(max_e);
  h->GetZaxis()->SetTitle("Energy Deposit (keV)");
  h->GetZaxis()->SetMaxDigits(3);
  h->DrawCopy("COLZ");

  // Tracker planes (only meaningful on XZ/YZ)
  if (drawTrackerPlanes) {
    for (double z_val : z_planes) {
      TLine* l = nullptr;
      if (viewXYXZYZ == 'X') l = new TLine(-DET_HALF_X, z_val,  DET_HALF_X, z_val);
      if (viewXYXZYZ == 'Y') l = new TLine(-DET_HALF_Y, z_val,  DET_HALF_Y, z_val);
      if (l) {
        l->SetLineColor(kGray+2);
        l->SetLineStyle(2);
        l->Draw("same");
      }
    }
  }

  // Draw points (one TGraph per point because marker size/color changes per hit)
  for (size_t k = 0; k < hitOrder.size(); ++k) {
    const HitData& hit = hits[hitOrder[k]];

    double a = 0.0, b = 0.0;
    if (viewXYXZYZ == 'X') { a = hit.X; b = hit.Z; }
    else if (viewXYXZYZ == 'Y') { a = hit.Y; b = hit.Z; }
    else { a = hit.X; b = hit.Y; }

    TGraph* g = new TGraph(1, &a, &b);

    const double frac = (max_e > 0.0) ? (hit.EnergyDeposit / max_e) : 0.0;
    g->SetMarkerSize(0.5 + 1.0 * frac);
    g->SetMarkerColor(TColor::GetColorPalette((int)(1000 * frac)));

    // Index: 1 tracker, 2 cal (as in your code)
    if (hit.Index == 1) g->SetMarkerStyle(20);
    else if (hit.Index == 2) g->SetMarkerStyle(25);
    else g->SetMarkerStyle(24);

    g->Draw("P same");
  }

  // Detector box outlines
  if (drawDetBox) {
    TLine* box = new TLine();
    box->SetLineColor(kRed);
    box->SetLineStyle(9);

    if (viewXYXZYZ == 'X') {
      box->DrawLine(-DET_HALF_X, DET_DOWN_Z,  DET_HALF_X, DET_DOWN_Z);
      box->DrawLine( DET_HALF_X, DET_DOWN_Z,  DET_HALF_X, DET_UP_Z);
      box->DrawLine( DET_HALF_X, DET_UP_Z,   -DET_HALF_X, DET_UP_Z);
      box->DrawLine(-DET_HALF_X, DET_UP_Z,   -DET_HALF_X, DET_DOWN_Z);
    } else if (viewXYXZYZ == 'Y') {
      box->DrawLine(-DET_HALF_Y, DET_DOWN_Z,  DET_HALF_Y, DET_DOWN_Z);
      box->DrawLine( DET_HALF_Y, DET_DOWN_Z,  DET_HALF_Y, DET_UP_Z);
      box->DrawLine( DET_HALF_Y, DET_UP_Z,   -DET_HALF_Y, DET_UP_Z);
      box->DrawLine(-DET_HALF_Y, DET_UP_Z,   -DET_HALF_Y, DET_DOWN_Z);
    } else {
      box->DrawLine(-DET_HALF_X, -DET_HALF_Y,  DET_HALF_X, -DET_HALF_Y);
      box->DrawLine( DET_HALF_X, -DET_HALF_Y,  DET_HALF_X,  DET_HALF_Y);
      box->DrawLine( DET_HALF_X,  DET_HALF_Y, -DET_HALF_X,  DET_HALF_Y);
      box->DrawLine(-DET_HALF_X,  DET_HALF_Y, -DET_HALF_X, -DET_HALF_Y);
    }
  }

  // Calorimeter box outlines (only on XZ/YZ)
  if (drawCalBox && (viewXYXZYZ == 'X' || viewXYXZYZ == 'Y')) {
    TLine* cbox = new TLine();
    cbox->SetLineColor(kBlue);
    cbox->SetLineStyle(9);

    const float half = (viewXYXZYZ == 'X') ? DET_HALF_X : DET_HALF_Y;

    cbox->DrawLine(-half, CAL_DOWN_Z,  half, CAL_DOWN_Z);
    cbox->DrawLine( half, CAL_DOWN_Z,  half, CAL_UP_Z);
    cbox->DrawLine( half, CAL_UP_Z,   -half, CAL_UP_Z);
    cbox->DrawLine(-half, CAL_UP_Z,   -half, CAL_DOWN_Z);
  }
}

void event_display(int event_number = 0, const char* filename = "events.root")
{
  gStyle->SetOptStat(0);
  gStyle->SetPalette(kRainBow);

  TFile* f = TFile::Open(filename);
  if (!f || f->IsZombie()) {
    std::cerr << "ERROR: Could not open " << filename << std::endl;
    return;
  }

  TTree* tree = (TTree*) f->Get("Events");
  if (!tree) {
    std::cerr << "ERROR: TTree 'Events' not found in " << filename << std::endl;
    f->Close();
    delete f;
    return;
  }

  const Long64_t n = tree->GetEntries();
  if (event_number < 0 || event_number >= n) {
    std::cerr << "ERROR: Invalid event number (" << event_number
              << "). Valid range is 0.." << (n - 1) << std::endl;
    f->Close(); delete f;
    return;
  }

  EventData* event = nullptr;
  tree->SetBranchAddress("Event", &event);
  tree->GetEntry(event_number); // FIX: no -1

  std::cout << "--- Loaded entry " << event_number
            << " (EventID=" << event->EventID << ") with "
            << event->Hits.size() << " Hits and "
            << event->Interactions.size() << " Interactions from file: "
            << filename << " ---" << std::endl;

  // Max energy for color scaling
  Float_t max_e = 0.0f;
  for (const auto& hit : event->Hits) max_e = std::max(max_e, hit.EnergyDeposit);
  if (max_e <= 0.0f) max_e = 1.0f;

  const Float_t AXIS_MAX_LIMIT = std::max({DET_HALF_X, DET_HALF_Y, DET_HALF_Z});
  const Float_t AXIS_LIMIT = AXIS_MAX_LIMIT + 1.0f;

  // Sort hits by time WITHOUT copying HitData
  const auto& hits = event->Hits;
  std::vector<size_t> order(hits.size());
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(),
            [&](size_t a, size_t b) { return hits[a].Time < hits[b].Time; });

  // Build text block (only once)
  TString full_output;
  full_output += TString::Format("FILE: %s, ENTRY: %d, EventID: %d",
                                 filename, event_number, event->EventID);
  full_output += "\n\n";

  for (size_t k = 0; k < order.size(); ++k) {
    const HitData& hit = hits[order[k]];

    TString det = "NONE";
    int layerhit = 0;
    if (hit.Index == 1) { det = "TRA"; layerhit = ComputeTrackerLayerFromZ(hit.Z); }
    else if (hit.Index == 2) { det = "CAL"; }

    // Match interactions referenced by hit.PrimaryParticleIDs (as in your original logic)
    TString interaction_types_list;
    TString interaction_time_list;
    TString interaction_particle_list;

    for (const auto& pid : hit.PrimaryParticleIDs) {
      for (const auto& inter : event->Interactions) {
        if (inter.Index != pid) continue;

        interaction_types_list += inter.Type; interaction_types_list += " ";

        const Double_t time_in_ns = inter.Time * 1.0e9;
        interaction_time_list += TString::Format("%.3f ", time_in_ns);

        Int_t outp = inter.OutgoingParticleCode;
        TString outp_s = TString::Format("%d", outp);
        if (outp == 1) outp_s = "PH";
        if (outp == 2) outp_s = "E+";
        if (outp == 3) outp_s = "E-";
        interaction_particle_list += outp_s; interaction_particle_list += " ";
      }
    }

    interaction_types_list.Remove(TString::kTrailing, ' ');
    interaction_time_list.Remove(TString::kTrailing, ' ');
    interaction_particle_list.Remove(TString::kTrailing, ' ');

    TString line = TString::Format(
      "HIT: %s; LAYER: %d; Edep(keV): %.3f; TIME(ns): %.3f; INT TYPE: %s; INT TIME(ns): %s; OutP: %s",
      det.Data(),
      layerhit,
      hit.EnergyDeposit,
      hit.Time * 1.0e9,
      interaction_types_list.Data(),
      interaction_time_list.Data(),
      interaction_particle_list.Data()
    );

    full_output += line;
    full_output += "\n";
  }

  // Canvas
  TCanvas* c1 = new TCanvas("c1", "Event Display (Hits Only)", 1200, 1000);
  c1->Divide(2, 2);

  // Pad 1: text
  c1->cd(1);
  DrawTextPanel((TPad*) gPad, full_output);

  // Pad 2: XY
  c1->cd(2);
  DrawHitCloud2D((TPad*) gPad, "h_xy", "XY View;X (cm);Y (cm)", AXIS_LIMIT, max_e, order, hits, 'Z' /* XY */, false /* planes */, true /* det */, false /* cal */);
  DrawTracks2D((TPad*)gPad, event->Interactions, 'Z', true);
  // Pad 3: XZ
  c1->cd(3);
  DrawHitCloud2D((TPad*) gPad, "h_xz", "XZ View;X (cm);Z (cm)", AXIS_LIMIT, max_e, order, hits, 'X', true /* planes */, true /* det */, true /* cal */);
  DrawTracks2D((TPad*)gPad, event->Interactions, 'X', true);

  // Pad 4: YZ
  c1->cd(4);
  DrawHitCloud2D((TPad*) gPad, "h_yz", "YZ View;Y (cm);Z (cm)", AXIS_LIMIT, max_e, order, hits, 'Y', true /* planes */, true /* det */, true /* cal */);
  DrawTracks2D((TPad*)gPad, event->Interactions, 'Y', true);

  c1->cd();
  c1->Update();

  f->Close();
  delete f;
}
