#ifndef StLambdaMaker_h
#define StLambdaMaker_h

#include "StMaker.h"
#include "StarClassLibrary/StPhysicalHelixD.hh"

class StPicoDst;
class StPicoDstMaker;
class StPicoEvent;
class StPicoTrack;
class TString;
class HistManager;
class TVector3;

class StLambdaMaker;

StLambdaMaker* createStLambdaMaker(const char* name, StPicoDstMaker* picoMaker, const char* outName);
extern "C" void* createStLambdaMakerC(const char* name, void* picoMaker, const char* outName);

class StLambdaMaker : public StMaker {
public:
  StLambdaMaker(const char* name, StPicoDstMaker* picoMaker, const char* outName);
  virtual ~StLambdaMaker();

  virtual Int_t Init();
  virtual Int_t Make();
  virtual void Clear(Option_t* opt = "");
  virtual Int_t Finish();

  void WriteHistograms();

private:
  StPicoDstMaker* mPicoDstMaker;
  StPicoDst* mPicoDst;
  TString mOutName;
  Int_t mEventCounter;
  HistManager* m_histManager;

  Bool_t PassEventCuts(Int_t nTracks);
  Bool_t PassProtonCuts(StPicoTrack* trk, const TVector3& pVtx);
  Bool_t PassPionCuts(StPicoTrack* trk, const TVector3& pVtx);
  StPhysicalHelixD MakeHelix(StPicoTrack* trk, Double_t bField);
  Bool_t MakeLambdaHelix(StPicoTrack* p, StPicoTrack* pi, Double_t bField,
                         TVector3& v0, TVector3& momP, TVector3& momPi, Double_t& dca12);
};

#endif
