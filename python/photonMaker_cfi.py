import FWCore.ParameterSet.Config as cms
import CMS3.NtupleMaker.configProcessName as configProcessName

photonMaker = cms.EDProducer("PhotonMaker",
                             aliasPrefix = cms.untracked.string("photons"),
                             minEt       = cms.double(10.), #gev, min to keep
                             photonsInputTag = cms.InputTag("slimmedPhotons"),
                             )

