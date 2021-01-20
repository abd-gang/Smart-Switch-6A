# Smart-Switch-5A
Smart switch 6 amperes
This switch can be used with the all eletric appliances less than 6 amperes in order to save them from over and under voltage and high current, power fluctuations, etc.
See the technical details below-----

Short circuit
Output from current sensor is given to negative of the internal comparator of the attiny88 through the resistor network.
Whenever the peak current goes above 7 amperes, interrupt is generated and we count the 5 cycles and if the current exceeds for the continuously 5 cycles, we turned off the switch. 
Only power off and power on can bring the switch on back. 

Timing descriptions
ADC
ADC Channel 4 - voltage
ADC Channel 5 - current
ADC Clock – 1MHz 0r 1us 
ADC conversion time - 25 clock cycles - 25*1 = 25us
 
Voltage and Power
 
No of samples for voltage = 50 (ac input voltage positive half cycle)
No of samples for current = 50
 
No of half cycles for power = 10  
Time taken for power measurement = 10*20 = 200ms
 
Over Power and short circuit
 
Over power cut-off time – 200ms
No of Surge current cycles ignored = 5
Short-Circuit current cut-off time - 100ms
 
Timer
 
Timer0 clock - 8us
Timer0 count = 30, 255-30 = 225
Timer0 overflow interrupt = 225*8 = 1.8ms 
Start-up counter = 6
Time for every start-up counter after power up = 300*1.8 = 0.54sec
Total delay of 5*.54 = 2.7s is given after powering up. 
Display updating after every = 3*.54 = 1.62sec
 
 
Electrical Working Ranges

170 < input rms voltage < 270 -------- Relay On, else Off.
0 < input rms power < 2100 ------------Relay On, else Off.
Current < 9amp --------------------------Relay On, else Off. 

