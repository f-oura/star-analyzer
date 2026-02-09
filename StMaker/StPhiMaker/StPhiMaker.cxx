#include "StPhiMaker.h"
#include "ConfigManager.h"
#include "HistManager.h"
#include "cuts/EventCutConfig.h"
#include "cuts/TrackCutConfig.h"
#include "cuts/PhiCutConfig.h"
#include "StPicoDstMaker/StPicoDstMaker.h"
#include "StPicoEvent/StPicoDst.h"
#include "StPicoEvent/StPicoTrack.h"
#include "StPicoEvent/StPicoEvent.h"
#include "StPicoEvent/StPicoBTofPidTraits.h"
#include "StarClassLibrary/StPhysicalHelixD.hh"
#include "StarClassLibrary/StThreeVectorF.hh"
#include "StarClassLibrary/SystemOfUnits.h"

#include "TFile.h"
#include "TMath.h"
#include "TSystem.h"
#include "TVector3.h"
#include "TVector2.h"

#include <iostream>
#include <vector>
#include <utility>

namespace {
  const Double_t kKaonMass = 0.493677;
}

//-----------------------------------------------------------------------------
StPhiMaker::StPhiMaker(const char* name, StPicoDstMaker* picoMaker, const char* outName)
    : StMaker(name),
      mPicoDstMaker(picoMaker),
      mPicoDst(0),
      mOutName(outName),
      mEventCounter(0),
      m_histManager(0) {}

//-----------------------------------------------------------------------------
StPhiMaker::~StPhiMaker() {
  if (m_histManager) {
    delete m_histManager;
    m_histManager = 0;
  }
}

//-----------------------------------------------------------------------------
StPhiMaker* createStPhiMaker(const char* name, StPicoDstMaker* picoMaker, const char* outName) {
  return new StPhiMaker(name, picoMaker, outName);
}

extern "C" void* createStPhiMakerC(const char* name, void* picoMaker, const char* outName) {
  return (void*)createStPhiMaker(name, (StPicoDstMaker*)picoMaker, outName);
}

//-----------------------------------------------------------------------------
Int_t StPhiMaker::Init() {
  std::string histPath = ConfigManager::GetInstance().GetHistConfigPath();
  if (histPath.empty()) {
    std::cerr << "[StPhiMaker] GetHistConfigPath() returned empty; no histograms will be filled." << std::endl;
    m_histManager = 0;
    return kStOK;
  }
  m_histManager = new HistManager();
  if (!m_histManager->LoadFromFile(histPath.c_str())) {
    std::cerr << "[StPhiMaker] Failed to load hist config from " << histPath << std::endl;
    delete m_histManager;
    m_histManager = 0;
    return kStOK;
  }
  return kStOK;
}

//-----------------------------------------------------------------------------
void StPhiMaker::Clear(Option_t* opt) {}

