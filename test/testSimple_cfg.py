import FWCore.ParameterSet.Config as cms

process = cms.Process("CMS2")

from Configuration.EventContent.EventContent_cff import *

process.configurationMetadata = cms.untracked.PSet(
        version = cms.untracked.string('$Revision: 1.5 $'),
        annotation = cms.untracked.string('CMS2'),
        name = cms.untracked.string('CMS2 test configuration')
)

# load event level configurations
#process.load("Configuration.StandardSequences.Services_cff")
#process.load("Configuration.StandardSequences.Reconstruction_cff")
#process.load("Configuration.StandardSequences.MixingNoPileUp_cff")
#process.load("Configuration.StandardSequences.VtxSmearedGauss_cff")
process.load("Configuration.StandardSequences.Geometry_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = "MC_31X_V3::All"

process.options = cms.untracked.PSet(
    Rethrow = cms.untracked.vstring('ProductNotFound')
)

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.threshold = ''
process.MessageLogger.cerr.FwkReport.reportEvery = 100


#----------------------------------------------------
#CMS2 includes
#----------------------------------------------------
process.load("CMS2.NtupleMaker.beamSpotMaker_cfi")
#CMS2Btagging
#process.load("CMS2.NtupleMaker.bTaggingSequence_cfi")
#CMS2TrkBtagging
#process.load("CMS2.NtupleMaker.bTaggingTrkSequence_cfi")
#process.load("CMS2.NtupleMaker.bTagMaker_cfi")
#process.load("CMS2.NtupleMaker.calotauMaker_cfi")
#process.load("CMS2.NtupleMaker.candToGenAssMaker_cfi")
#process.load("CMS2.NtupleMaker.conversionMaker_cfi")
#elCaloIsoSequence = cms.Sequence(egammaBasicClusterMerger*elCaloIsoMaker)
#process.load("CMS2.NtupleMaker.elCaloIsoSequence_cff")
#process.load("CMS2.NtupleMaker.electronMaker_cfi")
#electronSequence = cms.Sequence(uniqueElectrons*egammaIsolationSequenceCMS2*egammaElectronIDCMS2)
#process.load("CMS2.NtupleMaker.electronSequence_cfi")
#process.load("CMS2.NtupleMaker.elToJetAssMaker_cfi")
#process.load("CMS2.NtupleMaker.elToMuAssMaker_cfi")
#process.load("CMS2.NtupleMaker.elToTrackAssMaker_cfi")
process.load("CMS2.NtupleMaker.eventMaker_cfi")
#gammaSequence = cms.Sequence(gamIsoDepositsCMS2 + gamIsoFromDepositsCMS2)
#process.load("CMS2.NtupleMaker.gammaSequence_cfi") 
process.load("CMS2.NtupleMaker.genJetMaker_cfi")
process.load("CMS2.NtupleMaker.genMaker_cfi")
#process.load("CMS2.NtupleMaker.hypDilepMaker_cfi")
#process.load("CMS2.NtupleMaker.hypTrilepMaker_cfi")
#process.load("CMS2.NtupleMaker.hypQuadlepMaker_cfi")
#process.load("CMS2.NtupleMaker.hypIsoMaker_cfi")

process.load("CMS2.NtupleMaker.jetSequence_cff")
process.load("CMS2.NtupleMaker.jetMaker_cfi")
#process.load("CMS2.NtupleMaker.jetToElAssMaker_cfi")
#process.load("CMS2.NtupleMaker.jetToMuAssMaker_cfi")
process.load("CMS2.NtupleMaker.jptSequence_cff")
#process.load("CMS2.NtupleMaker.l1DigiMaker_cfi")
process.load("CMS2.NtupleMaker.metSequence_cff")
process.load("CMS2.NtupleMaker.metMaker_cfi")
process.load("CMS2.NtupleMaker.muonMaker_cfi")
#process.load("CMS2.NtupleMaker.muToElsAssMaker_cfi")
#process.load("CMS2.NtupleMaker.muToJetAssMaker_cfi")
#process.load("CMS2.NtupleMaker.muToTrackAssMaker_cfi")
#process.load("CMS2.NtupleMaker.patElectronMaker_cfi")
#process.load("CMS2.NtupleMaker.patJetMaker_cfi")
#process.load("CMS2.NtupleMaker.patMETMaker_cfi")
#process.load("CMS2.NtupleMaker.patMuonMaker_cfi")
process.load("CMS2.NtupleMaker.pdfinfoMaker_cfi")
process.load("CMS2.NtupleMaker.pfmetMaker_cfi")
#process.load("CMS2.NtupleMaker.pftauMaker_cfi")
#process.load("CMS2.NtupleMaker.photonMaker_cfi")
#process.load("CMS2.NtupleMaker.scMaker_cfi")
process.load("CMS2.NtupleMaker.tcmetMaker_cfi")
process.load("CMS2.NtupleMaker.trackMaker_cfi")
#process.load("CMS2.NtupleMaker.trackToElsAssMaker_cfi")
process.load("CMS2.NtupleMaker.trackToMuonAssMaker_cfi")
#process.load("CMS2.NtupleMaker.triggerEventMaker_cfi")
#process.load("CMS2.NtupleMaker.trkJetMaker_cfi")
#process.load("CMS2.NtupleMaker.trkMuonFilter_cfi")
process.load("CMS2.NtupleMaker.vertexMaker_cfi")

###Dilepton Filter
#process.load("CMS2.NtupleMaker.theFilter_cfi")

#-----------------------------------------------------------
# configure input data files and number of event to process
#-----------------------------------------------------------

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(10)
)
process.options = cms.untracked.PSet(
    Rethrow = cms.untracked.vstring('ProductNotFound')
)

