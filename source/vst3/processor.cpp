//	Copyright (c) 2022 Arne Scheffler
//  This code is distributed under the terms of the GNU General Public License

//  MVerb is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  at your option) any later version.
//
//  MVerb is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this MVerb.  If not, see <http://www.gnu.org/licenses/>.

#include "processor.h"
#include "cids.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/utility/processdataslicer.h"
#include "public.sdk/source/vst/utility/audiobuffers.h"
#include "public.sdk/source/vst/utility/vst2persistence.h"

#include <algorithm>

using namespace Steinberg;

namespace mverb {

//------------------------------------------------------------------------
// mverbProcessor
//------------------------------------------------------------------------
Processor::Processor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (kMVerbControllerUID);

	for (auto index = 0u; index < params.size (); ++index)
		params[index].setParamID (index);

	// some usefull defaults
	params[FloatMVerb::DAMPINGFREQ].setValue (0.);
	params[FloatMVerb::DENSITY].setValue (0.5);
	params[FloatMVerb::BANDWIDTHFREQ].setValue (1.);
	params[FloatMVerb::DECAY].setValue (0.5);
	params[FloatMVerb::PREDELAY].setValue (0.);
	params[FloatMVerb::SIZE].setValue (0.5);
	params[FloatMVerb::GAIN].setValue (1.);
	params[FloatMVerb::MIX].setValue (0.15);
	params[FloatMVerb::EARLYMIX].setValue (0.75);
}

//------------------------------------------------------------------------
Processor::~Processor ()
{}

//------------------------------------------------------------------------
tresult PLUGIN_API Processor::initialize (FUnknown* context)
{
	// Here the Plug-in will be instanciated
	
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk)
	{
		return result;
	}

	//--- create Audio IO ------
	addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Processor::terminate ()
{
	// Here the Plug-in will be de-instanciated, last possibility to remove some memory!
	
	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API Processor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
template<typename T, Vst::SymbolicSampleSizes SampleSize>
void Processor::processT (Vst::ProcessData& data)
{
	bool doBypass = params.back ().flushChanges () > 0.5;
	if (doBypass)
	{
		if (data.numSamples > 0)
		{
			for (auto channel = 0; channel < 2; ++channel)
			{
				if (Vst::getChannelBuffers<SampleSize> (data.inputs[0])[channel] !=
				    Vst::getChannelBuffers<SampleSize> (data.outputs[0])[channel])
					memcpy (Vst::getChannelBuffers<SampleSize> (data.outputs[0])[channel],
							Vst::getChannelBuffers<SampleSize> (data.inputs[0])[channel],
							data.numSamples * sizeof (float));
			}
		}
		auto& mVerb = std::get<std::unique_ptr<T>> (verb);
		std::for_each (params.begin (), params.end (), [&] (auto& p) {
			p.flushChanges ([&] (auto value) { mVerb->setParameter (p.getParamID (), value); });
		});
	}
	else
	{
		Vst::ProcessDataSlicer slicer (32);
		slicer.process<Vst::SymbolicSampleSizes::kSample32> (data, [this] (auto& data) {
			auto& mVerb = std::get<std::unique_ptr<T>> (verb);
			std::for_each (params.begin (), params.end (), [&] (auto& p) {
				p.advance (data.numSamples,
				           [&] (auto value) { mVerb->setParameter (p.getParamID (), value); });
			});
			mVerb->process (Vst::getChannelBuffers<SampleSize> (data.inputs[0]),
			               Vst::getChannelBuffers<SampleSize> (data.outputs[0]), data.numSamples);
		});
	}
}

//------------------------------------------------------------------------
tresult PLUGIN_API Processor::process (Vst::ProcessData& data)
{
	stateTransfer.accessTransferObject_rt ([&] (const StateData& data) {
		for (auto index = 0; index < data.size (); ++index)
			params[index].setValue (data[index]);
	});
	if (data.inputParameterChanges)
	{
		auto numChanges = data.inputParameterChanges->getParameterCount ();
		for (auto index = 0; index < numChanges; ++index)
		{
			if (auto queue = data.inputParameterChanges->getParameterData (index))
				params[queue->getParameterId ()].beginChanges (queue);
		}
	}

	if (data.numSamples == 0)
	{
		std::for_each (params.begin (), params.end (), [] (auto& p) { p.flushChanges (); });
	}
	else if (data.symbolicSampleSize == Vst::SymbolicSampleSizes::kSample32)
		processT<FloatMVerb, Vst::SymbolicSampleSizes::kSample32> (data);
	else
		processT<DoubleMVerb, Vst::SymbolicSampleSizes::kSample64> (data);

	std::for_each (params.begin (), params.end (), [] (auto& p) { p.endChanges (); });

	return kResultOk;
}

//------------------------------------------------------------------------
template<typename T>
void Processor::setupProcessingT (Steinberg::Vst::ProcessSetup& newSetup)
{
	verb = std::make_unique<T> ();
	std::for_each (params.begin (), params.end (), [&] (auto& p) {
		std::get<std::unique_ptr<T>> (verb)->setParameter (p.getParamID (), p.getValue ());
	});
	std::get<std::unique_ptr<T>> (verb)->setSampleRate (newSetup.sampleRate);
}

//------------------------------------------------------------------------
tresult PLUGIN_API Processor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	if (newSetup.symbolicSampleSize == Vst::SymbolicSampleSizes::kSample32)
	{
		setupProcessingT<FloatMVerb> (newSetup);
	}
	else
	{
		setupProcessingT<DoubleMVerb> (newSetup);
	}

	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API Processor::canProcessSampleSize (int32 symbolicSampleSize)
{
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;
	if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue;

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Processor::setBusArrangements (Vst::SpeakerArrangement* inputs,
                                                       int32 numIns,
                                                       Vst::SpeakerArrangement* outputs,
                                                       int32 numOuts)
{
	if (numIns != numOuts || numIns != 1 || inputs[0] != Vst::SpeakerArr::kStereo ||
	    outputs[0] != Vst::SpeakerArr::kStereo)
		return kResultFalse;
	return AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
}

//------------------------------------------------------------------------
tresult PLUGIN_API Processor::setState (IBStream* state)
{
	if (!state)
		return kInvalidArgument;

	if (auto stateData = VST3::tryVst2StateLoad (*state, {'emVB'}))
	{
		if (stateData->programs.empty ())
			return kResultFalse;
		auto data = std::make_unique<StateData> ();
		if (stateData->programs[0].values.size() != data->size())
			return kResultFalse;
		for (auto idx = 0; idx < data->size (); ++idx)
			data->at (idx) = stateData->programs[0].values[idx];
		stateTransfer.transferObject_ui (std::move (data));
		return kResultTrue;
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Processor::getState (IBStream* state)
{
	if (!state)
		return kInvalidArgument;

	VST3::Vst2xState data;
	data.programs.resize (1);
	for (auto idx = 0; idx <= FloatMVerb::NUM_PARAMS; ++idx)
		data.programs[0].values.push_back (params[idx].getValue ());
	data.programs[0].fxUniqueID = 'emVB';
	data.fxUniqueID = 'emVB';
	data.isBypassed = params.back ().getValue () > 0.5;
	if (VST3::writeVst2State (data, *state))
		return kResultTrue;
	return kResultFalse;
}

//------------------------------------------------------------------------
} // namespace mverb
