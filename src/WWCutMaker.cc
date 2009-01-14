// -*- C++ -*-
//
// Package:    NtupleMaker
// Class:      WWCutMaker
// 
/**\class WWCutMaker WWCutMaker.cc CMS2/NtupleMaker/src/WWCutMaker.cc

Description: create branches for the WW analysis cut variables (this
is meant to make life easier for TCut lovers)

Implementation:
- loop over dilepton candidates
- calculate quantities that are not straightforward

Calling the cut functions is somewhat roundabout, but for good reason:
- We want to use the standard selections.C code
- This code is written to run on the CMS2 tree, so it calls CMS2 methods
- So we make a "mini-CMS2" object that we then feed to selections.C

Not very pretty, but it keeps us from having to drag around two copies
of the selection code, which would get out of sync sooner or later

*/
//
// Original Author:  Johannes Muelmenstaedt 
//		     (but just calls functions in selection.C)
// $Id: WWCutMaker.cc,v 1.6 2009/01/14 06:00:38 jmuelmen Exp $
//
//

// system include files
#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "CMS2/NtupleMaker/interface/WWCutMaker.h"
#include "CMS2/NtupleMaker/interface/MatchUtilities.h"
#include "CMS2/NtupleMaker/interface/METUtilities.h"


#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/JetReco/interface/CaloJet.h"

#include "Math/VectorUtil.h"
#include "TMath.h"
#include "Math/LorentzVector.h"
#include "TDatabasePDG.h"

static CMS2Adapter cms2;

