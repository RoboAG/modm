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

#include <chrono>

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

#ifdef __DOXYGEN__
	// declarations for documentation purposes
	// might be converted to a concept TimerInstance
	using CountType = uint8_t;
	static constexpr CountType max = 0xff;

	using ExtendedCountType = uint32_t;

	enum class ClockSource;
	template<ClockSource, class SystemClock>
	struct ClockSourceTraits
	{
		using ClockPeriods =
			std::chrono::duration<ExtendedCountType, std::ratio<1, SystemClock::Timer>>;
	};

	enum class Prescaler;
	template<Prescaler>
	struct PrescalerTraits
	{
		using Ratio = std::ratio<1>;
	};

	enum class WaveformGenerationMode;
	template<WaveformGenerationMode>
	struct WaveformGenerationModeTraits
	{
		// either a constant value or a reference to the register used for dynamic top values
		static constexpr CountType top = max;

		// if applicable
		static constexpr PwmMode pwmMode = PwmMode::FastPwm;

		static constexpr ExtendedCountType
		countsPerPeriod(CountType topValue);
		static constexpr CountType
		computeTopValue(ExtendedCountType countsPerPeriod);
	};

	// inherited by WaveformGenerationModeTraits specializations
	struct SingleSlopeModeTraits;
	struct DoubleSlopeModeTraits;

	static void initialize(WaveformGenerationMode, ClockSource);
	static void setClockSource(ClockSource);
	static void setClockSource(Prescaler);
	static void
	stop();

	class OutputChannel
	{
	public:
		template<class Signal>
		static void connect(OutputMode);

		template<class Signal>
		static void
		disconnect();

		static void setOutputCompareRegister(CountType);
	};
#endif  // __DOXYGEN__
};

}  // namespace modm::platform

#endif  // MODM_AVR_TIMER_BASE_HPP
