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

#ifndef MODM_AVR_TIMER_BASE_HPP
#define MODM_AVR_TIMER_BASE_HPP

#include <stdint.h>

#include <modm/platform/clock/clock.hpp>

#include "chrono_utils.hpp"

namespace modm::platform
{

struct Timer
{
	enum class OutputMode : uint8_t
	{
		Disconnected = 0,
		Toggle = 1,
		SetLow = 2,
		SetHigh = 3,
		PwmNormal = 2,
		PwmInverted = 3,
	};

	enum class PwmMode : uint8_t
	{
		FastPwm,
		PhaseCorrectPwm,
		PhaseAndFrequencyCorrectPwm,
	};

	template<bool dualSlope>
	static constexpr auto
	topToCounts(auto topValue)
	{
		if constexpr (dualSlope)
		{
			return topValue * 2;
		} else
		{
			return topValue + 1;
		}
	}

	template<bool dualSlope>
	static constexpr auto
	countsToTop(auto countsPerPeriod)
	{
		if constexpr (dualSlope)
		{
			return countsPerPeriod / 2;
		} else
		{
			return countsPerPeriod - 1;
		}
	}

	static constexpr bool
	isDualSlope(PwmMode pwmMode)
	{
		return pwmMode == PwmMode::PhaseCorrectPwm ||
			   pwmMode == PwmMode::PhaseAndFrequencyCorrectPwm;
	}
};

}  // namespace modm::platform

#endif  // MODM_AVR_TIMER_BASE_HPP
