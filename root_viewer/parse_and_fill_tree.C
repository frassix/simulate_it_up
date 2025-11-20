#include "parse_and_fill_tree.h"

void parse_and_fill_tree(const char* inputFile, const char* outputFile) {

  TFile *file = new TFile(outputFile, "RECREATE");

  TTree *tree = new TTree("Events", "Parsed Simulation Events");

  RunInfo *runInfo = new RunInfo();
  tree->Branch("RunInfo", "RunInfo", &runInfo, 64000, 0);
  EventData *event = new EventData();
  tree->Branch("Event", "EventData", &event, 64000, 99); 

  std::ifstream input(inputFile);
  if (!input.is_open()) {
    std::cerr << "Error: Could not open input file " << inputFile << std::endl;
    delete file;
    delete event;
    return;
  }

  std::string line;
  std::cout << "Starting parsing process..." << std::endl;

  bool runInfoFilled = false;
  bool start_parsing = false;

  while (std::getline(input, line)) {
    std::stringstream ss(line);
    std::string tag;
    ss >> tag;

    if (tag == "SimulationStartAreaFarField") {
      ss >> runInfo->SimStartAreaFarField;
    }else if (tag == "BeamType") {
      std::string type_str;
      if (!(ss >> type_str)) { continue; }
      runInfo->BeamType = type_str;
      if (!(ss >> runInfo->BeamTheta)) { continue; }
      if (!(ss >> runInfo->BeamPhi)) { continue; }
    } else if (tag == "SpectralType") {
      std::string type_str;
      if (!(ss >> type_str)) { continue; }
      runInfo->SpectralType = type_str;
      if (!(ss >> runInfo->SpectralEnergy)) { continue; }
    }	

    if (tag == "TB") {
      start_parsing = true;
      cout << runInfo->SimStartAreaFarField << "   "
	   << runInfo->BeamType << "   "
	   << runInfo->BeamTheta << "   "
	   << runInfo->BeamPhi << "   "
	   << runInfo->SpectralType << "   "
	   << runInfo->SpectralEnergy << "  " 
	   <<  endl;
      if (!runInfoFilled) {
	//tree->Fill(); 
	runInfoFilled = true;
      }	  
    }else if (!start_parsing) {
      continue; 
    }
	
    if (tag == "EN") {
      if (event->TriggerID != 0)
	tree->Fill();
      break; 
    }

    if (tag == "SE") {
      if (event->TriggerID != 0) {
	tree->Fill(); 
      }
      *event = EventData(); 
    } 
    else if (tag == "ID") {
      ss >> event->TriggerID >> event->EventID;
    } 
    else if (tag == "TI") { ss >> event->InitialTime; }
    else if (tag == "ED") { ss >> event->TotDepositedEnergy; }
    else if (tag == "EC") { ss >> event->EscapedEnergy; }
    else if (tag == "NS") { ss >> event->NSMaterialEnergy; }
    else if (tag == "PM") { 
      std::string module_str;
      Float_t energy_val;
      ss >> module_str; 
      event->PhysicsModuleType = module_str;
      ss >> energy_val;
      event->PhysicsModuleEnergy = energy_val;	  
    }
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
	    
            
      event->Interactions.push_back(ia);
    }
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
      while(std::getline(ss, segment, ';')){
	std::stringstream(segment) >> particleID;
	hit.PrimaryParticleIDs.push_back(particleID);
      }
	    
      hit.Multiplicity = hit.PrimaryParticleIDs.size();
	    
      event->Hits.push_back(hit);
    }
  }
    

  // --- Cleanup ---
  std::cout << "Parsing complete. Total events written: " << tree->GetEntries() << std::endl;
  file->Write();
  file->Close();
  delete event;
  delete file;
}
