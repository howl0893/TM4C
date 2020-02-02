#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"

int main(void)
{
    /*
     * We’re going to compute our timer delays using the variable ui32Period
     * */
    uint32_t ui32Period;

    /*
     * Configure the system clock to run at 40MHz
     * */
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    /*
     * enable the GPIO peripheral and configure the pins connected to the LEDs as outputs
     * */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    /*
     * before calling any peripheral specific driverLibfunction we must enable the clock to that peripheral.
     * The second statement configures Timer 0 as a 32-bit timer in periodic mode. Note that when Timer 0 is
     * configured asa32-bit timer, it combines the two 16-bit timers Timer 0A and Timer 0B.
     * */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    /*
     * To toggle a GPIO at 10Hz and a 50% duty cycle, you need to generate an interrupt at ½ of the desired period.
     * This calculated period is then loaded into the Timer’s Interval Load register using the TimerLoadSetfunction
     * of the driverLib Timer API. Note that you have to subtract one from the timer period since the interrupt fires
     *  at the zero count.
     * */
    ui32Period = (SysCtlClockGet() / 5) / 2;
    TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);

    /*
     * Next, we have to enable the interrupt ... not only in the timer module, but also in the NVIC (the Nested Vector
     * Interrupt Controller, the Cortex M4’s interrupt controller). IntMasterEnable()is the master interrupt enable API
     * for all interrupts. IntEnableenables the specific vector associated with Timer0A. TimerIntEnable, enables a specific
     * event within the timer to generate an interrupt. In this case we are enabling an interrupt to be generated on a
     * timeout of Timer 0A.
     * */
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();

    /*
     * Finally we can enable the timer. This will start the timer and interrupts will begin triggering on the timeouts.
     * */
    TimerEnable(TIMER0_BASE, TIMER_A);

    /*super-loop - toggling of pins happens via interrupt*/
    while(1)
    {
    }
}

void Timer0IntHandler(void)
{
    // Clear the timer interrupt
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Read the current state of the GPIO pin and
    // write back the opposite state
    if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2))
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
    }
    else
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);
    }
}
