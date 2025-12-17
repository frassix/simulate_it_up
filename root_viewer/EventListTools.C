#include "EventListTools.h"

#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>

#include "TFile.h"
#include "TTree.h"

#include "EventData.h"

// -----------------------------
// Helper: Z -> layer mapping
// Uses the same logic you used in AnalyzeEvents.C
// Returns 0 if not assigned to any layer
// -----------------------------
static int ComputeLayerFromZ(float z_cm)
{
  int   layerhit        = 0;
  float current_Z_limit = 11.5f;
  for (int layer = 1; layer <= 10; ++layer) {
    if (z_cm > current_Z_limit) {
      layerhit = layer;
      break;
    }
    current_Z_limit -= 1.5f;
  }
  return layerhit;
}

// -----------------------------
// Helper: event matches config?
// -----------------------------
static bool EventMatches(const EventData& event, const EventListConfig& cfg)
{
  auto hitPassesIndex = [&](const HitData& h) {
    return (!cfg.requireHitIndex1) || (h.Index == 1);
  };

  if (cfg.mode == SelectionMode::kEnergyRange) {
    const float emin = std::min(cfg.emin, cfg.emax);
    const float emax = std::max(cfg.emin, cfg.emax);

    for (const auto& hit : event.Hits) {
      if (!hitPassesIndex(hit)) continue;
      if (hit.EnergyDeposit >= emin && hit.EnergyDeposit <= emax) {
        return true;
      }
    }
    return false;
  }

  if (cfg.mode == SelectionMode::kLayer) {
    const int target = cfg.layer;
    for (const auto& hit : event.Hits) {
      if (!hitPassesIndex(hit)) continue;
      const int layerhit = ComputeLayerFromZ(hit.Z);
      if (layerhit == target) {
        return true;
      }
    }
    return false;
  }

  return false;
}

// -----------------------------
// Core: collect EventIDs
// -----------------------------
static std::vector<int> CollectEventIDs(TTree* tree, const EventListConfig& cfg)
{
  std::vector<int> ids;
  if (!tree) return ids;

  EventData* event = nullptr;
  tree->SetBranchAddress("Event", &event);

  const Long64_t nEntries = tree->GetEntries();
  ids.reserve(static_cast<size_t>(std::max<Long64_t>(nEntries, 0)));

  std::unordered_set<int> seen;
  if (cfg.uniqueEventIDs) {
    seen.reserve(static_cast<size_t>(nEntries));
  }

  for (Long64_t i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    if (!event) continue;

    if (EventMatches(*event, cfg)) {
      const int id = event->EventID;  // IMPORTANT: returning ONLY EventID (as requested)

      if (cfg.uniqueEventIDs) {
        if (seen.insert(id).second) {
          ids.push_back(id);
        }
      } else {
        ids.push_back(id);
      }
    }
  }

  return ids;
}

// -----------------------------
// Public API: from file
// -----------------------------
void PrintEventList(const char* input_root_file, const EventListConfig& cfg)
{
  TFile* f = TFile::Open(input_root_file);
  if (!f || f->IsZombie()) {
    std::cerr << "ERROR: cannot open input ROOT file: " << input_root_file << std::endl;
    if (f) { f->Close(); delete f; }
    return;
  }

  TTree* tree = dynamic_cast<TTree*>(f->Get("Events"));
  if (!tree) {
    std::cerr << "ERROR: TTree 'Events' not found in: " << input_root_file << std::endl;
    f->Close(); delete f;
    return;
  }

  PrintEventList(tree, cfg);

  f->Close();
  delete f;
}

void WriteEventListRoot(const char* input_root_file,
                        const char* output_root_file,
                        const EventListConfig& cfg)
{
  TFile* f = TFile::Open(input_root_file);
  if (!f || f->IsZombie()) {
    std::cerr << "ERROR: cannot open input ROOT file: " << input_root_file << std::endl;
    if (f) { f->Close(); delete f; }
    return;
  }

  TTree* tree = dynamic_cast<TTree*>(f->Get("Events"));
  if (!tree) {
    std::cerr << "ERROR: TTree 'Events' not found in: " << input_root_file << std::endl;
    f->Close(); delete f;
    return;
  }

  WriteEventListRoot(tree, output_root_file, cfg);

  f->Close();
  delete f;
}

// -----------------------------
// Public API: from in-memory tree
// -----------------------------
void PrintEventList(TTree* eventsTree, const EventListConfig& cfg)
{
  if (!eventsTree) {
    std::cerr << "ERROR: PrintEventList got null TTree*" << std::endl;
    return;
  }

  const auto ids = CollectEventIDs(eventsTree, cfg);

  std::cout << "Selected EventIDs: " << ids.size() << std::endl;
  for (const int id : ids) {
    std::cout << id << std::endl;
  }
}

void WriteEventListRoot(TTree* eventsTree, const char* output_root_file, const EventListConfig& cfg)
{
  if (!eventsTree) {
    std::cerr << "ERROR: WriteEventListRoot got null TTree*" << std::endl;
    return;
  }

  const auto ids = CollectEventIDs(eventsTree, cfg);

  TFile out(output_root_file, "RECREATE");
  if (out.IsZombie()) {
    std::cerr << "ERROR: cannot create output ROOT file: " << output_root_file << std::endl;
    return;
  }

  Int_t EventID = 0;
  TTree outTree("EventList", "Selected Event IDs");
  outTree.Branch("EventID", &EventID, "EventID/I");

  for (int id : ids) {
    EventID = id;
    outTree.Fill();
  }

  out.cd();
  outTree.Write();
  out.Close();

  std::cout << "Wrote " << ids.size() << " EventIDs to: " << output_root_file << std::endl;
}
