/*
 * EE120BFinalProject.c
 *
 * Created: 6/3/2018 9:06:45 PM
 * Author : winso
 */ 

#include <avr/io.h>
#include <bit.h>
#include <avr/eeprom.h>
#include "timer.h"
#include "nokia5110.c"
#include "PWM.c"

//Notes for Alarm
#define E4 329.63
#define G4 392.00
#define D4 293.66
#define C4 277.18
#define B3 246.94
//global variables
unsigned char AlarmSetDisplay = 0;
unsigned char TimeSetDisplay = 0;
unsigned char AlarmOn = 0;
unsigned char AlarmOff = 0;
unsigned short Time = 42200;
unsigned short AlarmTime = 0;
unsigned char AlarmAMPM = 0;
unsigned char TimeAMPM = 0;
unsigned char timeChange = 0;
unsigned char SMTimerAMPM = 0;
unsigned short newTime = 3600;
unsigned char MotorOn = 0;
unsigned char PlaySpeaker = 0;

//Helper Functions
void DisplayTime (unsigned short val){
	unsigned char hour1 = val/36000;
	val = val%36000;
	unsigned char hour2 = val/3600;
	val = val%3600;
	unsigned char minute1 = val/600;
	val = val%600;
	unsigned char minute2 = val/60;
	val = val%60;
	unsigned char second1 = val/10;
	val = val%10;
	unsigned char second2 = val;
	
	//Offset by 48 for ASCII Digits
	hour1 = hour1 + 48;
	hour2 = hour2 + 48;
	minute1 = minute1 + 48;
	minute2 = minute2 + 48;
	second1 = second1 + 48;
	second2 =second2 + 48;
	
		nokia_lcd_set_cursor(3,25);
		nokia_lcd_write_char(hour1,2);
		
		nokia_lcd_set_cursor(15,25);
		nokia_lcd_write_char(hour2,2);
		
		nokia_lcd_set_cursor(30,25);
		nokia_lcd_write_char(minute1,2);
		
		nokia_lcd_set_cursor(42,25);
		nokia_lcd_write_char(minute2,2);
		
		nokia_lcd_set_cursor(57,25);
		nokia_lcd_write_char(second1,2);
		
		nokia_lcd_set_cursor(69,25);
		nokia_lcd_write_char(second2,2);
		nokia_lcd_render();
	
}

void AlarmTimeDisplay(unsigned short val , unsigned char val2){
	unsigned char hour1 = val/36000;
	val = val%36000;
	unsigned char hour2 = val/3600;
	val = val%3600;
	unsigned char minute1 = val/600;
	val = val%600;
	unsigned char minute2 = val/60;
	val = val%60;
	unsigned char second1 = val/10;
	val = val%10;
	unsigned char second2 = val;
	
	//Offset by 48 for ASCII Digits
	hour1 = hour1 + 48;
	hour2 = hour2 + 48;
	minute1 = minute1 + 48;
	minute2 = minute2 + 48;
	second1 = second1 + 48;
	second2 =second2 + 48;
	
	nokia_lcd_set_cursor(3,13);
	nokia_lcd_write_char(hour1,1);
	
	nokia_lcd_set_cursor(10,13);
	nokia_lcd_write_char(hour2,1);
	
	nokia_lcd_set_cursor(20,13);
	nokia_lcd_write_char(minute1,1);
	
	nokia_lcd_set_cursor(27,13);
	nokia_lcd_write_char(minute2,1);
	
	nokia_lcd_set_cursor(37,13);
	nokia_lcd_write_char(second1,1);
	
	nokia_lcd_set_cursor(44,13);
	nokia_lcd_write_char(second2,1);
	
	nokia_lcd_set_cursor(51,13);
	
	if(val2 == 0){
		nokia_lcd_write_string("AM" ,1);
	}
	else{
		nokia_lcd_write_string("PM",1);
	}
}


