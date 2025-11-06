#include <modm/architecture/interface/delay.hpp>
#include <modm/platform/gpio/pins.hpp>
#include <modm/platform/timer/timer_0.hpp>

// repeatedly fades an LED connected to OC0A (pin D6) on and off
int
main()
{
	using Timer = modm::platform::Timer0;
	using Pwm =
		Timer::FixedFrequencyPwm<Timer::PwmMode::FastPwm, Timer::ClockSource::ClkIoDiv64, 0xff>;

	// results in a PWM frequency of 976.5 Hz at F_CPU = 16 MHz
	Pwm::initialize();

	modm::platform::GpioD6::setOutput();
	Pwm::ChannelA::connect<modm::platform::GpioD6::Oca>();

	while (true)
	{
		for (uint8_t dutyCycle = 0; dutyCycle < 255; ++dutyCycle)
		{
			Pwm::ChannelA::setDutyCycle(dutyCycle);

			// need to use delay instead of anything based on modm:platform:clock
			// because that would use timer 0 which is required for the PWM generation instead
			modm::delay_ms(4);
		}

		for (uint8_t dutyCycle = 255; dutyCycle > 0; --dutyCycle)
		{
			Pwm::ChannelA::setDutyCycle(dutyCycle);

			modm::delay_ms(4);
		}
	}
}