using namespace std;
namespace CMS2_selections {
/* The following is a verbatim copy of the selections.C used to obtain
 * the "Feb results" (i.e., the February cuts applied to the CSA07
 * samples).  Unfortunately, we have no way of keeping the
 * ntuple-level selections.C in sync with these selection functions
 * without creating instability in the meaning of the flags that
 * WWCutMaker sets. */

//===========================================================
//
// Various selection functions are kept here
//
//============================================================

//----------------------------------------------------------------
// A ridicolusly simple function, but since the Z veto is used 
// in two places, might as well centralize it to keep consistency
//----------------------------------------------------------------
bool inZmassWindow (float mass) {
  if (mass > 76. && mass < 106.) return true;
  return false;
}
//----------------------------------------------------------------
// true electron
//----------------------------------------------------------------
// bool trueElectron(int index) {
//   if ( abs(cms2.els_mc_id()[index]) != 11 ) return false;
//   if ( abs(cms2.els_mc_motherid()[index]) == 23 || abs(cms2.els_mc_motherid()[index]) == 24) return true;
//   return false;
// }
// //----------------------------------------------------------------
// // true muon
// //----------------------------------------------------------------
// bool trueMuon(int index) {
//    if ( abs(cms2.mus_mc_id()[index]) != 13 ) return false;
//    if ( abs(cms2.mus_mc_motherid()[index]) == 23 || abs(cms2.mus_mc_motherid()[index]) == 24) return true;
//   return false;
// }
//----------------------------------------------------------------
// Electron ID without isolation
//----------------------------------------------------------------
bool goodElectronWithoutIsolation(int index) {
  if ( cms2.els_tightId().at(index)     !=  1) return false;
  if ( cms2.els_closestMuon().at(index) != -1) return false;
  if ( abs(cms2.els_d0().at(index)) > 0.025)   return false;
  return true;
}
//----------------------------------------------------------------
// Muon ID without isolation
//---------------------------------------------------------------
bool goodMuonWithoutIsolation(int index) {
  if (cms2.mus_gfit_chi2().at(index)/cms2.mus_gfit_ndof().at(index) > 5.) return false;
  if (abs(cms2.mus_d0().at(index))   > 0.25) return false;
  if (cms2.mus_validHits().at(index) < 7)    return false;
  return true;
}
//-----------------------------------------------------------
// Electron Isolation
//-----------------------------------------------------------
bool passElectronIsolation(int index) {
  double sum = cms2.els_tkIso().at(index);
  double pt  = cms2.els_p4().at(index).pt();
   if ( pt/(pt+sum) < 0.92) return false;
  return true;  
} 
//-----------------------------------------------------------
// Muon Isolation
//-----------------------------------------------------------
bool passMuonIsolation(int index) {
  double sum =  cms2.mus_iso03_sumPt().at(index) +  
                cms2.mus_iso03_emEt().at(index)  +
                cms2.mus_iso03_hadEt().at(index);
  double pt  = cms2.mus_p4().at(index).pt();
  if ( pt/(pt+sum) < 0.92) return false;
  return true;  
}
//--------------------------------------------
// Muon ID with isolation
//--------------------------------------------
bool goodMuonIsolated(int index) {
  if (!goodMuonWithoutIsolation(index)) return false;
  if (!passMuonIsolation(index))       return false;
  return true;
}
//--------------------------------------------
// Electron ID with isolation
//--------------------------------------------
bool goodElectronIsolated(int index) {
  if (!goodElectronWithoutIsolation(index)) return false;
  if (!passElectronIsolation(index))       return false;
  return true;
}
//--------------------------------------------
// Pass 2 MET selection
//--------------------------------------------
bool pass2Met (int i_hyp) {
  // for e-e and mu-mu
  if (cms2.hyp_type()[i_hyp] == 0 || cms2.hyp_type()[i_hyp] == 3) {
    if (cms2.hyp_met()[i_hyp] < 30) return false;
    //    if ( fabs(hyp_p4[i_hyp]->mass()-90.0)<10.0) return false;
    if( cms2.hyp_met()[i_hyp]/cms2.hyp_p4()[i_hyp].pt()<0.6 && 
	acos(cos(cms2.hyp_metPhi()[i_hyp]-cms2.hyp_p4()[i_hyp].phi()-3.1416))<0.25 ) return false;
  }
  // for e-mu and mu-e
  if (cms2.hyp_type()[i_hyp] == 1 || cms2.hyp_type()[i_hyp] == 2) {
    if (cms2.hyp_met()[i_hyp] < 20) return false;
  }
  return true;
}

double nearestDeltaPhi(double Phi, int i_hyp)
{
  //WARNING!  This was designed to work in a dilepton environment - NOT a trilepton 
  double tightDPhi = TMath::Min(TMath::Abs(cms2.hyp_lt_p4()[i_hyp].Phi() - Phi), 2*TMath::Pi() - TMath::Abs(cms2.hyp_lt_p4()[i_hyp].Phi() - Phi));
  double looseDPhi = TMath::Min(TMath::Abs(cms2.hyp_ll_p4()[i_hyp].Phi() - Phi), 2*TMath::Pi() - TMath::Abs(cms2.hyp_ll_p4()[i_hyp].Phi() - Phi));

  return TMath::Min(tightDPhi, looseDPhi);

}//END nearest DeltaPhi                                                                                                                                 

double MetSpecial(double Met, double MetPhi, int i_hyp)
{
  //Warning, this was designed to work in a dilepton environment - NOT a trilepton  
  double DeltaPhi = nearestDeltaPhi(MetPhi,i_hyp);

  if (DeltaPhi < TMath::Pi()/2) return Met*TMath::Sin(DeltaPhi);
  else return Met;

  return -1.0;
}//END MetSpecial calculation  

//--------------------------------------------
// Pass 4 MET selection
// Use MetSpecial from CDF for now
//--------------------------------------------
bool pass4Met(int i_hyp) {
  double metspec = MetSpecial(cms2.hyp_met()[i_hyp], cms2.hyp_metPhi()[i_hyp], i_hyp);
  if (cms2.hyp_type()[i_hyp] == 0 || cms2.hyp_type()[i_hyp] == 3) {
    if ( metspec < 20 ) return false;
    //if ( metspec < 20 && hyp_p4->mass() < 90 ) return false;
    if ( cms2.hyp_met()[i_hyp] < 45 ) return false;
  }
  else if (cms2.hyp_type()[i_hyp] == 1 || cms2.hyp_type()[i_hyp] == 2) {
    //if ( metspec < 20 && hyp_p4->mass() < 90 ) return false;
    if ( metspec < 20 ) return false;
  }
  return true;
}
//-------------------------------------------------
// Auxiliary function to scan the doc line and 
// identify DY-> ee vs mm vs tt
//-------------------------------------------------
int getDrellYanType() {
  bool foundZ;
  int size = cms2.genps_id().size();
  for (int jj=0; jj<size; jj++) {
    if (cms2.genps_id().at(jj) == 23) {
      foundZ = true;
      if (jj+3 > size) {
	std::cout << 
	  "Found Z but not enough room in doc lines for leptons?" << std::endl;
        return 999;
      }
      if (abs(cms2.genps_id().at(jj+1)) == 11) return 0;  //DY->ee
      if (abs(cms2.genps_id().at(jj+1)) == 13) return 1;  //DY->mm
      if (abs(cms2.genps_id().at(jj+1)) == 15) return 2;  //DY->tautau
    }
  }
  std::cout << "Does not look like a DY event" << std::endl;
  return 999;
}

//--------------------------------------------
// Booleans for DY
//------------------------------------------
bool isDYee() {
  if (getDrellYanType() == 0) return true;
  return false;
}
bool isDYmm() {
  if (getDrellYanType() == 1) return true;
  return false;
}
bool isDYtt() {
  if (getDrellYanType() == 2) return true;
  return false;
}

//--------------------------------------------------------------------
// Veto events if there are two leptons in the 
// event that make the Z mass.  This uses the mus and els
// blocks, ie, it is a veto that can use the 3rd (4th,5th,..)
// lepton in the event.
//
// Both leptons must be 20 GeV, and pass the same cuts as 
// the hypothesis leptons, except that one of them can be non-isolated
//---------------------------------------------------------------------
bool additionalZveto() {

  // true if we want to veto this event
  bool veto=false;

  // first, look for Z->mumu
  for (unsigned int i=0; i < cms2.mus_p4().size(); i++) {
    if (cms2.mus_p4().at(i).pt() < 20.)     continue;
    if (!goodMuonWithoutIsolation(i)) continue;

    for (unsigned int j=i+1; j < cms2.mus_p4().size(); j++) {
      if (cms2.mus_p4().at(j).pt() < 20.) continue;
      if (!goodMuonWithoutIsolation(j)) continue;
      if (cms2.mus_charge().at(i) == cms2.mus_charge().at(j)) continue;

      // At least one of them has to pass isolation
      if (!passMuonIsolation(i) && !passMuonIsolation(j)) continue;

      // Make the invariant mass
      ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > 
                vec = cms2.mus_p4().at(i) + cms2.mus_p4().at(j);
      if ( inZmassWindow(vec.mass()) ) return true;

    }
  }

  // now, look for Z->ee
  for (unsigned int i=0; i < cms2.els_p4().size(); i++) {
    if (cms2.els_p4().at(i).pt() < 20.)     continue;
    if (!goodElectronWithoutIsolation(i)) continue;

    for (unsigned int j=i+1; j<cms2.els_p4().size(); j++) {
      if (cms2.els_p4().at(j).pt() < 20.) continue;
      if (!goodElectronWithoutIsolation(j)) continue;
      if (cms2.els_charge().at(i) == cms2.els_charge().at(j)) continue;

      // At least one of them has to pass isolation
      if (!passElectronIsolation(i) && !passElectronIsolation(j)) continue;

      // Make the invariant mass
      ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > 
                vec = cms2.els_p4().at(i) + cms2.els_p4().at(j);
      if ( inZmassWindow(vec.mass()) ) return true;

    }
  }
  // done
  return veto;
}

//------------------------------------------------------------
// Not a selection function per se, but useful nonetheless:
// dumps the documentation lines for this event
//------------------------------------------------------------
void dumpDocLines() {
  int size = cms2.genps_id().size();
  // Initialize particle database
  TDatabasePDG *pdg = new TDatabasePDG();
  std::cout << "------------------------------------------" << std::endl;
  for (int j=0; j<size; j++) {
    cout << setw(9) << left << pdg->GetParticle(cms2.genps_id().at(j))->GetName() << " "
         << setw(7) << right << setprecision(4) << cms2.genps_p4().at(j).pt() << "  "
         << setw(7) << right << setprecision(4) << cms2.genps_p4().at(j).phi() << "  "
         << setw(10) << right << setprecision(4) << cms2.genps_p4().at(j).eta() << "  "
         << setw(10) << right << setprecision(4) << cms2.genps_p4().at(j).mass() << endl;
  }
  // Clean up
  delete pdg;
}
}

