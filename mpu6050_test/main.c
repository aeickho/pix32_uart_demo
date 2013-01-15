#include <p32xxxx.h>
#include <plib.h>
#include <stdint.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "uart.h"
#include "portsetup.h"
#include "basic.h"
#include "delay.h"
#include "tools/printf.h"

#include "i2c.h"
#include "mpu6050.h"
#include "common.h"

int
main (void)
{
  unsigned char Data = 0x00;
  uint8_t inData;
  int i = 0;
//  uint32_t actualClock;

  portsetup ();


  /* Configure PB frequency and wait states */
  SYSTEMConfigPerformance (SystemClock ());
//  UART1Init (SystemClock ());
  UART2Init (SystemClock ());

  INTConfigureSystem (INT_SYSTEM_CONFIG_MULT_VECTOR);
  INTEnableInterrupts ();

  UART2PutStr
    (".............................................................................hallo\r\n");
  UART2PutStr ("UART2 Welt\r\n");

  init_printf ();
  // try to reset nrf chip

  UART2PutStr ("done\n\r");
  UART2PutStr ("setup i2c");

  Setup_I2C ();
  UART2PutStr (" done \n\r");
  UART2PutStr ("Setup_MPU6050");

  Setup_MPU6050 ();

  UART2PutStr (" done \n\r");



  UART2PutStr ("read MPU6050_ADDRESS");

//Calibrate_Gyros();


  LDByteReadI2C (MPU6050_ADDRESS, MPU6050_RA_WHO_AM_I, &Data, 1);
  UART2PutStr (" done \n\r");
  tfp_printf ("%x.....", MPU6050_ADDRESS);

  tfp_printf ("%x.....", Data);

  if (Data == 0x68)
    {
      tfp_printf ("\nI2C Read Test Passed, MPU6050 Address: 0x%x", Data);
    }



  while (1)
    {
      Get_Accel_Values ();
      tfp_printf ("val %d %d %d\n", ACCEL_XOUT, ACCEL_YOUT, ACCEL_ZOUT);
//      delay_ms (100);
    }
}
