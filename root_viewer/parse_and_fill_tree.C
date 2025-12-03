#include "parse_and_fill_tree.h"
#include "EventData.h"

// -------------------------------------------------------------------
// Core parsing routine: parse from an input stream into an existing TTree
// This is what we will reuse in the integrated pipeline.
// -------------------------------------------------------------------
void parse_and_fill_tree_core(std::istream& input,
                              TTree&        tree,
                              RunInfo&      runInfo,
                              EventData&    event)
{
  std::string line;
  std::cout << "Starting parsing process..." << std::endl;

  bool start_parsing = false;

  while (std::getline(input, line)) {
    std::stringstream ss(line);
    std::string tag;
    ss >> tag;

    // ---- Run-level info (before TB / events) ----
    if (tag == "SimulationStartAreaFarField") {
      ss >> runInfo.SimStartAreaFarField;
    } else if (tag == "BeamType") {
      std::string type_str;
      if (!(ss >> type_str)) { continue; }
      runInfo.BeamType = type_str;
      if (!(ss >> runInfo.BeamTheta)) { continue; }
      if (!(ss >> runInfo.BeamPhi))   { continue; }
    } else if (tag == "SpectralType") {
      std::string type_str;
      if (!(ss >> type_str)) { continue; }
      runInfo.SpectralType = type_str;
      if (!(ss >> runInfo.SpectralEnergy)) { continue; }
    }

    // "TB" marks the beginning of the event block
    if (tag == "TB") {
      start_parsing = true;
      std::cout << runInfo.SimStartAreaFarField << "   "
                << runInfo.BeamType            << "   "
                << runInfo.BeamTheta           << "   "
                << runInfo.BeamPhi             << "   "
                << runInfo.SpectralType        << "   "
                << runInfo.SpectralEnergy      << std::endl;
    } else if (!start_parsing) {
      // Ignore everything until TB appears
      continue;
    }

    // ---- End of simulation ----
    if (tag == "EN") {
      if (event.TriggerID != 0) {
        tree.Fill();
      }
      break; // Done parsing
    }

    // ---- Event-level tags ----
    if (tag == "SE") {
      // Start of a new event: if we have an existing event, store it
      if (event.TriggerID != 0) {
        tree.Fill();
      }
      // Reset event to a fresh default-constructed state
      event = EventData();
    }
    else if (tag == "ID") {
      ss >> event.TriggerID >> event.EventID;
    }
    else if (tag == "TI") {
      ss >> event.InitialTime;
    }
    else if (tag == "ED") {
      ss >> event.TotDepositedEnergy;
    }
    else if (tag == "EC") {
      ss >> event.EscapedEnergy;
    }
    else if (tag == "NS") {
      ss >> event.NSMaterialEnergy;
    }
    else if (tag == "PM") {
      std::string module_str;
      Float_t energy_val;
      ss >> module_str;
      event.PhysicsModuleType = module_str;
      ss >> energy_val;
      event.PhysicsModuleEnergy = energy_val;
    }

    // ---- Interaction lines (IA) ----
    else if (tag == "IA") {
      InteractionData ia;
      std::string type_str;
      ss >> type_str;
      ia.Type = type_str;

      std::string segment;

      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Index;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.ParentInteractionID;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.DetectorID;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Time;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.X;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Y;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Z;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.MotherParticleCode;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Px_in;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Py_in;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Pz_in;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Dx_in;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Dy_in;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Dz_in;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Energy_in;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.OutgoingParticleCode;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Px_out;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Py_out;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Pz_out;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Dx_out;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Dy_out;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Dz_out;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> ia.Energy_out;

      event.Interactions.push_back(ia);
    }

    // ---- Hit lines (HTsim) ----
    else if (tag == "HTsim") {
      HitData hit;
      std::string segment;

      hit.Multiplicity = 0;
      hit.PrimaryParticleIDs.clear();
      Int_t particleID;

      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> hit.Index;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> hit.X;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> hit.Y;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> hit.Z;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> hit.EnergyDeposit;
      if (!std::getline(ss, segment, ';')) { continue; }
      std::stringstream(segment) >> hit.Time;

      while (std::getline(ss, segment, ';')) {
        std::stringstream(segment) >> particleID;
        hit.PrimaryParticleIDs.push_back(particleID);
      }

      hit.Multiplicity = static_cast<Int_t>(hit.PrimaryParticleIDs.size());

      event.Hits.push_back(hit);
    }

  } // end while lines

  std::cout << "Parsing complete. Total events in tree: " << tree.GetEntries() << std::endl;
}

// -------------------------------------------------------------------
// Legacy / standalone interface: .sim -> .root with TTree "Events"
// This keeps your old workflow working, but the *core* logic above
// is what we’ll reuse for the integrated pipeline.
// -------------------------------------------------------------------
void parse_and_fill_tree(const char* inputFile, const char* outputFile)
{
  // Create output ROOT file
  TFile file(outputFile, "RECREATE");
  if (file.IsZombie()) {
    std::cerr << "Error: Could not create output file " << outputFile << std::endl;
    return;
  }

  // Create TTree and branches
  TTree tree("Events", "Parsed Simulation Events");

  RunInfo  runInfo;
  EventData event;

  tree.Branch("RunInfo", &runInfo);
  tree.Branch("Event",  &event, 64000, 99);

  // Open input .sim file
  std::ifstream input(inputFile);
  if (!input.is_open()) {
    std::cerr << "Error: Could not open input file " << inputFile << std::endl;
    return;
  }

  // Use the core parser
  parse_and_fill_tree_core(input, tree, runInfo, event);

  // Write to disk (for the old workflow)
  file.cd();
  tree.Write();
  file.Close();
}