void AMPMHelperFunction(unsigned char val){ //sets AMPM for Display
				if(val == 0){
					nokia_lcd_set_cursor(69,0);
					nokia_lcd_write_string("AM",1);
				}
				else{
					nokia_lcd_set_cursor(69,0);
					nokia_lcd_write_string("PM",1);
				}
}
// Returns '\0' if no key pressed, else returns char '1', '2', ... '9', 'A', ...
// If multiple keys pressed, returns leftmost-topmost one
// Keypad must be connected to port C
/* Keypad arrangement
        PC4 PC5 PC6 PC7
   col  1   2   3   4
row
PC0 1   1 | 2 | 3 | A
PC1 2   4 | 5 | 6 | B
PC2 3   7 | 8 | 9 | C
PC3 4   * | 0 | # | D
*/
unsigned char GetKeypadKey() {

	PORTA = 0xEF; // Enable col 4 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('1'); }
	if (GetBit(PINA,1)==0) { return('4'); }
	if (GetBit(PINA,2)==0) { return('7'); }
	if (GetBit(PINA,3)==0) { return('*'); }

	// Check keys in col 2
	PORTA = 0xDF; // Enable col 5 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('2'); }
	if (GetBit(PINA,1)==0) { return('5'); }
	if (GetBit(PINA,2)==0) { return('8'); }
	if (GetBit(PINA,3)==0) { return('0'); }
	

	// Check keys in col 3
	PORTA = 0xBF; // Enable col 6 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
		if (GetBit(PINA,0)==0) { return('3'); }
		if (GetBit(PINA,1)==0) { return('6'); }
		if (GetBit(PINA,2)==0) { return('9'); }
		if (GetBit(PINA,3)==0) { return('#'); }
	
	// Check keys in col 4	
	PORTA = 0x7F;
	asm("nop");
		if (GetBit(PINA,0)==0) { return('A'); }
		if (GetBit(PINA,1)==0) { return('B'); }
		if (GetBit(PINA,2)==0) { return('C'); }
		if (GetBit(PINA,3)==0) { return('D'); }

	return('\0'); // default value

}


enum SMDisplayStates{SMDisplay_Init, SMDisplay_Wait, SMDisplay_AlarmOn, SMDisplay_AlarmOff, SMDisplay_Hold} SMDisplayState;
	
void SMDisplay(){
	switch(SMDisplayState){ //Transitions
		case SMDisplay_Init:
			SMDisplayState = SMDisplay_Wait;
			break;
		case SMDisplay_Wait:
			if((AlarmSetDisplay || TimeSetDisplay)){
				SMDisplayState = SMDisplay_Hold;
			}
			else if(!(AlarmSetDisplay || TimeSetDisplay) && (AlarmOn == 1)){
				SMDisplayState = SMDisplay_AlarmOn;
			}
			else if(AlarmOn == 0 &&  !(TimeSetDisplay || AlarmSetDisplay)){
				SMDisplayState = SMDisplay_AlarmOff;
			}
			break;
		case SMDisplay_AlarmOn:
			SMDisplayState = SMDisplay_Wait;
			break;
		case SMDisplay_AlarmOff:
			SMDisplayState = SMDisplay_Wait;
			break;
		case SMDisplay_Hold:
			if(TimeSetDisplay || AlarmSetDisplay){
				SMDisplayState = SMDisplay_Hold;
			}
			else{
				SMDisplayState = SMDisplay_Wait;
			}
			break;
		default:
		SMDisplayState = SMDisplay_Init;
		break;
	}
	switch(SMDisplayState){
		case SMDisplay_Init:
			break;
		case SMDisplay_Wait:
			if(AlarmOn == 1){
				AlarmTimeDisplay(AlarmTime , AlarmAMPM);
			}
			AMPMHelperFunction(SMTimerAMPM);
			DisplayTime(Time);
			break;
		case SMDisplay_AlarmOn:
			nokia_lcd_clear();
			AlarmTimeDisplay(AlarmTime, AlarmAMPM);
			AMPMHelperFunction(SMTimerAMPM);
			nokia_lcd_set_cursor(4,5);
			nokia_lcd_write_string("ALARM ON",1);
			DisplayTime(Time);
			break;
		case SMDisplay_AlarmOff:
			nokia_lcd_clear();
			AMPMHelperFunction(SMTimerAMPM);		
			nokia_lcd_set_cursor(4,5);
			nokia_lcd_write_string("ALARM OFF",1);
			DisplayTime(Time);
			break;
		case SMDisplay_Hold:
			break;	
	}	
}

