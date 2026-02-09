#include "cuts/MixingConfig.h"
#include "YamlParser.h"
#include <map>
#include <string>
#include <iostream>

MixingConfig& MixingConfig::GetInstance() {
  static MixingConfig instance;
  return instance;
}

MixingConfig::MixingConfig() {
  SetDefaults();
}

MixingConfig::~MixingConfig() {
}

void MixingConfig::SetDefaults() {
  nVzBins = 10;
  nCentralityBins = 9;
  nEventPlaneBins = 1;
  bufferSize = 20;
}

Bool_t MixingConfig::LoadFromFile(const Char_t* filename) {
  return ParseYamlFile(filename);
}

Bool_t MixingConfig::ParseYamlFile(const Char_t* filename) {
  std::map<std::string, std::string> values;
  if (!YamlParser::ParseFile(filename, values)) {
    std::cerr << "WARNING: Failed to parse " << filename 
              << ", using default values" << std::endl;
    return kFALSE;
  }
  
  if (values.find("nVzBins") != values.end()) {
    nVzBins = YamlParser::ToInt(values["nVzBins"], nVzBins);
  }
  if (values.find("nCentralityBins") != values.end()) {
    nCentralityBins = YamlParser::ToInt(values["nCentralityBins"], nCentralityBins);
  }
  if (values.find("nEventPlaneBins") != values.end()) {
    nEventPlaneBins = YamlParser::ToInt(values["nEventPlaneBins"], nEventPlaneBins);
  }
  if (values.find("bufferSize") != values.end()) {
    bufferSize = YamlParser::ToInt(values["bufferSize"], bufferSize);
  }
  
  return kTRUE;
}