using namespace reco;
using namespace edm;
using namespace CMS2_selections;

typedef math::XYZTLorentzVector LorentzVector;

WWCutMaker::WWCutMaker(const edm::ParameterSet& iConfig)
     : dileptonInputTag	(iConfig.getParameter<InputTag>("dileptonInputTag")),
       muonsInputTag	(iConfig.getParameter<InputTag>("muonsInputTag")),
       electronsInputTag(iConfig.getParameter<InputTag>("electronsInputTag")),
       eltomuassInputTag(iConfig.getParameter<InputTag>("eltomuassInputTag")),
       genpsInputTag	(iConfig.getParameter<InputTag>("genpsInputTag"))
{
     produces<vector<int> >		("wwoppsign"		).setBranchAlias("ww_oppsign"		);
     produces<vector<int> >		("wwpasszveto"		).setBranchAlias("ww_passzveto"		);
     produces<vector<int> >		("wwisdyee"		).setBranchAlias("ww_isdyee"		);
     produces<vector<int> >		("wwisdymm"		).setBranchAlias("ww_isdymm"		);
     produces<vector<int> >		("wwisdytt"		).setBranchAlias("ww_isdytt"		);
     produces<vector<float> >		("wwpmet"		).setBranchAlias("ww_pmet"		);
     produces<vector<int> >		("wwpass4met"		).setBranchAlias("ww_pass4met"		);
     produces<vector<int> >		("wwpass2met"		).setBranchAlias("ww_pass2met"		);
     produces<vector<int> >		("wwltgoodmu"		).setBranchAlias("ww_ltgoodmu"		);
     produces<vector<int> >		("wwllgoodmu"		).setBranchAlias("ww_llgoodmu"		);
     produces<vector<int> >		("wwltgoodmuiso"	).setBranchAlias("ww_ltgoodmuiso"	);
     produces<vector<int> >		("wwllgoodmuiso"	).setBranchAlias("ww_llgoodmuiso"	);
     produces<vector<int> >		("wwltgoodel"		).setBranchAlias("ww_ltgoodel"		);
     produces<vector<int> >		("wwllgoodel"		).setBranchAlias("ww_llgoodel"		);
     produces<vector<int> >		("wwltgoodeliso"	).setBranchAlias("ww_ltgoodeliso"	);
     produces<vector<int> >		("wwllgoodeliso"	).setBranchAlias("ww_llgoodeliso"	);
     produces<vector<int> >		("wwpassaddzveto"	).setBranchAlias("ww_passaddzveto"	);
     produces<vector<int> >		("wwpassFebselnojetveto").setBranchAlias("ww_passFebsel_no_jetveto");
     produces<vector<int> >		("wwpassFebselwithjetveto").setBranchAlias("ww_passFebsel_with_jetveto");
}

