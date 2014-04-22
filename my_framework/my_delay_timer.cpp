// for linking the definition of the delay timer function to the declaration
#include "my_delay_timer.h"

#include "my_sys_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <peripheral/int.h>
#include <peripheral/timer.h>

#ifdef __cplusplus
}
#endif

static unsigned int gMillisecondsInOperation;

void my_delay_timer_init(void)
{
   static bool already_initialized = false;

   if (!already_initialized)
   {
      // activate the timer that I will use for my "delay milliseconds" function
      OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_256, T1_TICK_PR);
      ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_1);
   }
}

void delay_ms(unsigned int milliseconds)
{
   unsigned int millisecondCount = gMillisecondsInOperation;
   while((gMillisecondsInOperation - millisecondCount) < milliseconds);
}

unsigned int get_elapsed_time(void)
{
   return gMillisecondsInOperation;
}

// we are using the XC32 C++ compiler, but this ISR handler registration macro
// only seems to work with the XC32 C compiler, so we have to declare it as
// "extern"
// Note: The second argument to the macro was designed by Microchip to work
// with the compiler (or maybe it was the preprocessor; I don't know).  It is
// of the form "IPL" 
#ifdef __cplusplus
extern "C"
#endif
void __ISR(_TIMER_1_VECTOR, IPL7AUTO) Timer1Handler(void)
{
   gMillisecondsInOperation++;

   // clear the interrupt flag
   mT1ClearIntFlag();
}

