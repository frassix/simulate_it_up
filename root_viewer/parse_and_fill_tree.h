#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#include "TObject.h"

//

struct RunInfo : public TObject {
  Float_t SimStartAreaFarField; 
  TString BeamType;
  Float_t BeamTheta;
  Float_t BeamPhi;
  TString SpectralType;
  Float_t SpectralEnergy;
     
  ClassDef(RunInfo, 1)
};


// lines with IA

struct InteractionData : public TObject {
  TString Type; 
  Int_t Index;
  Int_t ParentInteractionID;
  Int_t DetectorID;
  Double_t Time; 
  Float_t X, Y, Z;
  Int_t MotherParticleCode;
  Float_t Px_in, Py_in, Pz_in;
  Float_t Dx_in, Dy_in, Dz_in;
  Float_t Energy_in;
  Int_t OutgoingParticleCode;
  Float_t Px_out, Py_out, Pz_out;
  Float_t Dx_out, Dy_out, Dz_out;
  Float_t Energy_out;
  // Int_t InteractionID;


  ClassDef(InteractionData, 1) 
};
ClassImp(InteractionData)


// Lines with HITSim 

struct HitData : public TObject {
    Int_t Index;
    Float_t X, Y, Z;
    Float_t EnergyDeposit;
    Double_t Time;
    Int_t Multiplicity;
    std::vector<Int_t> PrimaryParticleIDs; 

    ClassDef(HitData, 1)
};
ClassImp(HitData)


// Saving main event informations
// Event block starts with SE

struct EventData : public TObject {
  Int_t TriggerID = 0; 
  Int_t EventID;
  Double_t InitialTime;
  Float_t TotDepositedEnergy;
  Float_t EscapedEnergy; //guess
  Float_t NSMaterialEnergy;
  TString PhysicsModuleType;
  Float_t PhysicsModuleEnergy;

  std::vector<InteractionData> Interactions;
  std::vector<HitData> Hits;

  ClassDef(EventData, 1)
};
ClassImp(EventData)


