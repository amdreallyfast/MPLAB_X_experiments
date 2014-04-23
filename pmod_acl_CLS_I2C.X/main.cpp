extern "C"
{
#include <peripheral/ports.h>	// Enable port pins for input or output
#include <peripheral/system.h>	// Setup the system and perihperal clocks for best performance
#include <peripheral/i2c.h>
#include <string.h>
}

#include "my_sys_config.h"
#include "../my_framework/my_I2C_handler.h"
#include "../my_framework/my_delay_timer.h"
#include "../my_framework/WOOP_WOOP_WOOP.h"


// Oscillator Settings
/*
 * SYSCLK = 80 MHz (8 MHz Crystal/ FPLLIDIV * FPLLMUL / FPLLODIV)
 * PBCLK = 40 MHz
 * Primary Osc w/PLL (XT+,HS+,EC+PLL)
 * WDT OFF
 * Other options are don't care
 */
#pragma config FNOSC = PRIPLL       // Oscillator selection
#pragma config POSCMOD = EC         // Primary oscillator mode
#pragma config FPLLIDIV = DIV_2     // PLL input divider
#pragma config FPLLMUL = MUL_20     // PLL multiplier
#pragma config FPLLODIV = DIV_1     // PLL output divider
#pragma config FPBDIV = DIV_2       // Peripheral bus clock divider
#pragma config FSOSCEN = OFF        // Secondary oscillator enable


int main(void)
{
   bool ret_val = false;

   my_delay_timer delay_timer_ref = my_delay_timer::get_instance();
   delay_timer_ref.init(SYS_CLOCK / PB_DIV);

   my_i2c_handler i2c_driver = my_i2c_handler::get_instance();

   // setup the LEDs
   PORTSetPinsDigitalOut(IOPORT_B, BIT_10 | BIT_11 | BIT_12 | BIT_13);
   PORTClearBits(IOPORT_B, BIT_10 | BIT_11 | BIT_12 | BIT_13);

   // enable multivector interrupts so the timer1 interrupt vector can lead to
   // the interrupt handler;
   // this also enables interrupts (apparently), thus starting the timer
   INTEnableSystemMultiVectoredInt();

   if (!i2c_driver.setupI2C(I2C2))
   {
      PORTSetBits(IOPORT_B, BIT_10);
      while(1);
   }

   if (!i2c_driver.myI2CInitCLS(I2C2))
   {
      PORTSetBits(IOPORT_B, BIT_11);
      while(1);
   }
   i2c_driver.myI2CWriteToLine(I2C2, "CLS initialized", 1);

   if (!i2c_driver.myI2CInitAccel(I2C2))
   {
      i2c_driver.myI2CWriteToLine(I2C2, "Accel init fail", 1);
      while(1);
   }
   i2c_driver.myI2CWriteToLine(I2C2, "Accel initialized", 1);

   // I2C initialization done; reset onboard LEDs for use in other things
   PORTClearBits(IOPORT_B, BIT_10 | BIT_11 | BIT_12 | BIT_13);


   // loop forever
   bool read_success = false;
   ACCEL_DATA acl_data;
   char message[20];
   int i = 0;
   while(1)
   {
      PORTToggleBits(IOPORT_B, BIT_13);
      delay_timer_ref.delay_ms(200);

      read_success = true;

      if (!i2c_driver.readAccel(I2C2, &acl_data))
      {
         i2c_driver.myI2CWriteToLine(I2C2, "Accel read fail", 1);
         read_success = false;
      }

      if (read_success)
      {
         snprintf(message, CLS_LINE_SIZE, "X:%5.2f Y:%5.2f", acl_data.X, acl_data.Y);
         i2c_driver.myI2CWriteToLine(I2C2, message, 1);
         snprintf(message, CLS_LINE_SIZE, "Z:%5.2f", acl_data.Z);
         i2c_driver.myI2CWriteToLine(I2C2, message, 2);
      }
      else
      {
         snprintf(message, CLS_LINE_SIZE, "i = '%d'", i);
         i2c_driver.myI2CWriteToLine(I2C2, message, 1);
         i += 1;
      }

      delay_timer_ref.delay_ms(200);
   }
}
