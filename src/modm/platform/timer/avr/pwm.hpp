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
#include <cstdint>
#include <modm/math/units.hpp>
#include <modm/math/utils/arithmetic_traits.hpp>
#include <modm/platform/clock/clock.hpp>
#include <utility>

#include "timer_base.hpp"

namespace modm::platform
{

template<class TimerInstance, Timer::PwmMode pwmMode, TimerInstance::ClockSource prescaler,
		 TimerInstance::CountType topValue>
class FixedTopPwm
{

	static consteval std::pair<typename TimerInstance::WaveformGenerationMode, bool>
	selectWgMode()
	{
		if constexpr (pwmMode == Timer::PwmMode::FastPwm)
		{
			if constexpr (topValue == 0xff &&
						  requires { TimerInstance::WaveformGenerationMode::FastPwm8Bit; })
				return {TimerInstance::WaveformGenerationMode::FastPwm8Bit, false};
			else if constexpr (requires { TimerInstance::WaveformGenerationMode::FastPwmOcra; })
				return {TimerInstance::WaveformGenerationMode::FastPwmOcra, true};
			else
				static_assert(false, "Fast PWM is unsupported by the chosen timer");
		} else if constexpr (pwmMode == Timer::PwmMode::PhaseCorrectPwm)
		{
			if constexpr (topValue == 0xff &&
						  requires { TimerInstance::WaveformGenerationMode::PhaseCorrectPwm8Bit; })
				return {TimerInstance::WaveformGenerationMode::PhaseCorrectPwm8Bit, false};
			else if constexpr (requires {
								   TimerInstance::WaveformGenerationMode::PhaseCorrectPwmOcra;
							   })
				return {TimerInstance::WaveformGenerationMode::PhaseCorrectPwmOcra, true};
			else
				static_assert(false, "Phase Correct PWM is unsupported by the chosen timer");
		} else if constexpr (pwmMode == Timer::PwmMode::PhaseAndFrequencyCorrectPwm)
		{
			if constexpr (topValue == 0xff && requires {
							  TimerInstance::WaveformGenerationMode::
								  PhaseAndFrequencyCorrectPwm8Bit;
						  })
				return {TimerInstance::WaveformGenerationMode::PhaseAndFrequencyCorrectPwm8Bit,
						false};
			else if constexpr (requires {
								   TimerInstance::WaveformGenerationMode::
									   PhaseAndFrequencyCorrectPwmOcra;
							   })
				return {TimerInstance::WaveformGenerationMode::PhaseAndFrequencyCorrectPwmOcra,
						true};
			else
				static_assert(false, "Phase Correct PWM is unsupported by the chosen timer");
		}
	}

	static constexpr TimerInstance::WaveformGenerationMode wgMode = selectWgMode().first;

	static constexpr bool ocraAsTop = selectWgMode().second;

public:
	static void
	initialize()
	{
		TimerInstance::initialize(wgMode, prescaler);

		if constexpr (ocraAsTop) { TimerInstance::OutputChannelA::compareRegister = topValue; }
	}

public:
	template<class OutputChannel>
	class Channel : private OutputChannel
	{
		static_assert(!(std::is_same_v<OutputChannel, typename TimerInstance::OutputChannelA> &&
						ocraAsTop),
					  "OCRA already used to define TOP");

	public:
		template<class Signal>
		static void
		connect(bool invert = false)
		{
			OutputChannel::template connect<Signal>(invert ? Timer::OutputMode::PwmInverted
														   : Timer::OutputMode::PwmNormal);
		}

		using OutputChannel::disconnect;

		static void
		setDutyCycle(percent_t dutyCycle)
		{
			OutputChannel::compareRegister =
				static_cast<TimerInstance::CountType>(dutyCycle * static_cast<float>(topValue));
		}

		static void
		setDutyCycle(TimerInstance::CountType dutyCycleFraction)
		{
			using WideCountType = modm::WideType<typename TimerInstance::CountType>;
			typename TimerInstance::CountType compareValue = static_cast<TimerInstance::CountType>(
				((WideCountType(topValue) + 1) * WideCountType(dutyCycleFraction)) >> 8);
			OutputChannel::compareRegister = compareValue;
		}
	};
};

template<class TimerInstance, Timer::PwmMode pwmMode, CtpDurationWrapper periodWrapped>
class FixedPeriodPwmHelper
{
	static constexpr bool dualSlope = Timer::isDualSlope(pwmMode);

	static constexpr ClockCycles<SystemClock::Timer> period =
		std::chrono::round<ClockCycles<SystemClock::Timer>>(periodWrapped.unwrap());

	static constexpr TimerInstance::ClockSource prescaler =
		TimerInstance::template selectPrescalerForMaxResolution<dualSlope>(period);

