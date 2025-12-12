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
Timer16Bit::clockSourcePeriod(ClockSource clock)
{
	switch (clock)
	{
		case ClockSource::ClkIo:
			return ClockCycles<SystemClock::Timer>{1};
		case ClockSource::ClkIoDiv8:
			return ClockCycles<SystemClock::Timer>{8};
		case ClockSource::ClkIoDiv64:
			return ClockCycles<SystemClock::Timer>{64};
		case ClockSource::ClkIoDiv256:
			return ClockCycles<SystemClock::Timer>{256};
		case ClockSource::ClkIoDiv1024:
			return ClockCycles<SystemClock::Timer>{1024};
		default:
			return ClockCycles<SystemClock::Timer>{0};
	}
}

constexpr Timer16Bit::ClockSource
Timer16Bit::getPrescaler(ClockCycles<SystemClock::Timer> minTickPeriod)
{
	constexpr ClockSource prescalerOptions[] = {ClockSource::ClkIo, ClockSource::ClkIoDiv8,
												ClockSource::ClkIoDiv64, ClockSource::ClkIoDiv256,
												ClockSource::ClkIoDiv1024};
	for (ClockSource prescaler : prescalerOptions)
	{
		if (minTickPeriod <= clockSourcePeriod(prescaler)) return prescaler;
	}
	return ClockSource::Stopped;
}

template<bool dualSlope>
constexpr Timer16Bit::ClockSource
Timer16Bit::selectPrescalerForMaxResolution(ClockCycles<SystemClock::Timer> period)
{
	constexpr ClockCycles<SystemClock::Timer>::rep maxCountsPerPeriod =
		topToCounts<dualSlope>(ClockCycles<SystemClock::Timer>::rep(Timer16Bit::max));

	// FIXME find a proper way to round up
	return getPrescaler(++(period / maxCountsPerPeriod));
}

template<bool dualSlope>
constexpr Timer16Bit::CountType
Timer16Bit::computeTopValue(ClockSource prescaler, ClockCycles<SystemClock::Timer> period)
{
	ClockCycles<SystemClock::Timer> tickPeriod = clockSourcePeriod(prescaler);

	ClockCycles<SystemClock::Timer>::rep countsPerPeriod = period / tickPeriod;

	return CountType(countsToTop<dualSlope>(countsPerPeriod));
}

template<bool dualSlope>
constexpr ClockCycles<SystemClock::Timer>
Timer16Bit::computePeriod(ClockSource prescaler, CountType topValue)
{
	ClockCycles<SystemClock::Timer> tickPeriod = clockSourcePeriod(prescaler);

	ClockCycles<SystemClock::Timer>::rep countsPerPeriod =
		topToCounts<dualSlope>(ClockCycles<SystemClock::Timer>::rep(topValue));

	return countsPerPeriod * tickPeriod;
}

}  // namespace modm::platform
