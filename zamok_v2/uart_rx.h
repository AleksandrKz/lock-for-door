//ATtiny88

#ifndef uart_rx_h_
#define uart_rx_h_

#include <avr/io.h>
#include <avr/interrupt.h>

/*
*	���� ������������� ����� � ���� ������ ������� ����� ��������������
*	��� ���������� � �������� UART.
*/

//#define TXPORT PORTB		// ��� ����� ��� ��������
#define RXPORT PIND		// ��� ����� �� �����
//#define TXDDR DDRB		// ������� ����������� ����� �� ��������
#define RXDDR DDRD		// ������� ����������� ����� �� �����
//#define TXD 0			// ����� ���� ����� ��� ������������� �� ��������
#define RXD 3			// ����� ���� ����� ��� ������������� �� �����

/*
*	���� �������� ���������, ������������ �������� �������� ������ (�������)
*	������ BAUD_DIV �������������� ��������� �������:
*	BAUD_DIV = (CPU_CLOCK / DIV) / BAUD_RATE
*	��� CPU_CLOCK - �������� ������� �����������, BAUD_RATE - �������� �������� UART,
*	� DIV - �������� �������� ������� �������, ���������� ��������� TCCR0B.
*	��������, �������� ������� 9.6 ���, �������� �� 8, �������� ����� 9600 ���:
*	BAUD_DIV = (9 600 000 / 8) / 9600 = 125 (0x7D).
*	BAUD_DIV = (8 000 000 / 64) / 9600 = 13 (0x0D).
*	BAUD_DIV = (8 000 000 / 8) / 9600 = 104 (0x68).
*/

//#define T_DIV		0x01	// DIV = 1
#define T_DIV		0x02	// DIV = 8
//#define T_DIV		0x03	// DIV = 64
//#define T_DIV		0x04	// DIV = 256
//#define BAUD_DIV	0x68	// �������� = 9600 ���

#define BAUD_DIV 100

/*
*	���� ���� ���������� ���������� ���������� � ������� ��� ������ UART
*/

//volatile uint16_t txbyte;
volatile uint8_t rxbyte;
//volatile uint8_t txbitcount;
volatile uint8_t rxbitcount;
	
extern volatile uint8_t timeout;
extern volatile uint8_t fGetId;
extern uint8_t TimerTempCount;
extern void clear_buffer(uint8_t*, uint8_t);

void uart_init();
//void uart_send(uint8_t tb);
int8_t uart_recieve(uint8_t*);

int8_t GetId(uint8_t*, uint8_t);


#endif  /* uart_rx_h_ */