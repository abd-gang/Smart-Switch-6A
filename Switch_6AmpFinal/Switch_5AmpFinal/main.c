
#ifndef F_CPU
# define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include "Variable.h"
#include <avr/wdt.h>
#include <avr/power.h>

#define MUX_ADC4 0x44 // voltage
#define MUX_ADC5 0x45 // current

#define ADMUX_VOLTAGE_IN	MUX_ADC4 // input voltage = channel 4
#define ADMUX_CURRENT_IN 	MUX_ADC5 // input current = channel 5

#define ADC_DISABLE {ADCSRA &= (~(1 << ADEN));}
#define ADC_ENABLE  {ADCSRA |= (1<<ADEN) | (1<<ADSC);}

#define TIMER1_ENABLE			TCCR1B|=(1<<CS11) | (1<<WGM12)
#define TIMER1_DISABLE			{TCCR1B&=(~(1<<CS11)) /*| (1<<WGM12)*/;}
#define Interrupt1_Enable		{EIMSK|=(1<<(INT1));}
#define Interrupt1_Disable		{EIMSK&=(~(1<<INT1));}

void spi_slave_init()
{
	SPCR = (1<<SPE)|(1<<SPIE);    //Enable SPI in slave mode
}

void pin_change_int_init()
{
	PCICR=(1<<PCIE1);   // pin change interrupt for on/off for relay on//off remotely
	PCIFR=0;
	PCMSK1=(1<<PCINT8);
}

void adc_init()
{
	ADCSRA = (1 << ADEN) | (1 << ADIE)//| (1<<ADATE)
	//| (0x6);//0x6 = prescaling 64
	//| (0x4);//0x6 = prescaling 16
	//| (0x2);
	|0x03; // by 8 // best
}

void timer_init()
{
	//Timer0 // not using in this code
	// 	TCNT0 = 30;
	// 	TIMSK0 = (1 << TOIE0);	//Timer/Counter0 Overflow Interrupt Enable
	// 	TCCR0A = 0x02;	//clock I/O /8 (From prescaler) // resulting timer0 clock 1 us
	// 	//set to overflow//max count=255// 1 time overflow gives time of 255*1= 255us
	
	//Timer1
	TCCR1B = (1<<CS11) | (1<<WGM12); //clock I/O /8 (From prescaler) // resulting timer0 clock 1 us
	TCNT1=0;
	OCR1A=9600;//9.6ms
	TIMSK1 =  (1<<OCIE1A);
}

void interrupt_init()
{
	EICRA = _BV(ISC11) + _BV(ISC10);
	EIMSK = _BV(INT1);
}

void watch_dog_init()
{
	WDTCSR |= (_BV(WDCE) | _BV(WDE));   // Enable the WD Change Bit
	WDTCSR =  _BV(WDIE) |               // Enable WDT Interrupt
	_BV(WDP2) | _BV(WDP1);   // Set Timeout to ~1 seconds
}

void analog_comp_init()
{
	ACSR = (0<<ACD)|(1<<ACBG)|(1<<ACIE)|(0<<ACIC)|(1<<ACIS1)|(0<<ACIS0); // analog comparator internal BG falling edge
}

void spi_tranceiver ()
{
	sendDataVP[j]='z'; j++;
	sendDataVP[j]='z'; j++;
	sendDataVP[j]='S'; j++;
	sendDataVP[j]=','; j++;

	//long int dataVP = active_power_rms;
	volatile long int dataVP = active_power_rms<8?2:active_power_rms;
	while(dataVP)
	{
		volatile uint8_t temp=dataVP%10;
		sendDataVP[j]=temp+48;wdt_reset();
		j++;
		dataVP=dataVP/10;
	}
	
	sendDataVP[j]='P'; j++;
	sendDataVP[j]=','; j++;

	dataVP = voltage_in_rms;
	while(dataVP)
	{
		uint8_t temp=dataVP%10;
		sendDataVP[j]=temp+48;
		j++;
		dataVP=dataVP/10;
	}
	sendDataVP[j]='V'; j++;
	sendDataVP[j]='a'; j++; sendDataVP[j]='a';
	countbuff=j;
}
//
// void spi_tranceiver ()
// {
// 	sendDataVP[j]='z'; j++;
// 	sendDataVP[j]='z'; j++;
// 	sendDataVP[j]='S'; j++;
// 	sendDataVP[j]=','; j++;
//
// 	//long int dataVP = active_power_rms;
// 	volatile long int dataVP = active_power_rms<8?0:active_power_rms;
// 	if (!dataVP)
// 	{
// 		sendDataVP[j]=48;
// 		j++;
// 	}
// 	else
// 	{
// 		while(dataVP)
// 		{
// 			volatile uint8_t temp=dataVP%10;
// 			sendDataVP[j]=temp+48;wdt_reset();
// 			j++;
// 			dataVP=dataVP/10;
// 		}
// 	}
// 	sendDataVP[j]='P'; j++;
// 	sendDataVP[j]=','; j++;
//
// 	dataVP = voltage_in_rms;
// 	while(dataVP)
// 	{
// 		uint8_t temp=dataVP%10;
// 		sendDataVP[j]=temp+48;
// 		j++;
// 		dataVP=dataVP/10;
// 	}
// 	sendDataVP[j]='V'; j++;
// 	sendDataVP[j]='a'; j++; sendDataVP[j]='a';
// 	countbuff=j;
// }