enum SMTimerStates{SMTimer_Init, SMTimer_Count, SMTimer_Increment, SMTimer_timeChange} SMTimerState;
void SMTimer(){
	static unsigned char i = 0;
	
	switch(SMTimerState){ //transitions
		case SMTimer_Init:
			SMTimerState = SMTimer_Count;
			break;
		case SMTimer_Count:
			if(timeChange){
				SMTimerState = SMTimer_timeChange;
			}
			else if((i <= 4) && (!timeChange)){
				SMTimerState = SMTimer_Count;
			}
			else if((i > 4) && !timeChange){
				SMTimerState = SMTimer_Increment;
			}
			break;
		case SMTimer_Increment:
			if(!timeChange){
				SMTimerState = SMTimer_Count;
			}
			else if(timeChange){
				SMTimerState = SMTimer_timeChange;
			}
			break;
			
		case SMTimer_timeChange:
			SMTimerState = SMTimer_Count;
			break;
		default:
			SMTimerState = SMTimer_Init;
			break;
	}
	switch(SMTimerState){
		case SMTimer_Init:
			break;
		case SMTimer_Count: 
			++i;
			break;
		case SMTimer_Increment:
			i = 0;
			if(Time == 43200){
				if(SMTimerAMPM == 0){
					SMTimerAMPM = 1;
				}
				else{
					SMTimerAMPM = 0;
				}
			}
			if(Time == 46799){
				Time = 3600;
			}
			else{
				++Time;
			}
			++i;
			break;
		case SMTimer_timeChange:
			Time = newTime;
			i=1;
			if(newTime == 43200){
				if(TimeAMPM == 0){
					TimeAMPM = 1;
				}
				else{
					TimeAMPM = 0;
				}
			}
			SMTimerAMPM = TimeAMPM;
			break;
	}
}

enum SMAlarmSetStates{SMAlarmSet_Init, SMAlarmSet_Wait, SMAlarmSet_WaitRelease, SMAlarmSet_CodeEnterWait, SMAlarmSet_CodeEnterRelease, SMAlarmSet_ValidCheck,SMAlarmSet_AMPM, SMAlarmSet_Flush1, SMAlarmSet_Flush2 ,SMAlarmSet_Clear} SMAlarmSetState;
void SMAlarmSet(){
	static unsigned char i = 0;
	static unsigned char hour1,hour2,minute1,minute2,second1,second2 = 0;
	static unsigned short temp;
	switch(SMAlarmSetState){
		case SMAlarmSet_Init:
			SMAlarmSetState = SMAlarmSet_Wait;
			break;
		case SMAlarmSet_Wait: 
			if(GetKeypadKey() != 'B'){
				SMAlarmSetState = SMAlarmSet_Wait;
				AlarmSetDisplay = 0;
			}
			else if(GetKeypadKey() == 'B' && TimeSetDisplay == 0){
				SMAlarmSetState = SMAlarmSet_WaitRelease;
				AlarmSetDisplay = 1;
			}
			break;
		case SMAlarmSet_WaitRelease:
			if(GetKeypadKey() != '\0'){
				SMAlarmSetState = SMAlarmSet_WaitRelease;
			}
			else if(GetKeypadKey() == '\0'){
				SMAlarmSetState = SMAlarmSet_CodeEnterWait;
				nokia_lcd_clear();
				nokia_lcd_render();
			}
			break;
		case SMAlarmSet_CodeEnterWait:
			if(i > 5){
				i=0;
				SMAlarmSetState = SMAlarmSet_ValidCheck;
			}
			else{
				nokia_lcd_set_cursor(0,0);
				nokia_lcd_write_string("Alarm Set" ,1);
				nokia_lcd_render();
				
				if(GetKeypadKey() != '1' && GetKeypadKey() != '2' && GetKeypadKey() != '3' && GetKeypadKey() != '4' && GetKeypadKey() != '5' && GetKeypadKey() != '6' && GetKeypadKey() != '7' && GetKeypadKey() != '8' && GetKeypadKey() != '9' && GetKeypadKey() != '0'){
					SMAlarmSetState = SMAlarmSet_CodeEnterWait;
				}
				else{ 
					if(i==0){
						hour1 = GetKeypadKey();
						nokia_lcd_set_cursor(3,25);
						nokia_lcd_write_char(hour1,2);
						nokia_lcd_render();
					}
					else if(i==1){
						hour2 = GetKeypadKey();
						nokia_lcd_set_cursor(15,25);
						nokia_lcd_write_char(hour2,2);
						nokia_lcd_render();
					}
					else if(i==2){
						minute1 = GetKeypadKey();					
						nokia_lcd_set_cursor(30,25);
						nokia_lcd_write_char(minute1,2);
						nokia_lcd_render();
					}
					else if(i==3){
						minute2 = GetKeypadKey();
						nokia_lcd_set_cursor(42,25);
						nokia_lcd_write_char(minute2,2);
						nokia_lcd_render();
					}
					else if(i==4){
						second1 = GetKeypadKey();
						nokia_lcd_set_cursor(57,25);
						nokia_lcd_write_char(second1,2);
						nokia_lcd_render();
					}
					else if(i==5){
						second2 = GetKeypadKey();
						nokia_lcd_set_cursor(69,25);
						nokia_lcd_write_char(second2,2);
						nokia_lcd_render();
					}
					++i;
					SMAlarmSetState = SMAlarmSet_CodeEnterRelease;
				}
			}
			break;
		case SMAlarmSet_CodeEnterRelease:
			if(GetKeypadKey() != '\0'){
				SMAlarmSetState = SMAlarmSet_CodeEnterRelease;
			}
			else{
				SMAlarmSetState = SMAlarmSet_CodeEnterWait;
			}
			break;
		case SMAlarmSet_ValidCheck:
				if(hour1 != '0' || hour1 != '1'){
					SMAlarmSetState = SMAlarmSet_Flush1;
				}
				if(hour1 == '0' || hour1 == '1'){
					temp = (hour1 - 48) * 36000 + (hour2 - 48) * 3600 + (minute1 - 48) * 600 + (minute2 - 48) * 60 + (second1 - 48) * 10 + (second2 - 48);
					if(3600 <= temp && temp  <= 46799){
						SMAlarmSetState = SMAlarmSet_Flush2;
					}
					else{
						SMAlarmSetState = SMAlarmSet_Flush1;
					}
				}
			break;
		case SMAlarmSet_Flush1:
			SMAlarmSetState = SMAlarmSet_CodeEnterWait;
			break;
		case SMAlarmSet_Flush2:
			SMAlarmSetState = SMAlarmSet_AMPM;
			break;
		case SMAlarmSet_AMPM:
			nokia_lcd_set_cursor(0,0);
			nokia_lcd_write_string("AM or PM?" ,1);
			nokia_lcd_render();
			if(GetKeypadKey() != 'C' && GetKeypadKey() != 'D'){
				SMAlarmSetState = SMAlarmSet_AMPM;
			}
			else {
				eeprom_update_word((uint16_t*)50, temp);
				if(GetKeypadKey() == 'C'){
					AlarmAMPM = 0;
					eeprom_update_byte((uint8_t*)100, 0);
				}
				else if(GetKeypadKey() == 'D'){
					AlarmAMPM = 1;
					eeprom_update_byte((uint8_t*)100, 1);
				}
				SMAlarmSetState = SMAlarmSet_Clear;
			}
			break;
		case SMAlarmSet_Clear:
			SMAlarmSetState = SMAlarmSet_Wait;
			break;
		default:
			SMAlarmSetState = SMAlarmSet_Init;
			break;
	}
	switch(SMAlarmSetState){
		case SMAlarmSet_Init:
			break;
		case SMAlarmSet_Wait:
			break;
		case SMAlarmSet_CodeEnterWait:
			break;
		case SMAlarmSet_CodeEnterRelease:
			break;
		case SMAlarmSet_ValidCheck:
			break;
		case SMAlarmSet_Flush1:
			nokia_lcd_clear();
			nokia_lcd_set_cursor(5,10);
			nokia_lcd_write_string("INVALID" ,2);
			nokia_lcd_render();		
			nokia_lcd_clear();
			nokia_lcd_render();
			break;
		case SMAlarmSet_Flush2:
			AlarmTime = temp;
			break;
		case SMAlarmSet_AMPM:
			break;
		case SMAlarmSet_Clear:
			nokia_lcd_clear();
			nokia_lcd_render();
			break;
	}
}

