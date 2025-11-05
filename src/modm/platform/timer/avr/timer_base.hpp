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

	enum class Prescaler;

	enum class WaveformGenerationMode;

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
