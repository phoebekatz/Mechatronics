/*
 * Lab 10.cpp
 *
 * Created: 4/9/2024 10:53:45 AM
 * Author : phoeb
 * Send numbers to 7 segment displays
 */ 

#include <avr/io.h>

void wait(volatile int); //declaring wait function. the actual function instructions are at the end.
int sendToMax(unsigned char, unsigned char);

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_repeated_start(void);
void i2c_write_to_address(unsigned char address);
void i2c_read_from_address(unsigned char address);
void i2c_write_data(unsigned char data);
unsigned char i2c_read_data(unsigned char ack);

int main(void)
{
	unsigned char tempFull; //both temp digits
	unsigned char temp1; // digit in the tens place
	unsigned char temp2;// digit in the ones place
		
	DDRB = 0b00101100; // Set pins SCK, MOSI, and SS as output
	
	SPCR = 0b01010001; // enable the SPI, set to Main mode 0, SCK = Fosc/16, lead with MSB
	
	sendToMax(0b00001001, 0b11111111); //enables decoding
	sendToMax(0b00001011, 0b00000010); //set scan limit = 2
	sendToMax(0b00001100, 0b00000001); //turn on display
	sendToMax(0b00001001, 0b11111111); //disables decoding
	
	// I2C (TWI) Setup
	//PRR = PRR & 0b01111111; // Ensure that TWI is powered on (PRTWI = 0)
	DDRC = 0b00000000; // Define all PORTC bits as input (specifically PC4 and PC5)
	PORTC = PORTB | 0b00110000;  // set internal pull-up resistors on SCL and SDA lines (PC5 and PC4) of I2C bus
	i2c_init();	
	
	//set resolution
	i2c_start();
	i2c_write_to_address(0b00110000); //write address
	i2c_write_data(0x08);//resolution register
	i2c_write_data(0x00); //resolution = +-0.5C
	i2c_stop();
	
	while (1)
	{
		wait(100);
		//read temp from sensor
		i2c_start();
		i2c_write_to_address(0b00011000);  // sensor address
		i2c_write_data(0x05);
		i2c_repeated_start();	
		i2c_read_from_address(0b00011000);// sensor read address
		unsigned char upperByte = i2c_read_data(1);
		unsigned char lowerByte = i2c_read_data(0);
		
		//wait(500);
		//sendToMax(0b00000001,0x00); //light display 0 with the ones place digit
		//sendToMax(0b00000010,0x00);//light display 1 with the tens place digit
		//wait(500);
		
		//convert data to degrees 
		upperByte = upperByte & 0x1F; //clears alert bits
		if((upperByte & 0x10) == 0x10)//if temp is negative
		{
			upperByte = upperByte & 0x0F; // clear sign bit
			tempFull = 256 - (upperByte*16 + lowerByte/16);
		}
		else // temp is positive
		{
			tempFull = (upperByte*16) + (lowerByte/16);
		}
		
		unsigned char tempFahrFull = ((tempFull*1.8) + 32);
		temp1=tempFahrFull/10%10;
		temp2=tempFahrFull%10;
		
		//FOR CELSIUS
		//temp1=tempFull/10%10;
		//temp2=tempFull%10;
		
		//display temp on 7 segment display
		sendToMax(0b00000001,temp2); //light display 0 with the ones place digit
		sendToMax(0b00000010,temp1);//light display 1 with the tens place digit
	}
}

void wait(volatile int multiple)
{
	while(multiple > 0)
	{
		TCCR0A = 0x00; //clears WGM00 and WGM01 (bits 0 and 1) to ensure timer/counter is in normal mode
		TCNT0 = 0; //preload value for testing on count = 250
		TCCR0B = 0b00000011;
		while(TCNT0 < 0xFA); //exits when count = 250 (req. preload of 0 to make count 250)
		TCCR0B = 0x00;//stop timeR0
		multiple--;
	}
}

int sendToMax(unsigned char command, unsigned char data)
{
	PORTB = PORTB & 0b11111011; //clear SS
	
	SPDR = command;
	while(!(SPSR&(1<<SPIF) )); //wait for transmission to finish
	
	SPDR = data;
	while(!(SPSR&(1<<SPIF) )); //wait for transmission to finish
	
	PORTB = PORTB | 0b00000100; //set SS bit to end transmission
	
	return 0;
}

void i2c_init(void) { // initialize i2c
	TWSR = 0b00000001; // pre-scaler is set to 4
	TWBR = 0b01100100; // Put 100 into TWBR to define SCL frequency as 6.369kHz for 16MHz oscillator
	TWCR = 0b00000100; // TWEN = 1 (enable TWI)
}

void i2c_start(void) { // send start command
	//while (!(TWCR & (1<<TWINT))); //while (!(TWCR & 0b10000000));   // wait for idle condition -- TWINT bit must be high to proceed -- not needed if single main is used
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);  //TWCR | 0b10100100;       // initiate START condition -- write 1 to TWINT to clear it and initiate action, set TWSTA bit, set TWEN bit
	while (!(TWCR & (1<<TWINT))); //while (!(TWCR & 0b10000000));   // wait for action to finish (poll TWINT bit)
	// if ((TWSR & 0xF8) != START) // error checking -- need to predefine START = 0x08 and ERROR() function.
	// ERROR();
}

