
#ifndef VARIABLE_H_
#define VARIABLE_H_

#include<util/delay.h>
#include <avr/wdt.h>

#define RELAY_BIT   0x10
#define relayOn()  {PORTD |= (RELAY_BIT);}
#define relayOff() {PORTD &=~(RELAY_BIT);}

#define CUTOFF_VOLTAGE_LO	170
#define CUTOFF_VOLTAGE_HI	270
#define Reconnect			260
#define CUTOFF_POWER_LO		10
#define CUTOFF_POWER_HI		1700

volatile long voltage_in_rms = 0, voltage_out_rms = 0,active_power_rms = 0,active_power_rms1 = 0,active_power_rms2=0;
volatile float power_in_rms1=0,power_in_rms=0;
volatile long unsigned int adcSamples=0, adcSamples1=0;
volatile unsigned char status=1, countISR=0;;
volatile uint8_t countADCv=0,countADCc=0;
volatile unsigned char counter=6,counter1=1,HIGH_CUT_DISPLAY=0,LOW_CUT_DISPLAY=0,HIGH_DISPLAY=1;

volatile unsigned char j=0, countbuff=0;

volatile unsigned char sendDataVP[15] ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

volatile uint8_t conP=0;
void Relay(unsigned int Voltage)
{
	if (Voltage > CUTOFF_VOLTAGE_HI || (status==0)){relayOff();HIGH_CUT_DISPLAY=1;}
	else if ((Voltage >0 && Voltage <(CUTOFF_VOLTAGE_LO-10)) || (status==0)){relayOff();}
	else if (!HIGH_CUT_DISPLAY  && Voltage > CUTOFF_VOLTAGE_LO && Voltage < CUTOFF_VOLTAGE_HI && counter==0 && (status==1)){relayOn();HIGH_DISPLAY=0;}
	else if (Voltage > CUTOFF_VOLTAGE_LO && Voltage < Reconnect && counter==0 && (status==1)){relayOn();HIGH_CUT_DISPLAY=0;}
}

volatile int arrVoltage[200];
int countvoltage=0, countvoltage1=0;

void process_voltage_in(uint16_t adc_val)
{
	static uint8_t countAvg=0;
	countAvg++;
// 	 	arrVoltage[countvoltage]=adc_val;
//  		++countvoltage;
//  		if (countvoltage>=200)
//  		{
//  			countvoltage=0;
//  		}
	voltage_out_rms+=adc_val;
	if (countAvg>=5)
	{
		
		voltage_in_rms=(voltage_out_rms/countAvg)/2.16 ;
		Relay(voltage_in_rms);
		
// 		arrVoltage[countvoltage]=voltage_in_rms;
// 		++countvoltage;
// 		if (countvoltage>=200)
// 		{
// 			countvoltage=0;
// 		}
		voltage_out_rms=0;
		countAvg=0;
	}
}

void process_power_out(unsigned long adc_val)
{
	static int countPower=0, countPower1=0;
	countPower++;
	power_in_rms1+=adc_val;

	if (countPower>=5)
	{
		power_in_rms=power_in_rms1/countPower - 515;
		power_in_rms=(((power_in_rms*0.00322)*7.57));
		active_power_rms1+=power_in_rms*voltage_in_rms;
		countPower=0;countPower1++;power_in_rms1=0;power_in_rms=0;

		if (countPower1==2)
		{
			active_power_rms2=active_power_rms1/countPower1;
			if(active_power_rms2<100) active_power_rms=active_power_rms2-7;
			else active_power_rms=active_power_rms2*1.1;
			
			if (active_power_rms>=1300)
			{
				while(1){
					relayOff();
					wdt_reset();
				}

			}
			// 			arrVoltage[countvoltage]=active_power_rms;
			// 			++countvoltage;
			// 			if (countvoltage>=200)
			// 			{
			// 				countvoltage=0;
			// 				//relayOn();
			// 			}
			active_power_rms1=0;
			countPower1=0;
		}
	}
}

#endif /* VARIABLE_H_ */