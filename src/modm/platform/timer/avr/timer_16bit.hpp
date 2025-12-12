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

#ifndef MODM_AVR_TIMER_16BIT_HPP
#define MODM_AVR_TIMER_16BIT_HPP

#include <modm/architecture/interface/register.hpp>
#include <modm/math/utils/bit_constants.hpp>

#include "timer_base.hpp"

namespace modm::platform
{

struct Timer16Bit : Timer
{
	using CountType = uint16_t;
	static constexpr CountType max = 0xffff;

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
		PhaseCorrectPwm9Bit = 2,
		PhaseCorrectPwm10Bit = 3,
		CtcOcra = 4,
		FastPwm8Bit = 5,
		FastPwm9Bit = 6,
		FastPwm10Bit = 7,
		PhaseAndFrequencyCorrectPwmIcr = 8,
		PhaseAndFrequencyCorrectPwmOcra = 9,
		PhaseCorrectPwmIcr = 10,
		PhaseCorrectPwmOcra = 11,
		CtcIcr = 12,
		FastPwmIcr = 14,
		FastPwmOcra = 15,
	};

	static constexpr ClockCycles<SystemClock::Timer>
	clockSourcePeriod(ClockSource clock);

	static constexpr ClockSource
	getPrescaler(ClockCycles<SystemClock::Timer> minTickPeriod);

	template<bool dualSlope>
	static constexpr ClockSource
	selectPrescalerForMaxResolution(ClockCycles<SystemClock::Timer> period);

	template<bool dualSlope>
	static constexpr CountType
	computeTopValue(ClockSource prescaler, ClockCycles<SystemClock::Timer> period);

	template<bool dualSlope>
	static constexpr ClockCycles<SystemClock::Timer>
	computePeriod(ClockSource prescaler, CountType topValue);

protected:
	enum class TccrA : uint8_t
	{
	};
	MODM_FLAGS8(TccrA);

	enum class TccrB : uint8_t
	{
	};
	MODM_FLAGS8(TccrB);

	using WgmA_t = Configuration<TccrA_t, WaveformGenerationMode, Bit0 | Bit1>;
	using WgmB_t = Configuration<TccrB_t, WaveformGenerationMode, Bit2 | Bit3, 1>;

	using ClockSelect_t = Configuration<TccrB_t, ClockSource, 0b111>;
};

}  // namespace modm::platform

#include "timer_16bit_impl.hpp"

#endif  // MODM_AVR_TIMER_16BIT_HPP
