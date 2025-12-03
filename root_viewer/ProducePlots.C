#include "TFile.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TF1.h" 
#include "TStyle.h" 
#include "TGraph.h"
#include "TLegend.h"
#include <iostream>

void ProducePlots(const char* resultsFile = "results.root", double E=0) {
    
    
    gStyle->SetPalette(kRainBow);
    gStyle->SetPadGridX(kTRUE);
    gStyle->SetPadGridY(kTRUE);

    gStyle->SetOptStat(0); 
    gStyle->SetOptFit(1111);
   
    gStyle->SetStatX(0.4); // Posizione X destra del box Stat/Fit (Default)
    gStyle->SetStatY(0.90); // Posizione Y superiore
    gStyle->SetStatW(0.15); // Larghezza del box
    gStyle->SetStatH(0.15); // Altezza del box

 
    TFile *f = TFile::Open(resultsFile);
    if (!f || f->IsZombie()) {
        std::cerr << "ERROR: Cannot open results file: " << resultsFile << std::endl;
        return;
    }


    TH2F *h2EdepTrackerVsZ = (TH2F*)f->Get("h2EdepTrackerVsZ"); 
    TProfile *pEdepVsZ = (TProfile*)f->Get("pEdepVsZ");
    TH2F *h2TrackerXvsY = (TH2F*)f->Get("h2TrackerXvsY");
    TH1F *hEdepTrackerLayerALL = (TH1F*)f->Get("hEdepTrackerLayerALL");

    if (!h2EdepTrackerVsZ || !pEdepVsZ || !h2TrackerXvsY || !hEdepTrackerLayerALL) {
        std::cerr << "ERROR: One or more required histograms/profiles could not be found." << std::endl;
	if (!h2EdepTrackerVsZ) cout << "MISSING h2EdepTrackerVsZ" << endl;
	if (!pEdepVsZ) cout << "MISSING pEdepVsZ" << endl;
	if (!h2TrackerXvsY) cout << "MISSING h2TrackerXvsY" << endl;
	if (!hEdepTrackerLayerALL) cout << "MISSING hEdepTrackerLayerALL" << endl;	
	f->Close();
        return;
    }
    

    TCanvas *c1 = new TCanvas("c1_EdepVsZ", "Energy Deposit Profile (2D)", 900, 600);
    h2EdepTrackerVsZ->GetYaxis()->SetRangeUser(0.0, 1500.0);
    // h2EdepTrackerVsZ->GetXaxis()->SetRangeUser(0.0, 50.0);
    h2EdepTrackerVsZ->Draw("COLZ");
    c1->SetRightMargin(0.14); 
    c1->SetLeftMargin(0.15);  
    c1->SetTopMargin(0.08);
    //c1->Update();
    c1->Print("plot_Ezoom_EdepVsZ.png"); 
    
    TCanvas *c2 = new TCanvas("c2_ProfileFit", "Average Energy Profile with Fit", 900, 600);
    pEdepVsZ->Draw("E1");
    
    TF1 *f_const = new TF1("f_const", "pol0", pEdepVsZ->GetXaxis()->GetXmin(), pEdepVsZ->GetXaxis()->GetXmax());
    pEdepVsZ->Fit(f_const, "SRE"); 
    // c2->Update();
    c2->Print("plot_Ezoom_AverageProfile_Fit.png");
    

    TCanvas *c3 = new TCanvas("c3_TransverseMap", "Hit Map X vs Y", 800, 800);
    h2TrackerXvsY->Draw("COLZ");
    c3->SetRightMargin(0.14); 
    c3->SetLeftMargin(0.15);  
    c3->SetTopMargin(0.08);
    //c3->Update();
    c3->Print("plot_Ezoom_TransverseMap.png");

    gStyle->SetOptStat(1111);
    gStyle->SetStatX(0.9); // Posizione X destra del box Stat/Fit (Default)
    gStyle->SetStatY(0.90); // Posizione Y superiore
    gStyle->SetStatW(0.20); // Larghezza del box
    gStyle->SetStatH(0.20); // Altezza del box


    Double_t CE_position = 0;
    
    //Compton Edge
    if(E==100)   CE_position= 28.13;
    if(E==300)   CE_position= 162.02;
    if(E==500)   CE_position= 330.91;
    if(E==700)   CE_position= 512.82;
    if(E==1000)  CE_position= 796.50;
    if(E==3000)  CE_position= 2764.55;
    if(E==5000)  CE_position= 4756.92;
    if(E==7000)  CE_position= 6753.50;
    if(E==10000) CE_position= 9750.87;
    

    Double_t y_min = hEdepTrackerLayerALL->GetMinimum();
    Double_t y_max = hEdepTrackerLayerALL->GetMaximum();

    TLine *v_line = new TLine(CE_position, y_min, CE_position, y_max);

    v_line->SetLineColor(kRed);
    v_line->SetLineWidth(3);
    v_line->SetLineStyle(1);
    
    const Int_t n_quantiles = 3;
    Double_t quantiles[n_quantiles]={0};         
    Double_t probabilities[n_quantiles]={0.90,0.95,0.99};

    hEdepTrackerLayerALL->GetQuantiles(n_quantiles, quantiles, probabilities);

    std::cout << "--- Calculated Quantiles ---" << std::endl;
    std::cout << "90th Percentile: " << quantiles[0] << std::endl;
    std::cout << "95th Percentile: " << quantiles[1] << std::endl;
    std::cout << "99th Percentile: " << quantiles[2] << std::endl;

    TLine *q1_line = new TLine(quantiles[0], y_min, quantiles[0], y_max);
    q1_line->SetLineColor(kGreen+2);
    q1_line->SetLineWidth(3);
    q1_line->SetLineStyle(4);

    TLine *q2_line = new TLine(quantiles[1], y_min, quantiles[1], y_max);
    q2_line->SetLineColor(kCyan+2);
    q2_line->SetLineWidth(3);
    q2_line->SetLineStyle(6);

    TLine *q3_line = new TLine(quantiles[2], y_min, quantiles[2], y_max);
    q3_line->SetLineColor(kMagenta+2);
    q3_line->SetLineWidth(3);
    q3_line->SetLineStyle(8);

      
    TCanvas *c4 = new TCanvas("c4_Spectrum", "Energy Deposit Spectrum (All Events)", 800, 600);
    hEdepTrackerLayerALL->Draw("HIST");
    hEdepTrackerLayerALL->GetXaxis()->SetRangeUser(0.0, 1500);
    hEdepTrackerLayerALL->GetYaxis()->SetRangeUser(0.0, 130000);
    hEdepTrackerLayerALL->SetFillColor(kBlue);
    v_line->Draw("same");
    q1_line->Draw("same");
    q2_line->Draw("same");
    q3_line->Draw("same");   
    c4->Update();
    c4->Print("plot_Ezoom_EnergySpectrum.png");
    
 
    f->Close();
    //  delete f;
    //delete f_const; 
    
    std::cout << "\nAnalysis complete. All plots saved to current directory as PNG files." << std::endl;
}

