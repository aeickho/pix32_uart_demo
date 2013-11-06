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

signed short GetTemp ();


int
main (void)
{
  unsigned char Data = 0x00;

  uint32_t ctime[10];

  portsetup ();

      uint8_t data[20];
      int8_t *sdat = (int8_t *) data;


  SYSTEMConfigPerformance (SystemClock ());

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

  tfp_printf ("read \r\n");
  LDByteReadI2C (MPU6050_ADDRESS, MPU6050_RA_ACCEL_XOUT_H, data, 14);
  tfp_printf ("val %u %u %d %d %d %d %d %d %d\n\r", ctime[0], ctime[1],
	      (sdat[0] << 8) | data[1], (sdat[2] << 8) | data[3],
	      (sdat[4] << 8) | data[5], (sdat[8] << 8) | data[9],
	      (sdat[10] << 8) | data[11], (sdat[12] << 8) | data[13],
	      (sdat[6] << 8) | data[7]);

  LDByteReadI2C (MPU6050_ADDRESS,MPU6050_RA_FIFO_COUNTH, data, 2);
  tfp_printf ("MPU6050_RA_FIFO_COUNT %u \r\n",(sdat[0] << 8) | data[1]);


  // config fifo
    
  LDByteWriteI2C (MPU6050_ADDRESS, MPU6050_RA_FIFO_EN,
		 MPU6050_MASK_FIFO_EN_TEMP | MPU6050_MASK_FIFO_EN_XG |
		 MPU6050_MASK_FIFO_EN_YG | MPU6050_MASK_FIFO_EN_ZG |
		 MPU6050_MASK_FIFO_EN_ACCEL);



  // enable fifo mode
  LDByteWriteI2C (MPU6050_ADDRESS,MPU6050_RA_USER_CTRL,MPU6050_MASK_USER_CTRL_FIFO_EN);
  // 
while ( 1 )
  {
  LDByteReadI2C (MPU6050_ADDRESS,MPU6050_RA_FIFO_COUNTH, data, 2);
  tfp_printf ("MPU6050_RA_FIFO_COUNT %u \r\n",(sdat[0] << 8) | data[1]);
  }
  
  
  while (1)
    {

      ctime[0] = ReadCoreTimer ();

      LDByteReadI2C (MPU6050_ADDRESS, MPU6050_RA_ACCEL_XOUT_H, data, 14);
      ctime[1] = ReadCoreTimer ();

      tfp_printf ("val %u %u %d %d %d %d %d %d %d\n\r", ctime[0], ctime[1],
		  (sdat[0] << 8) | data[1], (sdat[2] << 8) | data[3],
		  (sdat[4] << 8) | data[5], (sdat[8] << 8) | data[9],
		  (sdat[10] << 8) | data[11], (sdat[12] << 8) | data[13],
		  (sdat[6] << 8) | data[7]);
    }
}
