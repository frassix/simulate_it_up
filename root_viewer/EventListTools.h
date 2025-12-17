#ifndef EVENTLISTTOOLS_H
#define EVENTLISTTOOLS_H

#include <string>
class TTree;

// Selection configuration
enum class SelectionMode {
  kEnergyRange,
  kLayer
};

struct EventListConfig {
  SelectionMode mode = SelectionMode::kEnergyRange;

  // Energy selection (keV)
  float emin = 0.0f;
  float emax = 0.0f;

  // Layer selection (1..10)
  int layer = 1;

  // Optional filters
  bool requireHitIndex1 = true;   // mimic your analyzer: consider only hits with hit.Index == 1
  bool uniqueEventIDs   = true;   // de-duplicate EventIDs
};

// Print selected EventIDs to stdout
void PrintEventList(const char* input_root_file, const EventListConfig& cfg);

// Write selected EventIDs to a ROOT file (TTree "EventList", branch "EventID")
void WriteEventListRoot(const char* input_root_file,
                        const char* output_root_file,
                        const EventListConfig& cfg);

// Same operations, but starting from an in-memory tree (for your integrated pipeline use-case)
void PrintEventList(TTree* eventsTree, const EventListConfig& cfg);
void WriteEventListRoot(TTree* eventsTree, const char* output_root_file, const EventListConfig& cfg);

#endif
