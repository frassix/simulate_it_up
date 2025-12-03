#ifndef EVENTDATA_H
#define EVENTDATA_H

#include <TObject.h>
#include <TString.h>
#include <vector>

//--------------------------------------------------
// Run-level information
//--------------------------------------------------
struct RunInfo : public TObject {
  Float_t  SimStartAreaFarField; 
  TString  BeamType;
  Float_t  BeamTheta;
  Float_t  BeamPhi;
  TString  SpectralType;
  Float_t  SpectralEnergy;

  RunInfo() = default;
  virtual ~RunInfo() = default;

  ClassDef(RunInfo, 1)
};

//--------------------------------------------------
// Lines with IA: interaction-level information
//--------------------------------------------------
struct InteractionData : public TObject {
  TString Type; 
  Int_t   Index;
  Int_t   ParentInteractionID;
  Int_t   DetectorID;
  Double_t Time; 
  Float_t  X, Y, Z;
  Int_t   MotherParticleCode;
  Float_t Px_in, Py_in, Pz_in;
  Float_t Dx_in, Dy_in, Dz_in;
  Float_t Energy_in;
  Int_t   OutgoingParticleCode;
  Float_t Px_out, Py_out, Pz_out;
  Float_t Dx_out, Dy_out, Dz_out;
  Float_t Energy_out;
  // Int_t InteractionID;

  InteractionData() = default;
  virtual ~InteractionData() = default;

  ClassDef(InteractionData, 1) 
};

//--------------------------------------------------
// Lines with HITSim: hit-level information
//--------------------------------------------------
struct HitData : public TObject {
  Int_t   Index;
  Float_t X, Y, Z;
  Float_t EnergyDeposit;
  Double_t Time;
  Int_t   Multiplicity;
  std::vector<Int_t> PrimaryParticleIDs; 

  HitData() = default;
  virtual ~HitData() = default;

  ClassDef(HitData, 1)
};

//--------------------------------------------------
// Event block (starts with SE in the sim file)
//--------------------------------------------------
struct EventData : public TObject {
  Int_t    TriggerID;            // was defaulted to 0
  Int_t    EventID;
  Double_t InitialTime;
  Float_t  TotDepositedEnergy;
  Float_t  EscapedEnergy;        // guess
  Float_t  NSMaterialEnergy;
  TString  PhysicsModuleType;
  Float_t  PhysicsModuleEnergy;

  std::vector<InteractionData> Interactions;
  std::vector<HitData>         Hits;

  EventData() 
    : TriggerID(0),
      EventID(0),
      InitialTime(0.0),
      TotDepositedEnergy(0.0),
      EscapedEnergy(0.0),
      NSMaterialEnergy(0.0),
      PhysicsModuleType(""),
      PhysicsModuleEnergy(0.0) {}

  virtual ~EventData() = default;

  ClassDef(EventData, 1)
};

#endif // EVENTDATA_H