	static constexpr TimerInstance::CountType topValue =
		TimerInstance::template computeTopValue<dualSlope>(prescaler, period);

public:
	using Impl = FixedTopPwm<TimerInstance, pwmMode, prescaler, topValue>;
};

template<class TimerInstance, Timer::PwmMode pwmMode, CtpDurationWrapper period>
using FixedPeriodPwm = FixedPeriodPwmHelper<TimerInstance, pwmMode, period>::Impl;

template<class TimerInstance, Timer::PwmMode pwmMode, frequency_t frequency>
using FixedFrequencyPwm =
	FixedPeriodPwm<TimerInstance, pwmMode,
				   ClockCycles<SystemClock::Timer>{std::chrono::seconds{1}} / frequency>;

template<class TimerInstance, Timer::PwmMode pwmMode, class... UsedOutputChannels>
class VariableFrequencyPwm
{
	static consteval TimerInstance::WaveformGenerationMode
	selectWgMode()
	{
		if constexpr (pwmMode == Timer::PwmMode::FastPwm &&
					  requires { TimerInstance::WaveformGenerationMode::FastPwmOcra; })
		{
			return TimerInstance::WaveformGenerationMode::FastPwmOcra;
		} else if constexpr (pwmMode == Timer::PwmMode::PhaseCorrectPwm && requires {
								 TimerInstance::WaveformGenerationMode::PhaseCorrectPwmOcra;
							 })
		{
			return TimerInstance::WaveformGenerationMode::PhaseCorrectPwmOcra;
		} else if constexpr (pwmMode == Timer::PwmMode::PhaseAndFrequencyCorrectPwm && requires {
								 TimerInstance::WaveformGenerationMode::
									 PhaseAndFrequencyCorrectPwmOcra;
							 })
		{
			return TimerInstance::WaveformGenerationMode::PhaseAndFrequencyCorrectPwmOcra;
		} else
			static_assert(false, "The PWM mode is unsupported by the chosen timer");
	}

	static constexpr TimerInstance::WaveformGenerationMode wgMode = selectWgMode();

public:
	template<class OutputChannel>
	class Channel : private OutputChannel
	{
		static_assert(!(std::is_same_v<OutputChannel, typename TimerInstance::OutputChannelA>),
					  "OCRA already used to define TOP");

		inline static TimerInstance::CountType dutyCycleFraction = 0;

		static void
		updateTopValue(TimerInstance::CountType newTop)
		{
			using WideCountType = modm::WideType<typename TimerInstance::CountType>;
			typename TimerInstance::CountType compareValue = static_cast<TimerInstance::CountType>(
				((WideCountType(newTop) + 1) * WideCountType(dutyCycleFraction)) >> 8);
			OutputChannel::compareRegister = compareValue;
		}

		friend VariableFrequencyPwm;

	public:
		template<class Signal>
		static void
		connect(bool invert = false)
		{
			OutputChannel::template connect<Signal>(invert ? Timer::OutputMode::PwmInverted
														   : Timer::OutputMode::PwmNormal);
		}

		using OutputChannel::disconnect;

		static void
		setDutyCycle(percent_t dutyCycle)
		{
			setDutyCycle(static_cast<TimerInstance::CountType>(dutyCycle * 255.f));
		}

		static void
		setDutyCycle(TimerInstance::CountType dutyCycleFraction)
		{
			Channel::dutyCycleFraction = dutyCycleFraction;
			typename TimerInstance::CountType compareValue =
				TimerInstance::OutputChannelA::compareRegister;

			using WideCountType = modm::WideType<typename TimerInstance::CountType>;
			compareValue = static_cast<TimerInstance::CountType>(
				((WideCountType(compareValue) + 1) * WideCountType(dutyCycleFraction)) >> 8);
			OutputChannel::compareRegister = compareValue;
		}
	};

	static void
	initialize()
	{
		TimerInstance ::initialize(wgMode, TimerInstance::ClockSource::Stopped);
	}

	static void
	setPeriod(ClockCycles<SystemClock::Timer> period)
	{
		typename TimerInstance::ClockSource prescaler =
			TimerInstance::template selectPrescalerForMaxResolution<Timer::isDualSlope(pwmMode)>(
				period);
		typename TimerInstance::CountType topValue =
			TimerInstance::template computeTopValue<Timer::isDualSlope(pwmMode)>(prescaler, period);

		TimerInstance::setClockSource(prescaler);
		TimerInstance::OutputChannelA::compareRegister = topValue;

		(Channel<UsedOutputChannels>::updateTopValue(topValue), ...);
	}
};

}  // namespace modm::platform

#endif  // MODM_AVR_TIMER_PWM_HPP
