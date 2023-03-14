//ATtiny88

#ifndef uart_rx_h_
#define uart_rx_h_

#include <avr/io.h>
#include <avr/interrupt.h>

/*
*	Ќиже настраиваютс€ порты и пины портов которые будут использоватьс€
*	как передатчик и приемник UART.
*/

//#define TXPORT PORTB		// »м€ порта дл€ передачи
#define RXPORT PIND		// »м€ порта на прием
//#define TXDDR DDRB		// –егистр направлени€ порта на передачу
#define RXDDR DDRD		// –егистр направлени€ порта на прием
//#define TXD 0			// Ќомер бита порта дл€ использовани€ на передачу
#define RXD 3			// Ќомер бита порта дл€ использовани€ на прием

/*
*	Ќиже задаютс€ константы, определ€ющие скорость передачи данных (бодрейт)
*	расчет BAUD_DIV осуществл€етс€ следующим образом:
*	BAUD_DIV = (CPU_CLOCK / DIV) / BAUD_RATE
*	где CPU_CLOCK - тактова€ частота контроллера, BAUD_RATE - желаема€ скорость UART,
*	а DIV - значение делител€ частоты таймера, задающеес€ регистром TCCR0B.
*	Ќапример, тактова€ частота 9.6 ћ√ц, делитель на 8, скорость порта 9600 бод:
*	BAUD_DIV = (9 600 000 / 8) / 9600 = 125 (0x7D).
*	BAUD_DIV = (8 000 000 / 64) / 9600 = 13 (0x0D).
*	BAUD_DIV = (8 000 000 / 8) / 9600 = 104 (0x68).
*/

//#define T_DIV		0x01	// DIV = 1
#define T_DIV		0x02	// DIV = 8
//#define T_DIV		0x03	// DIV = 64
//#define T_DIV		0x04	// DIV = 256
//#define BAUD_DIV	0x68	// —корость = 9600 бод

#define BAUD_DIV 100

/*
*	Ќиже идут объ€влени€ глобальных переменных и функций дл€ работы UART
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