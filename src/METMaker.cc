//-*- C++ -*-
//
// Package:    METMaker
// Class:      METMaker
// 
/**\class METMaker METMaker.cc CMS2/METMaker/src/METMaker.cc

   Description: <one line class summary>

   Implementation:
   <Notes on implementation>
*/
//
// Original Author:  pts/4
//         Created:  Fri Jun  6 11:07:38 CDT 2008
// $Id: METMaker.cc,v 1.29 2012/05/10 02:25:40 macneill Exp $
//
//

// system include files
#include <memory>
#include <vector>
#include "TMath.h"

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "CMS3/NtupleMaker/interface/METMaker.h"

#include "DataFormats/METReco/interface/MET.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonReco/interface/MuonMETCorrectionData.h"
#include "DataFormats/Common/interface/ValueMap.h" 

#include "DataFormats/CaloTowers/interface/CaloTower.h"

typedef math::XYZTLorentzVectorF LorentzVector;
using namespace reco;
using namespace edm;
using namespace std;

//
// constructors and destructor
//

METMaker::METMaker(const edm::ParameterSet& iConfig) {

     /* 
	met -> raw caloMET. Does not include the HO by default in 21X
	metHO -> raw caloMET with HO
	metNOHF -> no HF but includes HO
	metNOHFHO -> no HF and no HO
	use scheme B thresholds
	https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideMETObjects
     */     

     aliasprefix_ = iConfig.getUntrackedParameter<std::string>("aliasPrefix");
     std::string branchprefix = aliasprefix_;
     if(branchprefix.find("_") != std::string::npos)
	  branchprefix.replace(branchprefix.find("_"),1,"");
     
     produces<bool> (branchprefix+"hbheFilter").setBranchAlias(aliasprefix_+"_hbheFilter");

     produces<float> (branchprefix+"met"          ).setBranchAlias(aliasprefix_+"_met"          );
     produces<float> (branchprefix+"metPhi"       ).setBranchAlias(aliasprefix_+"_metPhi"       );
     produces<float> (branchprefix+"metSig"       ).setBranchAlias(aliasprefix_+"_metSig"       );
     produces<float> (branchprefix+"metHO"        ).setBranchAlias(aliasprefix_+"_metHO"        );
     produces<float> (branchprefix+"metHOPhi"     ).setBranchAlias(aliasprefix_+"_metHOPhi"     );
     produces<float> (branchprefix+"metHOSig"     ).setBranchAlias(aliasprefix_+"_metHOSig"     );
     produces<float> (branchprefix+"metNoHF"      ).setBranchAlias(aliasprefix_+"_metNoHF"      );
     produces<float> (branchprefix+"metNoHFPhi"   ).setBranchAlias(aliasprefix_+"_metNoHFPhi"   );
     produces<float> (branchprefix+"metNoHFSig"   ).setBranchAlias(aliasprefix_+"_metNoHFSig"   );
     produces<float> (branchprefix+"metNoHFHO"    ).setBranchAlias(aliasprefix_+"_metNoHFHO"    );
     produces<float> (branchprefix+"metNoHFHOPhi" ).setBranchAlias(aliasprefix_+"_metNoHFHOPhi" );
     produces<float> (branchprefix+"metNoHFHOSig" ).setBranchAlias(aliasprefix_+"_metNoHFHOSig" );

     //same as above, but using Opt
     produces<float> (branchprefix+"metOpt"          ).setBranchAlias(aliasprefix_+"_metOpt"          );
     produces<float> (branchprefix+"metOptPhi"       ).setBranchAlias(aliasprefix_+"_metOptPhi"       );
     produces<float> (branchprefix+"metOptSig"       ).setBranchAlias(aliasprefix_+"_metOptSig"       );
     produces<float> (branchprefix+"metOptHO"        ).setBranchAlias(aliasprefix_+"_metOptHO"        );
     produces<float> (branchprefix+"metOptHOPhi"     ).setBranchAlias(aliasprefix_+"_metOptHOPhi"     );
     produces<float> (branchprefix+"metOptHOSig"     ).setBranchAlias(aliasprefix_+"_metOptHOSig"     );
     produces<float> (branchprefix+"metOptNoHF"      ).setBranchAlias(aliasprefix_+"_metOptNoHF"      );
     produces<float> (branchprefix+"metOptNoHFPhi"   ).setBranchAlias(aliasprefix_+"_metOptNoHFPhi"   );
     produces<float> (branchprefix+"metOptNoHFSig"   ).setBranchAlias(aliasprefix_+"_metOptNoHFSig"   );
     produces<float> (branchprefix+"metOptNoHFHO"    ).setBranchAlias(aliasprefix_+"_metOptNoHFHO"    );
     produces<float> (branchprefix+"metOptNoHFHOPhi" ).setBranchAlias(aliasprefix_+"_metOptNoHFHOPhi" );
     produces<float> (branchprefix+"metOptNoHFHOSig" ).setBranchAlias(aliasprefix_+"_metOptNoHFHOSig" );

     //raw CaloMET corrected for Muons
     produces<float> (branchprefix+"metMuonCorr"     ).setBranchAlias(aliasprefix_+"_metMuonCorr"     );
     produces<float> (branchprefix+"metMuonCorrPhi"  ).setBranchAlias(aliasprefix_+"_metMuonCorrPhi"  );
     produces<float> (branchprefix+"metMuonCorrSig"  ).setBranchAlias(aliasprefix_+"_metMuonCorrSig"  );

     //raw CaloMET corrected for JES (L2L3) and Muons
     produces<float> (branchprefix+"metMuonJESCorr"      ).setBranchAlias(aliasprefix_+"_metMuonJESCorr"      );
     produces<float> (branchprefix+"metMuonJESCorrPhi"   ).setBranchAlias(aliasprefix_+"_metMuonJESCorrPhi"   );
     produces<float> (branchprefix+"metMuonJESCorrSig"   ).setBranchAlias(aliasprefix_+"_metMuonJESCorrSig"   );

     // sumet
     produces<float> (branchprefix+"sumet"               ).setBranchAlias(aliasprefix_+"_sumet"                );  
     produces<float> (branchprefix+"sumetHO"             ).setBranchAlias(aliasprefix_+"_sumetHO"              );
     produces<float> (branchprefix+"sumetNoHF"           ).setBranchAlias(aliasprefix_+"_sumetNoHF"            );
     produces<float> (branchprefix+"sumetNoHFHO"         ).setBranchAlias(aliasprefix_+"_sumetNoHFHO"          );  
     produces<float> (branchprefix+"sumetOpt"            ).setBranchAlias(aliasprefix_+"_sumetOpt"             );
     produces<float> (branchprefix+"sumetOptHO"          ).setBranchAlias(aliasprefix_+"_sumetOptHO"           );
     produces<float> (branchprefix+"sumetOptNoHF"        ).setBranchAlias(aliasprefix_+"_sumetOptNoHF"         );
     produces<float> (branchprefix+"sumetOptNoHFHO"      ).setBranchAlias(aliasprefix_+"_sumetOptNoHFHO"       );  
     produces<float> (branchprefix+"sumetMuonCorr"       ).setBranchAlias(aliasprefix_+"_sumetMuonCorr"        );
     produces<float> (branchprefix+"sumetMuonJESCorr"    ).setBranchAlias(aliasprefix_+"_sumetMuonJESCorr"     );

     // store muon value map quantities
     if(aliasprefix_ == "evt") {
	  produces<vector<int> >   ("musmetflag"   ).setBranchAlias("mus_met_flag"              );
	  produces<vector<float> > ("musmetdeltax" ).setBranchAlias("mus_met_deltax"            );
	  produces<vector<float> > ("musmetdeltay" ).setBranchAlias("mus_met_deltay"            );
     }
     else {
	  produces<vector<int> >   (branchprefix+"musmetflag"   ).setBranchAlias(aliasprefix_+"_"+"mus_met_flag"              );
	  produces<vector<float> > (branchprefix+"musmetdeltax" ).setBranchAlias(aliasprefix_+"_"+"mus_met_deltax"            );
	  produces<vector<float> > (branchprefix+"musmetdeltay" ).setBranchAlias(aliasprefix_+"_"+"mus_met_deltay"            );
     }

     produces<float> (branchprefix+"ecalmet"            ).setBranchAlias(aliasprefix_+"_ecalmet"               );
     produces<float> (branchprefix+"hcalmet"            ).setBranchAlias(aliasprefix_+"_hcalmet"               );
     produces<float> (branchprefix+"ecalmetPhi"         ).setBranchAlias(aliasprefix_+"_ecalmetPhi"            );
     produces<float> (branchprefix+"hcalmetPhi"         ).setBranchAlias(aliasprefix_+"_hcalmetPhi"            );

     produces<float> (branchprefix+"endcappmet"         		).setBranchAlias(aliasprefix_+"_endcapp_met"       		);
     produces<float> (branchprefix+"endcapmmet"         		).setBranchAlias(aliasprefix_+"_endcapm_met"       		);
     produces<float> (branchprefix+"ecalendcappmet"     		).setBranchAlias(aliasprefix_+"_ecalendcapp_met"       	);
     produces<float> (branchprefix+"ecalendcapmmet"     		).setBranchAlias(aliasprefix_+"_ecalendcapm_met"       	);
     produces<float> (branchprefix+"hcalendcappmet"     		).setBranchAlias(aliasprefix_+"_hcalendcapp_met"       	);
     produces<float> (branchprefix+"hcalendcapmmet"     		).setBranchAlias(aliasprefix_+"_hcalendcapm_met"       	);
     produces<float> (branchprefix+"endcappmetPhi"         	).setBranchAlias(aliasprefix_+"_endcapp_metPhi"       	);
     produces<float> (branchprefix+"endcapmmetPhi"         	).setBranchAlias(aliasprefix_+"_endcapm_metPhi"       	);
     produces<float> (branchprefix+"ecalendcappmetPhi"     	).setBranchAlias(aliasprefix_+"_ecalendcapp_metPhi"       );
     produces<float> (branchprefix+"ecalendcapmmetPhi"     	).setBranchAlias(aliasprefix_+"_ecalendcapm_metPhi"       );
     produces<float> (branchprefix+"hcalendcappmetPhi"     	).setBranchAlias(aliasprefix_+"_hcalendcapp_metPhi"       );
     produces<float> (branchprefix+"hcalendcapmmetPhi"     	).setBranchAlias(aliasprefix_+"_hcalendcapm_metPhi"       );
     
     produces<float> (branchprefix+"metEtGt3").setBranchAlias(aliasprefix_+"_met_EtGt3");
     produces<float> (branchprefix+"metPhiEtGt3").setBranchAlias(aliasprefix_+"_metPhi_EtGt3");
     produces<float> (branchprefix+"sumetEtGt3").setBranchAlias(aliasprefix_+"_sumet_EtGt3");

     met_token                = consumes<edm::View<reco::CaloMET> >(iConfig.getParameter<edm::InputTag>("met_tag_"));
     metHO_token              = consumes<edm::View<reco::CaloMET> >(iConfig.getParameter<edm::InputTag>("metHO_tag_"));
     metNoHF_token            = consumes<edm::View<reco::CaloMET> >(iConfig.getParameter<edm::InputTag>("metNoHF_tag_"));
     metNoHFHO_token          = consumes<edm::View<reco::CaloMET> >(iConfig.getParameter<edm::InputTag>("metNoHFHO_tag_"));

     metOpt_token             = consumes<edm::View<reco::CaloMET> >(iConfig.getParameter<edm::InputTag>("metOpt_tag_"));
     metOptHO_token           = consumes<edm::View<reco::CaloMET> >(iConfig.getParameter<edm::InputTag>("metOptHO_tag_"));
     metOptNoHF_token         = consumes<edm::View<reco::CaloMET> >(iConfig.getParameter<edm::InputTag>("metOptNoHF_tag_"));
     metOptNoHFHO_token       = consumes<edm::View<reco::CaloMET> >(iConfig.getParameter<edm::InputTag>("metOptNoHFHO_tag_"));
  
     MuonJEScorMET_token      = consumes<edm::View<reco::CaloMET> >(iConfig.getParameter<edm::InputTag>("MuonJEScorMET_tag_"));
     corMetGlobalMuons_token  = consumes<edm::View<reco::CaloMET> >(iConfig.getParameter<edm::InputTag>("corMetGlobalMuons_tag_"));

     muon_vm_token            = consumes<edm::View<reco::CaloMET> >(iConfig.getParameter<edm::InputTag>("muon_vm_tag_"));
     muon_token               = consumes<edm::View<reco::CaloMET> >(iConfig.getParameter<edm::InputTag>("muon_tag_"));

     caloTowerInputTag     = iConfig.getParameter<edm::InputTag>("caloTower_tag_");
     towerEtThreshold      = iConfig.getParameter<double>       ("towerEtThreshold_");
     make_eta_rings        = iConfig.getParameter<bool>         ("make_eta_rings_");
     hbheNoiseFilterToken               = consumes<bool>(iConfig.getParameter<edm::InputTag>("hbheNoiseFilterInputTag_"));

     if( make_eta_rings ) {
	  produces<vector<float> > (branchprefix+"towermetetaslice"         ).setBranchAlias(aliasprefix_+"_towermet_etaslice"         );
	  produces<vector<float> > (branchprefix+"ecalmetetaslice"          ).setBranchAlias(aliasprefix_+"_ecalmet_etaslice"          );
	  produces<vector<float> > (branchprefix+"hcalmetetaslice"          ).setBranchAlias(aliasprefix_+"_hcalmet_etaslice"          );
	  produces<vector<float> > (branchprefix+"towermetetaslicePhi"      ).setBranchAlias(aliasprefix_+"_towermet_etaslicePhi"      );
	  produces<vector<float> > (branchprefix+"ecalmetetaslicePhi"       ).setBranchAlias(aliasprefix_+"_ecalmet_etaslicePhi"       );
	  produces<vector<float> > (branchprefix+"hcalmetetaslicePhi"       ).setBranchAlias(aliasprefix_+"_hcalmet_etaslicePhi"       );
     }

}