enum SMTimeSetStates{SMTimeSet_Init, SMTimeSet_Wait, SMTimeSet_WaitRelease, SMTimeSet_CodeEnterWait, SMTimeSet_CodeEnterRelease, SMTimeSet_ValidCheck,SMTimeSet_AMPM, SMTimeSet_Flush1, SMTimeSet_Flush2 ,SMTimeSet_Clear} SMTimeSetState;
void SMTimeSet(){
	static unsigned char i = 0;
	static unsigned char hour1,hour2,minute1,minute2,second1,second2 = 0;
	static unsigned short temp;
	switch(SMTimeSetState){
		case SMTimeSet_Init:
		SMTimeSetState = SMTimeSet_Wait;
		break;
		case SMTimeSet_Wait:
		if(GetKeypadKey() != 'A'){
			SMTimeSetState = SMTimeSet_Wait;
			TimeSetDisplay = 0;
		}
		else if(GetKeypadKey() == 'A' && AlarmSetDisplay == 0){
			SMTimeSetState = SMTimeSet_WaitRelease;
			TimeSetDisplay = 1;
		}
		break;
		case SMTimeSet_WaitRelease:
		if(GetKeypadKey() != '\0'){
			SMTimeSetState = SMTimeSet_WaitRelease;
		}
		else if(GetKeypadKey() == '\0'){
			SMTimeSetState = SMTimeSet_CodeEnterWait;
			nokia_lcd_clear();
			nokia_lcd_render();
		}
		break;
		case SMTimeSet_CodeEnterWait:
		if(i > 5){
			i=0;
			SMTimeSetState = SMTimeSet_ValidCheck;
		}
		else{
			nokia_lcd_set_cursor(0,0);
			nokia_lcd_write_string("Time Set" ,1);
			nokia_lcd_render();
			
			if(GetKeypadKey() != '1' && GetKeypadKey() != '2' && GetKeypadKey() != '3' && GetKeypadKey() != '4' && GetKeypadKey() != '5' && GetKeypadKey() != '6' && GetKeypadKey() != '7' && GetKeypadKey() != '8' && GetKeypadKey() != '9' && GetKeypadKey() != '0'){
				SMTimeSetState = SMTimeSet_CodeEnterWait;
			}
			else{
				if(i==0){
					hour1 = GetKeypadKey();
					nokia_lcd_set_cursor(3,25);
					nokia_lcd_write_char(hour1,2);
					nokia_lcd_render();
				}
				else if(i==1){
					hour2 = GetKeypadKey();
					nokia_lcd_set_cursor(15,25);
					nokia_lcd_write_char(hour2,2);
					nokia_lcd_render();
				}
				else if(i==2){
					minute1 = GetKeypadKey();
					nokia_lcd_set_cursor(30,25);
					nokia_lcd_write_char(minute1,2);
					nokia_lcd_render();
				}
				else if(i==3){
					minute2 = GetKeypadKey();
					nokia_lcd_set_cursor(42,25);
					nokia_lcd_write_char(minute2,2);
					nokia_lcd_render();
				}
				else if(i==4){
					second1 = GetKeypadKey();
					nokia_lcd_set_cursor(57,25);
					nokia_lcd_write_char(second1,2);
					nokia_lcd_render();
				}
				else if(i==5){
					second2 = GetKeypadKey();
					nokia_lcd_set_cursor(69,25);
					nokia_lcd_write_char(second2,2);
					nokia_lcd_render();
				}
				++i;
				SMTimeSetState = SMTimeSet_CodeEnterRelease;
			}
		}
		break;
		case SMTimeSet_CodeEnterRelease:
		if(GetKeypadKey() != '\0'){
			SMTimeSetState = SMTimeSet_CodeEnterRelease;
		}
		else{
			SMTimeSetState = SMTimeSet_CodeEnterWait;
		}
		break;
		case SMTimeSet_ValidCheck:
		if(hour1 != '0' || hour1 != '1'){
			SMTimeSetState = SMTimeSet_Flush1;
		}
		if(hour1 == '0' || hour1 == '1'){
			temp = (hour1 - 48) * 36000 + (hour2 - 48) * 3600 + (minute1 - 48) * 600 + (minute2 - 48) * 60 + (second1 - 48) * 10 + (second2 - 48);
			if(3600 <= temp && temp  <= 46799){
				SMTimeSetState = SMTimeSet_Flush2;
			}
			else{
				SMTimeSetState = SMTimeSet_Flush1;
			}
		}
		break;
		case SMTimeSet_Flush1:
		SMTimeSetState = SMTimeSet_CodeEnterWait;
		break;
		case SMTimeSet_Flush2:
		SMTimeSetState = SMTimeSet_AMPM;
		break;
		case SMTimeSet_AMPM:
		nokia_lcd_set_cursor(0,0);
		nokia_lcd_write_string("AM or PM?" ,1);
		nokia_lcd_render();
		if(GetKeypadKey() != 'C' && GetKeypadKey() != 'D'){
			SMTimeSetState = SMTimeSet_AMPM;
		}
		else {
			if(GetKeypadKey() == 'C'){
				TimeAMPM = 0;
			}
			if(GetKeypadKey() == 'D'){
				TimeAMPM = 1;
			}
			SMTimeSetState = SMTimeSet_Clear;
		}
		break;
		case SMTimeSet_Clear:
		SMTimeSetState = SMTimeSet_Wait;
		break;
		default:
		SMTimeSetState = SMTimeSet_Init;
		break;
	}
	switch(SMTimeSetState){
		case SMTimeSet_Init:
		break;
		case SMTimeSet_Wait:
		timeChange = 0;
		break;
		case SMTimeSet_CodeEnterWait:
		break;
		case SMTimeSet_CodeEnterRelease:
		break;
		case SMTimeSet_ValidCheck:
		break;
		case SMTimeSet_Flush1:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(5,10);
		nokia_lcd_write_string("INVALID" ,2);
		nokia_lcd_render();
		nokia_lcd_clear();
		nokia_lcd_render();
		temp = 0;
		hour1 = 0;
		hour2 = 0;
		minute1 = 0;
		minute2 = 0;
		second1 = 0;
		second2 = 0;
		break;
		case SMTimeSet_Flush2:
		newTime = temp;
		temp = 0;
		hour1 = 0;
		hour2 = 0;
		minute1 = 0;
		minute2 = 0;
		second1 = 0;
		second2 = 0;
		break;
		case SMTimeSet_AMPM:
		break;
		case SMTimeSet_Clear:
		timeChange  = 1;
		nokia_lcd_clear();
		nokia_lcd_render();
		break;
	}
}