void init()
{
	clock_prescale_set(clock_div_1); // cpu clock 8 MHz
	DDRC = 0xCE; //6;
	PORTC|=1<<0;   // pulled high
	DDRB = 0xD3; // test
	DDRD = 0x7F; // lcd and spi
	PORTC&=(~(1<<7)); // test
	
	ADMUX = ADMUX_VOLTAGE_IN;      //initial adc setting
	DDRD&=(~(1<<3));//PD@=input int1
	cli();
	interrupt_init();
	adc_init();
	timer_init();
	watch_dog_init();
	analog_comp_init();
	spi_slave_init();
	pin_change_int_init();
	relayOff();
	sei(); // //global interrupt enable
	Interrupt1_Enable;
}

ISR( TIMER0_OVF_vect)
{//not in use
}
static int countCycles=0;
ISR( INT1_vect)
{
		arrVoltage[countvoltage]=333;
			++countvoltage;
			if (countvoltage>=200)
			{
				countvoltage=0;
			}
	
	ACSR|=(1<<ACI);
	ACSR&=(~(1<<ACD)); // enable comparator
	/*static int countCycles=0;*/
	countCycles++;
	if(countCycles>=100)
	{
		countCycles=0;
		j=0; spi_tranceiver();
		//printf("%c %c %c %c %c", {sendDataVP[3]},{sendDataVP[4]},{sendDataVP[5]},{sendDataVP[6]},{sendDataVP[7]})
		countISR=0;
	}
	_delay_us(300);
	
	TCNT1=0;
	TIFR1|=(1<<TOV1);
	TIMER1_ENABLE; // start timer for 10ms
	ADMUX = ADMUX_VOLTAGE_IN; ADC_ENABLE;
	Interrupt1_Disable; ////// to be removed
	wdt_reset();
}


static char flag=0;
uint16_t checkVal=0;
ISR( TIMER1_COMPA_vect)
{

	if (!flag) // 9ms routine
	{
			arrVoltage[countvoltage]=444;
			++countvoltage;
			if (countvoltage>=200)
			{
				countvoltage=0;
			}
		TCNT1=0;
		ADCSRA|=(1<<ADIF);
		ADC_DISABLE;
		OCR1A=7000;
		flag=1;
		//TIMER1_DISABLE;
		checkVal=adcSamples/countADCv;
		//process_voltage_in(adcSamples/countADCv);
		process_voltage_in(checkVal);
		process_power_out(adcSamples1/countADCc);
		adcSamples1=0; adcSamples=0; countADCv=0; countADCc=0;
		// 		TCNT1=0;
		// 		OCR1A=//3000;-
		// 		13000;
		// 		flag=1;
		// 		ADMUX = ADMUX_VOLTAGE_IN; ADC_ENABLE;
		// 		//PORTC^=(1<<7);
		PORTC|=(1<<7);
	}
	else // 9 ms + 7 ms routine
	{
			arrVoltage[countvoltage]=555;
			++countvoltage;
			if (countvoltage>=200)
			{
				countvoltage=0;
			}
		TIFR1|=(1<<TOV1);
		TIMER1_DISABLE;
		TIFR1|=(1<<TOV1);
		TCNT1=0;
		OCR1A=9600;
		EIFR=3;
		Interrupt1_Enable;
		
		flag=0;
		//PORTC^=(1<<7);
		PORTC&=~(1<<7);
	}
}
int countB = 0;
ISR( TIMER1_COMPB_vect)
{
	countB++;
}
//
// ISR( TIMER1_COMPA_vect)
// {
// 	TCNT1=0;
// 	ADC_DISABLE;
// 	TIMER1_DISABLE;
// 	process_voltage_in(adcSamples/countADCv);
// 	process_power_out(adcSamples1/countADCc);
// 	adcSamples1=0; adcSamples=0; countADCv=0; countADCc=0;
// 	//_delay_ms(7);  //  to move it in next cycle
// 	//EIFR=3;
// 	Interrupt1_Enable; ////// to be removed
// }

ISR(ANALOG_COMP_vect)
{
	if(counter==0)
	{
		//countCycles=0;
		countISR++;
		ACSR|=(1<<ACI);
		ACSR|=((1<<ACD)); // disable comparator
		if (countISR==10)
		{
			while(1)
			{
				relayOff();
				countISR=0;	wdt_reset();
			}
		}
	}
}

ISR( ADC_vect)
{

	int val = ADCW;
	switch(ADMUX)
	{
		case ADMUX_VOLTAGE_IN:
		adcSamples+=val; countADCv++;
	//	if(val< 50 || val > 800)
		{
// 			arrVoltage[countvoltage]=val;
// 			++countvoltage;
// 			if (countvoltage>=200)
// 			{
// 				countvoltage=0;
// 			}
		}
				arrVoltage[countvoltage]=22;
		++countvoltage;
		if (countvoltage>=200)
		{
			countvoltage=0;
		}
		ADMUX=ADMUX_CURRENT_IN;
		ADC_ENABLE;
		break;
		
		case ADMUX_CURRENT_IN:
		adcSamples1+=val; countADCc++;
		ADMUX=ADMUX_VOLTAGE_IN;
		ADC_ENABLE;
		break;
	}
}

ISR(SPI_STC_vect)
{
	if(counter==0 && j!=0)
	{
		//uint8_t data=SPDR; // get the data from master // not using currently
		SPDR=sendDataVP[j--];
	}
	else
	{
		j=countbuff; return;
	}
}

ISR(PCINT1_vect)
{
	if (counter==0)
	{
		if(bit_is_clear(PINC,PINC0))
		{
			status=0; relayOff();
		}
		else if(!(bit_is_clear(PINC,PINC0)))
		{
			status=1; relayOn();
		}
	}
}


int main(void)
{
	init();
	while (1)
	{
		if(counter)
		{
			counter--; _delay_ms(300);
		}
		else counter=0;
	}
}