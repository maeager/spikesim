// TypeDefs.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TYPEDEFS_H
#define TYPEDEFS_H


#include "Macros.h"
#include "CreatedTypeMaps.h"

#include "ConfigBase.h"
#include "InterfaceBase.h"
#include "NeuronTemplate.h"
#include "SynapseTemplate.h"

#include "DataCommonNeuron.h"
#include "DataRecordNeuron.h"
#include "DataPlastNeuron.h"
#include "IFMech.h"
#include "InstantIFMech.h"
#include "PoissonMech.h"
#include "PoissonParameters.h"
#include "CorrInputMech.h"
#include "DeltaCorrInputRefs.h"
#include "ShortTimeCorrInputRefs.h"
#include "OscillatoryCorrInputRefs.h"

#include "DataIndivSynapse.h"
#include "DataLightSynapse.h"
#include "DataPlastSynapse.h"
#include "InterfaceBase.h"
#include "DoubleExpSynMech.h"
#include "CondSynMech.h"
#include "InstantSynMech.h"
#include "NoPlastMech.h"
#include "STDPMech.h"
#include "STDPFunction.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// NEURONS
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// Correlated Poisson Pulse Train Generators
NEURON_TYPE(CorrInputPlastNrn,      DataPlastNeuron,    CorrInputMech<DeltaCorrInputRef>)
NEURON_TYPE(CorrInputReccordNrn,    DataRecordNeuron,   CorrInputMech<DeltaCorrInputRef>)
NEURON_TYPE(CorrInputBasicNrn,      DataCommonNeuron,   CorrInputMech<DeltaCorrInputRef>)

NEURON_TYPE(CorrInput2PlastNrn,     DataPlastNeuron,    CorrInputMech<CorrInputRefShiftedCopy>)
NEURON_TYPE(CorrInput2ReccordNrn,   DataRecordNeuron,   CorrInputMech<CorrInputRefShiftedCopy>)
NEURON_TYPE(CorrInput2BasicNrn,     DataCommonNeuron,   CorrInputMech<CorrInputRefShiftedCopy>)

NEURON_TYPE(CorrInput3PlastNrn, DataPlastNeuron, CorrInputMech<ShortTimeCorrInputRef>)
NEURON_TYPE(CorrInput3ReccordNrn, DataRecordNeuron, CorrInputMech<ShortTimeCorrInputRef>)
NEURON_TYPE(CorrInput3BasicNrn, DataCommonNeuron, CorrInputMech<ShortTimeCorrInputRef>)

NEURON_TYPE(CorrInput4PlastNrn, DataPlastNeuron, CorrInputMech<OscillatoryDeltaCorrInputRef>)
NEURON_TYPE(CorrInput4ReccordNrn, DataRecordNeuron, CorrInputMech<OscillatoryDeltaCorrInputRef>)
NEURON_TYPE(CorrInput4BasicNrn, DataCommonNeuron, CorrInputMech<OscillatoryDeltaCorrInputRef>)

/////////////////////////////////////////////////
// Poisson Neurons (for palstic and non plastic synapses)
typedef PoissonMech<ConstantPoissonParameter, FuncIdentity> ConstLinPoissonMech;
NEURON_TYPE(PoissonPlastNrnLin,     DataPlastNeuron,    ConstLinPoissonMech)
NEURON_TYPE(PoissonRecNrnLin,       DataRecordNeuron,   ConstLinPoissonMech)
NEURON_TYPE(PoissonBasicNrnLin,     DataCommonNeuron,   ConstLinPoissonMech)

typedef PoissonMech<ConstantPoissonParameter, FuncSigmoid> ConstSigPoissonMech;
NEURON_TYPE(PoissonPlastNrnSig,     DataPlastNeuron,    ConstSigPoissonMech)
NEURON_TYPE(PoissonRecNrnSig,       DataRecordNeuron,   ConstSigPoissonMech)
NEURON_TYPE(PoissonBasicNrnSig,     DataCommonNeuron,   ConstSigPoissonMech)

typedef PoissonMech<OscillatoryPoissonParameter, FuncIdentity> OscilLinPoissonMech;
NEURON_TYPE(PoissonPlastNrnLinOsc,  DataPlastNeuron,    OscilLinPoissonMech)
NEURON_TYPE(PoissonRecNrnLinOsc,    DataRecordNeuron,   OscilLinPoissonMech)
NEURON_TYPE(PoissonBasicNrnLinOsc,  DataCommonNeuron,   OscilLinPoissonMech)

typedef PoissonMech<OscillatoryPoissonParameter, FuncSigmoid> OscilSigPoissonMech;
NEURON_TYPE(PoissonPlastNrnSigOsc,  DataPlastNeuron,    OscilSigPoissonMech)
NEURON_TYPE(PoissonRecNrnSigOsc,    DataRecordNeuron,   OscilSigPoissonMech)
NEURON_TYPE(PoissonBasicNrnSigOsc,  DataCommonNeuron,   OscilSigPoissonMech)

/////////////////////////////////////////////////
// Integrate-&-Fire Neurons (for palstic and non plastic synapses)
NEURON_TYPE(IFPlastNrn, DataPlastNeuron, IFMech)
NEURON_TYPE(IFRecordNrn, DataRecordNeuron, IFMech)
NEURON_TYPE(IFBasicNrn, DataCommonNeuron, IFMech)

NEURON_TYPE(InstantIFPlastNrn, DataPlastNeuron, InstantIFMech)
NEURON_TYPE(InstantIFRecordNrn, DataRecordNeuron, InstantIFMech)
NEURON_TYPE(InstantIFBasicNrn, DataCommonNeuron, InstantIFMech)





///////////////////////////////////////////////////////////////////////////////////////////////////
// SYNAPSES
///////////////////////////////////////////////////////////////////////////////////////////////////
typedef STDPMech<AdditiveSTDPNegExpFunction, ClipToBounds> AdditiveSTDP;

/////////////////////////////////////////////////
// Kernel Synapses (with STDP, with individual fixed weight and with shared fixed weight)
SYNAPSE_TYPE(AlphaSTDPSyn, DataPlastSynapse, DoubleExpSynMech, AdditiveSTDP)
SYNAPSE_TYPE(AlphaNonPlastSyn, DataIndivSynapse, DoubleExpSynMech, NoPlastMech)
SYNAPSE_TYPE(AlphaLightSyn, DataLightSynapse, DoubleExpSynMech, NoPlastMech)

/////////////////////////////////////////////////
// Conductance Synapses (with STDP, with individual fixed weight and with shared fixed weight)
SYNAPSE_TYPE(CondSTDPSyn, DataPlastSynapse, CondSynMech, AdditiveSTDP)
SYNAPSE_TYPE(CondNonPlastSyn, DataIndivSynapse, CondSynMech, NoPlastMech)
SYNAPSE_TYPE(CondLightSyn, DataLightSynapse, CondSynMech, NoPlastMech)

/////////////////////////////////////////////////
// Instantaneous Synapses (with STDP, with individual fixed weight and with shared fixed weight)
SYNAPSE_TYPE(InstantSTDPSyn, DataPlastSynapse, InstantSynMech, AdditiveSTDP)
SYNAPSE_TYPE(InstantNonPlastSyn, DataIndivSynapse, InstantSynMech, NoPlastMech)
SYNAPSE_TYPE(InstantLightSyn, DataLightSynapse, InstantSynMech, NoPlastMech)



#endif // !defined(TYPEDEFS_H)
