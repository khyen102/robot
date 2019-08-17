#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "ADC.h"

// volatile 뜻은 변덕스러움 변수가 마음대로 바뀔떄 그거 방지해주는 함수



/*****************************스위치 시작****************************/									// SWITCH
#define Switch_DDR DDRA
#define Switch_CLR 0x00

#define SW_0 !(PINA & 0x01)		//안되면 not 없애기
#define SW_1 !(PINA & 0x02)
#define SW_2 !(PINA & 0x04)
#define SW_3 !(PINA & 0x08)
#define SW_4 !(PINA & 0x10)
#define SW_5 !(PINA & 0x20)
#define SW_6 !(PINA & 0x40)
#define SW_7 !(PINA & 0x80)
/*****************************스위치 끝****************************/

/*****************************LCD 시작*****************************/										// LCD
#define LCD_DATA		PORTC
#define LCD_CTRL		PORTB
#define LCD_DATA_DDR	DDRC
#define LCD_CTRL_DDR	DDRB

//*****************************
//		PB7		PB6		PB5
//		EN		RW		RS
//*****************************

#define LCD_RS	0x20
#define LCD_RW	0x40
#define LCD_EN	0x80

void LCD_data(char data)
{
	LCD_DATA = data;
	LCD_CTRL |= LCD_RS | LCD_EN;
	_delay_us(1);
	LCD_CTRL &= ~LCD_EN;
	LCD_CTRL &= ~LCD_RS;
	_delay_us(50);
}
void LCD_command(char command)
{
	LCD_DATA = command;
	LCD_CTRL |= LCD_EN;
	_delay_us(1);
	LCD_CTRL &= ~LCD_EN;
	_delay_ms(5);
}
void LCD_STR(char *str)	//ASCII문자열 출력
{
	while (*str)
		LCD_data(*str++);
}
void LCD_pos(int y, int x)
{
	LCD_command(0x80 | (x + y * 0x40));	//1000xxxx=첫째줄, 1100xxxx=둘째줄
}
void LCD_clear(void)
{
	LCD_command(1);	//00000001=clear
}

void LCD_init(void)	//LCD 초기화
{
	LCD_DATA_DDR = 0xFF;
	LCD_CTRL_DDR |= 0xE0;//				0/1				  0/1
	LCD_command(0x38);//0011NF00	N:1 or 2line	F:5X11 or 5X8
	LCD_command(0x0C);//000011CB	C:커서 on/off	B:깜빡임 on/off
	LCD_command(0x06);//increment mode, entire shift off
	LCD_clear();
}

void LCD_out(int y, int x, char *str)	//세로 가로 데이터
{
	LCD_pos(y, x);
	LCD_STR(str);
}

void LCD_numout(int y, int x, int n)	//세로 가로 숫자
{
	char str[16];
	LCD_pos(y, x);
	sprintf(str, "%4d", n);
	LCD_STR(str);
}
/*****************************LCD 끝*****************************/

/*****************************UART 시작**************************/									// UART, 블루투스, 크루즈
#define BT_ACCEPT UCSR0B = 0x98
#define BT_REJECT UCSR0B = 0x18

//-----------UART0-------------블루투스//

volatile int bt_count = 0;
volatile char bt_data[4] = { 0 };
volatile int X_axis = 0, Y_axis = 0;
volatile char bt = 0;//청소로봇용
volatile char bt1 = 'O';
void serial0_init()	//Baudrate 9600, 송수신 on
{
	UCSR0A = 0x00;	//
	UCSR0B = 0x98;	//10000000 : Rx complete  01000000 : Tx complete  00100000 : UDR Empty
	UCSR0C = 0x06;	//8bit
	UBRR0H = 0x00;
	UBRR0L = 103;
	//16MHz Baud	2400	4800	9600	14400	19200	28800	38400
	//				416		207		103		68		51		34		25
}
char rx0_char(void)
{
	while (!(UCSR0A & 0x80));
	return UDR0;
}
void tx0_char(char tx_data)
{
	while (!(UCSR0A & 0x20));
	UDR0 = tx_data;
}
void tx0_string(char *str)
{
	while (*str)
	tx0_char(*str++);
}
void tx_Axis(unsigned int X, unsigned int Y)
{
	tx0_char((char)(X >> 8));
	tx0_char((char)(X % 0x100));
	tx0_char((char)(Y >> 8));
	tx0_char((char)(Y % 0x100));
}
ISR(USART0_RX_vect)
{
	bt=UDR0; 
}

//-----------UART1-----------크루즈

void serial1_init()	//Baudrate 115200, 송수신 on
{
	UCSR1A = 0x00;
	UCSR1B = 0x98;	//10000000 : Rx complete  01000000 : Tx complete  00100000 : UDR Empty
	UCSR1C = 0x06;
	UBRR1H = 0x00;
	UBRR1L = 8;
	//16MHz Baud	2400	4800	9600	14400	19200	28800	38400
	//				416		207		103		68		51		34		25
}
unsigned char rx1_char(void)
{
	while (!(UCSR1A & 0x80));
	return UDR1;
}
void tx1_char(char tx_data)
{
	while (!(UCSR1A & 0x20));
	UDR1 = tx_data;
}
void tx1_string(char *str)
{
	while (*str)
	tx1_char(*str++);
}

volatile int cruiz_count = 0;
volatile signed short gRate = 0;
volatile short int gAngle = 0;
volatile short int rate;
volatile short int angle;
volatile short check_sum;
volatile char data_string[8] = { 0 };

ISR(USART1_RX_vect)  //앵글값 정의내려준거
{
	data_string[cruiz_count] = UDR1;
	cruiz_count++;
	
	AngTemp = gAngle;
	
	if(AngFlag == 0)
	{
		SumAng = gAngle +359;
	}
	if(AngFlag == 1)
	{
		SumAng = gAngle;
	}
	if(AngFlag == 2)
	{
		SumAng = gAngle - 359;
	}

	if(cruiz_count == 8)
	{
		rate = (data_string[2] & 0xFF) | ((data_string[3] << 8) & 0xFF00);
		angle = (data_string[4] & 0xFF) | ((data_string[5] << 8) & 0XFF00);
		//Verify checksum
		check_sum = 0xFFFF + rate + angle;
		//Scale and store data
		gRate = rate / 100;
		gAngle = angle / 100;
		cruiz_count = 0;
	}
	
	DifAng = AngTemp - gAngle;
	if(DifAng >350)
	{
		AngFlag -= 1;
		//SumAng = gAngle + 360;
	}
	if(DifAng <-350)
	{
		AngFlag += 1;
		//SumAng = gAngle - 360;
	}
}
/***************************UART 끝**************************/
int main ()
{
	
}