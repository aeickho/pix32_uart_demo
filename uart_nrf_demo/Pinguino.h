#ifndef _PINGUINO_H_
#define _PINGUINO_H_

#define mGetLED_1()         LATBbits.LATB15
#define mGetLED_2()         LATAbits.LATA10

#define mLED_1_On()         LATBSET = _LATB_LATB15_MASK; //mLED_1 = 1;
#define mLED_2_On()         LATASET = _LATA_LATA10_MASK;// mLED_2_bit; //mLED_2

#define mLED_1_Off()        LATBCLR = _LATB_LATB15_MASK; //mLED_1_bit; //mLED_1
#define mLED_2_Off()        LATACLR = _LATA_LATA10_MASK; //mLED_2_bit; //mLED_2

#define mLED_1_Toggle()     LATBINV = _LATB_LATB15_MASK;
#define mLED_2_Toggle()     LATAINV = _LATA_LATA10_MASK; //!mLED_2;




void delay_7us(void);
void delay_1ms (void);
void _delay_ms (unsigned int length);
void delay_ms (unsigned int length);
            
#endif
