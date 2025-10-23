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

#include <chrono>

namespace modm::platform
{

template<>
struct Timer8Bit::PrescalerTraits<Timer8Bit::Prescaler::Div1>
{
	using Ratio = std::ratio<1>;
};
template<>
struct Timer8Bit::PrescalerTraits<Timer8Bit::Prescaler::Div8>
{
	using Ratio = std::ratio<8>;
};
template<>
struct Timer8Bit::PrescalerTraits<Timer8Bit::Prescaler::Div64>
{
	using Ratio = std::ratio<64>;
};
template<>
struct Timer8Bit::PrescalerTraits<Timer8Bit::Prescaler::Div256>
{
	using Ratio = std::ratio<256>;
};
template<>
struct Timer8Bit::PrescalerTraits<Timer8Bit::Prescaler::Div1024>
{
	using Ratio = std::ratio<1024>;
};

template<Timer8Bit::ClockSource clockSource, class SystemClock>
	requires requires {
		typename Timer8Bit::PrescalerTraits<static_cast<Timer8Bit::Prescaler>(clockSource)>;
	}
struct Timer8Bit::ClockSourceTraits<clockSource, SystemClock>
{
	using PrescalerRatio = PrescalerTraits<static_cast<Prescaler>(clockSource)>::Ratio;
	using ClockPeriods = std::chrono::duration<
		Timer8Bit::ExtendedCountType,
		std::ratio_multiply<PrescalerRatio, std::ratio<1, SystemClock::Timer>>>;
};

template<class SystemClock>
struct Timer8Bit::ClockSourceTraits<Timer8Bit::ClockSource::Stopped, SystemClock>
{};
template<class SystemClock>
struct Timer8Bit::ClockSourceTraits<Timer8Bit::ClockSource::ExternalFalling, SystemClock>
{};
template<class SystemClock>
struct Timer8Bit::ClockSourceTraits<Timer8Bit::ClockSource::ExternalRising, SystemClock>
{};

struct Timer8Bit::SingleSlopeModeTraits
{
	static constexpr ExtendedCountType
	countsPerPeriod(CountType topValue)
	{
		return ExtendedCountType(topValue) + 1;
	}

	static constexpr CountType
	computeTopValue(ExtendedCountType countsPerPeriod)
	{
		return CountType(countsPerPeriod - 1);
	}
};

struct Timer8Bit::DualSlopeModeTraits
{
	static constexpr ExtendedCountType
	countsPerPeriod(CountType topValue)
	{
		return ExtendedCountType(topValue) * 2;
	}

	static constexpr CountType
	computeTopValue(ExtendedCountType countsPerPeriod)
	{
		return CountType(countsPerPeriod / 2);
	}
};

template<>
struct Timer8Bit::WaveformGenerationModeTraits<Timer8Bit::WaveformGenerationMode::Normal>
	: SingleSlopeModeTraits
{
	static constexpr CountType top = Timer8Bit::max;
};

template<>
struct Timer8Bit::WaveformGenerationModeTraits<
	Timer8Bit::WaveformGenerationMode::PhaseCorrectPwm8Bit> : DualSlopeModeTraits
{
	static constexpr CountType top = Timer8Bit::max;

	static constexpr PwmMode pwmMode = PwmMode::PhaseCorrectPwm;
};

template<>
struct Timer8Bit::WaveformGenerationModeTraits<Timer8Bit::WaveformGenerationMode::Ctc>
	: SingleSlopeModeTraits
{};

template<>
struct Timer8Bit::WaveformGenerationModeTraits<Timer8Bit::WaveformGenerationMode::FastPwm8Bit>
	: SingleSlopeModeTraits
{
	static constexpr CountType top = Timer8Bit::max;

	static constexpr PwmMode pwmMode = PwmMode::FastPwm;
};

template<>
struct Timer8Bit::WaveformGenerationModeTraits<
	Timer8Bit::WaveformGenerationMode::PhaseCorrectPwmOcra> : DualSlopeModeTraits
{};

template<>
struct Timer8Bit::WaveformGenerationModeTraits<Timer8Bit::WaveformGenerationMode::FastPwmOcra>
	: SingleSlopeModeTraits
{};

}  // namespace modm::platform