enum SMAlarmStates{SMAlarm_Init, SMAlarm_Wait, SMAlarm_Press, SMAlarm_Release , SMAlarm_Off}SMAlarmState;

void SMAlarm(){
	unsigned char button = ~PIND & 0x01;
	switch(SMAlarmState){
		case SMAlarm_Init:
			SMAlarmState = SMAlarm_Wait;
			break;
		case SMAlarm_Wait:
			if(button == 0 && !AlarmOff){
				SMAlarmState = SMAlarm_Wait;
			}
			else if(button == 0 && AlarmOff){
				SMAlarmState = SMAlarm_Off;
			}
			else if(button == 1 && !AlarmSetDisplay && !TimeSetDisplay){ //Allows button press if you are not attempting to change the time or the alarm
				SMAlarmState = SMAlarm_Press;
			}
			break;
		case SMAlarm_Press:
			SMAlarmState = SMAlarm_Release;
			break;
		case SMAlarm_Release:
			if(button == 1){
				SMAlarmState = SMAlarm_Release;
			}
			else{
				SMAlarmState = SMAlarm_Wait;
			}
			break;
		case SMAlarm_Off:
			SMAlarmState = SMAlarm_Wait;
			break;
	}
	switch(SMAlarmState){
		case SMAlarm_Init:
			AlarmOn =0;
			break;
		case SMAlarm_Wait:
			break;
		case SMAlarm_Press:
			if(AlarmOn == 0 && (PIND & 0x02) != 0x02){
				AlarmOn = 1;
			}
			else{
				AlarmOn = 0;
			}
			break;
		case SMAlarm_Release:
			break;
		case SMAlarm_Off:
			AlarmOn = 0;
			break;
	}
}

