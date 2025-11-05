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

#ifndef MODM_AVR_TIMER_PWM_HPP
#define MODM_AVR_TIMER_PWM_HPP

#include <modm/math/units.hpp>

#include "timer_8bit.hpp"
#include "timer_base.hpp"

namespace modm::platform
{

template<class TimerInstance, Timer::PwmMode pwmMode, TimerInstance::ClockSource prescaler,
		 TimerInstance::CountType topValue>
class FixedFrequencyPwm
{

	template<TimerInstance::WaveformGenerationMode firstMode,
			 TimerInstance::WaveformGenerationMode... remainingModes>
	class WgmSelector
	{
	public:
		static constexpr std::optional<typename TimerInstance::WaveformGenerationMode>
		selectMode();
	};

public:
	static constexpr TimerInstance::WaveformGenerationMode selectedWaveformGenerationMode =
		TimerInstance::template UseWaveformGenerationModes<WgmSelector>::selectMode().value();
	static constexpr TimerInstance::ClockSource selectedPrescaler = prescaler;
	static constexpr TimerInstance::CountType selectedTopValue = topValue;

	static void
	initialize();

	template<class OutputChannel>
	class Output
	{
		// Doesn't work for modes with fixed topValue where topRegister does not exist.
		// static_assert(&OutputChannel::outputCompareRegister !=
		// 			  &TimerInstance::template WaveformGenerationModeTraits<
		// 				  selectedWaveformGenerationMode>::topRegister);

	public:
		template<class Signal>
		static void
		connect(bool invert = false);

		static void
		setCompareValue(TimerInstance::CountType compareValue);
		static void
		setDutyCycle(percent_t dutyCycle);
	};
};

}  // namespace modm::platform

#include "pwm_impl.hpp"

#endif  // MODM_AVR_TIMER_PWM_HPP
