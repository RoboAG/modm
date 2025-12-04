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

#ifdef __DOXYGEN__
	// declarations for documentation purposes
	// might be converted to a concept TimerInstance
	using CountType = uint8_t;
	static constexpr CountType max = 0xff;

	using ExtendedCountType = uint32_t;

	enum class ClockSource;

	enum class WaveformGenerationMode;

	static void initialize(WaveformGenerationMode, ClockSource);
	static void setClockSource(ClockSource);
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

protected:
	template<PwmMode pwmMode, typename WideCountType, typename CountType>
	static constexpr WideCountType
	topToCounts(CountType topValue)
	{
		if constexpr (pwmMode == PwmMode::FastPwm)
		{
			return WideCountType(topValue) + 1;
		} else if constexpr (pwmMode == PwmMode::PhaseCorrectPwm ||
							 pwmMode == PwmMode::PhaseAndFrequencyCorrectPwm)
		{
			return WideCountType(topValue) * 2;
		} else
		{
			static_assert(false, "unknown PwmMode");
		}
	}

	template<PwmMode pwmMode, typename CountType, typename WideCountType>
	static constexpr CountType
	countsToTop(WideCountType countsPerPeriod)
	{
		if constexpr (pwmMode == PwmMode::FastPwm)
		{
			return CountType(countsPerPeriod - 1);
		} else if constexpr (pwmMode == PwmMode::PhaseCorrectPwm ||
							 pwmMode == PwmMode::PhaseAndFrequencyCorrectPwm)
		{
			return CountType(countsPerPeriod / 2);
		} else
		{
			static_assert(false, "unknown PwmMode");
		}
	}

	template<class Timer, PwmMode pwmMode, CtpDurationWrapper periodWrapped>
	class FixedPeriodPwmHelper
	{
		static constexpr ClockCycles<SystemClock::Timer> period =
			std::chrono::round<ClockCycles<SystemClock::Timer>>(periodWrapped.unwrap());

		static constexpr ClockCycles<SystemClock::Timer>::rep maxCountsPerPeriod =
			topToCounts<pwmMode, ClockCycles<SystemClock::Timer>::rep>(Timer::max);

		static constexpr Timer::ClockSource prescaler =
			Timer::selectPrescaler(period / maxCountsPerPeriod);

		static constexpr ClockCycles<SystemClock::Timer> clockSourcePeriod =
			Timer::clockSourcePeriod(prescaler);

		static_assert(clockSourcePeriod.count() > 0,
					  "requested frequency too low / period too long");

		static constexpr ClockCycles<SystemClock::Timer>::rep countsPerPeriod =
			period / clockSourcePeriod;

		static constexpr Timer::CountType topValue =
			countsToTop<pwmMode, typename Timer::CountType>(countsPerPeriod);

	public:
		using Impl = Timer::template FixedTopPwm<pwmMode, prescaler, topValue>;
	};
};

}  // namespace modm::platform

#endif  // MODM_AVR_TIMER_BASE_HPP
