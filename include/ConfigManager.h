#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "Rtypes.h"
#include <map>
#include <string>

// Forward declarations
class EventCutConfig;
class TrackCutConfig;
class PIDCutConfig;
class V0CutConfig;
class PhiCutConfig;
class LambdaCutConfig;
class Lambda1520CutConfig;
class Sigma1385CutConfig;
class MixingConfig;

class ConfigManager {
public:
  static ConfigManager& GetInstance();
  Bool_t LoadConfig(const Char_t* mainConfigPath = "config/main.yaml");

  /** Return full path for hist config from main.yaml key "hist".
   *  Returns empty string if key missing or value empty. */
  std::string GetHistConfigPath();

  // Access to cut config classes
  EventCutConfig& GetEventCuts();
  TrackCutConfig& GetTrackCuts();
  PIDCutConfig& GetPIDCuts();
  V0CutConfig& GetV0Cuts();
  PhiCutConfig& GetPhiCuts();
  LambdaCutConfig& GetLambdaCuts();
  Lambda1520CutConfig& GetLambda1520Cuts();
  Sigma1385CutConfig& GetSigma1385Cuts();
  MixingConfig& GetMixingConfig();

private:
  ConfigManager();
  ~ConfigManager();
  ConfigManager(const ConfigManager&);
  ConfigManager& operator=(const ConfigManager&);

  Bool_t ParseMainConfig(const Char_t* filename);
  Bool_t LoadConfigFile(const Char_t* basePath, const Char_t* relativePath, const Char_t* configType);

  EventCutConfig* eventCuts;
  TrackCutConfig* trackCuts;
  PIDCutConfig* pidCuts;
  V0CutConfig* v0Cuts;
  PhiCutConfig* phiCuts;
  LambdaCutConfig* lambdaCuts;
  Lambda1520CutConfig* lambda1520Cuts;
  Sigma1385CutConfig* sigma1385Cuts;
  MixingConfig* mixingConfig;

  Bool_t isLoaded;
  std::map<std::string, std::string> m_mainConfigValues;  ///< Parsed key-value from main.yaml
  std::string m_configBasePath;  ///< Project root (path before /config/, trailing slash included)
};

#endif