//-----------------------------------------------------------------------------
Int_t StPhiMaker::Make() {
  if (!mPicoDstMaker) {
    return kStWarn;
  }
  mPicoDst = mPicoDstMaker->picoDst();
  if (!mPicoDst) {
    return kStWarn;
  }

  StPicoEvent* event = mPicoDst->event();
  if (!event) {
    return kStWarn;
  }

  mEventCounter++;

  TVector3 pVtx = event->primaryVertex();
  Float_t vzVpd = event->vzVpd();
  Int_t refMult = event->refMult();
  Float_t vr = TMath::Sqrt(pVtx.X() * pVtx.X() + pVtx.Y() * pVtx.Y());

  // Event-level fills
  if (m_histManager) {
    m_histManager->Fill("hVz", pVtx.Z());
    m_histManager->Fill("hVxVy", pVtx.X(), pVtx.Y());
    m_histManager->Fill("hRefMult", refMult);
    m_histManager->Fill("hVzVsRun", (Double_t)event->runId(), pVtx.Z());
    m_histManager->Fill("hRefMultVsVz", pVtx.Z(), refMult);
    EventCutConfig& ev = ConfigManager::GetInstance().GetEventCuts();
    if (TMath::Abs(vzVpd) < ev.maxAbsVzVpd) {
      m_histManager->Fill("hVzDiff", pVtx.Z() - vzVpd);
    }
    std::vector<unsigned int> triggerIds = event->triggerIds();
    for (size_t i = 0; i < triggerIds.size(); i++) {
      m_histManager->Fill("hTriggerIds", triggerIds[i]);
    }
  }

  if (!PassEventCuts(pVtx.Z(), vr, refMult, vzVpd)) {
    return kStOK;
  }

  Bool_t useTOF = kFALSE;
  const Int_t kMaxKaons = 2000;
  std::vector<Track_t> kaonsPlus;
  std::vector<Track_t> kaonsMinus;
  kaonsPlus.reserve(kMaxKaons / 2);
  kaonsMinus.reserve(kMaxKaons / 2);

  Int_t nTracks = mPicoDst->numberOfTracks();
  PhiCutConfig& phiCfg = ConfigManager::GetInstance().GetPhiCuts();
  if (phiCfg.maxNTr > 0 && nTracks > phiCfg.maxNTr) {
    return kStOK;
  }

  Double_t Qx = 0.0, Qy = 0.0;
  Int_t nTofMatch = 0;

  for (Int_t itrk = 0; itrk < nTracks; itrk++) {
    StPicoTrack* trk = mPicoDst->track(itrk);
    if (!trk) continue;
    if (!PassTrackCuts(trk, pVtx)) continue;

    TVector3 pMom = trk->pMom();
    Float_t pt = pMom.Perp();
    Float_t eta = pMom.PseudoRapidity();
    Float_t phi = pMom.Phi();

    if (m_histManager) {
      m_histManager->Fill("hPt", pt);
      m_histManager->Fill("hEta", eta);
      m_histManager->Fill("hPhi", phi);
      m_histManager->Fill("hNHitsFit", trk->nHitsFit());
      m_histManager->Fill("hNHitsRatio", (Float_t)trk->nHitsFit() / (Float_t)trk->nHitsMax());
      m_histManager->Fill("hDCA", trk->gDCA(pVtx).Mag());
      m_histManager->Fill("hCharge", trk->charge());
      m_histManager->Fill("hChi2", trk->chi2());
      m_histManager->Fill("hDedxVsP", pMom.Mag(), trk->dEdx());
      m_histManager->Fill("hNSigmaPionVsP", pMom.Mag(), trk->nSigmaPion());
      m_histManager->Fill("hNSigmaKaonVsP", pMom.Mag(), trk->nSigmaKaon());
      m_histManager->Fill("hNSigmaProtonVsP", pMom.Mag(), trk->nSigmaProton());
    }

    if (pt >= phiCfg.minPtEp && pt <= phiCfg.maxPtEp && TMath::Abs(eta) < phiCfg.maxEtaEp) {
      Qx += TMath::Cos(2.0 * phi);
      Qy += TMath::Sin(2.0 * phi);
    }

    Int_t btofIndex = trk->bTofPidTraitsIndex();
    if (btofIndex >= 0) nTofMatch++;

    if (!PassKaonCuts(trk, pVtx)) continue;

    Track_t track;
    BuildTrack(track, trk, event, pVtx);
    if (useTOF && btofIndex >= 0) {
      StPicoBTofPidTraits* tof = mPicoDst->btofPidTraits(btofIndex);
      if (tof) {
        Double_t beta = tof->btofBeta();
        if (beta > 1e-4) {
          Double_t oneOverBeta = 1.0 / beta;
          track.mass2 = pMom.Mag2() * (oneOverBeta * oneOverBeta - 1.0);
          track.tofMatch = kTRUE;
        } else {
          track.mass2 = -999.0;
          track.tofMatch = kFALSE;
        }
      } else {
        track.mass2 = -999.0;
        track.tofMatch = kFALSE;
      }
    } else {
      track.mass2 = -999.0;
      track.tofMatch = kFALSE;
    }

    if (IsKaon(track, useTOF)) {
      if (track.charge > 0 && (Int_t)kaonsPlus.size() < kMaxKaons) {
        kaonsPlus.push_back(track);
      } else if (track.charge < 0 && (Int_t)kaonsMinus.size() < kMaxKaons) {
        kaonsMinus.push_back(track);
      }
    }
  }

  if (m_histManager) m_histManager->Fill("hTofMatchMult", nTofMatch);

  // Phi reconstruction: ReconstructPhi pairs
  for (size_t iPlus = 0; iPlus < kaonsPlus.size(); iPlus++) {
    for (size_t iMinus = 0; iMinus < kaonsMinus.size(); iMinus++) {
      Double_t invMass;
      TVector3 phiMom, dcaPosPlus, dcaPosMinus;
      if (!ReconstructPhi(kaonsPlus[iPlus], kaonsMinus[iMinus], invMass, phiMom, dcaPosPlus, dcaPosMinus)) continue;

      Double_t openingAngle = CalculateOpeningAngle(kaonsPlus[iPlus], kaonsMinus[iMinus]);
      Double_t pairRapidity = CalculatePairRapidity(invMass, phiMom);
      if (m_histManager) {
        m_histManager->Fill("hOpeningAngle_Raw", openingAngle);
        m_histManager->Fill("hPairRapidity_Raw", pairRapidity);
        m_histManager->Fill("hPairPt_Raw", phiMom.Pt());
        m_histManager->Fill("hOpeningAngle_vs_MKK", openingAngle, invMass);
        m_histManager->Fill("hPairRapidity_vs_MKK", pairRapidity, invMass);
        m_histManager->Fill("hOpeningAngle_vs_Pt", openingAngle, phiMom.Pt());
        m_histManager->Fill("hOpeningAngle_vs_Rapidity", openingAngle, pairRapidity);
        m_histManager->Fill("hPairRapidity_vs_Pt", pairRapidity, phiMom.Pt());
        m_histManager->Fill("hMKK_vs_Pt", phiMom.Pt(), invMass);
        m_histManager->Fill("hMKK_SameEvent", invMass);
      }

      PhiCutConfig& phiCut = ConfigManager::GetInstance().GetPhiCuts();
      Bool_t passAngle = (openingAngle >= phiCut.minOpeningAngle && openingAngle <= phiCut.maxOpeningAngle);
      Bool_t passRapidity = (pairRapidity >= phiCut.minPairRapidity && pairRapidity <= phiCut.maxPairRapidity);
      if (m_histManager) {
        if (passAngle) m_histManager->Fill("hMKK_OpeningAngleCut", invMass);
        if (passRapidity) m_histManager->Fill("hMKK_RapidityCut", invMass);
        if (passAngle && passRapidity) {
          m_histManager->Fill("hMKK_BothCuts", invMass);
          m_histManager->Fill("hOpeningAngle_AfterCuts", openingAngle);
          m_histManager->Fill("hPairRapidity_AfterCuts", pairRapidity);
          m_histManager->Fill("hPairPt_AfterCuts", phiMom.Pt());
        }
      }
    }
  }

  // All combinations (simple invariant mass)
  if (m_histManager) {
    for (size_t iPlus = 0; iPlus < kaonsPlus.size(); iPlus++) {
      for (size_t iMinus = 0; iMinus < kaonsMinus.size(); iMinus++) {
        Double_t invMass = CalculateInvariantMass(kaonsPlus[iPlus], kaonsMinus[iMinus], kKaonMass, kKaonMass);
        m_histManager->Fill("hMKK_AllCombinations", invMass);
      }
    }
  }

  // Event plane
  TVector2 Q(Qx, Qy);
  if (m_histManager) {
    m_histManager->Fill("hQxQy", Qx, Qy);
    if (Q.Mod() > 0) {
      Double_t psi2 = 0.5 * TMath::ATan2(Qy, Qx);
      if (psi2 < 0) psi2 += TMath::Pi();
      m_histManager->Fill("hPsi2", psi2);
    }
    m_histManager->Fill("hN", 0);
  }
  return kStOK;
}

