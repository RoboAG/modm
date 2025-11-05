/*
 * Copyright (c) 2025, Janek Neugebauer
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#ifndef MODM_AVR_TIMER_8BIT_HPP
#define MODM_AVR_TIMER_8BIT_HPP

#include "timer_base.hpp"

namespace modm::platform
{

struct Timer8Bit : Timer
{
	using CountType = uint8_t;
	static constexpr CountType max = 0xff;

	using ExtendedCountType = uint32_t;

	enum class ClockSource : uint8_t
	{
		Stopped = 0,
		ClkIo = 1,
		ClkIoDiv8 = 2,
		ClkIoDiv64 = 3,
		ClkIoDiv256 = 4,
		ClkIoDiv1024 = 5,
		ExternalFalling = 6,
		ExternalRising = 7,
	};

	enum class WaveformGenerationMode : uint8_t
	{
		Normal = 0,
		PhaseCorrectPwm8Bit = 1,
		Ctc = 2,
		FastPwm8Bit = 3,
		PhaseCorrectPwmOcra = 5,
		FastPwmOcra = 7,
	};
};

}  // namespace modm::platform

#include "timer_8bit_impl.hpp"

#endif  // MODM_AVR_TIMER_8BIT_HPP
