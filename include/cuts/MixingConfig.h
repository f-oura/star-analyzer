#ifndef MIXING_CONFIG_H
#define MIXING_CONFIG_H

#include "Rtypes.h"

class MixingConfig {
public:
  static MixingConfig& GetInstance();
  Bool_t LoadFromFile(const Char_t* filename);
  
  // Config values (public member variables)
  Int_t nVzBins;
  Int_t nCentralityBins;
  Int_t nEventPlaneBins;
  Int_t bufferSize;

  // Set default values
  void SetDefaults();
  
private:
  MixingConfig();
  ~MixingConfig();
  MixingConfig(const MixingConfig&);
  MixingConfig& operator=(const MixingConfig&);
  
  Bool_t ParseYamlFile(const Char_t* filename);
};

#endif