//-----------------------------------------------------------------------------
Int_t StPhiMaker::Finish() {
  if (mOutName != "") {
    TFile* fout = new TFile(mOutName.Data(), "RECREATE");
    fout->cd();
    WriteHistograms();
    fout->Close();
  }
  std::cout << "StPhiMaker::Finish() processed " << mEventCounter << " events" << std::endl;
  return kStOK;
}

//-----------------------------------------------------------------------------
void StPhiMaker::WriteHistograms() {
  if (m_histManager) m_histManager->Write();
}

//-----------------------------------------------------------------------------
Bool_t StPhiMaker::PassEventCuts(Float_t vz, Float_t vr, Int_t refMult, Float_t vzVpd) {
  EventCutConfig& ev = ConfigManager::GetInstance().GetEventCuts();
  if (TMath::Abs(vz) > ev.maxVz) return kFALSE;
  if (vr > ev.maxVr) return kFALSE;
  if (refMult < ev.minRefMult) return kFALSE;
  if (refMult > ev.maxRefMult) return kFALSE;
  if (TMath::Abs(vz - vzVpd) > ev.maxVzDiff && TMath::Abs(vzVpd) < ev.maxAbsVzVpd) return kFALSE;
  return kTRUE;
}

//-----------------------------------------------------------------------------
Bool_t StPhiMaker::PassTrackCuts(StPicoTrack* trk, TVector3& pVtx) {
  TrackCutConfig& tr = ConfigManager::GetInstance().GetTrackCuts();
  if (trk->nHitsFit() < tr.minNHitsFit) return kFALSE;
  if ((Float_t)trk->nHitsFit() / (Float_t)trk->nHitsMax() < tr.minNHitsRatio) return kFALSE;
  if (trk->nHitsDedx() < tr.minNHitsDedx) return kFALSE;
  if (trk->chi2() > tr.maxChi2) return kFALSE;
  TVector3 pMom = trk->pMom();
  if (pMom.Mag() < 1e-4) return kFALSE;
  Float_t pt = pMom.Perp();
  Float_t eta = pMom.PseudoRapidity();
  if (pt < tr.minPt || pt > tr.maxPt) return kFALSE;
  if (TMath::Abs(eta) > tr.maxEta) return kFALSE;
  if (trk->gDCA(pVtx).Mag() > tr.maxDCA) return kFALSE;
  return kTRUE;
}