process.source = cms.Source("PoolSource",
    skipEvents = cms.untracked.uint32(0),
    fileNames = cms.untracked.vstring(
	'file:/home/fgolf/E01614FB-6C89-DE11-8089-003048C57816.root'
	)	
)

#-------------------------------------------------
# JEC
#-------------------------------------------------

#############   Define the L2 correction service #####
process.L2RelativeJetCorrector = cms.ESSource("L2RelativeCorrectionService", 
     tagName = cms.string('Summer08Redigi_L2Relative_SC5Calo'),
     label = cms.string('L2RelativeJetCorrector')
)

#############   Define the L3 correction service #####
process.L3AbsoluteJetCorrector = cms.ESSource("L3AbsoluteCorrectionService", 
     tagName = cms.string('Summer08Redigi_L3Absolute_SC5Calo'),
     label = cms.string('L3AbsoluteJetCorrector')
)

#############   Define another L2 correction service #####
process.L2RelativeJetCorrector2 = cms.ESSource("L2RelativeCorrectionService", 
     tagName = cms.string('Summer08Redigi_L2Relative_SC5Calo'),
     label = cms.string('L2RelativeJetCorrector2')
)

#############   Define another L3 correction service #####
process.L3AbsoluteJetCorrector2 = cms.ESSource("L3AbsoluteCorrectionService", 
     tagName = cms.string('Summer08Redigi_L3Absolute_SC5Calo'),
     label = cms.string('L3AbsoluteJetCorrector2')
)

#############   Define the EMF correction service ####
#process.L4EMFJetCorrector = cms.ESSource("L4EMFCorrectionService", 
#    tagName = cms.string('CMSSW_152_L4EMF'),
#    label = cms.string('L4EMFJetCorrector')
#)

#############   Define the chain corrector service - L2L3 ###
process.L2L3JetCorrector = cms.ESSource("JetCorrectionServiceChain",  
    correctors = cms.vstring('L2RelativeJetCorrector','L3AbsoluteJetCorrector'),
    label = cms.string('L2L3JetCorrector')
)

#############   Define the chain corrector service - L2L3L4 ###
process.L2L3L4JetCorrector = cms.ESSource("JetCorrectionServiceChain",  
#    correctors = cms.vstring('L2RelativeJetCorrector2','L3AbsoluteJetCorrector2','L4EMFJetCorrector'),
    correctors = cms.vstring('L2RelativeJetCorrector','L3AbsoluteJetCorrector'),
    label = cms.string('L2L3L4JetCorrector')
)

# set the record's IOV. Must be defined once. Choose ANY correction service. #
process.prefer("L2L3L4JetCorrector") 

#-------------------------------------------------
# PAT configuration
#-------------------------------------------------

## change jet collection# PAT Layer 0+1
#process.load("PhysicsTools.PatAlgos.patSequences_cff")
#process.patDefaultSequence = cms.Sequence(
#    process.beforeLayer1Objects *    # using '*', as the order is fixed.
#    process.allLayer1Objects *
#    process.selectedLayer1Objects
#)
#
#from PhysicsTools.PatAlgos.tools.jetTools import *
#switchJetCollection(process, 'prunedUncorrectedCMS2Jets', doJTA = True, doBTagging = True, jetCorrLabel = ('SC5', 'Calo'), doType1MET = True, genJetCollection = cms.InputTag("sisCone5GenJets") )

## necessary fixes to run 2.2.X on 2.1.X data
#from PhysicsTools.PatAlgos.tools.cmsswVersionTools import run22XonSummer08AODSIM
#run22XonSummer08AODSIM(process)


