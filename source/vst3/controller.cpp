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

#include "controller.h"
#include "cids.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "../MVerb.h"

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

	using Verb = MVerb<float>;

	parameters.addParameter (new Vst::RangeParameter (STR ("Damping"), Verb::DAMPINGFREQ, STR ("%"), 0., 100., 0.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Density"), Verb::DENSITY, STR ("%"), 0., 100., 50.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Bandwidth"), Verb::BANDWIDTHFREQ, STR ("%"), 0., 100., 100.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Predelay"), Verb::PREDELAY, STR ("%"), 0., 100., 0.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Size"), Verb::SIZE, STR ("%"), 0., 100., 0.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Decay"), Verb::DECAY, STR ("%"), 0., 100., 50.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Gain"), Verb::GAIN, STR ("%"), 0., 100., 100.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Mix"), Verb::MIX, STR ("%"), 0., 100., 15.))->setPrecision (0);
	parameters.addParameter (new Vst::RangeParameter (STR ("Early/Late Mix"), Verb::EARLYMIX, STR ("%"), 0., 100., 75.))->setPrecision (0);

	parameters.addParameter (STR ("Bypass"), nullptr, 1, 0.,
	                         Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass,
	                         Verb::NUM_PARAMS);

	
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
	// Here you get the state of the component (Processor part)
	if (!state)
		return kResultFalse;

	return kResultOk;
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
tresult PLUGIN_API Controller::setParamNormalized (Vst::ParamID tag, Vst::ParamValue value)
{
	// called by host to update your parameters
	tresult result = EditControllerEx1::setParamNormalized (tag, value);
	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Controller::getParamStringByValue (Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
{
	// called by host to get a string for given normalized value of a specific parameter
	// (without having to set the value!)
	return EditControllerEx1::getParamStringByValue (tag, valueNormalized, string);
}

//------------------------------------------------------------------------
tresult PLUGIN_API Controller::getParamValueByString (Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
{
	// called by host to get a normalized value from a string representation of a specific parameter
	// (without having to set the value!)
	return EditControllerEx1::getParamValueByString (tag, string, valueNormalized);
}

//------------------------------------------------------------------------
} // namespace mverb
