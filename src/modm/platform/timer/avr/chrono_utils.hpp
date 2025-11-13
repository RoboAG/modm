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

#ifndef MODM_AVR_TIMER_CHRONO_UTILS_HPP
#define MODM_AVR_TIMER_CHRONO_UTILS_HPP

#include <chrono>
#include <modm/math/units.hpp>

namespace modm
{

template<frequency_t frequency>
using ClockCycles = std::chrono::duration<int32_t, std::ratio<1, frequency>>;

template<class Duration>
struct CtpDurationWrapper
{
	using DurationType = Duration;

	DurationType::rep count;

	constexpr CtpDurationWrapper(DurationType duration) : count{duration.count()} {}

	constexpr DurationType
	unwrap() const
	{
		return DurationType{count};
	}

	constexpr
	operator DurationType() const
	{
		return unwrap();
	}
};

}  // namespace modm

#endif  // MODM_AVR_TIMER_CHRONO_UTILS_HPP
