

#include "event_display.h"

using namespace std;

const Float_t DET_HALF_X = 45.182 / 2.0; // 22.591
const Float_t DET_HALF_Y = 45.182 / 2.0; // 22.591
const Float_t DET_HALF_Z = 26.0 / 2.0;   // 10.0
const Float_t DET_UP_Z = 13.0 ;   
const Float_t DET_DOWN_Z = -2.0;
const Float_t CAL_UP_Z = -3.0 ;   
const Float_t CAL_DOWN_Z = -13.0; 

const std::vector<double> z_planes = {-1.11320, 0.38680, 1.88680, 3.38680, 4.88680, 6.38680, 7.88680, 9.38680, 10.88680, 12.38680};

void event_display(int event_number = 0, const char* filename = "events.root") {
    
 
  gStyle->SetOptStat(0);
  gStyle->SetPalette(kRainBow); 


  TFile *f = TFile::Open(filename);
  if (!f || f->IsZombie()) {
    std::cerr << "ERROR: Could not open " << filename << ". Please check the file path." << std::endl;
    return;
  }
 
  TTree *tree = (TTree*)f->Get("Events");
  if (!tree) {
    std::cerr << "ERROR: TTree  not found in " << filename << "." << std::endl;
    f->Close();
    return;
  }
    
  if (event_number < 0 || event_number >= tree->GetEntries()) {
    std::cerr << "ERROR: Invalid event number (" << event_number << 
      "). Valid range is 0 to " << tree->GetEntries() - 1 << "." << std::endl;
    f->Close();
    return;
  }
    
  EventData *event = 0; 
  tree->SetBranchAddress("Event", &event); 
  tree->GetEntry(event_number-1);

  std::cout << "--- Loaded Event " << event->EventID << " with " 
  	    << event->Hits.size() << " Hits and " 
  	    << event->Interactions.size() << " Interactions from file: " << filename << " ---" << std::endl;
   
    
  Float_t max_e = 0.0;
  for (const auto& hit : event->Hits) {
    if (hit.EnergyDeposit > max_e) {
      max_e = hit.EnergyDeposit;
    }
  }
    
  const Float_t AXIS_MAX_LIMIT = std::max({DET_HALF_X, DET_HALF_Y, DET_HALF_Z});
  const Float_t AXIS_PADDING = 1.0; 
  const Float_t AXIS_LIMIT = AXIS_MAX_LIMIT + AXIS_PADDING;

  int n_hits = event->Hits.size();
  Int_t *hit_type = new Int_t[n_hits];
  Double_t *hit_x = new Double_t[n_hits];
  Double_t *hit_y = new Double_t[n_hits];
  Double_t *hit_z = new Double_t[n_hits];
  Double_t *hit_e = new Double_t[n_hits];

  TString full_output = "";
  TString filename_ts(filename);
  full_output += TString::Format("FILE: %s, EVENT: %d",
				 filename_ts.Data(), event_number);
  full_output += "\n";
  full_output += "\n";

  for (int i = 0; i < n_hits; ++i) {
    hit_type[i]= event->Hits[i].Index;
    hit_x[i] = event->Hits[i].X;
    hit_y[i] = event->Hits[i].Y;
    hit_z[i] = event->Hits[i].Z;
    hit_e[i] = event->Hits[i].EnergyDeposit;

    string det="NONE";
    int layerhit=0;
    TString interaction_types_list = "";
	
    if(hit_type[i]==1){
      det="TRA";
      float current_Z_limit = DET_UP_Z-1.5;
      for (int layer = 1; layer <= 10; ++layer) {
	if (hit_z[i] > current_Z_limit) {
	  layerhit = layer;	 
	  break;
	}
	current_Z_limit -= 1.5;
      } 
    }else if(hit_type[i]==2){
      det="CAL";

    }

    // cout << "HIT: " << det
    //	 << ";   LAYER: " << layerhit
    //	 << ";   INTERACTION TYPE: ";

     int n_int = event->Interactions.size();
     for (int j = 0; j < n_int; ++j) {
       for (const auto& particleID : event->Hits[i].PrimaryParticleIDs) {
	 if(event->Interactions[j].Index==particleID){
	   // cout <<  event->Interactions[j].Type << "  " ;
	   interaction_types_list += event->Interactions[j].Type;
	   interaction_types_list += "  ";
	 }
       }
     }
     // cout << endl;

     interaction_types_list.Remove(TString::kTrailing, ' ');
     TString hit_detector = det;
     Int_t hit_layer = layerhit;

     TString current_line =
       TString::Format( "HIT: %s;   LAYER: %d;   INTERACTION TYPE: %s",
			hit_detector.Data(), 
			hit_layer, 
			interaction_types_list.Data()
			);
     std::cout << current_line.Data() << std::endl;
     full_output += current_line;
     full_output += "\n";
  }

    

  TCanvas *c1 = new TCanvas("c1", "Event Display (Hits Only)", 1200, 1000);
  c1->Divide(2, 2);

  c1->cd(1);
  //TPad *pad1 = (TPad*)c1->GetPad(1);
  //pad1->SetFillColor(kGray + 1); // Background color for the empty pad
  //pad1->DrawFrame(-1, -1, 1, 1, "3D View Placeholder");

  TObjArray *lines = full_output.Tokenize('\n');
  if (lines->GetEntries() == 0) {
        delete lines;
        return;
  }
  Double_t x_start = 0.05; 
  Double_t y_start = 0.95;
  Double_t line_spacing = 0.035; 
  Double_t current_y = y_start;

  TIter next(lines);
  TObjString *obj_line;
  while ((obj_line = (TObjString*)next())) {
    const char *text = obj_line->GetString().Data();
    TLatex *text_line = new TLatex(x_start, current_y, text);
    text_line->SetTextSize(0.03); 
    text_line->SetNDC();        
    text_line->Draw("same");
    current_y -= line_spacing;
    if (current_y < 0.05) {
      break; 
    }
  }
  lines->Delete();
  delete lines;
  gPad->Update();
    
  /* c1->cd(1); */
  /* gPad->SetGrid(); */

  /* TView *view = TView::CreateView(1);  */
  /* const Double_t rmin[] = {-AXIS_LIMIT, -AXIS_LIMIT, -AXIS_LIMIT}; */
  /* const Double_t rmax[] = {AXIS_LIMIT, AXIS_LIMIT, AXIS_LIMIT};  */
  /* view->SetRange(rmin, rmax); */
    
  /* TH3F *h3d = new TH3F("h3d", "3D View (Interactions and Hits);X (cm);Y (cm);Z (cm)", 10, -AXIS_LIMIT, AXIS_LIMIT, 10, -AXIS_LIMIT, AXIS_LIMIT, 10, -AXIS_LIMIT, AXIS_LIMIT); */
  /* h3d->SetMarkerSize(0);  */
  /* h3d->Draw(); */
  /* gPad->GetView(); */
    
  /* TGraph2D *g2d = new TGraph2D(n_hits, hit_x, hit_y, hit_z); */
  /* g2d->Draw("P0"); */
  /* gPad->Update(); */
    
    
  // TPolyMarker3D *hits3d = new TPolyMarker3D(n_hits, hit_x, hit_y, hit_z);
  // hits3d->SetMarkerStyle(8); 
  //hits3d->SetMarkerColor(kRed+2);
  //hits3d->SetMarkerSize(0.7);
  //hits3d->Draw("same");


  /*
    int n_int = event->Interactions.size();
    if (n_int > 0) {
    Double_t *int_x = new Double_t[n_int];
    Double_t *int_y = new Double_t[n_int];
    Double_t *int_z = new Double_t[n_int];
    for (int i = 0; i < n_int; ++i) {
    int_x[i] = event->Interactions[i].X;
    int_y[i] = event->Interactions[i].Y;
    int_z[i] = event->Interactions[i].Z;
    }
    TPolyMarker3D *ints3d = new TPolyMarker3D(n_int, int_x, int_y, int_z);
    ints3d->SetMarkerStyle(29); // Big star
    ints3d->SetMarkerColor(kBlue+2);
    ints3d->SetMarkerSize(2.0);
    ints3d->Draw("same");
    delete[] int_x; delete[] int_y; delete[] int_z;
    }
  */

    
  c1->cd(2);
  gPad->SetGrid();

  gPad->SetRightMargin(0.15);
    
  TH2F *h_xy = new TH2F("h_xy", "XY View ;X (cm);Y (cm)", 100, -AXIS_LIMIT, AXIS_LIMIT, 100, -AXIS_LIMIT, AXIS_LIMIT);
  h_xy->SetMinimum(0);
  h_xy->SetMaximum(max_e);
  h_xy->GetZaxis()->SetTitle("Energy Deposit (keV)");
  h_xy->GetZaxis()->SetMaxDigits(3);
  h_xy->DrawCopy("COLZ");

  // TH2F *h_colorbar = new TH2F("h_colorbar", "", 1, 0, 1, 100, 0, max_e);
  //h_colorbar->SetMinimum(0);
  //h_colorbar->SetMaximum(max_e);
  //h_colorbar->GetZaxis()->SetTitle("Energy Deposit (MeV)");
  //h_colorbar->Fill(0.,0.,0.);
  // h_colorbar->Draw("COLZsame");
    
  for (int i = 0; i < n_hits; ++i) {
    TGraph *g = new TGraph(1, &hit_x[i], &hit_y[i]);
    g->SetMarkerSize(0.5 + 1.0 * (hit_e[i] / max_e)); 
    g->SetMarkerColor(TColor::GetColorPalette( (int)(1000 * hit_e[i] / max_e) ));
    if(hit_type[i]==1)g->SetMarkerStyle(20);
    else if(hit_type[i]==2)g->SetMarkerStyle(25);
    g->Draw("P same");
  }

  TLine *line_xy = new TLine();
  line_xy->SetLineColor(kRed);
  line_xy->SetLineStyle(9);
    
  line_xy->DrawLine(-DET_HALF_X, -DET_HALF_Y, DET_HALF_X, -DET_HALF_Y);
  line_xy->DrawLine(DET_HALF_X, -DET_HALF_Y, DET_HALF_X, DET_HALF_Y);
  line_xy->DrawLine(DET_HALF_X, DET_HALF_Y, -DET_HALF_X, DET_HALF_Y);
  line_xy->DrawLine(-DET_HALF_X, DET_HALF_Y, -DET_HALF_X, -DET_HALF_Y);
    

  c1->cd(3);
  gPad->SetGrid();
  gPad->SetRightMargin(0.15);
    
  TH2F *h_xz = new TH2F("h_xz", "XZ View;X (cm);Z (cm)", 100, -AXIS_LIMIT, AXIS_LIMIT, 100, -AXIS_LIMIT, AXIS_LIMIT);
  h_xz->SetMinimum(0);
  h_xz->SetMaximum(max_e);
  h_xz->GetZaxis()->SetTitle("Energy Deposit (keV)");
  h_xz->GetZaxis()->SetMaxDigits(3);
  h_xz->DrawCopy("COLZ");


  for (double z_val : z_planes) {
    TLine *tline_xz = new TLine(-DET_HALF_X, z_val, DET_HALF_X, z_val);
    tline_xz->SetLineColor(kGray+2);
    tline_xz->SetLineStyle(2); 
    tline_xz->Draw("same");
  }

  for (int i = 0; i < n_hits; ++i) {
    TGraph *g = new TGraph(1, &hit_x[i], &hit_z[i]);
    g->SetMarkerSize(0.5 + 1.0 * (hit_e[i] / max_e)); 
    g->SetMarkerColor(TColor::GetColorPalette( (int)(1000 * hit_e[i] / max_e) ));
    if(hit_type[i]==1)g->SetMarkerStyle(20);
    else if(hit_type[i]==2)g->SetMarkerStyle(25);
    g->Draw("P same");
  }

  TLine *line_xz = new TLine();
  line_xz->SetLineColor(kRed);
  line_xz->SetLineStyle(9);

  line_xz->DrawLine(-DET_HALF_X, DET_DOWN_Z, DET_HALF_X, DET_DOWN_Z);
  line_xz->DrawLine(DET_HALF_X, DET_DOWN_Z, DET_HALF_X, DET_UP_Z);
  line_xz->DrawLine(DET_HALF_X, DET_UP_Z, -DET_HALF_X, DET_UP_Z);
  line_xz->DrawLine(-DET_HALF_X, DET_UP_Z, -DET_HALF_X, DET_DOWN_Z);


  TLine *cline_xz = new TLine();
  cline_xz->SetLineColor(kBlue);
  cline_xz->SetLineStyle(9);

  cline_xz->DrawLine(-DET_HALF_X, CAL_DOWN_Z, DET_HALF_X, CAL_DOWN_Z);
  cline_xz->DrawLine(DET_HALF_X, CAL_DOWN_Z, DET_HALF_X, CAL_UP_Z);
  cline_xz->DrawLine(DET_HALF_X, CAL_UP_Z, -DET_HALF_X, CAL_UP_Z);
  cline_xz->DrawLine(-DET_HALF_X, CAL_UP_Z, -DET_HALF_X, CAL_DOWN_Z);


  c1->cd(4);
  gPad->SetGrid();
  gPad->SetRightMargin(0.15);
    
  TH2F *h_yz = new TH2F("h_yz", "YZ View;Y (cm);Z (cm)", 
			100, -AXIS_LIMIT, AXIS_LIMIT, 
			100, -AXIS_LIMIT, AXIS_LIMIT);

  h_yz->SetMinimum(0);
  h_yz->SetMaximum(max_e);
  h_yz->GetZaxis()->SetTitle("Energy Deposit (keV)");
  h_yz->GetZaxis()->SetMaxDigits(3);
  h_yz->DrawCopy("COLZ");

  for (double z_val : z_planes) {
    TLine *tline_yz = new TLine(-DET_HALF_Y, z_val, DET_HALF_Y, z_val);
    tline_yz->SetLineColor(kGray+2);
    tline_yz->SetLineStyle(2); 
    tline_yz->Draw("same");
  }

  for (int i = 0; i < n_hits; ++i) {
    TGraph *g = new TGraph(1, &hit_y[i], &hit_z[i]);
    g->SetMarkerSize(0.5 + 1.0 * (hit_e[i] / max_e)); 
    g->SetMarkerColor(TColor::GetColorPalette( (int)(1000 * hit_e[i] / max_e) ));
    if(hit_type[i]==1)g->SetMarkerStyle(20);
    else if(hit_type[i]==2)g->SetMarkerStyle(25);
    g->Draw("P same");
  }
 
  TLine *line_yz = new TLine();
  line_yz->SetLineColor(kRed);
  line_yz->SetLineStyle(9);


  line_yz->DrawLine(-DET_HALF_Y, DET_DOWN_Z, DET_HALF_Y, DET_DOWN_Z);
  line_yz->DrawLine(DET_HALF_Y, DET_DOWN_Z, DET_HALF_Y, DET_UP_Z);
  line_yz->DrawLine(DET_HALF_Y, DET_UP_Z, -DET_HALF_Y, DET_UP_Z);
  line_yz->DrawLine(-DET_HALF_Y, DET_UP_Z, -DET_HALF_Y, DET_DOWN_Z);

  TLine *cline_yz = new TLine();
  cline_yz->SetLineColor(kBlue);
  cline_yz->SetLineStyle(9);


  cline_yz->DrawLine(-DET_HALF_Y, CAL_DOWN_Z, DET_HALF_Y, CAL_DOWN_Z);
  cline_yz->DrawLine(DET_HALF_Y, CAL_DOWN_Z, DET_HALF_Y, CAL_UP_Z);
  cline_yz->DrawLine(DET_HALF_Y, CAL_UP_Z, -DET_HALF_Y, CAL_UP_Z);
  cline_yz->DrawLine(-DET_HALF_Y, CAL_UP_Z, -DET_HALF_Y, CAL_DOWN_Z);

  c1->cd();
  c1->Update();

  delete[] hit_type;
  delete[] hit_x;
  delete[] hit_y;
  delete[] hit_z;
  delete[] hit_e;

  f->Close(); 
}