void HardCodedPerc(){
    //float perc_90[] = {68.3535, 78.7843, 132.448, 176.385, 197.768, 216.113, 226.790, 228.937, 231.542, 229.864, 190.951, 184.898, 184.853, 187.849};
    //float perc_95[] = {93.9143, 85.9935, 148.095, 204.306, 235.637, 257.888, 273.354, 275.644, 279.568, 281.255, 242.211, 231.472, 231.780, 233.674};
    //float perc_99[] = {99.9770, 155.395, 163.775, 235.988, 296.935, 331.019, 363.623, 366.154, 369.450, 371.823, 350.141, 344.384, 333.087, 341.964};

    float perc_90[] = {68.9587, 212.131, 244.153, 216.509, 214.560, 226.646};
    float perc_95[] = {93.1682, 250.241, 293.539, 269.050, 263.835, 272.871};
    float perc_99[] = {99.9708, 306.352, 388.979, 382.279, 377.336, 386.884};
    float energy[]  = {   100.,    500.,   1000.,   3000.,   5000.,  10000.};

    //float energy[]  = {   100.,    200.,    300.,    400.,    500.,    600.,    700.,    800.,    900.,    1000.,  3000.,   5000.,   7000.,  10000.};
    
    float compton_edge[] = {28.13,   162.02,  330.91,  512.82,  796.50,  2764.55,  4756.92,  6753.50,  9750.87};
    float edge_energy[]  = {  100.,    300.,    500.,    700.,   1000.,    3000.,    5000.,    7000.,   10000.};

    TCanvas *c4 = new TCanvas("Energy deposited", "Energy Deposit per pixel", 800, 600);
    TGraph *g1 = new TGraph(6, energy, perc_99);
    TGraph *g2 = new TGraph(6, energy, perc_95);
    TGraph *g3 = new TGraph(6, energy, perc_90);
    TGraph *g4 = new TGraph(9, edge_energy, compton_edge);


    g1->SetTitle("Energy deposited in the tracker - Pitch 250 um, Thickness 700 um");
    g1->GetXaxis()->SetTitle("Source Energy [keV]");
    g1->GetYaxis()->SetTitle("Energy Deposited [keV]");

    g1->SetMarkerStyle(21);
    g2->SetMarkerStyle(21);
    g3->SetMarkerStyle(21);
    g4->SetMarkerStyle(21);

    g1->SetMarkerSize(1);
    g2->SetMarkerSize(1);
    g3->SetMarkerSize(1);
    g4->SetMarkerSize(1);

    g1->SetMarkerColor(kViolet-6);
    g2->SetMarkerColor(kCyan-3);
    g3->SetMarkerColor(kGreen+2);
    g4->SetMarkerColor(kRed);

    g1->SetLineColor(kViolet-6);
    g2->SetLineColor(kCyan-3);
    g3->SetLineColor(kGreen+2);
    g4->SetLineColor(kRed);

    g1->SetLineStyle(6);
    g2->SetLineStyle(6);
    g3->SetLineStyle(6);

    g1->SetLineWidth(2);
    g2->SetLineWidth(2);
    g3->SetLineWidth(2);
    g4->SetLineWidth(2);

    auto legend = new TLegend(0.6,0.15,0.85,0.3);
    legend->AddEntry(g4,"Compton Edge (Theo)","pl");
    legend->AddEntry(g3,"90th Percentile","pl");
    legend->AddEntry(g2,"95th Percentile","pl");
    legend->AddEntry(g1,"99th Percentile","pl");
    

    g1->Draw("APL");
    g2->Draw("PLsame");
    g3->Draw("PLsame");
    g4->Draw("PLsame");
    legend->Draw("same");
}