enum SMMainStates{SMMain_Init, SMMain_Wait, SMMain_Compare, SMMain_AlarmGo} SMMainState;

void SMMain(){
	static unsigned char match = 0;
	switch(SMMainState){
		case SMMain_Init:
			SMMainState = SMMain_Wait;
			break;
		case SMMain_Wait:
			if(AlarmOn == 0){
				SMMainState = SMMain_Wait;
			}
			else if(AlarmOn == 1){
				SMMainState = SMMain_Compare;
			}
			break;
		case SMMain_Compare:
			if(AlarmOn == 0){
				SMMainState = SMMain_Wait;
			}
			else if(AlarmOn == 1 && !match){
				SMMainState = SMMain_Compare;
			}
			else if(AlarmOn == 1 && match){
				SMMainState = SMMain_AlarmGo;
			}
			break;
		case SMMain_AlarmGo:
			if(AlarmOff == 0){
				SMMainState = SMMain_AlarmGo;
			}
			else if(AlarmOff == 1){
				SMMainState =SMMain_Wait;
			}
			break;
	}
	switch(SMMainState){
		case SMMain_Init:
			break;
		case SMMain_Wait:
			PlaySpeaker = 0;
			MotorOn = 0;
			match = 0;
			break;
		case SMMain_Compare:
			if(Time == AlarmTime && SMTimerAMPM == AlarmAMPM){
				match = 1;
			}
			else {
				match = 0;
			}
			break;
		case SMMain_AlarmGo:
			MotorOn = 1;
			PlaySpeaker = 1;
	}
}

enum SMMotorStates{ SMMotor_Init, SMMotor_Wait, SMMotor_On, SMMotor_Off, SMMotor_Returned} SMMotorState;
	