//-----------------------------------------------------------------------------
Bool_t StPhiMaker::PassKaonCuts(StPicoTrack* trk, TVector3& pVtx) {
  if (!PassTrackCuts(trk, pVtx)) return kFALSE;
  PhiCutConfig& phi = ConfigManager::GetInstance().GetPhiCuts();
  if (trk->gDCA(pVtx).Mag() > phi.maxDCAKaon) return kFALSE;
  if (TMath::Abs(trk->nSigmaKaon()) > phi.nSigmaKaon) return kFALSE;
  return kTRUE;
}

//-----------------------------------------------------------------------------
Bool_t StPhiMaker::PassTrackCuts(const Track_t& trk) {
  TrackCutConfig& tr = ConfigManager::GetInstance().GetTrackCuts();
  if (trk.nHitsFit < tr.minNHitsFit) return kFALSE;
  if ((Float_t)trk.nHitsFit / (Float_t)trk.nHitsMax < tr.minNHitsRatio) return kFALSE;
  if (trk.nHitsDedx < tr.minNHitsDedx) return kFALSE;
  if (trk.DCA > tr.maxDCA) return kFALSE;
  if (TMath::Abs(trk.eta) > tr.maxEta) return kFALSE;
  if (trk.pT < tr.minPt || trk.pT > tr.maxPt) return kFALSE;
  if (trk.chi2 > tr.maxChi2) return kFALSE;
  return kTRUE;
}

//-----------------------------------------------------------------------------
Bool_t StPhiMaker::IsKaon(const Track_t& trk, Bool_t useTOF) {
  if (!PassTrackCuts(trk)) return kFALSE;
  PhiCutConfig& phi = ConfigManager::GetInstance().GetPhiCuts();
  if (trk.DCA > phi.maxDCAKaon) return kFALSE;
  if (TMath::Abs(trk.nSigmaKaon) > phi.nSigmaKaon) return kFALSE;
  if (useTOF && trk.tofMatch) {
    if (trk.mass2 < phi.minMass2Kaon || trk.mass2 > phi.maxMass2Kaon) return kFALSE;
  }
  return kTRUE;
}

//-----------------------------------------------------------------------------
void StPhiMaker::BuildTrack(Track_t& track, StPicoTrack* pico, StPicoEvent* event, TVector3& pVtx) {
  TVector3 gmom = pico->gMom();
  TVector3 org = pico->origin();
  track.originX = org.X();
  track.originY = org.Y();
  track.originZ = org.Z();
  track.momentumX = gmom.X();
  track.momentumY = gmom.Y();
  track.momentumZ = gmom.Z();
  track.BField = event->bField();
  track.pT = gmom.Perp();
  track.eta = gmom.PseudoRapidity();
  track.phi = gmom.Phi();
  track.charge = pico->charge();
  track.nHitsFit = pico->nHitsFit();
  track.nHitsMax = pico->nHitsMax();
  track.nHitsDedx = pico->nHitsDedx();
  track.DCA = pico->gDCA(pVtx).Mag();
  track.chi2 = pico->chi2();
  track.nSigmaKaon = pico->nSigmaKaon();
}

//-----------------------------------------------------------------------------
StPhysicalHelixD StPhiMaker::BuildHelix(const Track_t& trk) {
  StThreeVectorF gmomSt(trk.momentumX, trk.momentumY, trk.momentumZ);
  StThreeVectorF orgSt(trk.originX, trk.originY, trk.originZ);
  StPhysicalHelixD helix(gmomSt, orgSt, trk.BField * units::kilogauss, static_cast<float>(trk.charge));
  return helix;
}