//
// member functions
//

// ------------ method called to produce the data  ------------
void WWCutMaker::produce(Event& iEvent, const edm::EventSetup& iSetup)
{
     //-----------------------------------------------------------
     // electron variables
     //-----------------------------------------------------------

     // charge
     InputTag els_charge_tag(electronsInputTag.label(),"elscharge");
     Handle<vector<int> > els_charge_h;
     iEvent.getByLabel(els_charge_tag, els_charge_h);
     const vector<int> *els_charge = els_charge_h.product();

     // closest muon
     InputTag els_closestMuon_tag(eltomuassInputTag.label(),"elsclosestMuon");
     Handle<vector<int> > els_closestMuon_h;
     iEvent.getByLabel(els_closestMuon_tag, els_closestMuon_h);
     const vector<int> *els_closestMuon = els_closestMuon_h.product();

     //electron d0
     InputTag els_d0_tag(electronsInputTag.label(),"elsd0");
     Handle<vector<float> > els_d0_h;
     iEvent.getByLabel(els_d0_tag, els_d0_h);
     const vector<float> *els_d0 = els_d0_h.product();

     // electron p4
     InputTag els_p4_tag(electronsInputTag.label(),"elsp4");
     Handle<vector<LorentzVector> > els_p4_h;
     iEvent.getByLabel(els_p4_tag, els_p4_h);
     const vector<LorentzVector> *els_p4 = els_p4_h.product();
  
     // tight id
     InputTag els_tightId_tag(electronsInputTag.label(),"elstightId");
     Handle<vector<int> > els_tightId_h;
     iEvent.getByLabel(els_tightId_tag, els_tightId_h);
     const vector<int> *els_tightId = els_tightId_h.product();

     //track isolation in dR = 0.3
     InputTag els_tkIso_tag(electronsInputTag.label(),"elstkIso");
     Handle<vector<float> > els_tkIso_h;
     iEvent.getByLabel(els_tkIso_tag, els_tkIso_h);
     const vector<float> *els_tkIso = els_tkIso_h.product();
  
     //-----------------------------------------------------------
     // genp variables
     //-----------------------------------------------------------

     // id
     InputTag genps_id_tag(genpsInputTag.label(),"genpsid");
     Handle<vector<int> > genps_id_h;
     iEvent.getByLabel(genps_id_tag, genps_id_h);
     const vector<int> *genps_id = genps_id_h.product();

     // electron p4
     InputTag genps_p4_tag(genpsInputTag.label(),"genpsp4");
     Handle<vector<LorentzVector> > genps_p4_h;
     iEvent.getByLabel(genps_p4_tag, genps_p4_h);
     const vector<LorentzVector> *genps_p4 = genps_p4_h.product();

     //-----------------------------------------------------------
     // dilepton hypo variables
     //-----------------------------------------------------------

     // type
     InputTag hyp_type_tag(dileptonInputTag.label(),"hyptype");
     Handle<vector<int> > hyp_type_h;
     iEvent.getByLabel(hyp_type_tag, hyp_type_h);
     const vector<int> *hyp_type = hyp_type_h.product();

     // met
     InputTag hyp_met_tag(dileptonInputTag.label(),"hypmet");
     Handle<vector<float> > hyp_met_h;
     iEvent.getByLabel(hyp_met_tag, hyp_met_h);
     const vector<float> *hyp_met = hyp_met_h.product();

     // met phi
     InputTag hyp_metPhi_tag(dileptonInputTag.label(),"hypmetPhi");
     Handle<vector<float> > hyp_metPhi_h;
     iEvent.getByLabel(hyp_metPhi_tag, hyp_metPhi_h);
     const vector<float> *hyp_metPhi = hyp_metPhi_h.product();

     // hyp p4
     InputTag hyp_p4_tag(dileptonInputTag.label(),"hypp4");
     Handle<vector<LorentzVector> > hyp_p4_h;
     iEvent.getByLabel(hyp_p4_tag, hyp_p4_h);
     const vector<LorentzVector> *hyp_p4 = hyp_p4_h.product();

     // ll p4
     InputTag hyp_ll_p4_tag(dileptonInputTag.label(),"hypllp4");
     Handle<vector<LorentzVector> > hyp_ll_p4_h;
     iEvent.getByLabel(hyp_ll_p4_tag, hyp_ll_p4_h);
     const vector<LorentzVector> *hyp_ll_p4 = hyp_ll_p4_h.product();

     // lt p4
     InputTag hyp_lt_p4_tag(dileptonInputTag.label(),"hypltp4");
     Handle<vector<LorentzVector> > hyp_lt_p4_h;
     iEvent.getByLabel(hyp_lt_p4_tag, hyp_lt_p4_h);
     const vector<LorentzVector> *hyp_lt_p4 = hyp_lt_p4_h.product();

     // ll id
     InputTag hyp_ll_id_tag(dileptonInputTag.label(),"hypllid");
     Handle<vector<int> > hyp_ll_id_h;
     iEvent.getByLabel(hyp_ll_id_tag, hyp_ll_id_h);
     const vector<int> *hyp_ll_id = hyp_ll_id_h.product();

     // lt id
     InputTag hyp_lt_id_tag(dileptonInputTag.label(),"hypltid");
     Handle<vector<int> > hyp_lt_id_h;
     iEvent.getByLabel(hyp_lt_id_tag, hyp_lt_id_h);
     const vector<int> *hyp_lt_id = hyp_lt_id_h.product();

     // ll index
     InputTag hyp_ll_index_tag(dileptonInputTag.label(),"hypllindex");
     Handle<vector<int> > hyp_ll_index_h;
     iEvent.getByLabel(hyp_ll_index_tag, hyp_ll_index_h);
     const vector<int> *hyp_ll_index = hyp_ll_index_h.product();

     // lt index
     InputTag hyp_lt_index_tag(dileptonInputTag.label(),"hypltindex");
     Handle<vector<int> > hyp_lt_index_h;
     iEvent.getByLabel(hyp_lt_index_tag, hyp_lt_index_h);
     const vector<int> *hyp_lt_index = hyp_lt_index_h.product();
     
     // njets
     InputTag hyp_njets_tag(dileptonInputTag.label(),"hypnjets");
     Handle<vector<int> > hyp_njets_h;
     iEvent.getByLabel(hyp_njets_tag, hyp_njets_h);
     const vector<int> *hyp_njets = hyp_njets_h.product();

     //-----------------------------------------------------------
     // muon variables
     //-----------------------------------------------------------

     // muon charge
     edm::InputTag mus_charge_tag(muonsInputTag.label(),"muscharge");
     edm::Handle<std::vector<int> > mus_charge_h;
     iEvent.getByLabel(mus_charge_tag, mus_charge_h);
     const vector<int> *mus_charge = mus_charge_h.product();

     //muon p4
     InputTag mus_p4_tag(muonsInputTag.label(),"musp4");
     Handle<vector<LorentzVector> > mus_p4_h;
     iEvent.getByLabel(mus_p4_tag, mus_p4_h);
     const vector<LorentzVector> *mus_p4 = mus_p4_h.product();

     //# of validHits on the muon track
     InputTag mus_validHits_tag(muonsInputTag.label(),"musvalidHits");
     Handle<vector<int> > mus_validHits_h;
     iEvent.getByLabel(mus_validHits_tag, mus_validHits_h);
     const vector<int> *mus_validHits = mus_validHits_h.product();

     //muond0
     InputTag mus_d0_tag(muonsInputTag.label(),"musd0");
     Handle<vector<float> > mus_d0_h;
     iEvent.getByLabel(mus_d0_tag, mus_d0_h);
     const vector<float> *mus_d0 = mus_d0_h.product();

     //chi2
     InputTag mus_gfit_chi2_tag(muonsInputTag.label(),"musgfitchi2");
     Handle<vector<float> > mus_gfit_chi2_h;
     iEvent.getByLabel(mus_gfit_chi2_tag, mus_gfit_chi2_h);
     const vector<float> *mus_gfit_chi2 = mus_gfit_chi2_h.product();
  
     //ndof
     InputTag mus_gfit_ndof_tag(muonsInputTag.label(),"musgfitndof");
     Handle<vector<float> > mus_gfit_ndof_h;
     iEvent.getByLabel(mus_gfit_ndof_tag, mus_gfit_ndof_h);
     const vector<float> *mus_gfit_ndof = mus_gfit_ndof_h.product();
  
     //track isolation in dR = 0.3 from the muon object
     InputTag mus_iso03_sumPt_tag(muonsInputTag.label(),"musiso03sumPt");
     Handle<vector<float> > mus_iso03_sumPt_h;
     iEvent.getByLabel(mus_iso03_sumPt_tag, mus_iso03_sumPt_h);
     const vector<float> *mus_iso03_sumPt = mus_iso03_sumPt_h.product();

     // em isolation in dR = 0.3 from the muon object
     InputTag mus_iso03_emEt_tag(muonsInputTag.label(),"musiso03emEt");
     Handle<vector<float> > mus_iso03_emEt_h;
     iEvent.getByLabel(mus_iso03_emEt_tag, mus_iso03_emEt_h);
     const vector<float> *mus_iso03_emEt = mus_iso03_emEt_h.product();

     // had isolation in dR = 0.3 from the muon object
     InputTag mus_iso03_hadEt_tag(muonsInputTag.label(),"musiso03hadEt");
     Handle<vector<float> > mus_iso03_hadEt_h;
     iEvent.getByLabel(mus_iso03_hadEt_tag, mus_iso03_hadEt_h);
     const vector<float> *mus_iso03_hadEt = mus_iso03_hadEt_h.product();

     CMS2Data myCMS2 = {
	  els_charge       ,
	  els_closestMuon  ,
	  els_d0           ,
	  els_p4           ,
	  els_tightId      ,
	  els_tkIso        ,
	  genps_id         ,
	  genps_p4         ,
	  hyp_ll_p4        ,
	  hyp_lt_p4        ,
	  hyp_lt_id        ,
	  hyp_ll_id        ,
	  hyp_lt_index	   ,
	  hyp_ll_index	   ,
	  hyp_met          ,
	  hyp_metPhi       ,
	  hyp_p4           ,
	  hyp_type         ,
	  mus_charge       ,
	  mus_d0           ,
	  mus_gfit_chi2    ,
	  mus_gfit_ndof    ,
	  mus_iso03_emEt   ,
	  mus_iso03_hadEt  ,
	  mus_iso03_sumPt  ,
	  mus_p4           ,
	  mus_validHits    
     };
     cms2 = CMS2Adapter(myCMS2);

     // now we're ready to fill our own variables
     auto_ptr<vector<int> >		vec_ww_oppsign   	(new vector<int> );
     auto_ptr<vector<int> >		vec_ww_passzveto     	(new vector<int> );
     auto_ptr<vector<int> >		vec_ww_isdyee    	(new vector<int> );
     auto_ptr<vector<int> >		vec_ww_isdymm    	(new vector<int> );
     auto_ptr<vector<int> >		vec_ww_isdytt    	(new vector<int> );
     auto_ptr<vector<float> >		vec_ww_pmet      	(new vector<float>);
     auto_ptr<vector<int> >		vec_ww_pass4met  	(new vector<int> );
     auto_ptr<vector<int> >		vec_ww_pass2met  	(new vector<int> );
     auto_ptr<vector<int> >		vec_ww_ltgoodmu  	(new vector<int>  );
     auto_ptr<vector<int> >		vec_ww_llgoodmu  	(new vector<int>  );
     auto_ptr<vector<int> >		vec_ww_ltgoodmuiso	(new vector<int>  );
     auto_ptr<vector<int> >		vec_ww_llgoodmuiso	(new vector<int>  );
     auto_ptr<vector<int> >		vec_ww_ltgoodel  	(new vector<int>  );
     auto_ptr<vector<int> >		vec_ww_llgoodel  	(new vector<int>  );
     auto_ptr<vector<int> >		vec_ww_ltgoodeliso	(new vector<int>  );
     auto_ptr<vector<int> >		vec_ww_llgoodeliso	(new vector<int>  );
     auto_ptr<vector<int> >		vec_ww_passaddzveto  	(new vector<int> );
     auto_ptr<vector<int> >		vec_ww_passFebsel_no_jetveto	(new vector<int> );
     auto_ptr<vector<int> >		vec_ww_passFebsel_with_jetveto	(new vector<int> );

     // additional Z veto is applied per-event, not per-candidate
     int ww_passaddzveto = not additionalZveto();

     // the remaining cuts are applied per-candidate
     for (unsigned int i_hyp = 0; i_hyp < hyp_p4->size(); ++i_hyp) {
	  int ww_oppsign = true;
	  if (cms2.hyp_lt_id()[i_hyp] * cms2.hyp_ll_id()[i_hyp] > 0)
	       ww_oppsign = false;
	  int ww_passzveto = true;
	  if (cms2.hyp_type()[i_hyp] == 0 || cms2.hyp_type()[i_hyp] == 3) 
	       if (inZmassWindow(cms2.hyp_p4()[i_hyp].mass())) 
		    ww_passzveto = false;
	  int ww_isdyee = isDYee();
	  int ww_isdymm = isDYmm();
	  int ww_isdytt = isDYtt();
	  float ww_pmet = MetSpecial(cms2.hyp_met()[i_hyp], cms2.hyp_metPhi()[i_hyp], i_hyp);
	  int ww_pass4met = pass4Met(i_hyp);
	  int ww_pass2met = pass2Met(i_hyp);
	  int ww_ltgoodmu = -1; // if lt is not a muon, the quality var is -1
	  if (abs(cms2.hyp_lt_id()[i_hyp]) == 13)
	       ww_ltgoodmu = goodMuonWithoutIsolation(cms2.hyp_lt_index()[i_hyp]);
	  int ww_llgoodmu = -1;
	  if (abs(cms2.hyp_ll_id()[i_hyp]) == 13)
	       ww_llgoodmu = goodMuonWithoutIsolation(cms2.hyp_ll_index()[i_hyp]);
	  int ww_ltgoodmuiso = -1;
	  if (abs(cms2.hyp_lt_id()[i_hyp]) == 13)
	       ww_ltgoodmuiso = goodMuonIsolated(cms2.hyp_lt_index()[i_hyp]);
	  int ww_llgoodmuiso = -1;
	  if (abs(cms2.hyp_ll_id()[i_hyp]) == 13)
	       ww_llgoodmuiso = goodMuonIsolated(cms2.hyp_ll_index()[i_hyp]);
	  int ww_ltgoodel = -1;
	  if (abs(cms2.hyp_lt_id()[i_hyp]) == 11)
	       ww_ltgoodel = goodElectronWithoutIsolation(cms2.hyp_lt_index()[i_hyp]);
	  int ww_llgoodel = -1;
	  if (abs(cms2.hyp_ll_id()[i_hyp]) == 11)
	       ww_llgoodel = goodElectronWithoutIsolation(cms2.hyp_ll_index()[i_hyp]);
	  int ww_ltgoodeliso = -1;
	  if (abs(cms2.hyp_lt_id()[i_hyp]) == 11)
	       ww_ltgoodeliso = goodElectronIsolated(cms2.hyp_lt_index()[i_hyp]);
	  int ww_llgoodeliso = -1;
	  if (abs(cms2.hyp_ll_id()[i_hyp]) == 11)
	       ww_llgoodeliso = goodElectronIsolated(cms2.hyp_ll_index()[i_hyp]);
	  int ww_passFebsel_no_jetveto = 
	       ww_oppsign && ww_passzveto && ww_pass4met && ww_pass2met &&
	       ww_ltgoodmuiso && ww_llgoodmuiso && 
	       ww_ltgoodeliso && ww_llgoodeliso &&
	       ww_passaddzveto;
	  int ww_passFebsel_with_jetveto = 
	       ww_passFebsel_no_jetveto && (*hyp_njets)[i_hyp] == 0;
	  
	  vec_ww_oppsign       ->push_back(ww_oppsign     );
	  vec_ww_passzveto     ->push_back(ww_passzveto   );
	  vec_ww_isdyee        ->push_back(ww_isdyee      );
	  vec_ww_isdymm        ->push_back(ww_isdymm      );
	  vec_ww_isdytt        ->push_back(ww_isdytt      );
	  vec_ww_pmet          ->push_back(ww_pmet        );
	  vec_ww_pass4met      ->push_back(ww_pass4met    );
	  vec_ww_pass2met      ->push_back(ww_pass2met    );
	  vec_ww_ltgoodmu      ->push_back(ww_ltgoodmu    );
	  vec_ww_llgoodmu      ->push_back(ww_llgoodmu    );
	  vec_ww_ltgoodmuiso   ->push_back(ww_ltgoodmuiso );
	  vec_ww_llgoodmuiso   ->push_back(ww_llgoodmuiso );
	  vec_ww_ltgoodel      ->push_back(ww_ltgoodel    );
	  vec_ww_llgoodel      ->push_back(ww_llgoodel    );
	  vec_ww_ltgoodeliso   ->push_back(ww_ltgoodeliso );
	  vec_ww_llgoodeliso   ->push_back(ww_llgoodeliso );
	  vec_ww_passaddzveto  ->push_back(ww_passaddzveto);
	  vec_ww_passFebsel_no_jetveto->push_back(ww_passFebsel_no_jetveto);
	  vec_ww_passFebsel_with_jetveto->push_back(ww_passFebsel_with_jetveto);
     }
     
     iEvent.put(vec_ww_oppsign     	, "wwoppsign"		);
     iEvent.put(vec_ww_passzveto   	, "wwpasszveto"		);
     iEvent.put(vec_ww_isdyee      	, "wwisdyee"		);
     iEvent.put(vec_ww_isdymm      	, "wwisdymm"		);
     iEvent.put(vec_ww_isdytt      	, "wwisdytt"		);
     iEvent.put(vec_ww_pmet        	, "wwpmet"		);
     iEvent.put(vec_ww_pass4met    	, "wwpass4met"		);
     iEvent.put(vec_ww_pass2met    	, "wwpass2met"		);
     iEvent.put(vec_ww_ltgoodmu    	, "wwltgoodmu"		);
     iEvent.put(vec_ww_llgoodmu    	, "wwllgoodmu"		);
     iEvent.put(vec_ww_ltgoodmuiso 	, "wwltgoodmuiso"	);
     iEvent.put(vec_ww_llgoodmuiso 	, "wwllgoodmuiso"	);
     iEvent.put(vec_ww_ltgoodel    	, "wwltgoodel"		);
     iEvent.put(vec_ww_llgoodel    	, "wwllgoodel"		);
     iEvent.put(vec_ww_ltgoodeliso 	, "wwltgoodeliso"	);
     iEvent.put(vec_ww_llgoodeliso 	, "wwllgoodeliso"	);
     iEvent.put(vec_ww_passaddzveto	, "wwpassaddzveto"	);
     iEvent.put(vec_ww_passFebsel_no_jetveto, "wwpassFebselnojetveto");
     iEvent.put(vec_ww_passFebsel_with_jetveto, "wwpassFebselwithjetveto");
}

//define this as a plug-in
DEFINE_FWK_MODULE(WWCutMaker);