void i2c_stop(void) { // send stop command
	while (!(TWCR & (1<<TWINT))); //while (!(TWCR & 0b10000000)) ;  // wait for action to finish (poll TWINT bit)
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO); //TWCR = TWCR | 0b10010100;       // initiate STOP condition -- write 1 to TWINT to clear it and initiate action, set TWSTO bit and set TWEN bit
}

void i2c_repeated_start(void) {
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);  //TWCR | 0b10100100;       // initiate START condition -- write 1 to TWINT to clear it and initiate action, set TWSTA bit, set TWEN bit
	while (!(TWCR & (1<<TWINT))); //while (!(TWCR & 0b10000000));   // wait for action to finish (poll TWINT bit)
	// if ((TWSR & 0xF8) != START) // error checking -- need to predefine START = 0x10 and ERROR() function.
	// ERROR();
}

void i2c_write_to_address( unsigned char address) { //Write an address byte to the I2C2 bus in form of SLA_W (address to write to)
	unsigned char SLA_W = address<<1; // create SLA_W byte by shifting address and leaving R/W bit clear
	while (!(TWCR & (1<<TWINT))); //while (!(TWCR & 0b10000000)) ;  // wait for idle condition -- TWINT bit must be high to proceed
	TWDR = SLA_W;       // Load TWDR with address plus R/W bit
	TWCR = (1<<TWINT) | (1<<TWEN); //TWCR = TWCR | 0b10000100;       // initiate Write -- write 1 to TWINT to clear it and initiate action, and set TWEN
	while (!(TWCR & (1<<TWINT))); //while (!(TWCR & 0b10000000)) ;  // wait for action to finish (poll TWINT bit)
	// if ((TWSR & 0xF8) != MT_SLA_ACK) // error checking -- need to predefine MT_SLA_ACK and ERROR() function depending on possible outcomes: 0x18, 0x20, or 0x38.
	//ERROR();
}

void i2c_read_from_address(unsigned char address) { //Write an address byte to the I2C bus in form of SLA_R (address to read from)
	unsigned char SLA_R = address<<1 | 1; // create SLA_R byte by shifting address and setting R/W bit
	while (!(TWCR & (1<<TWINT))); //while (!(TWCR & 0b10000000)) ;  // wait for idle condition -- TWINT bit must be high to proceed
	TWDR = SLA_R;       // Load TWDR with address plus R/W bit
	TWCR = (1<<TWINT) | (1<<TWEN); //TWCR = TWCR | 0b10000100;       // initiate Write -- write 1 to TWINT to clear it and initiate action, and set TWEN
	while (!(TWCR & (1<<TWINT))); //while (!(TWCR & 0b10000000)) ;  // wait for action to finish (poll TWINT bit)
	// if ((TWSR & 0xF8) != MR_SLA_ACK) // error checking -- need to predefine MR_SLA_ACK and ERROR() function depending on possible outcomes: 0x38, 0x40, or 0x48.
	//ERROR();
}

void i2c_write_data( unsigned char data) { //Write data byte to the I2C2 bus
	while (!(TWCR & (1<<TWINT))); //while (!(TWCR & 0b10000000)) ;  // wait for idle condition -- TWINT bit must be high to proceed
	TWDR = data;       // Load TWDR with data to be sent
	TWCR = (1<<TWINT) | (1<<TWEN); //TWCR = TWCR | 0b10000100;       // initiate Write -- write 1 to TWINT to clear it and initiate action, and set TWEN
	while (!(TWCR & (1<<TWINT))); //while (!(TWCR & 0b10000000)) ;  // wait for action to finish (poll TWINT bit)
	// if ((TWSR & 0xF8) != MT_DATA_ACK) // error checking -- need to predefine MT_DATA_ACK and ERROR() function depending on possible outcomes: 0x28 or 0x30.
	//ERROR();
}


unsigned char i2c_read_data(unsigned char ACK) 
{ //Read a byte of data from a secondary on the I2C bus
	unsigned char data;
	//clear interrupt
	while (!(TWCR & (1<<TWINT))); //while (!(TWCR & 0b10000000)) ;  // wait for idle condition -- TWINT bit must be high to proceed
	//enable receive mode
	if (ACK) // check for whether ACK or NO_ACK should be sent upon receipt of byte from secondary
	TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN); //TWCR = TWCR | 0b11000100;       // initiate Read with ACK -- write 1 to TWINT to clear it and initiate action, and set TWEA and TWEN
	else
	TWCR = (1<<TWINT) | (1<<TWEN); //TWCR = TWCR | 0b10000100;       // initiate Read with NO_ACK-- write 1 to TWINT to clear it and initiate action, and set TWEN
	
	while (!(TWCR & 0b10000000)) ;
    // wait for action to finish (poll TWINT bit. code moves on when TWINT = 0). while (!(TWCR & (1<<TWINT))); 
	// if ((TWSR & 0xF8) != MR_SLA_ACK) // error checking -- need to predefine MR_SLA_ACK and ERROR() function depending on possible outcomes: 0x50 or 0x58.
	//ERROR();suspicious

	//If multiple bytes are to be read, this function can be repeated with proper ACK or NO_ACK until done.
	data = TWDR;  // read the received data from secondary
	return(data);
}