//-----------------------------------------------------------------------------
Double_t StPhiMaker::CalculateDCA(const Track_t& trk1, const Track_t& trk2, TVector3& dcaPos1, TVector3& dcaPos2) {
  StPhysicalHelixD helix1 = BuildHelix(trk1);
  StPhysicalHelixD helix2 = BuildHelix(trk2);
  std::pair<Double_t, Double_t> pathLengths = helix1.pathLengths(helix2);
  StThreeVectorF pos1 = helix1.at(pathLengths.first);
  StThreeVectorF pos2 = helix2.at(pathLengths.second);
  dcaPos1 = TVector3(pos1.x(), pos1.y(), pos1.z());
  dcaPos2 = TVector3(pos2.x(), pos2.y(), pos2.z());
  TVector3 dcaVec = dcaPos1 - dcaPos2;
  return dcaVec.Mag();
}

//-----------------------------------------------------------------------------
Bool_t StPhiMaker::ReconstructPhi(const Track_t& kPlus, const Track_t& kMinus, Double_t& invMass, TVector3& phiMom, TVector3& dcaPosPlus, TVector3& dcaPosMinus) {
  PhiCutConfig& phi = ConfigManager::GetInstance().GetPhiCuts();
  StPhysicalHelixD helixPlus = BuildHelix(kPlus);
  StPhysicalHelixD helixMinus = BuildHelix(kMinus);
  Double_t dca = CalculateDCA(kPlus, kMinus, dcaPosPlus, dcaPosMinus);
  if (dca > phi.maxDCAKK) return kFALSE;

  std::pair<Double_t, Double_t> pathLengths = helixPlus.pathLengths(helixMinus);
  StThreeVectorF pPlus = helixPlus.momentumAt(pathLengths.first, kPlus.BField * units::kilogauss);
  StThreeVectorF pMinus = helixMinus.momentumAt(pathLengths.second, kMinus.BField * units::kilogauss);

  Double_t EPlus = TMath::Sqrt(kKaonMass * kKaonMass + pPlus.mag() * pPlus.mag());
  Double_t EMinus = TMath::Sqrt(kKaonMass * kKaonMass + pMinus.mag() * pMinus.mag());
  phiMom = TVector3(pPlus.x() + pMinus.x(), pPlus.y() + pMinus.y(), pPlus.z() + pMinus.z());
  Double_t E = EPlus + EMinus;
  invMass = TMath::Sqrt(E * E - phiMom.Mag2());
  return kTRUE;
}

//-----------------------------------------------------------------------------
Double_t StPhiMaker::CalculateInvariantMass(const Track_t& trk1, const Track_t& trk2, Double_t mass1, Double_t mass2) {
  TVector3 p1;
  p1.SetPtEtaPhi(trk1.pT, trk1.eta, trk1.phi);
  TVector3 p2;
  p2.SetPtEtaPhi(trk2.pT, trk2.eta, trk2.phi);
  Double_t E1 = TMath::Sqrt(mass1 * mass1 + p1.Mag2());
  Double_t E2 = TMath::Sqrt(mass2 * mass2 + p2.Mag2());
  TVector3 p = p1 + p2;
  return TMath::Sqrt((E1 + E2) * (E1 + E2) - p.Mag2());
}

//-----------------------------------------------------------------------------
Double_t StPhiMaker::CalculateOpeningAngle(const Track_t& trk1, const Track_t& trk2) {
  TVector3 p1;
  p1.SetPtEtaPhi(trk1.pT, trk1.eta, trk1.phi);
  TVector3 p2;
  p2.SetPtEtaPhi(trk2.pT, trk2.eta, trk2.phi);
  Double_t mag1 = p1.Mag();
  Double_t mag2 = p2.Mag();
  if (mag1 < 1e-10 || mag2 < 1e-10) return TMath::Pi();
  Double_t cosTheta = p1.Dot(p2) / (mag1 * mag2);
  if (cosTheta > 1.0) cosTheta = 1.0;
  if (cosTheta < -1.0) cosTheta = -1.0;
  return TMath::ACos(cosTheta);
}

//-----------------------------------------------------------------------------
Double_t StPhiMaker::CalculatePairRapidity(Double_t invMass, const TVector3& phiMom) {
  Double_t P2 = phiMom.Mag2();
  Double_t E = TMath::Sqrt(invMass * invMass + P2);
  Double_t pz = phiMom.Z();
  if (E <= TMath::Abs(pz)) return 0.0;
  return 0.5 * TMath::Log((E + pz) / (E - pz));
}
