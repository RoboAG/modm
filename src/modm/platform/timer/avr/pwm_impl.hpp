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

#include <modm/math/units.hpp>

namespace modm::platform
{

template<class TimerInstance, Timer::PwmMode pwmMode, TimerInstance::ClockSource prescaler,
		 TimerInstance::CountType topValue>
template<TimerInstance::WaveformGenerationMode firstMode,
		 TimerInstance::WaveformGenerationMode... remainingModes>
constexpr std::optional<typename TimerInstance::WaveformGenerationMode>
FixedFrequencyPwm<TimerInstance, pwmMode, prescaler,
				  topValue>::WgmSelector<firstMode, remainingModes...>::selectMode()
{
	using FirstModeTraits = TimerInstance::template WaveformGenerationModeTraits<firstMode>;
	if constexpr (requires { FirstModeTraits::topValue; })
	{
		// TODO
	}
	if constexpr (sizeof...(remainingModes) == 0)
	{
		return std::nullopt;
	} else
	{
		return WgmSelector<remainingModes...>::selectMode();
	}
}

template<class TimerInstance, Timer::PwmMode mode, TimerInstance::ClockSource prescaler,
		 TimerInstance::CountType topValue>
void
FixedFrequencyPwm<TimerInstance, mode, prescaler, topValue>::initialize()
{
	TimerInstance::initialize(selectedWaveformGenerationMode,
							  static_cast<TimerInstance::ClockSource>(selectedPrescaler));

	using WgmTraits =
		TimerInstance::template WaveformGenerationModeTraits<selectedWaveformGenerationMode>;
	if constexpr (requires { WgmTraits::top = selectedTopValue; })
	{
		// WaveformGenerationMode with dynamic TOP
		// set topValue in the register referenced by WgmTraits::top
		WgmTraits::top = selectedTopValue;
	} else
	{
		// WaveformGenerationMode with fixed TOP
		// double check whether it matches the desired topValue
		static_assert(WgmTraits::topValue == selectedTopValue);
	}
}

template<class TimerInstance, Timer::PwmMode mode, TimerInstance::ClockSource prescaler,
		 TimerInstance::CountType topValue>
template<class OutputChannel>
template<class Signal>
void
FixedFrequencyPwm<TimerInstance, mode, prescaler, topValue>::Output<OutputChannel>::connect(
	bool invert)
{
	OutputChannel::template connect<Signal>(invert ? Timer::OutputMode::PwmInverted
												   : Timer::OutputMode::PwmNormal);
}

template<class TimerInstance, Timer::PwmMode mode, TimerInstance::ClockSource prescaler,
		 TimerInstance::CountType topValue>
template<class OutputChannel>
void
FixedFrequencyPwm<TimerInstance, mode, prescaler, topValue>::Output<OutputChannel>::setCompareValue(
	TimerInstance::CountType compareValue)
{
	OutputChannel::outputCompareRegister = compareValue;
}

template<class TimerInstance, Timer::PwmMode mode, TimerInstance::ClockSource prescaler,
		 TimerInstance::CountType topValue>
template<class OutputChannel>
void
FixedFrequencyPwm<TimerInstance, mode, prescaler, topValue>::Output<OutputChannel>::setDutyCycle(
	percent_t dutyCycle)
{
	typename TimerInstance::CountType compareValue =
		static_cast<TimerInstance::CountType>((dutyCycle) * float(selectedTopValue));
	setCompareValue(compareValue);
}

}  // namespace modm::platform
