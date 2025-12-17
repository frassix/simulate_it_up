#ifndef EVENT_DISPLAY_H
#define EVENT_DISPLAY_H

#include <string>
#include <vector>

#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TPad.h"
#include "TGraph.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TLine.h"
#include "TPolyMarker3D.h"
#include "TView.h"
#include "TGraph2D.h"
#include "TLatex.h"

// Importa le strutture dati comuni:
#include "EventData.h"

// Qui metti SOLO dichiarazioni legate al display.
// Esempio (adatta ai tuoi nomi reali):
void EventDisplay(const char* input_root_file,
                  Long64_t entry = 0,
                  bool onlyIndex1Hits = true);

#endif
