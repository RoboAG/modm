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

#include "timer_base.hpp"

namespace modm::platform
{

template<class TimerInstance, Timer::PwmMode pwmMode, TimerInstance::ClockSource prescaler,
		 TimerInstance::CountType topValue>
class FixedTopPwm
{
	static constexpr TimerInstance::WaveformGenerationMode wgMode = [] {
		if constexpr (pwmMode == Timer::PwmMode::FastPwm)
		{
			return topValue == 0xff ? TimerInstance::WaveformGenerationMode::FastPwm8Bit
									: TimerInstance::WaveformGenerationMode::FastPwmOcra;
		} else if constexpr (pwmMode == Timer::PwmMode::PhaseCorrectPwm)
		{
			return topValue == 0xff ? TimerInstance::WaveformGenerationMode::PhaseCorrectPwm8Bit
									: TimerInstance::WaveformGenerationMode::PhaseCorrectPwmOcra;
		} else
		{
			static_assert(false, "unsupported PwmMode");
		}
	}();

	static constexpr bool ocraAsTop =
		wgMode == TimerInstance::WaveformGenerationMode::FastPwmOcra ||
		wgMode == TimerInstance::WaveformGenerationMode::PhaseCorrectPwmOcra;

public:
	static void
	initialize()
	{
		TimerInstance::initialize(wgMode, prescaler);

		if constexpr (ocraAsTop) { TimerInstance::OutputChannelA::setCompareRegister(topValue); }
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
			OutputChannel::setCompareRegister(
				static_cast<TimerInstance::CountType>(dutyCycle * static_cast<float>(topValue)));
		}

		static void
		setDutyCycle(uint8_t dutyCycleFraction)
		{
			uint8_t compareValue =
				uint8_t(((uint16_t(topValue) + 1) * uint16_t(dutyCycleFraction)) >> 8);
			OutputChannel::setCompareRegister(compareValue);
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
	static constexpr TimerInstance::WaveformGenerationMode wgMode = [] {
		if constexpr (pwmMode == Timer::PwmMode::FastPwm)
		{
			return TimerInstance::WaveformGenerationMode::FastPwmOcra;
		} else if constexpr (pwmMode == Timer::PwmMode::PhaseCorrectPwm)
		{
			return TimerInstance::WaveformGenerationMode::PhaseCorrectPwmOcra;
		} else
		{
			static_assert(false, "unsupported PwmMode");
		}
	}();

public:
	template<class OutputChannel>
	class Channel : private OutputChannel
	{
		static_assert(!(std::is_same_v<OutputChannel, typename TimerInstance::OutputChannelA>),
					  "OCRA already used to define TOP");

		inline static uint8_t dutyCycleFraction = 0;

		static void
		updateTopValue(uint8_t newTop)
		{
			uint8_t compareValue =
				uint8_t(((uint16_t(newTop) + 1) * uint16_t(dutyCycleFraction)) >> 8);
			OutputChannel::setCompareRegister(compareValue);
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
			setDutyCycle(uint8_t(dutyCycle * 255.f));
		}

		static void
		setDutyCycle(uint8_t dutyCycleFraction)
		{
			Channel::dutyCycleFraction = dutyCycleFraction;
			uint8_t compareValue =
				uint8_t(((uint16_t(OCR0A /* TODO implement generic read access */) + 1) *
						 uint16_t(dutyCycleFraction)) >>
						8);
			OutputChannel::setCompareRegister(compareValue);
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
		TimerInstance::OutputChannelA::setCompareRegister(topValue);

		(Channel<UsedOutputChannels>::updateTopValue(topValue), ...);
	}
};

}  // namespace modm::platform

#endif  // MODM_AVR_TIMER_PWM_HPP
