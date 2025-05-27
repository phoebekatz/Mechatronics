/*
 * Project Code.cpp
 * Mechatronics Spring 2024
 * Created: 4/17/2024 1:54:52 AM
 * Author : Phoebe Katz
 * Assisting team members: Michael Manko, Jack Berkowitz
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

//GLOBAL VARIABLES
int phase_step = 1;
int totalPillCount = 0;
int colorPhase = 0; // 0 corresponds to red
int newPhase = 0;
unsigned char pillOrganization[6][2]= { {0,0},{0,0},{0,0},{0,0},{0,0},{0,0} }; // columns: morning,  evening. rows: R,O,Y,G,B,P (colors) 

//FUNCTIONS
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

void lcdCommand(unsigned char data)
{
	unsigned char highMask = 0b11110000;
	unsigned char lowMask = 0b00001111;
	
	unsigned char highNibble = (highMask & data) >> 4; //clears bottom 4 bits, leaves top 4. moves bits to be right justified
	unsigned char lowNibble = lowMask & data; // leaves bottom 4 bits, clears top 4
	
	PORTC = (highMask & PORTC) | highNibble ; //send high nibble
	PORTD = PORTD | 0b00000010; //set pin connected to RS
	PORTD = PORTD | 0b00000001; // set pin connected to E
	wait(1);
	PORTD = PORTD & 0b11111110; //clear pin connected to E
	wait(1);
	
	PORTC = (highMask & PORTC) | lowNibble; // send low nibble
	PORTD = PORTD | 0b00000010; //set pin connected to RS
	PORTD = PORTD | 0b00000001; // set pin connected to E
	wait(1);
	PORTD = PORTD & 0b11111110; //clear pin connected to E
	wait(1);
}
void lcdSetUpCommand(unsigned char data)
{
	unsigned char highMask = 0b11110000;
	unsigned char lowMask = 0b00001111;
	
	unsigned char highNibble = (highMask & data) >> 4; //clears bottom 4 bits, leaves top 4. moves bits to be right justified
	unsigned char lowNibble = lowMask & data; // leaves bottom 4 bits, clears top 4
	
	PORTC = (highMask & PORTC) | highNibble ; //send high nibble
	PORTD = PORTD & 0b11111101; //clear pin connected to RS
	PORTD = PORTD | 0b00000001; // set pin connected to E
	wait(5);
	PORTD = PORTD & 0b11111110; //clear pin connected to E
	wait(5);
	
	PORTC = (highMask & PORTC) | lowNibble; // send low nibble
	PORTD = PORTD & 0b11111101; //clear pin connected to RS
	PORTD = PORTD | 0b00000001; // set pin connected to E
	wait(1);
	PORTD = PORTD & 0b11111110; //clear pin connected to E
	wait(1);
}
void lcdStart(void)
{
	lcdSetUpCommand(0x02); //4 bit initialization
	wait(6);
	lcdSetUpCommand(0x28); //2 lines, 5x8 matrix,4-bit mode
	wait(6);
	lcdSetUpCommand(0x0E); // screen on, cursor blinking
	wait(6);
	lcdSetUpCommand(0x01); //clear display
	wait(6);
	lcdSetUpCommand(0x01); //clear again
	wait(6);
	lcdSetUpCommand(0x06); //shifts cursor to the right
	wait(6);
}
void lcdString (char *string)
{
	int i;
	for(i=0;string[i]!=0;i++)		//Send each char of string until you run out of characters
	{
		lcdCommand(string[i]);
	}
}

void lcdPrintVariable(int data)
{
	if (data == 0)
	{
		lcdString("0");
	}
	else if (data == 1)
	{
		lcdString("1");
	}
	else if (data == 2)
	{
		lcdString("2");
	}
	else if (data==3)
	{
		lcdString("3");
	}
	else if(data==4)
	{
		lcdString("4");
	}
	else if(data==5)
	{
		lcdString("5");
	}
	else if(data == 6)
	{
		lcdString("6");
	}
	else if(data == 7)
	{
		lcdString("7");
	}
	else if(data == 8)
	{
		lcdString("8");
	}
	else if (data == 9)
	{
		lcdString("9");
	}
	else if (data == 10)
	{
		lcdString("10");
	}
	else
	{
		lcdString("error");
	}
}
void printNewPhase(void)
{
	if (colorPhase < 6)
	{
		lcdSetUpCommand(0b00000010); //return home
		if (colorPhase == 1) // orange
		{
			lcdString("How many orange?");
			lcdString("                           AM:0   PM:0   ");
		}
		if (colorPhase == 2) //yellow
		{
			lcdString("How many yellow?");
			lcdString("                           AM:0   PM:0   ");
		}
		if (colorPhase == 3) //green
		{
			lcdString("How many green?");
			lcdString("                            AM:0   PM:0   ");
		}
		if (colorPhase == 4) //blue
		{
			lcdString("How many blue?");
			lcdString("                             AM:0   PM:0   ");
		}
		if (colorPhase == 5)//brown
		{
			lcdString("How many brown?");
			lcdString("                            AM:0   PM:0   ");
		}
	}
	
	else if(colorPhase == 6) //moving on to motor phase
	{
		lcdSetUpCommand(0b00000010); //return home
		lcdSetUpCommand(0b00000001); //clear display
		wait(2);
		lcdString("Please pour                              your pills");
		//lcdPrintVariable(totalPillCount);ABLE TO CORRECTLY DO THIS
	}
	
}
void lcdPrintColor(void)
{
	lcdSetUpCommand(0b00000010); //return home
	lcdString("How many ");
	if (colorPhase == 0)
	{
		lcdString("red?                              ");
	}
	else if (colorPhase == 1)
	{
		lcdString("orange?                           ");
	}
	else if (colorPhase == 2)
	{
		lcdString("yellow?                           ");
	}
	else if (colorPhase == 3)
	{
		lcdString("green?                            ");
	}
	else if(colorPhase == 4)
	{
		lcdString("blue?                             ");
	}
	else if(colorPhase == 5)
	{
		lcdString("brown?                            ");
	}
	else
	{
		lcdString("error");
	}
	lcdString("AM:");
	lcdPrintVariable(pillOrganization[colorPhase][0]);
	lcdString("   PM:");
	lcdPrintVariable(pillOrganization[colorPhase][1]);
}

void step1_CCW() //motor 1/PM attached to port d
{
	switch (phase_step)
	{
		case 1:
		// step to 2
		PORTD = (PORTD & 0b00001111) | 0b00010000;
		phase_step = 2;
		break;
		case 2:
		// step to 3
		PORTD = (PORTD & 0b00001111) | 0b01000000;
		phase_step = 3;
		break;
		case 3:
		// step to 4;
		PORTD = (PORTD & 0b00001111) | 0b00100000;
		phase_step = 4;
		break;
		case 4:
		// step to 1;
		PORTD = (PORTD & 0b00001111) | 0b10000000;
		phase_step = 1;
		break;
	}
}
void step1_CW() //motor 1/PM attached to port d
{
	
	switch (phase_step)
	{
		case 1:
		// step to 4
		PORTD = (PORTD & 0b00001111) | 0b00100000;
		phase_step = 4;
		break;
		case 2:
		// step to 1
		PORTD = (PORTD & 0b00001111) | 0b10000000;
		phase_step = 1;
		break;
		case 3:
		// step to 2;
		PORTD = (PORTD & 0b00001111) | 0b00010000;
		phase_step = 2;
		break;
		case 4:
		// step to 3;
		PORTD = (PORTD & 0b00001111) | 0b01000000;
		phase_step = 3;
		break;
	}
}
void step2_CCW() // motor 2/AM attached to port b
{
	switch (phase_step)
	{
		case 1:
		// step to 2
		PORTB = (PORTB & 0b11110000) | 0b00000001;
		phase_step = 2;
		break;
		case 2:
		// step to 3
		PORTB = (PORTB & 0b11110000) | 0b00000100;
		phase_step = 3;
		break;
		case 3:
		// step to 4;
		PORTB = (PORTB & 0b11110000) | 0b00000010;
		phase_step = 4;
		break;
		case 4:
		// step to 1;
		PORTB = (PORTB & 0b11110000) | 0b00001000;
		phase_step = 1;
		break;
	}
}
void step2_CW() //motor 2/AM attached to port b
{
	
	switch (phase_step)
	{
		case 1:
		// step to 4
		PORTB = (PORTB & 0b11110000) | 0b00000010;
		phase_step = 4;
		break;
		case 2:
		// step to 1
		PORTB = (PORTB & 0b11110000) | 0b00001000;
		phase_step = 1;
		break;
		case 3:
		// step to 2;
		PORTB = (PORTB & 0b11110000) | 0b00000001;
		phase_step = 2;
		break;
		case 4:
		// step to 3;
		PORTB = (PORTB & 0b11110000) | 0b00000100;
		phase_step = 3;
		break;
	}
}

int readColor(void)
{
	//checks B5, C4, then C5
	if ( (PINB & 0b00100000) && (!(PINC & 0b00010000)) && (!(PINC & 0b00100000)) )//red
	{
		return 0; 
	}
	if ( (PINB & 0b00100000) && (PINC & 0b00010000) && (!(PINC & 0b00100000)) ) //orange
	{
		return 1;
	}
	if ( (PINB & 0b00100000) && (!(PINC & 0b00010000)) && (PINC & 0b00100000) ) //yellow
	{
		return 2;
	}
	if ( (!(PINB & 0b00100000)) && (PINC & 0b00010000) && (!(PINC & 0b00100000)) )//green
	{
		return 3; 
	}
	if ( (!(PINB & 0b00100000)) && (!(PINC & 0b00010000)) && (PINC & 0b00100000) )//blue
	{
		return 4; 
	}
	else if ( (!(PINB & 0b00100000)) && (PINC & 0b00010000) && (PINC & 0b00100000) ) //brown
	{
		return 5; 
	}
	else if ( (!(PINB & 0b00100000)) && (!(PINC & 0b00010000)) && (!(PINC & 0b00100000)) ) //nothing
	{
		return 6;
	}
	else //error
	{
		return 10;
	}
}
int decideLocation(int color)
{
	for (int location = 0; location < 2; location++)
	{
		if  (pillOrganization[color][location] > 0)
		{
			pillOrganization[color][location] --;
			totalPillCount --;
			return location;
		}
	}
	return 2; 

}
void actuateMotor(int location) //CHANGE TIMING
{
	if (location == 0) //AM
	{
		wait(1000); //SUBJECT TO CHANGE
		int num0 = 14;
		while(num0 > 0)
		{
			step2_CCW();
			wait(30);
			num0 --;
		}
		
		wait(1500);//SUBJECT TO CHANGE
		num0 = 14;
		while(num0 > 0)
		{
			step2_CW();
			wait(30);
			num0 --;
		}
	}
	
	else if(location == 1) //PM
	{
		wait(3000);
		int num1 = 14;
		while(num1 > 0)
		{
			step1_CCW();
			wait(30);
			num1 --;
		}
		
		wait(1500);
		num1 = 14;
		while(num1 > 0)
		{
			step1_CW();
			wait(30);
			num1 --;
		}
	}
} 

int main(void)
{
    //SET-UP
	wait(500); // LCD time to power on
	
	DDRB = 0b00001111; // motor to output, switches to input 
	DDRC = 0b00001111; //outputs connected to LCD (0,1,2,3)
	wait(20);
	DDRD = 0b11110011; // switches are cleared to input, LCD E & RS set to output
	
	lcdStart();
	wait(200); 
	
	EICRA = 0b00001010;// set to capture both switches' falling edge which corresponds to the moment when the switch is pressed
	EIMSK = 0b00000010; // enables INT0 & INT1
	sei(); //enable global interrupt
	
	// USER SELECTION (LCD + buttons)
	lcdString("How many red?");
	lcdString("                              AM:0   PM:0   ");
	wait(20);
	
	while (colorPhase < 6)
	{
		if (newPhase == 1)
		{
			printNewPhase(); //give LCD new display (for next color selection)
			newPhase = 0;
			lcdSetUpCommand(0b00000010); //return home
			lcdString("np");
		}
		
		if (!(PINB & 0b00010000)) //AM button. evaluates to true when button pressed
		{
			pillOrganization[colorPhase][0] ++;
			totalPillCount ++;
			lcdPrintColor();
			
			while(PINB & 0b00010000){}//gets stuck until button is released
			wait(50);//de bounce
		}
		
		if (!(PIND & 0b00000100)) //PM button. evaluates to true when button pressed
		{
			pillOrganization[colorPhase][1] ++;
			totalPillCount ++;
			lcdPrintColor();
			
			while(PIND & 0b00000100){}//gets stuck until button is released
			wait(50);//de bounce
		}
	}
	//while ((PIND & 0b00000100)){}; //THIS WORKS. gets stuck when button has not been pressed. wait until button is pressed
		
	//PILL SORTING
	lcdSetUpCommand(0b00000010); //return home
	lcdSetUpCommand(0b00000001); //clear display
	wait(200);
	
	lcdString("waiting");
	wait(100);
	int color;
	int location;
	
	while (totalPillCount > 0)
	{
		//while( (!(PINB & 0b00100000)) && (!(PINC & 0b00010000)) && (!(PINC & 0b00100000)) ) {} //gets stuck if any pins to arduino are low
		wait(12); 
		color = readColor();
		while (color > 5)
		{
			color = readColor(); //in case it messes up
		}
		lcdSetUpCommand(0b00000010); //return home
		lcdSetUpCommand(0b00000001); //clear display
		wait(2);
		lcdString("color read");
		
		location = decideLocation(color);
		actuateMotor(location);
	}
	
	lcdString("Sorting done!");
	
}

ISR(INT1_vect)
{
	wait(50); //debounce
	
	colorPhase ++;
	newPhase = 1;
	
	int i;
	while(PIND & 0b00001000)// wait for button to stop being pressed
	{
		i=0;
	}
	EIFR = 0b00000010; //clear INT1 interrupt flag
}