void SMMotor(){
	unsigned char sensor = PIND & 0x02;
	static unsigned char count = 0;
	switch(SMMotorState){
		case SMMotor_Init:
			SMMotorState = SMMotor_Wait;
			break;
		case SMMotor_Wait:
			if(MotorOn == 0){
				SMMotorState = SMMotor_Wait;
			}
			else if(MotorOn == 1){
				SMMotorState = SMMotor_On;
			}
			break;
		case SMMotor_On:
			if(count < 13){
				SMMotorState = SMMotor_On;
				++count;
			}
			else{
				count = 0;
				SMMotorState = SMMotor_Off;
			}
			break;
		case SMMotor_Off:
			if(sensor == 0x02){
				SMMotorState = SMMotor_Off;
			}
			else{
				SMMotorState = SMMotor_Returned;
			}
			break;
		case SMMotor_Returned:
			SMMotorState = SMMotor_Wait;
			break;
	}
	switch(SMMotorState){
		case SMMotor_Init:
			break;
		case SMMotor_Wait:
			AlarmOff = 0;
			break;
		case SMMotor_On:
			PORTB = 0x03;
			break;
		case SMMotor_Off:
			PORTB = 0x00;
			break;
		case SMMotor_Returned:
			AlarmOff = 1;
	}
}

enum SMPlaySpeakerStates{ SMPlaySpeaker_Init, SMPlaySpeaker_Wait, SMPlaySpeaker_Play0, SMPlaySpeaker_Play00, SMPlaySpeaker_Play1,SMPlaySpeaker_Play01,SMPlaySpeaker_Play2,SMPlaySpeaker_Play02,SMPlaySpeaker_Play3,SMPlaySpeaker_Play03,SMPlaySpeaker_Play4,SMPlaySpeaker_Play04,SMPlaySpeaker_Play5,SMPlaySpeaker_Play05, SMPlaySpeaker_Play005,SMPlaySpeaker_Play6,SMPlaySpeaker_Play06,SMPlaySpeaker_Play7,SMPlaySpeaker_Play07,SMPlaySpeaker_Play8,SMPlaySpeaker_Play08,SMPlaySpeaker_Play9,SMPlaySpeaker_Play09,SMPlaySpeaker_Play10,SMPlaySpeaker_Play010,SMPlaySpeaker_Play11,SMPlaySpeaker_Play011,SMPlaySpeaker_Play12,SMPlaySpeaker_Play012,SMPlaySpeaker_Play13,SMPlaySpeaker_Play013,SMPlaySpeaker_Play14,SMPlaySpeaker_Play014,SMPlaySpeaker_Play15,SMPlaySpeaker_Play015} SMPlaySpeakerState;
	
