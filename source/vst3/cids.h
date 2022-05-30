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

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace mverb {
//------------------------------------------------------------------------
static const Steinberg::FUID kMVerbProcessorUID (0xB2D18CA4, 0x01105C1A, 0xB7F76B14, 0xFEE77D9C);
static const Steinberg::FUID kMVerbControllerUID (0x91ADCB3E, 0x3D20592D, 0x8D319592, 0xD0383414);

#define MVerbVST3Category "Fx"

//------------------------------------------------------------------------
} // namespace mverb