METMaker::~METMaker()
{
}

void  METMaker::beginJob()
{
}

void METMaker::endJob()
{
}

// ------------ method called to produce the data  ------------
void METMaker::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  
     unique_ptr<bool>    evt_hbheFilter          (new bool      );
     unique_ptr<float>   evt_met                 (new float     );
     unique_ptr<float>   evt_metPhi              (new float     );
     unique_ptr<float>   evt_metSig              (new float     );
     unique_ptr<float>   evt_metHO               (new float     );
     unique_ptr<float>   evt_metHOPhi            (new float     );
     unique_ptr<float>   evt_metHOSig            (new float     );
     unique_ptr<float>   evt_metNoHF             (new float     );
     unique_ptr<float>   evt_metNoHFPhi          (new float     );
     unique_ptr<float>   evt_metNoHFSig          (new float     );
     unique_ptr<float>   evt_metNoHFHO           (new float     );
     unique_ptr<float>   evt_metNoHFHOPhi        (new float     );
     unique_ptr<float>   evt_metNoHFHOSig        (new float     );

     unique_ptr<float>   evt_metOpt              (new float     );
     unique_ptr<float>   evt_metOptPhi           (new float     );
     unique_ptr<float>   evt_metOptSig           (new float     );
     unique_ptr<float>   evt_metOptHO            (new float     );
     unique_ptr<float>   evt_metOptHOPhi         (new float     );
     unique_ptr<float>   evt_metOptHOSig         (new float     );
     unique_ptr<float>   evt_metOptNoHF          (new float     );
     unique_ptr<float>   evt_metOptNoHFPhi       (new float     );
     unique_ptr<float>   evt_metOptNoHFSig       (new float     );
     unique_ptr<float>   evt_metOptNoHFHO        (new float     );
     unique_ptr<float>   evt_metOptNoHFHOPhi     (new float     );
     unique_ptr<float>   evt_metOptNoHFHOSig     (new float     );
  
     unique_ptr<float>   evt_metMuonCorr         (new float     );
     unique_ptr<float>   evt_metMuonCorrPhi      (new float     );
     unique_ptr<float>   evt_metMuonCorrSig      (new float     );
     unique_ptr<float>   evt_metMuonJESCorr      (new float     );
     unique_ptr<float>   evt_metMuonJESCorrPhi   (new float     );
     unique_ptr<float>   evt_metMuonJESCorrSig   (new float     );

     unique_ptr<float>   evt_sumet               (new float     );
     unique_ptr<float>   evt_sumetHO	       (new float     );
     unique_ptr<float>   evt_sumetNoHF           (new float     );
     unique_ptr<float>   evt_sumetNoHFHO         (new float     );
     unique_ptr<float>   evt_sumetOpt            (new float     );
     unique_ptr<float>   evt_sumetOptHO          (new float     );
     unique_ptr<float>   evt_sumetOptNoHF        (new float     );
     unique_ptr<float>   evt_sumetOptNoHFHO      (new float     );
     unique_ptr<float>   evt_sumetMuonCorr       (new float     );
     unique_ptr<float>   evt_sumetMuonJESCorr    (new float     );

     unique_ptr<vector<int>   > mus_met_flag   ( new vector<int>   );
     unique_ptr<vector<float> > mus_met_deltax ( new vector<float> );
     unique_ptr<vector<float> > mus_met_deltay ( new vector<float> );

     unique_ptr<float> evt_ecalmet         (new float     );
     unique_ptr<float> evt_hcalmet         (new float     );
     unique_ptr<float> evt_ecalmetPhi      (new float     );
     unique_ptr<float> evt_hcalmetPhi      (new float     );

     unique_ptr<float> evt_endcapp_met         (new float );
     unique_ptr<float> evt_endcapm_met      	  (new float );
     unique_ptr<float> evt_ecalendcapp_met  	  (new float );
     unique_ptr<float> evt_ecalendcapm_met  	  (new float );
     unique_ptr<float> evt_hcalendcapp_met  	  (new float );
     unique_ptr<float> evt_hcalendcapm_met  	  (new float );
     unique_ptr<float> evt_endcapp_metPhi   	  (new float );
     unique_ptr<float> evt_endcapm_metPhi   	  (new float );
     unique_ptr<float> evt_ecalendcapp_metPhi  (new float );
     unique_ptr<float> evt_ecalendcapm_metPhi  (new float );
     unique_ptr<float> evt_hcalendcapp_metPhi  (new float );
     unique_ptr<float> evt_hcalendcapm_metPhi  (new float );

     unique_ptr<vector<float> > evt_towermet_etaslice        (new vector<float>     ); //towermet = ecalmet + hcalmet
     unique_ptr<vector<float> > evt_ecalmet_etaslice         (new vector<float>     );
     unique_ptr<vector<float> > evt_hcalmet_etaslice         (new vector<float>     );
     unique_ptr<vector<float> > evt_towermet_etaslicePhi     (new vector<float>     ); //towermet = ecalmet + hcalmet
     unique_ptr<vector<float> > evt_ecalmet_etaslicePhi      (new vector<float>     );
     unique_ptr<vector<float> > evt_hcalmet_etaslicePhi      (new vector<float>     );

     unique_ptr<float> evt_met_EtGt3 (new float);
     unique_ptr<float> evt_metPhi_EtGt3 (new float);
     unique_ptr<float> evt_sumet_EtGt3 (new float);

     edm::Handle<CaloTowerCollection> h_caloTowers;
     iEvent.getByLabel("towerMaker", h_caloTowers);

     edm::Handle< edm::View<reco::CaloMET> > met_h;
     edm::Handle< edm::View<reco::CaloMET> > metHO_h;
     edm::Handle< edm::View<reco::CaloMET> > metNoHF_h;
     edm::Handle< edm::View<reco::CaloMET> > metNoHFHO_h;

     edm::Handle< edm::View<reco::CaloMET> > metOpt_h;
     edm::Handle< edm::View<reco::CaloMET> > metOptHO_h;
     edm::Handle< edm::View<reco::CaloMET> > metOptNoHF_h;
     edm::Handle< edm::View<reco::CaloMET> > metOptNoHFHO_h;

     edm::Handle< edm::View<reco::CaloMET> > metMuonCorr_h;
     edm::Handle< edm::View<reco::CaloMET> > metMuonJESCorr_h;

     edm::Handle< edm::ValueMap<reco::MuonMETCorrectionData> > muon_vm_h;
     edm::Handle< reco::MuonCollection > muon_h;
     edm::Handle<bool> filter_h;     

     iEvent.getByToken(hbheNoiseFilterToken, filter_h);
     iEvent.getByToken(met_token      , met_h       );
     iEvent.getByToken(metHO_token    , metHO_h     );
     iEvent.getByToken(metNoHF_token  , metNoHF_h   );
     iEvent.getByToken(metNoHFHO_token, metNoHFHO_h );
     iEvent.getByToken(metOpt_token      , metOpt_h       );
     iEvent.getByToken(metOptHO_token    , metOptHO_h     );
     iEvent.getByToken(metOptNoHF_token  , metOptNoHF_h   );
     iEvent.getByToken(metOptNoHFHO_token, metOptNoHFHO_h );
     iEvent.getByToken(corMetGlobalMuons_token, metMuonCorr_h      );
     iEvent.getByToken(MuonJEScorMET_token,     metMuonJESCorr_h   );
     iEvent.getByToken(muon_vm_token, muon_vm_h );
     iEvent.getByToken(muon_token   , muon_h    );

     if (!met_h.isValid()) throw cms::Exception("GenMaker: Could not load met_h handle");
     if (!metHO_h.isValid()) throw cms::Exception("GenMaker: Could not load metHO_h handle");
     if (!metNoHF_h.isValid()) throw cms::Exception("GenMaker: Could not load metNoHF_h handle");
     if (!metNoHFHO_h.isValid()) throw cms::Exception("GenMaker: Could not load metNoHFHO_h handle");
     if (!metOpt_h.isValid()) throw cms::Exception("GenMaker: Could not load metOpt_h handle");
     if (!metOptHO_h.isValid()) throw cms::Exception("GenMaker: Could not load metOptHO_h handle");
     if (!metOptNoHF_h.isValid()) throw cms::Exception("GenMaker: Could not load metOptNoHF_h handle");
     if (!metOptNoHFHO_h.isValid()) throw cms::Exception("GenMaker: Could not load metOptNoHFHO_h handle");
     if (!metMuonCorr_h.isValid()) throw cms::Exception("GenMaker: Could not load metMuonCorr_h handle");
     if (!metMuonJESCorr_h.isValid()) throw cms::Exception("GenMaker: Could not load metMuonJESCorr_h handle");
     if (!muon_h.isValid()) throw cms::Exception("GenMaker: Could not load muon_h handle");
     if (!muon_vm_h.isValid()) throw cms::Exception("GenMaker: Could not load muon_vm_h handle");
     if (!filter_h.isValid()) throw cms::Exception("GenMaker: Could not load filter_h handle");

    
     *evt_hbheFilter   = filter_h.isValid() ? *filter_h : false;
     *evt_met			= met_h.isValid()		 ? ( met_h->front()		 ).et()			: -9999;
     *evt_metPhi		= met_h.isValid()		 ? ( met_h->front()		 ).phi()		: -9999;
     *evt_metSig		= met_h.isValid()		 ? ( met_h->front()		 ).metSignificance()	: -9999;
     *evt_sumet			= met_h.isValid()		 ? ( met_h->front()		 ).sumEt()		: -9999;
     *evt_metHO			= metHO_h.isValid()		 ? ( metHO_h->front()		 ).et()			: -9999;
     *evt_metHOPhi		= metHO_h.isValid()		 ? ( metHO_h->front()		 ).phi()		: -9999;
     *evt_metHOSig		= metHO_h.isValid()		 ? ( metHO_h->front()		 ).metSignificance()	: -9999;
     *evt_sumetHO		= metHO_h.isValid()		 ? ( metHO_h->front()		 ).sumEt()		: -9999;  
     *evt_metNoHF		= metNoHF_h.isValid()		 ? ( metNoHF_h->front()		 ).et()			: -9999;
     *evt_metNoHFPhi		= metNoHF_h.isValid()		 ? ( metNoHF_h->front()		 ).phi()		: -9999;
     *evt_metNoHFSig		= metNoHF_h.isValid()		 ? ( metNoHF_h->front()		 ).metSignificance()	: -9999;
     *evt_sumetNoHF		= metNoHF_h.isValid()		 ? ( metNoHF_h->front()		 ).sumEt()		: -9999;
     *evt_metNoHFHO		= metNoHFHO_h.isValid()		 ? ( metNoHFHO_h->front()	 ).et()			: -9999;
     *evt_metNoHFHOPhi		= metNoHFHO_h.isValid()		 ? ( metNoHFHO_h->front()	 ).phi()		: -9999;
     *evt_metNoHFHOSig		= metNoHFHO_h.isValid()		 ? ( metNoHFHO_h->front()	 ).metSignificance()	: -9999;     
     *evt_sumetNoHFHO		= metNoHFHO_h.isValid()		 ? ( metNoHFHO_h->front()	 ).sumEt()		: -9999;
     *evt_metOpt		= metOpt_h.isValid()		 ? ( metOpt_h->front()		 ).et()			: -9999;
     *evt_metOptPhi		= metOpt_h.isValid()		 ? ( metOpt_h->front()		 ).phi()		: -9999;
     *evt_metOptSig		= metOpt_h.isValid()		 ? ( metOpt_h->front()		 ).metSignificance()	: -9999;
     *evt_sumetOpt		= metOpt_h.isValid()		 ? ( metOpt_h->front()		 ).sumEt()		: -9999; 
     *evt_metOptHO		= metOptHO_h.isValid()		 ? ( metOptHO_h->front()	 ).et()			: -9999;
     *evt_metOptHOPhi		= metOptHO_h.isValid()		 ? ( metOptHO_h->front()	 ).phi()		: -9999;
     *evt_metOptHOSig		= metOptHO_h.isValid()		 ? ( metOptHO_h->front()	 ).metSignificance()	: -9999;
     *evt_sumetOptHO		= metOptHO_h.isValid()		 ? ( metOptHO_h->front()	 ).sumEt()		: -9999;    
     *evt_metOptNoHF		= metOptNoHF_h.isValid()	 ? ( metOptNoHF_h->front()	 ).et()			: -9999;
     *evt_metOptNoHFPhi		= metOptNoHF_h.isValid()	 ? ( metOptNoHF_h->front()	 ).phi()		: -9999;
     *evt_metOptNoHFSig		= metOptNoHF_h.isValid()	 ? ( metOptNoHF_h->front()	 ).metSignificance()	: -9999;
     *evt_sumetOptNoHF		= metOptNoHF_h.isValid()	 ? ( metOptNoHF_h->front()	 ).sumEt()		: -9999;
     *evt_metOptNoHFHO		= metOptNoHFHO_h.isValid()	 ? ( metOptNoHFHO_h->front()	 ).et()			: -9999;
     *evt_metOptNoHFHOPhi	= metOptNoHFHO_h.isValid()	 ? ( metOptNoHFHO_h->front()	 ).phi()		: -9999;
     *evt_metOptNoHFHOSig	= metOptNoHFHO_h.isValid()	 ? ( metOptNoHFHO_h->front()	 ).metSignificance()	: -9999;
     *evt_sumetOptNoHFHO	= metOptNoHFHO_h.isValid()       ? ( metOptNoHFHO_h->front()     ).sumEt()              : -9999;
     *evt_metMuonCorr		= metMuonCorr_h.isValid()	 ? ( metMuonCorr_h->front()	 ).et()			: -9999;
     *evt_metMuonCorrPhi	= metMuonCorr_h.isValid()	 ? ( metMuonCorr_h->front()	 ).phi()		: -9999;
     *evt_metMuonCorrSig	= metMuonCorr_h.isValid()	 ? ( metMuonCorr_h->front()	 ).metSignificance()	: -9999;
     *evt_sumetMuonCorr		= metMuonCorr_h.isValid()	 ? ( metMuonCorr_h->front()	 ).sumEt()		: -9999;
     *evt_metMuonJESCorr	= metMuonJESCorr_h.isValid()	 ? ( metMuonJESCorr_h->front()	 ).et()			: -9999;
     *evt_metMuonJESCorrPhi	= metMuonJESCorr_h.isValid()	 ? ( metMuonJESCorr_h->front()	 ).phi()		: -9999;
     *evt_metMuonJESCorrSig	= metMuonJESCorr_h.isValid()	 ? ( metMuonJESCorr_h->front()	 ).metSignificance()	: -9999;     
     *evt_sumetMuonJESCorr      = metMuonJESCorr_h.isValid()     ? ( metMuonJESCorr_h->front()   ).sumEt()              : -9999;

     

     edm::ValueMap<reco::MuonMETCorrectionData> muon_data = *muon_vm_h;

     const unsigned int nMuons = muon_h->size();

     // loop over muons and extract quantities from ValueMap
     for( unsigned int mus = 0; mus < nMuons; mus++ ) {

	  reco::MuonRef muref( muon_h, mus);
	  reco::MuonMETCorrectionData muCorrData = (muon_data)[muref];

	  mus_met_flag  ->push_back( muCorrData.type()  );
	  mus_met_deltax->push_back( muCorrData.corrX() );
	  mus_met_deltay->push_back( muCorrData.corrY() );
     }

     //MET from Towers
     const double etaborder = 1.479; //set where barrel ends in eta (+/- this)--this should be an entry in etaranges unless you want to mess up comparison with eta slices
     const double hf_eta = 3.; //HF eta
     double ecalmetx=0., ecalmety=0.;
     double hcalmetx=0., hcalmety=0.;
     double epmetx=0., epmety=0., emmetx=0., emmety=0.; //endcap plus and minus eta--both ecal and hcal
     double epecalx=0., epecaly=0., emecalx=0., emecaly=0.; //endcap ecal plus and minus eta
     double ephcalx=0., ephcaly=0., emhcalx=0., emhcaly=0.; //endcap hcal plus and minus eta
     const unsigned int N = 84; //number eta slices: 41 on each side, plus overflow on each side (just in case)
     double twretax[N], twretay[N]; //these six are for eta slices
     double ecaletax[N], ecaletay[N];
     double hcaletax[N], hcaletay[N];
     for( unsigned int j=0; j<N; j++ ) {
	  twretax[j]=0.;
	  twretay[j]=0.; 
	  ecaletax[j]=0.;
	  ecaletay[j]=0.;
	  hcaletax[j]=0.;
	  hcaletay[j]=0.;
     }
	  
     //ranges are symmetric about 0, last entry is upper edge of last tower.
     //this array has 83 entries, making 82 bins. So offset is 1 entry from the arrays above
     double etaranges[] = {-5.191, -4.889, -4.716, -4.538, -4.363, -4.191, -4.013, -3.839, -3.664, -3.489, -3.314, -3.139, -2.964, -2.853, -2.650, -2.5, -2.322, -2.172, -2.043, -1.93, -1.83, -1.74, -1.653, -1.566, -1.479, -1.392, -1.305, -1.218, -1.131, -1.044, -0.957, -0.879, -0.783, -0.696, -0.609, -0.522, -0.435, -0.348, -0.261, -0.174, -0.087, 0, 0.087, 0.174, 0.261, 0.348, 0.435, 0.522, 0.609, 0.696, 0.783, 0.879, 0.957, 1.044, 1.131, 1.218, 1.305, 1.392, 1.479, 1.566, 1.653, 1.74, 1.83, 1.93, 2.043, 2.172, 2.322, 2.5, 2.650, 2.853, 2.964, 3.139, 3.314, 3.489, 3.664, 3.839, 4.013, 4.191, 4.363, 4.538, 4.716, 4.889, 5.191};

     float cmetprime_x = 0.;
     float cmetprime_y = 0.;
     float csumetprime = 0.;
     for(CaloTowerCollection::const_iterator it = h_caloTowers->begin();
	 it != h_caloTowers->end(); it++) {
       
         if ((it->emEt()+it->hadEt()) > 3.) {
             float et = it->emEt() + it->hadEt();
             cmetprime_x -= et * cos(it->phi());
             cmetprime_y -= et * sin(it->phi());
             csumetprime += et;
         }

	  if(it->et() < towerEtThreshold)
	       continue;

	  double phi   = it->phi();
	  double eta   = it->eta();
	  double emEt  = it->emEt();
	  double hadEt = it->hadEt();
	  if( TMath::Abs(eta) > hf_eta ) { //no ecal in HF (fix broken method)
	       emEt = 0.;
	       hadEt = it->et();
	  }

	  ecalmetx += emEt*cos(phi);
	  ecalmety += emEt*sin(phi);
	  hcalmetx += hadEt*cos(phi);
	  hcalmety += hadEt*sin(phi);
	  if( eta < -etaborder ) { //endcap (HE+HF) minus
	       emmetx += emEt*cos(phi) + hadEt*cos(phi);
	       emmety += emEt*sin(phi) + hadEt*sin(phi);
	       emecalx += emEt*cos(phi);
	       emecaly += emEt*sin(phi);
	       emhcalx += hadEt*cos(phi);
	       emhcaly += hadEt*sin(phi);
	  }
	  else if( eta >= etaborder ) {
	       epmetx += emEt*cos(phi) + hadEt*cos(phi);
	       epmety += emEt*sin(phi) + hadEt*sin(phi);
	       epecalx += emEt*cos(phi);
	       epecaly += emEt*sin(phi);
	       ephcalx += hadEt*cos(phi);
	       ephcaly += hadEt*sin(phi);
	  }

	  if( !make_eta_rings ) continue; //rest of loop is just eta rings, so skip if false

	  int index = -1;
	  for( unsigned int j=0; j<N-2; j++ ) { //see comments above, below
	       if( eta < etaranges[0] ) //overflow negative eta
		    index = 0;
	       else if( eta >= etaranges[j] && eta < etaranges[j+1] )
		    index = j + 1;      //don't j++ here bc that changes j--see comment above
	       else if( eta >= etaranges[N-2] ) //overflow positive eta--warning: uses N (another -1 bc of c++ convention of starting at 0)
		    index = N-1;        //warning: uses N
	  }
	  if( index == -1 ) { //to be safe
	       cout << "METMaker: error in finding eta slice for tower " << it - h_caloTowers->begin() << endl;
	       continue;
	  }
    
	  twretax[index]  += emEt*cos(phi) + hadEt*cos(phi);
	  twretay[index]  += emEt*sin(phi) + hadEt*sin(phi);
    
	  ecaletax[index] += emEt*cos(phi);
	  ecaletay[index] += emEt*sin(phi);

	  hcaletax[index] += hadEt*cos(phi);
	  hcaletay[index] += hadEt*sin(phi);
     }	
     
     *evt_met_EtGt3    = sqrt(cmetprime_x * cmetprime_x + cmetprime_y * cmetprime_y);
     *evt_metPhi_EtGt3 = atan2(cmetprime_y, cmetprime_x);
     *evt_sumet_EtGt3  = csumetprime;

     *evt_ecalmet  = sqrt( ecalmetx*ecalmetx + ecalmety*ecalmety );
     *evt_hcalmet  = sqrt( hcalmetx*hcalmetx + hcalmety*hcalmety );
     *evt_ecalmetPhi  = atan2( -ecalmety, -ecalmetx );
     *evt_hcalmetPhi  = atan2( -hcalmety, -hcalmetx );

     *evt_endcapp_met        = sqrt( epmetx*epmetx + epmety*epmety );
     *evt_endcapm_met        = sqrt( emmetx*emmetx + emmety*emmety );
     *evt_ecalendcapp_met    = sqrt( epecalx*epecalx + epecaly*epecaly );
     *evt_ecalendcapm_met    = sqrt( emecalx*emecalx + emecaly*emecaly );
     *evt_hcalendcapp_met    = sqrt( ephcalx*ephcalx + ephcaly*ephcaly ); 
     *evt_hcalendcapm_met    = sqrt( emhcalx*emhcalx + emhcaly*emhcaly ); 
     *evt_endcapp_metPhi     = atan2( -epmety, -epmetx );
     *evt_endcapm_metPhi     = atan2( -emmety, -emmetx );
     *evt_ecalendcapp_metPhi = atan2( -epecaly, -epecalx );
     *evt_ecalendcapm_metPhi = atan2( -emecaly, -emecalx );
     *evt_hcalendcapp_metPhi = atan2( -ephcaly, -ephcalx );
     *evt_hcalendcapm_metPhi = atan2( -emhcaly, -emhcalx ); 

     for( unsigned int i=0; i<N; i++ ) { //put all eta slices in vector
	  if( !make_eta_rings ) break; //no point pushing if not putting
	  evt_towermet_etaslice->push_back( sqrt( twretax[i]*twretax[i]   + twretay[i]*twretay[i] ) );
	  evt_ecalmet_etaslice ->push_back( sqrt( ecaletax[i]*ecaletax[i] + ecaletay[i]*ecaletay[i] ) );
	  evt_hcalmet_etaslice ->push_back( sqrt( hcaletax[i]*hcaletax[i] + hcaletay[i]*hcaletay[i] ) );
	  evt_towermet_etaslicePhi->push_back( atan2( -twretay[i] , -twretax[i] ) );
	  evt_ecalmet_etaslicePhi ->push_back( atan2( -ecaletay[i], -ecaletax[i] ) );
	  evt_hcalmet_etaslicePhi ->push_back( atan2( -hcaletay[i], -hcaletax[i] ) );
     }

     

     std::string branchprefix = aliasprefix_;
     if(branchprefix.find("_") != std::string::npos)
	  branchprefix.replace(branchprefix.find("_"),1,"");

     iEvent.put(std::move(evt_hbheFilter        ),branchprefix+"hbheFilter"       );
     iEvent.put(std::move(evt_met               ),branchprefix+"met"              );
     iEvent.put(std::move(evt_metPhi            ),branchprefix+"metPhi"           );
     iEvent.put(std::move(evt_metSig            ),branchprefix+"metSig"           );
     iEvent.put(std::move(evt_metHO             ),branchprefix+"metHO"            );
     iEvent.put(std::move(evt_metHOPhi          ),branchprefix+"metHOPhi"         );
     iEvent.put(std::move(evt_metHOSig          ),branchprefix+"metHOSig"         );
     iEvent.put(std::move(evt_metNoHF           ),branchprefix+"metNoHF"          );
     iEvent.put(std::move(evt_metNoHFPhi        ),branchprefix+"metNoHFPhi"       );
     iEvent.put(std::move(evt_metNoHFSig        ),branchprefix+"metNoHFSig"       );
     iEvent.put(std::move(evt_metNoHFHO         ),branchprefix+"metNoHFHO"        );
     iEvent.put(std::move(evt_metNoHFHOPhi      ),branchprefix+"metNoHFHOPhi"     );
     iEvent.put(std::move(evt_metNoHFHOSig      ),branchprefix+"metNoHFHOSig"     );

     iEvent.put(std::move(evt_metOpt            ),branchprefix+"metOpt"           );
     iEvent.put(std::move(evt_metOptPhi         ),branchprefix+"metOptPhi"        );
     iEvent.put(std::move(evt_metOptSig         ),branchprefix+"metOptSig"        );
     iEvent.put(std::move(evt_metOptHO          ),branchprefix+"metOptHO"         );
     iEvent.put(std::move(evt_metOptHOPhi       ),branchprefix+"metOptHOPhi"      );
     iEvent.put(std::move(evt_metOptHOSig       ),branchprefix+"metOptHOSig"      );
     iEvent.put(std::move(evt_metOptNoHF        ),branchprefix+"metOptNoHF"       );
     iEvent.put(std::move(evt_metOptNoHFPhi     ),branchprefix+"metOptNoHFPhi"    );
     iEvent.put(std::move(evt_metOptNoHFSig     ),branchprefix+"metOptNoHFSig"    );
     iEvent.put(std::move(evt_metOptNoHFHO      ),branchprefix+"metOptNoHFHO"     );
     iEvent.put(std::move(evt_metOptNoHFHOPhi   ),branchprefix+"metOptNoHFHOPhi"  );
     iEvent.put(std::move(evt_metOptNoHFHOSig   ),branchprefix+"metOptNoHFHOSig"  );
  
     iEvent.put(std::move(evt_metMuonCorr       ),branchprefix+"metMuonCorr"      );
     iEvent.put(std::move(evt_metMuonCorrPhi    ),branchprefix+"metMuonCorrPhi"   );
     iEvent.put(std::move(evt_metMuonCorrSig    ),branchprefix+"metMuonCorrSig"   );
     iEvent.put(std::move(evt_metMuonJESCorr    ),branchprefix+"metMuonJESCorr"   );
     iEvent.put(std::move(evt_metMuonJESCorrPhi ),branchprefix+"metMuonJESCorrPhi");
     iEvent.put(std::move(evt_metMuonJESCorrSig ),branchprefix+"metMuonJESCorrSig");

     iEvent.put(std::move(evt_sumet             ),branchprefix+"sumet"            );  
     iEvent.put(std::move(evt_sumetHO           ),branchprefix+"sumetHO"	       );
     iEvent.put(std::move(evt_sumetNoHF         ),branchprefix+"sumetNoHF"        );
     iEvent.put(std::move(evt_sumetNoHFHO       ),branchprefix+"sumetNoHFHO"      );
     iEvent.put(std::move(evt_sumetOpt          ),branchprefix+"sumetOpt"         );
     iEvent.put(std::move(evt_sumetOptHO        ),branchprefix+"sumetOptHO"       );
     iEvent.put(std::move(evt_sumetOptNoHF      ),branchprefix+"sumetOptNoHF"     );
     iEvent.put(std::move(evt_sumetOptNoHFHO    ),branchprefix+"sumetOptNoHFHO"   );
     iEvent.put(std::move(evt_sumetMuonCorr     ),branchprefix+"sumetMuonCorr"    );
     iEvent.put(std::move(evt_sumetMuonJESCorr  ),branchprefix+"sumetMuonJESCorr" );

     if(aliasprefix_ == "evt") {
	  iEvent.put(std::move(mus_met_flag          ),"musmetflag"          );
	  iEvent.put(std::move(mus_met_deltax        ),"musmetdeltax"        );
	  iEvent.put(std::move(mus_met_deltay        ),"musmetdeltay"        );
     }
     else {
	  iEvent.put(std::move(mus_met_flag          ),branchprefix+"musmetflag"          );
	  iEvent.put(std::move(mus_met_deltax        ),branchprefix+"musmetdeltax"        );
	  iEvent.put(std::move(mus_met_deltay        ),branchprefix+"musmetdeltay"        );
     }

     iEvent.put(std::move(evt_ecalmet           ),branchprefix+"ecalmet"          );
     iEvent.put(std::move(evt_hcalmet           ),branchprefix+"hcalmet"          );
     iEvent.put(std::move(evt_ecalmetPhi        ),branchprefix+"ecalmetPhi"       );
     iEvent.put(std::move(evt_hcalmetPhi        ),branchprefix+"hcalmetPhi"       );

     iEvent.put(std::move(evt_endcapp_met       ),  branchprefix+"endcappmet"  );
     iEvent.put(std::move(evt_endcapm_met       ),  branchprefix+"endcapmmet"  );
     iEvent.put(std::move(evt_ecalendcapp_met   ),  branchprefix+"ecalendcappmet"  );
     iEvent.put(std::move(evt_ecalendcapm_met   ),  branchprefix+"ecalendcapmmet"  );
     iEvent.put(std::move(evt_hcalendcapp_met   ),  branchprefix+"hcalendcappmet"  );
     iEvent.put(std::move(evt_hcalendcapm_met   ),  branchprefix+"hcalendcapmmet"  );
     iEvent.put(std::move(evt_endcapp_metPhi    ),  branchprefix+"endcappmetPhi"  );
     iEvent.put(std::move(evt_endcapm_metPhi    ),  branchprefix+"endcapmmetPhi"  );
     iEvent.put(std::move(evt_ecalendcapp_metPhi),  branchprefix+"ecalendcappmetPhi"  );
     iEvent.put(std::move(evt_ecalendcapm_metPhi),  branchprefix+"ecalendcapmmetPhi"  );
     iEvent.put(std::move(evt_hcalendcapp_metPhi),  branchprefix+"hcalendcappmetPhi"  );
     iEvent.put(std::move(evt_hcalendcapm_metPhi),  branchprefix+"hcalendcapmmetPhi"  );

     if( make_eta_rings ) {
	  iEvent.put(std::move(evt_towermet_etaslice   ),branchprefix+"towermetetaslice" );
	  iEvent.put(std::move(evt_ecalmet_etaslice    ),branchprefix+"ecalmetetaslice"  );
	  iEvent.put(std::move(evt_hcalmet_etaslice    ),branchprefix+"hcalmetetaslice"  );
	  iEvent.put(std::move(evt_towermet_etaslicePhi),branchprefix+"towermetetaslicePhi" );
	  iEvent.put(std::move(evt_ecalmet_etaslicePhi ),branchprefix+"ecalmetetaslicePhi"  );
	  iEvent.put(std::move(evt_hcalmet_etaslicePhi ),branchprefix+"hcalmetetaslicePhi"  );
     }

     iEvent.put(std::move(evt_met_EtGt3), branchprefix+"metEtGt3");
     iEvent.put(std::move(evt_metPhi_EtGt3), branchprefix+"metPhiEtGt3");
     iEvent.put(std::move(evt_sumet_EtGt3), branchprefix+"sumetEtGt3");
  
}

//define this as a plug-in
DEFINE_FWK_MODULE(METMaker);




  