#-------------------------------------------------
# process output; first the event selection is
# defined: only those events that have passed the
# full production path are selected and written
# to file; the event content has been defined
# above
#-------------------------------------------------

## define event selection
process.EventSelection = cms.PSet(
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring('p')
    )
)


process.out_CMS2 = cms.OutputModule(
	"PoolOutputModule",
    process.EventSelection,
    verbose = cms.untracked.bool(True),
    dropMetaDataForDroppedData = cms.untracked.bool(True),
	fileName = cms.untracked.string('ntuple.root')
)

process.out_CMS2.outputCommands = cms.untracked.vstring( 'drop *' )
process.out_CMS2.outputCommands.extend(cms.untracked.vstring('keep *_*Maker_*_CMS2*'))
process.out_CMS2.outputCommands.extend(cms.untracked.vstring('keep *_*L2L3CorJetIC5JPT*_*_*'))
process.out_CMS2.outputCommands.extend(cms.untracked.vstring('keep *_*JetPlusTrack*_*_*'))

#-------------------------------------------------
# process paths;
#-------------------------------------------------

#process.CMS2Reco      = cms.Sequence(process.electronSequence * process.gammaSequence * process.cms2CaloJetSequence * process.CMS2Btagging * process.metCorSequence)
#process.CMS2Reco      = cms.Sequence(process.electronSequence * process.gammaSequence * process.cms2CaloJetSequence)
process.CMS2Reco = cms.Sequence(process.cms2CaloJetSequence * process.metCorSequence)

process.eventmakers   = cms.Sequence(process.beamSpotMaker * process.vertexMaker * process.eventMaker * process.pdfinfoMaker)

#process.trigmmakers   = cms.Sequence(process.l1DigiMaker * process.triggerEventMaker)

process.genmakers     = cms.Sequence(process.genMaker * process.genjetmaker)

#process.makers        = cms.Sequence(process.electronMaker * process.muonMaker * process.trackMaker * process.scMaker * process.jetMaker * process.JPTCorrections * process.trkmuonfilter * process.trkjetmaker * process.metMaker * process.tcmetMaker * process.calotauMaker * process.CMS2TrkBtagging * process.photonMaker)
#process.makers        = cms.Sequence(process.electronMaker * process.muonMaker * process.trackMaker * process.scMaker * process.jetMaker * process.JPTCorrections * process.trkmuonfilter * process.trkjetmaker * process.metMaker * process.tcmetMaker * process.calotauMaker * process.photonMaker)
process.makers = cms.Sequence(process.muonMaker * process.trackMaker * process.jetMaker * process.JPTCorrections * process.metMaker * process.tcmetMaker)

#process.assmakers     = cms.Sequence(process.jetToMuAssMaker * process.jetToElAssMaker * process.muToElsAssMaker * process.candToGenAssMaker * process.muToJetAssMaker * process.muToTrackAssMaker * process.elToTrackAssMaker * process.elToMuAssMaker * process.elToJetAssMaker * process.trackToMuonAssMaker * process.trackToElsAssMaker)
process.assmakers = cms.Sequence(process.trackToMuonAssMaker)

#process.hypmakers     = cms.Sequence(process.hypDilepMaker * process.hypTrilepMaker * process.hypQuadlepMaker * process.hypIsoMaker)

#process.othermakers   = cms.Sequence(process.elCaloIsoSequence * process.conversionMaker * process.bTagMaker * process.bTagTrkMaker)

#process.pflowmakers   = cms.Sequence(process.pfmetMaker * process.pftauMaker)
process.pflowmakers   = cms.Sequence(process.pfmetMaker)

#process.patmakers     = cms.Sequence(process.patMuonMaker * process.patElectronMaker * process.patJetMaker * process.patMETMaker)

#process.cms2          = cms.Sequence(process.eventmakers * process.trigmmakers * process.genmakers * process.makers * process.assmakers * process.othermakers * process.hypmakers)
#process.cms2          = cms.Sequence(process.eventmakers * process.trigmmakers * process.genmakers * process.makers * process.assmakers * process.hypmakers)
process.cms2 = cms.Sequence(process.eventmakers * process.genmakers * process.makers * process.assmakers)

#process.p             = cms.Path(process.CMS2Reco * process.cms2 * process.pflowmakers * process.patDefaultSequence * process.patmakers * process.theFilter)
#process.p             = cms.Path(process.cms2 * process.theFilter)
process.p = cms.Path(process.CMS2Reco * process.cms2 * process.pflowmakers)

process.outpath       = cms.EndPath(process.out_CMS2)
