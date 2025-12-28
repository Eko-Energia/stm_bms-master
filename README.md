BMS Master:

The project includes implementations of functionalities included in the BMS design:

    Some of these include: communication via CAN1 and CAN2 buses, radio communication, and reading the main battery current, 	temperature, and voltage values from the ADC.

	Project uses peripherals' drivers layer to standarize and simplify debugging and workflow of stm32's peripherals. 

Status

    Non-started stages:
		- communication via CAN1
    	- communication via CAN2
		- Radio communication (nRF905 via SPI)
		- RS485 Communication with BMS JK by Dikong
	
    Under construction stages:
		- reading the main battery voltage values from the ADC 
			-- reading voltage
			-- reading current
			-- reading temperature
		
    Completed stages:
    	Nonne

How to Run:
	To properly launch project on BMS Master PCB board, it only requires correct hardware connection. Loading project on STM32 core is held via ST-LINK V2.

Authors:

	- Bartosz Rychlicki 		  		– Firmware
	- Szymon Frączek & Wiktor Klaszczyk - PCB