void SMPlaySpeaker(){
	switch(SMPlaySpeakerState){
		case SMPlaySpeaker_Init:
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			break;
		case SMPlaySpeaker_Wait:
			if(PlaySpeaker == 0){
				SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
			}
			else if(PlaySpeaker == 1){
				SMPlaySpeakerState = SMPlaySpeaker_Play0;
			}
			break;
		case SMPlaySpeaker_Play0:
			if(PlaySpeaker == 0){
				SMPlaySpeakerState = SMPlaySpeaker_Wait;
				
			}
			else if(PlaySpeaker == 1){
				SMPlaySpeakerState = SMPlaySpeaker_Play00;
			}
			break;
		case SMPlaySpeaker_Play00:
			if(PlaySpeaker == 0){
				SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
			}
			else if(PlaySpeaker == 1){
				SMPlaySpeakerState = SMPlaySpeaker_Play1;
			}
			break;
		case SMPlaySpeaker_Play1:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play01;
		}
		break;
		case SMPlaySpeaker_Play01:
			if(PlaySpeaker == 0){
				SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
			}
			else if(PlaySpeaker == 1){
				SMPlaySpeakerState = SMPlaySpeaker_Play2;
		}
		break;
		case SMPlaySpeaker_Play2:
			if(PlaySpeaker == 0){
				SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
			}
			else if(PlaySpeaker == 1){
				SMPlaySpeakerState = SMPlaySpeaker_Play02;
			}
			break;
		case SMPlaySpeaker_Play02:
			if(PlaySpeaker == 0){
				SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
			}
			else if(PlaySpeaker == 1){
				SMPlaySpeakerState = SMPlaySpeaker_Play3;
			}
			break;
		case SMPlaySpeaker_Play3:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play03;
		}
		break;
		case SMPlaySpeaker_Play03:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play4;
		}
		break;
		case SMPlaySpeaker_Play4:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play04;
		}
		break;
		case SMPlaySpeaker_Play04:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play5;
		}
		break;
		case SMPlaySpeaker_Play5:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play05;
		}
		break;
		case SMPlaySpeaker_Play05:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play005;
		}
		break;
		case SMPlaySpeaker_Play005:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play06;
		}			
		case SMPlaySpeaker_Play6:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play06;
		}
		break;
		case SMPlaySpeaker_Play06:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play7;
		}
		break;
		case SMPlaySpeaker_Play7:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play07;
		}
		break;
		case SMPlaySpeaker_Play07:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play8;
		}
		break;
		case SMPlaySpeaker_Play8:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play08;
		}
		break;
		case SMPlaySpeaker_Play08:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play9;
		}
		break;
		case SMPlaySpeaker_Play9:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play09;
		}
		break;
		case SMPlaySpeaker_Play09:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play10;
		}
		break;
		case SMPlaySpeaker_Play10:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play010;
		}
		break;
		case SMPlaySpeaker_Play010:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play11;
		}
		break;
		case SMPlaySpeaker_Play11:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play011;
		}
		break;
		case SMPlaySpeaker_Play011:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play12;
		}
		break;
		case SMPlaySpeaker_Play12:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play012;
		}
		break;
		case SMPlaySpeaker_Play012:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play13;
		}
		break;
		case SMPlaySpeaker_Play13:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play013;
		}
		break;
		case SMPlaySpeaker_Play013:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play14;
		}
		break;
		case SMPlaySpeaker_Play14:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play014;
		}
		case SMPlaySpeaker_Play014:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play15;
		}
		break;
		case SMPlaySpeaker_Play15:
		if(PlaySpeaker == 0){
			SMPlaySpeakerState = SMPlaySpeaker_Wait;
			
		}
		else if(PlaySpeaker == 1){
			SMPlaySpeakerState = SMPlaySpeaker_Play015;
		}
		break;
		case SMPlaySpeaker_Play015:
		SMPlaySpeakerState = SMPlaySpeaker_Wait;
		break;
		default:
			SMPlaySpeakerState = SMPlaySpeaker_Init;
			break;
	}
	
	switch(SMPlaySpeakerState){
		case SMPlaySpeaker_Init:
			 break;
		case SMPlaySpeaker_Wait:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play0:
			set_PWM(E4);
			break;
		case SMPlaySpeaker_Play1:
			set_PWM(E4);
			break;
		case SMPlaySpeaker_Play2:
			set_PWM(G4);
			break;
		case SMPlaySpeaker_Play3:
			set_PWM(E4);
			break;
		case SMPlaySpeaker_Play4:
			set_PWM(D4);
			break;
		case SMPlaySpeaker_Play5:
			set_PWM(C4);
			break;
		case SMPlaySpeaker_Play6:
			set_PWM(B3);
			break;
		case SMPlaySpeaker_Play7:
			set_PWM(E4);
			break;
		case SMPlaySpeaker_Play8:
			set_PWM(E4);
			break;
		case SMPlaySpeaker_Play9:
			set_PWM(G4);
			break;
		case SMPlaySpeaker_Play10:
			set_PWM(E4);
			break;
		case SMPlaySpeaker_Play11:
			set_PWM(D4);
			break;
		case SMPlaySpeaker_Play12:
			set_PWM(C4);
			break;
		case SMPlaySpeaker_Play13:
			set_PWM(D4);
			break;
		case SMPlaySpeaker_Play14:
			set_PWM(C4);
			break;
		case SMPlaySpeaker_Play15:
			set_PWM(B3);
			break;
		case SMPlaySpeaker_Play00:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play01:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play02:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play03:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play04:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play05:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play005:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play06:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play07:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play08:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play09:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play010:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play011:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play012:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play013:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play014:
			set_PWM(0.0);
			break;
		case SMPlaySpeaker_Play015:
			set_PWM(0.0);
			break;
	}
}
int main(void)
{
    /* Replace with your application code */
	DDRA = 0xF0; PORTA = 0x0F; // PC7..4 outputs init 0s, PC3..0 inputs init 1s
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0x00; PORTD = 0xFF;
	TimerSet(250);
	TimerOn();
	nokia_lcd_init();
	nokia_lcd_clear();
	PWM_on();
	AlarmTime = eeprom_read_word((uint16_t*)50);
	AlarmAMPM = eeprom_read_byte((uint8_t*)100);
    while (1) 
    {	
		SMTimer();
		SMTimeSet();
		SMAlarmSet();
		SMAlarm();
		SMMain();
		SMMotor();
		SMPlaySpeaker();
		SMDisplay();
		
		while(!TimerFlag);
		TimerFlag = 0;
    }
}