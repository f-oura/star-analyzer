#ifndef EVENT_CUT_CONFIG_H
#define EVENT_CUT_CONFIG_H

#include "Rtypes.h"

class EventCutConfig {
public:
  static EventCutConfig& GetInstance();
  Bool_t LoadFromFile(const Char_t* filename);
  
  // Cut values (public member variables)
  Double_t maxVz;
  Double_t maxVr;
  Double_t minRefMult;
  Double_t maxRefMult;
  Double_t maxVzDiff;
  Int_t maxNTr;  // 0 or negative = no limit; event skipped when nTracks > maxNTr

  // Set default values
  void SetDefaults();
  
private:
  EventCutConfig();
  ~EventCutConfig();
  EventCutConfig(const EventCutConfig&);  // Copy constructor (disabled)
  EventCutConfig& operator=(const EventCutConfig&);  // Assignment operator (disabled)
  
  Bool_t ParseYamlFile(const Char_t* filename);
};

#endif

