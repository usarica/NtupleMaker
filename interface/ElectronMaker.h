// -*- C++ -*-
//
// Package:    NtupleMaker
// Class:      NtupleMaker
// 
/**\class NtupleMaker NtupleMaker.cc CMS2/NtupleMaker/src/NtupleMaker.cc

   Description: <one line class summary>

   Implementation:
   <Notes on implementation>
*/
//
// Original Author:  pts/4
//         Created:  Fri Jun  6 11:07:38 CDT 2008
// $Id: ElectronMaker.h,v 1.9 2009/08/30 18:50:03 fgolf Exp $
//
//
#ifndef NTUPLEMAKER_ELECTRONMAKER_H
#define NTUPLEMAKER_ELECTRONMAKER_H

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"
#include "DataFormats/EgammaReco/interface/BasicClusterShapeAssociation.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/Math/interface/Point3D.h"

#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"

#include "Math/VectorUtil.h"

//
// class decleration
//

class ElectronMaker : public edm::EDProducer {
public:
  explicit ElectronMaker (const edm::ParameterSet&);
  ~ElectronMaker();

private:
  virtual void beginJob(const edm::EventSetup&) ;
  virtual void produce(edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  bool identify(const edm::RefToBase<reco::GsfElectron> &, int);
  int classify(const edm::RefToBase<reco::GsfElectron> &);
  int classify_old(const edm::RefToBase<reco::GsfElectron> &);
  template<typename T> const edm::ValueMap<T>& getValueMap(const edm::Event& iEvent, edm::InputTag& inputTag);

  // ----------member data ---------------------------
  edm::InputTag electronsInputTag_;
  edm::InputTag beamSpotInputTag_;
  edm::InputTag ecalIsoTag_;
  edm::InputTag hcalIsoTag_;
  edm::InputTag tkIsoTag_;
  edm::InputTag eidRobustLooseTag_;
  edm::InputTag eidRobustTightTag_;
  edm::InputTag eidRobustHighEnergyTag_;
  edm::InputTag eidLooseTag_;
  edm::InputTag eidTightTag_;

  EcalClusterLazyTools* clusterTools_;
};

#endif

