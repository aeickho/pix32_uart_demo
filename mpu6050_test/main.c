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

 signed short GetTemp();


int
main (void)
{
  unsigned char Data = 0x00;
  uint8_t inData;
  int i = 0;
//  uint32_t actualClock;

 uint32_t ctime[10];
 
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
      int t,i;
      uint8_t data[20];
      int8_t *sdat=(int8_t *)data;
      
      ctime[0]=ReadCoreTimer ();
//      Get_Accel_Values ();
 //     Get_Gyro_Raw_Rates();
//      t=GetTemp();
      
 //     tfp_printf ("val %u %u %d %d %d %d %d %d %d\n\r", ctime[0], ctime[1], ACCEL_XOUT, ACCEL_YOUT, ACCEL_ZOUT,GYRO_XRATERAW,GYRO_YRATERAW, GYRO_ZRATERAW, t);
// LDByteReadI2C(unsigned char ControlByte, unsigned char Address, unsigned char *Data, unsigned char Length);
   LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_ACCEL_XOUT_H, data, 14);
      ctime[1]=ReadCoreTimer ();
 
   //(GYRO_XOUT_H<<8)|GYRO_XOUT_L
   tfp_printf ("val %u %u %d %d %d %d %d %d %d\n\r", ctime[0], ctime[1], (sdat[0]<<8)|data[1], (sdat[2]<<8)|data[3],(sdat[4]<<8)|data[5],
                                     (sdat[8]<<8)|data[9],(sdat[10]<<8)|data[11],(sdat[12]<<8)|data[13],(sdat[6]<<8)|data[7]);
// 
    
      
//      delay_ms (100);
    }
}
