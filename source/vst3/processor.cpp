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
tresult PLUGIN_API Processor::process (Vst::ProcessData& data)
{
	auto numChanges = data.inputParameterChanges->getParameterCount ();
	for (auto index = 0; index < numChanges; ++index)
	{
		if (auto queue = data.inputParameterChanges->getParameterData (index))
			params[queue->getParameterId ()].beginChanges (queue);
	}

	bool doBypass = params.back ().flushChanges () > 0.5;
	if (doBypass)
	{
		if (data.symbolicSampleSize == Vst::SymbolicSampleSizes::kSample32)
		{
			for (auto channel = 0; channel < 2; ++channel)
			{
				if (data.inputs[0].channelBuffers32[channel] !=
				    data.outputs[0].channelBuffers32[channel])
					memcpy (data.outputs[0].channelBuffers32[channel],
					        data.inputs[0].channelBuffers32[channel],
					        data.numSamples * sizeof (float));
			}
			auto& mVerb = std::get<FloatMVerb> (verb);
			std::for_each (params.begin (), params.end (), [&] (auto& p) {
				p.flushChanges ([&] (auto value) { mVerb.setParameter (p.getParamID (), value); });
			});
		}
		else if (data.symbolicSampleSize == Vst::SymbolicSampleSizes::kSample64)
		{
			for (auto channel = 0; channel < 2; ++channel)
			{
				if (data.inputs[0].channelBuffers64[channel] !=
				    data.outputs[0].channelBuffers64[channel])
					memcpy (data.outputs[0].channelBuffers64[channel],
					        data.inputs[0].channelBuffers64[channel],
					        data.numSamples * sizeof (float));
			}
			auto& mVerb = std::get<DoubleMVerb> (verb);
			std::for_each (params.begin (), params.end (), [&] (auto& p) {
				p.flushChanges ([&] (auto value) { mVerb.setParameter (p.getParamID (), value); });
			});
		}
	}
	else
	{
		Vst::ProcessDataSlicer slicer (32);
		if (data.symbolicSampleSize == Vst::SymbolicSampleSizes::kSample32)
		{
			slicer.process<Vst::SymbolicSampleSizes::kSample32> (data, [this] (auto& data) {
				auto& mVerb = std::get<FloatMVerb> (verb);
				std::for_each (params.begin (), params.end (), [&] (auto& p) {
					p.advance (data.numSamples,
					           [&] (auto value) { mVerb.setParameter (p.getParamID (), value); });
				});
				mVerb.process (data.inputs[0].channelBuffers32, data.outputs[0].channelBuffers32,
				               data.numSamples);
			});
		}
		else if (data.symbolicSampleSize == Vst::SymbolicSampleSizes::kSample64)
		{
			slicer.process<Vst::SymbolicSampleSizes::kSample64> (data, [this] (auto& data) {
				auto& mVerb = std::get<DoubleMVerb> (verb);
				std::for_each (params.begin (), params.end (), [&] (auto& p) {
					p.advance (data.numSamples,
					           [&] (auto value) { mVerb.setParameter (p.getParamID (), value); });
				});
				mVerb.process (data.inputs[0].channelBuffers64, data.outputs[0].channelBuffers64,
				               data.numSamples);
			});
		}
	}

	std::for_each (params.begin (), params.end (), [] (auto& p) { p.endChanges (); });

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Processor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	if (newSetup.symbolicSampleSize == Vst::SymbolicSampleSizes::kSample32)
	{
		verb = FloatMVerb ();
		std::for_each (params.begin (), params.end (), [&] (auto& p) {
			std::get<FloatMVerb> (verb).setParameter (p.getParamID (), p.getValue ());
		});
		std::get<FloatMVerb> (verb).setSampleRate (newSetup.sampleRate);
	}
	else
	{
		verb = DoubleMVerb ();
		std::for_each (params.begin (), params.end (), [&] (auto& p) {
			std::get<DoubleMVerb> (verb).setParameter (p.getParamID (), p.getValue ());
		});
		std::get<DoubleMVerb> (verb).setSampleRate (newSetup.sampleRate);
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
	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer (state, kLittleEndian);
	
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Processor::getState (IBStream* state)
{
	// here we need to save the model
	IBStreamer streamer (state, kLittleEndian);

	return kResultOk;
}

//------------------------------------------------------------------------
} // namespace mverb
