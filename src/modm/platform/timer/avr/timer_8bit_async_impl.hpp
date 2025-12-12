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

namespace modm::platform
{

constexpr ClockCycles<SystemClock::Timer>
Timer8BitAsync::clockSourcePeriod(ClockSource clock)
{
	switch (clock)
	{
		case ClockSource::ClkTimer:
			return ClockCycles<SystemClock::Timer>{1};
		case ClockSource::ClkTimerDiv8:
			return ClockCycles<SystemClock::Timer>{8};
		case ClockSource::ClkTimerDiv32:
			return ClockCycles<SystemClock::Timer>{32};
		case ClockSource::ClkTimerDiv64:
			return ClockCycles<SystemClock::Timer>{64};
		case ClockSource::ClkTimerDiv128:
			return ClockCycles<SystemClock::Timer>{128};
		case ClockSource::ClkTimerDiv256:
			return ClockCycles<SystemClock::Timer>{256};
		case ClockSource::ClkTimerDiv1024:
			return ClockCycles<SystemClock::Timer>{1024};
		default:
			return ClockCycles<SystemClock::Timer>{0};
	}
}

constexpr Timer8BitAsync::ClockSource
Timer8BitAsync::getPrescaler(ClockCycles<SystemClock::Timer> minTickPeriod)
{
	constexpr ClockSource prescalerOptions[] = {
		ClockSource::ClkTimer,       ClockSource::ClkTimerDiv8,   ClockSource::ClkTimerDiv32,
		ClockSource::ClkTimerDiv64,  ClockSource::ClkTimerDiv128, ClockSource::ClkTimerDiv256,
		ClockSource::ClkTimerDiv1024};
	for (ClockSource prescaler : prescalerOptions)
	{
		if (minTickPeriod <= clockSourcePeriod(prescaler)) return prescaler;
	}
	return ClockSource::Stopped;
}

template<bool dualSlope>
constexpr Timer8BitAsync::ClockSource
Timer8BitAsync::selectPrescalerForMaxResolution(ClockCycles<SystemClock::Timer> period)
{
	constexpr ClockCycles<SystemClock::Timer>::rep maxCountsPerPeriod =
		topToCounts<dualSlope>(ClockCycles<SystemClock::Timer>::rep(Timer8BitAsync::max));

	// FIXME find a proper way to round up
	return getPrescaler(++(period / maxCountsPerPeriod));
}

template<bool dualSlope>
constexpr Timer8BitAsync::CountType
Timer8BitAsync::computeTopValue(ClockSource prescaler, ClockCycles<SystemClock::Timer> period)
{
	ClockCycles<SystemClock::Timer> tickPeriod = clockSourcePeriod(prescaler);

	ClockCycles<SystemClock::Timer>::rep countsPerPeriod = period / tickPeriod;

	return CountType(countsToTop<dualSlope>(countsPerPeriod));
}

template<bool dualSlope>
constexpr ClockCycles<SystemClock::Timer>
Timer8BitAsync::computePeriod(ClockSource prescaler, CountType topValue)
{
	ClockCycles<SystemClock::Timer> tickPeriod = clockSourcePeriod(prescaler);

	ClockCycles<SystemClock::Timer>::rep countsPerPeriod =
		topToCounts<dualSlope>(ClockCycles<SystemClock::Timer>::rep(topValue));

	return countsPerPeriod * tickPeriod;
}

}  // namespace modm::platform
