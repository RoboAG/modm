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

#include <chrono>
#include <modm/math/units.hpp>
#include <utility>

#include "timer_base.hpp"

namespace modm::platform
{

template<class TimerInstance, Timer::PwmMode pwmMode, TimerInstance::ClockSource prescaler,
		 TimerInstance::CountType topValue>
class FixedTopPwm
{

	static consteval std::pair<typename TimerInstance::WaveformGenerationMode, bool>
	selectWgMode();

	static constexpr TimerInstance::WaveformGenerationMode wgMode = selectWgMode().first;

	static constexpr bool ocraAsTop = selectWgMode().second;

public:
	static void
	initialize();

	template<class OutputChannel>
	class Channel : private OutputChannel
	{
		static_assert(!(std::is_same_v<OutputChannel, typename TimerInstance::OutputChannelA> &&
						ocraAsTop),
					  "OCRA already used to define TOP");

	public:
		template<class Signal>
		static void
		connect(bool invert = false);

		using OutputChannel::disconnect;

		static void
		setDutyCycle(percent_t dutyCycle);

		static void
		setDutyCycle(TimerInstance::CountType dutyCycleFraction);
	};
};

namespace detail
{
template<class TimerInstance, Timer::PwmMode pwmMode, CtpDurationWrapper periodWrapped>
class FixedPeriodPwmHelper;
}

template<class TimerInstance, Timer::PwmMode pwmMode, CtpDurationWrapper period>
using FixedPeriodPwm = detail::FixedPeriodPwmHelper<TimerInstance, pwmMode, period>::Impl;

template<class TimerInstance, Timer::PwmMode pwmMode, frequency_t frequency>
using FixedFrequencyPwm =
	FixedPeriodPwm<TimerInstance, pwmMode,
				   ClockCycles<SystemClock::Timer>{std::chrono::seconds{1}} / frequency>;

template<class TimerInstance, Timer::PwmMode pwmMode, class... UsedOutputChannels>
class VariableFrequencyPwm
{
	static consteval TimerInstance::WaveformGenerationMode
	selectWgMode();

	static constexpr TimerInstance::WaveformGenerationMode wgMode = selectWgMode();

public:
	static void
	initialize();

	static void
	setPeriod(ClockCycles<SystemClock::Timer> period);

	template<class OutputChannel>
	class Channel : private OutputChannel
	{
		static_assert(!(std::is_same_v<OutputChannel, typename TimerInstance::OutputChannelA>),
					  "OCRA already used to define TOP");

		inline static TimerInstance::CountType dutyCycleFraction = 0;

		static void
		updateTopValue(TimerInstance::CountType newTop);

		friend VariableFrequencyPwm;

	public:
		template<class Signal>
		static void
		connect(bool invert = false);

		using OutputChannel::disconnect;

		static void
		setDutyCycle(percent_t dutyCycle);

		static void
		setDutyCycle(TimerInstance::CountType dutyCycleFraction);
	};
};

}  // namespace modm::platform

#include "pwm_impl.hpp"

#endif  // MODM_AVR_TIMER_PWM_HPP
