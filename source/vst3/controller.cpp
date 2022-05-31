//  Copyright (c) 2022 Arne Scheffler
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

#include "controller.h"
#include "cids.h"
#include "shared.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "public.sdk/source/vst/utility/vst2persistence.h"

using namespace Steinberg;

namespace mverb {

//------------------------------------------------------------------------
// mverbController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API Controller::initialize (FUnknown* context)
{
	// Here the Plug-in will be instanciated

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize (context);
	if (result != kResultOk)
	{
		return result;
	}

	parameters.addParameter (new Vst::RangeParameter (STR ("Damping"), FloatMVerb::DAMPINGFREQ, STR ("%"), 0., 100., 0.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Density"), FloatMVerb::DENSITY, STR ("%"), 0., 100., 50.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Bandwidth"), FloatMVerb::BANDWIDTHFREQ, STR ("%"), 0., 100., 100.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Predelay"), FloatMVerb::PREDELAY, STR ("%"), 0., 100., 0.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Size"), FloatMVerb::SIZE, STR ("%"), 0., 100., 0.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Decay"), FloatMVerb::DECAY, STR ("%"), 0., 100., 50.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Gain"), FloatMVerb::GAIN, STR ("%"), 0., 100., 100.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Mix"), FloatMVerb::MIX, STR ("%"), 0., 100., 15.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Early/Late Mix"), FloatMVerb::EARLYMIX, STR ("%"), 0., 100., 75.))->setPrecision (0);

	parameters.addParameter (STR ("Bypass"), nullptr, 1, 0.,
	                         Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass,
	                         FloatMVerb::NUM_PARAMS);

	
	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Controller::terminate ()
{
	// Here the Plug-in will be de-instanciated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API Controller::setComponentState (IBStream* state)
{
	if (!state)
		return kInvalidArgument;

	if (auto stateData = VST3::tryVst2StateLoad (*state, {'emVB'}))
	{
		if (stateData->programs.empty ())
			return kResultFalse;
		for (auto idx = 0; idx < stateData->programs[0].values.size (); ++idx)
		{
			if (auto param = parameters.getParameter (idx))
			{
				param->setNormalized (stateData->programs[0].values[idx]);
			}
		}
		return kResultTrue;
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Controller::setState (IBStream* state)
{
	// Here you get the state of the controller

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Controller::getState (IBStream* state)
{
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor

	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API Controller::createView (FIDString name)
{
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
		// create your editor here and return a IPlugView ptr of it
		auto* view = new VSTGUI::VST3Editor (this, "view", "editor.uidesc");
		return view;
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // namespace mverb
