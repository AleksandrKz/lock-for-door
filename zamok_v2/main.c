/*
 * zamok_v2.c
 *
 * Created: 26.03.2020 20:36:59
 * Author : user
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include "1wire.h"
#include "uart_rx.h"

#define CRC_ERROR 99
#define DEVICE_ERROR 88

#define ledDDR DDRC
#define ledPORT PORTC
#define ledPIN PINC
#define ledGreen PC0
#define ledRed PC7

#define znakDDR DDRD
#define znakPORT PORTD
#define znakPIN PIND
#define ledZnak PD4

#define motorDDR DDRB
#define motorPORT PORTB
#define motorPIN PINB
#define motorA PB0
#define motorB PB1

#define zumerPIN PINB
#define zumerDDR DDRB
#define zumerPORT PORTB
#define zumer PB6

#define lockSensorDDR DDRA
#define lockSensorPORT PORTA
#define lockSensorPIN PINA
#define lockSensor1 PA2
#define lockSensor2 PA3

#define displayDDR DDRA
#define displayPORT PORTA
#define dispResetPin PA1
#define dispClockPin PA0

#define backDoorButton PC1

///////////////////////////////EEPROM/////////////////////////////////////
uint8_t EEMEM pass_EP[] = {5,5,5,5}; //������ ��������
uint8_t EEMEM rfid_key_EP[5][6] = {{0x37, 0x00, 0x3F, 0x23, 0xFF, 0xD4}}; // 5 ����� ��� �������� �� �����
//////////////////////////////////////////////////////////////////////////

// ��������� ���������� 3*4
uint8_t key_code[3][4] = {{0x01, 0x04, 0x07,0x0A},
{0x02, 0x05, 0x08,0x00},
{0x03, 0x06, 0x09, 0x0B}};
	
uint8_t stack = 0;//��� ���������� �������� SREG
uint8_t crc;//��� ������� CRC ds18b20

uint8_t password[4] = {0,0,0,0};
uint8_t fKeyPress = 0; //���� ������� ������
volatile uint8_t fLock = 0; //��������� �����
uint8_t fBackDoor = 0;//���� ������ ������ ����������
uint8_t fStok = 0;//��������� �����(��������)
uint8_t fTimerKey = 0; //���� ��� ��������� ������� ����� ��������� ������� ������ ��� ������ key_count
uint8_t TimerKeyCount = 0; // ������� ��� ������� (���� �� 10 ��� �����������)
uint8_t key;//������� ������
uint8_t key_count = 0;//������� ������� ������
uint8_t longPressCount = 0;//������� ������������ ������� �������


uint8_t fTempRead = 0;//���� ��� ���������� ����������� � �������
uint8_t TimerTempCount = 0; //������� ������� ������� �����������
uint16_t temperatura;
uint8_t temperatura_tmp = 0;

uint8_t alarm = 0; //�������� ������ �� ���������� ������

volatile uint8_t timeout; //���� ��� ������ �����
uint8_t fReadRfid = 0; //���� ��� ������ ��������� ���������� rfid �����
volatile uint8_t fGetId = 0;//���� ������ ����� �� uart
uint8_t tmp_id[14] = {}; //����� ������ ������ � ����� rfid
uint8_t id_card[6] = {}; // ����� ��� ID(5) ����� + 1���� cheksum

uint8_t GetKey(void); //��������� ������� �������
void SendNumToDisplay(uint8_t); //�������� ����� �� ���������
void OpenDoor(void); //�������
void CloseDoor(void); //�������
uint8_t ChekPass(uint8_t*);//�������� ������ �� ������������
uint8_t ChekID(uint8_t*);// �������� �� �����
uint8_t ConvertID(uint8_t*, uint8_t*);//����������� ��������� ������ � ����� � id �����
void SaveID(uint8_t ,uint8_t*);// ���������� �� ����� � eeprom
void RemoveID(uint8_t);// �������� �� ����� � eeprom
void SavePass(uint8_t*);//��������� ������ � ������
uint8_t ChekLock(void);//�������� ��������� �����
void clear_buffer(uint8_t*, uint8_t); //���������� ������� 0xFF
void error_s(void);// ��������� ������ 
void done_s(void);//��������� ������
void led_blink(void);// 1 - �������� ����������� ����� �����
void led_blink3(void);// 3 - �������� ����������� ����� �����
void EEPROM_write(unsigned int, unsigned char);
unsigned char EEPROM_read(unsigned int);

ISR(TIMER1_COMPA_vect)
{
	//������ - ���� �������
	
	// ������� ��������� � ���������� ������ UART()
	if (timeout > 0)
	{
		timeout++;
		if (timeout>2)
		{
			TIMSK0 &= ~(1 << OCIE0B);	// ��������� ���������� TIM0_COMPB
			TIFR0 |= (1 << OCF0B);		// ������� ���� ���������� TIM0_COMPB
			EIFR |= (1 << INTF1);		// ������� ���� ���������� �� INT1
			EIMSK |= (1 << INT1);		// ��������� ���������� INT1
		}
	}
	if (alarm > 0)
	{
		alarm--;
		if(alarm == 0)
			zumerPORT &= ~(1<<zumer); //����� off
	}
	
	if (fTimerKey == 1)//������� ����� ������
	{
		TimerKeyCount++;
		if (TimerKeyCount > 9) //10���
		{
			key_count = 0;
			fTimerKey = 0;
			fBackDoor = 0;//10���, ����� ������ �� �������
			TimerKeyCount = 0;
		}
	}
	
	if(++TimerTempCount >15) //������ ���������� ����������� ��� �  15 ���
		fTempRead = 1;
	
	if (!(PINC&(1<<backDoorButton)))//���� ������ ������ �� ������
	{
		longPressCount++;
		if (longPressCount == 2)
		{
			zumerPIN |= 1<<zumer;//������ 2��� ����� ����
		}
		if (longPressCount == 4)
		{
			zumerPIN |= 1<<zumer;//������ 4��� ����� ���
		}
	}
}

ISR(PCINT1_vect)
{
	PCICR &= ~(1<<PCIE1);//��������� ���������� �� ������
	fKeyPress = 1; //������������� ���� ������� ������
	zumerPORT |= 1<<zumer;//����� on
	TimerTempCount = 0; //���������� ������� ������ �����������
}

int main(void)
{	
	//�������������� ����� ����������� � +
	PORTD |= (1<<PD5)|(1<<PD6)|(1<<PD7);//pull-up
	PORTB |= (1<<PB2)|(1<<PB3)|(1<<PB4)|(1<<PB5);//pull-up
	
	//��������� ������ ����������
	DDRD |= (1<<PD2)|(1<<PD1)|(1<<PD0); //������� �� �����
	PORTC |= (1<<PC5)|(1<<PC4)|(1<<PC3)|(1<<PC2)|(1<<PC1); //pull-up ������ ���������� � ������ �� ������ PB4
	
	ledDDR |= (1<<ledRed)|(1<<ledGreen); //���������� ��������� PC7 - red, PC0 - green, ����� �� �����
	motorDDR |= (1<<motorA)|(1<<motorB); //��������� ����� ���������� ���������� �����(��������), ����� �� �����
	motorPORT &= ~((1<<motorA)|(1<<motorB));
	
	//lockSensorDDR &= ~((1<<lockSensor1)|(1<<lockSensor2));
	//lockSensorPORT |= (1<<lockSensor1)|(1<<lockSensor2); // ��� ����������� ��������� ����� pull-up
	
	
	zumerDDR |= (1<<zumer);//����� - ���� �� �����
	
	displayDDR |= (1<<dispClockPin)|(1<<dispResetPin); //��������� ������ ��� ������� �� �����
	displayPORT |= (1<<dispClockPin)|(1<<dispResetPin);//��������
	
	znakDDR |= 1<<ledZnak;// ��� ��������� ����� ������ �� �����
	
	//��������� ���������� �� ������
	PCICR |= 1<<PCIE1;
	PCMSK1 |= (1<<PCINT13)|(1<<PCINT12)|(1<<PCINT11)|(1<<PCINT10)|(1<<PCINT9);
	
	//��������� ������� �� 1 ���
	OCR1A = 31250;
	TCCR1B |= (1<<WGM12)|(1<<CS12); //CTC | F_CPU/256
	TIMSK1 |= 1<<OCIE1A;
	
	//��������� watch dog.
	wdt_enable(WDTO_8S);
	
	//ds18b20
	onewire_init();
	
	onewire_skip();
	onewire_send(CONVERT_T);
	
	uart_init();
	
	sei();
	
	ChekLock();
	
	//SendNumToDisplay(55);//debug
	
	while(1)
	{
		if (fKeyPress == 1) //���� ������ ����� ���� ������
		{
			_delay_ms(35);// ����������� ��� �������
			if (!(PINC&(1<<backDoorButton)))//���� ������ ������ �� ������, �� ������ ��������� �����
			{
				
				while(!(PINC&(1<<backDoorButton))); //���� ���������� ������
				if (fLock)//���� �������, �� ���������
				{
					OpenDoor();
					if (longPressCount > 1)//��������� 2 � ������ ���
					{
						fBackDoor = 1;//��������� ����� ������
					}
					if (longPressCount > 3)//��������� 5 � ������ ���
					{
						fBackDoor = 2;//��������� ������ ����� rfid �����
					}
				}
				else
				{
					CloseDoor();
				}
				longPressCount = 0;
				key_count = 0;
			}
			else
			{
				key = GetKey();
				if (key == 0x0B) // 0x0B = #
				{
					if(!fLock) //���� ������ # � ����� ������, �� ��������� ���
						CloseDoor();
					fBackDoor = 0;
					key_count = 0;
				}
				else if (key == 0x0A && fBackDoor)//���� ������ 0x0A = * � ����� ������� ������� � ���������� ������ ����� 2 ���. �� ������ ������ � ������
				{
					SavePass(password);
					fBackDoor = 0;
					key_count = 0;
					done_s();
				}
				else if (key == 0x0A)// 0x0A = * �������� ������� ������ � ������ ������
				{
					key_count = 0;
				}
				else if (key != 0xFF)
				{
					//SendNumToDisplay(key);//debug
					
					password[key_count] = key;
					key_count += 1;
					if (key_count > 3)//����� 4 �����?
					{
						if(ChekPass(password) && fLock)
						{
							OpenDoor();
						}
						else if(fLock) //���� ������ �� ������ �� ��������� ������
						{
							error_s();
						}
						key_count = 0;//���������� ������� �������
					}
				}
				
			}
			fKeyPress = 0;//��������� ���� ������� ������
			TimerKeyCount = 0;//������� �������� ����� ������
			fTimerKey = 1; //�������� ������� �������� ����� ������
			
			if(alarm == 0)
				zumerPORT &= ~(1<<zumer); //����� off
			
			_delay_ms(25); //������� ����� ���������� ������
			PCIFR |= 1<<PCIF1;//����� ����� ����������(�� ������ ������)
			PCICR |= 1<<PCIE1;//�������� ����������
		}
		
		if (fTempRead == 1) //���� ������ �����������
		{
			stack = SREG;
			cli();
			if(onewire_skip())//���� ������ �������
			{
				onewire_read_scratchpad(scratchpad);// ������ ������ � �������
				SREG = stack;
				crc = 0;
				for(uint8_t i=0; i<8; i++)
				{
					crc = onewire_crc_update(crc, scratchpad[i]);
				}
				if(crc == scratchpad[8]) // ���� ����������� ����� �������
				{
					if (scratchpad[1] & 0x80) //��������� �� ���� �����������, ���� ������� ������ ����� 1 �� �����
					{
						temperatura = ((uint16_t)scratchpad[1]<<8) | scratchpad[0];
						//temperatura += 0x0008; //��� ���������� �������� +0.5
						temperatura = (((~temperatura) + 1)+0x0008)>>4;
						znakPORT |= 1<<ledZnak;// �������� ���� ������
					}
					else
					{
						temperatura = ((uint16_t)scratchpad[1]<<8) | (scratchpad[0]);
						temperatura += 0x0008; //��� ���������� �������� +0.5
						temperatura >>= 4;
						znakPORT &= ~(1<<ledZnak);// ����� ���� ������
					}
					
					if(temperatura_tmp != (uint8_t)temperatura) //���� ����������� ����������, �� ���������
						SendNumToDisplay((uint8_t)temperatura);
						
					temperatura_tmp = (uint8_t)temperatura;
				}
				else
				{
					led_blink3();// ������ CRC
					//SendNumToDisplay(CRC_ERROR);// ������ CRC //debug
				}
			}
			else
			{
				SREG = stack;
				SendNumToDisplay(DEVICE_ERROR);// ������ ����������� ������� �� ����
				led_blink3();//debug
			}
			
			fTempRead = 0;
			TimerTempCount = 0;
			
			stack = SREG;
			cli();
			onewire_skip();
			onewire_send(CONVERT_T);
			SREG = stack;
		}
		
		if (fGetId) //�������� �����?
		{			
			int8_t result;
			result = GetId(tmp_id,14);
			switch(result)
			{
				case 0: //����� ��������� �� ���������
					//timeout = 0;
					//fReadRfid = 0;
					break;
				
				case -1: // ������ ���������� ID �����
					//fReadRfid = 0;
					clear_buffer(tmp_id,14);
					clear_buffer(id_card,6);
					EIMSK |= (1 << INT1);		// ��������� ���������� INT1
					break;
					
				case 1:					
					if(!(ConvertID(tmp_id, id_card))) // �� �������� ���� � id_card
					{						
						//timeout = 0;
						break;
					}
					
					led_blink();
					if (fBackDoor == 2)
					{
						if (key < 6 && key > 0) //������ ���������� ��
						{
							SaveID(key, id_card);
							done_s();
						}
					}
					if(fLock)
					{
						if(ChekID(id_card)) //ID ��������� � ������� � ������?
						{
							OpenDoor(); //���������
							done_s();
						}
						else
						{
							error_s();//������ �������� � ��������� ����������� rfid
						}
					}
					
					//timeout = 0;
					//fReadRfid = 1;
					clear_buffer(tmp_id,14);
					clear_buffer(id_card,6);
					EIMSK |= (1 << INT1);		// ��������� ���������� INT1
					break;
			}
			fGetId = 0;
		}
		
		wdt_reset();
	} // end WHILE
} // end MAIN

uint8_t GetKey(void)
{
	uint8_t i,j,tmp = 0xFF;
	
	for (i=0;i<3;i++)
	{
		j=4;
		PORTD |= 0x07;
		asm volatile("nop\n\t"
					 "nop\n\t"
					 "nop\n\t");
		PORTD &= ~(0x04>>i);
		asm volatile("nop\n\t"
					 "nop\n\t"
					 "nop\n\t"
					 "nop\n\t"
					 "nop\n\t"); // �������� ��� ��������� ������ �����
		if((PINC & 0x20) == 0) j = 0;
		if((PINC & 0x10) == 0) j = 1;
		if((PINC & 0x08) == 0) j = 2;
		if((PINC & 0x04) == 0) j = 3;
		if (j!=4)
		{
			while(!(PINC&(0x20>>j))); //���� ���������� ������
			tmp = key_code[i][j];
			break;
		}
	}
	PORTD &= 0xF8;
	return tmp;
}

void SendNumToDisplay(uint8_t number)
{
	//����������� ����������
	/*//��������� ���������
	displayPORT |= 1<<dispResetPin;
	asm("nop");
	displayPORT &= ~(1<<dispResetPin);
	asm("nop");
	//������������� �����
	while(number--)
	{
		displayPORT |= 1<<dispClockPin;
		asm("nop");
		displayPORT &= ~(1<<dispClockPin);
		asm("nop");
	}*/
	// ��������� ����������
	displayPORT &= ~(1<<dispResetPin);
	_delay_us(10);
	displayPORT |= 1<<dispResetPin;
	_delay_us(10);
	while(number--)
	{
		displayPORT &= ~(1<<dispClockPin);
		_delay_us(10);
		displayPORT |= 1<<dispClockPin;
		_delay_us(10);
	}
}

void OpenDoor(void)
{
	motorPORT &= ~(1<<motorA);
	asm volatile("nop\n\tnop\n\tnop\n\t");
	//motorPORT |= 1<<motorB;//�� ��������� ��������
	motorPIN |= 1<<motorB;//��������� �������� 
	_delay_ms(50);//pausa 50ms
	//motorPORT &= ~(1<<motorB);//�� ��������� ��������
	motorPIN |= 1<<motorB;//��������� �������� 
	fLock = 0;
	
	ledPORT &= ~(1<<ledRed); //����� �������
	ledPORT |= 1<<ledGreen; // �������� �������
}

void CloseDoor(void)
{
	motorPORT &= ~(1<<motorB);
	asm volatile("nop\n\tnop\n\tnop\n\t");
	//motorPORT |= 1<<motorA;//��������� �������� 
	motorPIN |= 1<<motorA;//�� ��������� �������� 
	_delay_ms(50);//pausa 50ms
	//motorPORT &= ~(1<<motorA);//�� ��������� �������� 
	motorPIN |= 1<<motorA;//��������� �������� 
	fLock = 1;
	
	ledPORT &= ~(1<<ledGreen); //����� �������
	ledPORT |= 1<<ledRed; // �������� �������
}

void error_s(void)
{
	uint8_t tmp;
	tmp = ledPIN & (1<<ledRed);//���������� ������� ��������� �����
	for(uint8_t i=0; i<=4; i++)
	{
		ledPORT |= 1<<ledRed; // �������� �������
		zumerPORT |= 1<<zumer;//����� on
		_delay_ms(150);//pausa 150ms
		ledPORT &= ~(1<<ledRed); // ����� �������
		zumerPORT &= ~(1<<zumer);//����� off
		_delay_ms(150);//pausa 150ms
	}
	if(tmp)
		ledPORT |= 1<<ledRed; // �������� �������(�������������� ��������� �����)
}

void done_s(void)
{
	uint8_t tmp;
	tmp = ledPIN & (1<<ledGreen);//���������� ������� ��������� �����
	//zumerPORT &= ~(1<<zumer);//����� off
	zumerPORT |= 1<<zumer;//����� on
	for(uint8_t i=0; i<=4; i++)
	{
		ledPORT |= 1<<ledGreen; // �������� ������
		_delay_ms(80);//pausa 250ms
		ledPORT &= ~(1<<ledGreen); // ����� ������
		_delay_ms(80);//pausa 250ms
	}
	zumerPORT &= ~(1<<zumer);//����� off
	if(tmp)
		ledPORT |= 1<<ledGreen; // �������� ������(�������������� ��������� �����)
}

void led_blink3(void)
{
	for(uint8_t i=0; i<6; i++)
	{
		znakPIN |= 1<<ledZnak; // ����������� ����(������ 1 � portpin ������ ��������� ������ �� ���������������)
		_delay_ms(80);
	}
}

void led_blink(void)
{
	for(uint8_t i=0; i<2; i++)
	{
		znakPIN |= 1<<ledZnak; // ����������� ����(������ 1 � portpin ������ ��������� ������ �� ���������������)
		_delay_ms(80);
	}
}

uint8_t ChekLock(void)
{
	// PA2 �� ��������, �������� ���� �����, ���� �� �����������
	if(!(lockSensorPIN & (1<<lockSensor1))) //���� 0 �� PA2 �� ����� ������
	{
		fLock = 1;
		ledPORT &= ~(1<<ledGreen); //����� �������
		ledPORT |= 1<<ledRed; // �������� �������
	}
	else if(!(lockSensorPIN & (1<<lockSensor2))) //���� 0 �� PA3 �� ����� ������
	{
		fLock = 0;
		ledPORT &= ~(1<<ledRed); //����� �������
		ledPORT |= 1<<ledGreen; // �������� �������
	}
	else
	{
		fLock = 1;
		CloseDoor();
		ledPORT &= ~(1<<ledGreen); //����� �������
		ledPORT |= 1<<ledRed; // �������� �������
	}
	return fLock;
}

void SavePass(uint8_t* pass_buf)
{
	uint8_t i;
	for (i=0;i<4;i++)
	{
		eeprom_busy_wait();
		eeprom_write_byte((uint8_t *)pass_EP+i, *(pass_buf+i));
	}
}

uint8_t ChekPass(uint8_t* pass_buf)
{
	uint8_t i,tmp;
	
	for (i=0;i<4;i++)
	{
		eeprom_busy_wait();
		tmp = eeprom_read_byte((const uint8_t *)pass_EP+i);
		if(tmp != *(pass_buf+i))
		{
			return 0;
		}
	}
	return 1;
}

void SaveID(uint8_t position, uint8_t* buf)
{
	uint8_t i;
	position--;
	for (i=0;i<6;i++)
	{
		eeprom_busy_wait();
		eeprom_write_byte((uint8_t *)(((uint8_t *)(rfid_key_EP+position))+i), *(buf+i));
	}
}

void RemoveID(uint8_t position)
{
	uint8_t i;
	position--;
	for (i=0;i<6;i++)
	{
		eeprom_busy_wait();
		eeprom_write_byte((uint8_t *)(((uint8_t *)(rfid_key_EP+position))+i), 0xFF);
	}
}

uint8_t ChekID(uint8_t* buf)
{
	uint8_t i,j,tmp,result;
	
	for(j=0; j<9; j++)
	{
		result = 6;
		for(i=0; i<6; i++)
		{
			eeprom_busy_wait();
			tmp = eeprom_read_byte((const uint8_t *)((const uint8_t *)(rfid_key_EP+j)+i));
			if (tmp != *(buf+i))
			{
				result--;
				break;
			}
		}
		if(result == 6)
			return 1;
	}
	if(result<6)
		return 0;
	return 1;
}

uint8_t ConvertID(uint8_t* buf_tmp, uint8_t* buf_id)
{		
	for (uint8_t i=1; i<13; i++)
	{
		if(*(buf_tmp+i) > 47 && *(buf_tmp+i) < 58)
		{
			*(buf_tmp+i) = *(buf_tmp+i) & 0x0F;
		}
		else if(*(buf_tmp+i) > 64 && *(buf_tmp+i) < 71)
		{
			if((*(buf_tmp+i) & 0x0F) == 1)
				*(buf_tmp+i) = 0x0A;
			if((*(buf_tmp+i) & 0x0F) == 2)
				*(buf_tmp+i) = 0x0B;
			if((*(buf_tmp+i) & 0x0F) == 3)
				*(buf_tmp+i) = 0x0C;
			if((*(buf_tmp+i) & 0x0F) == 4)
				*(buf_tmp+i) = 0x0D;
			if((*(buf_tmp+i) & 0x0F) == 5)
				*(buf_tmp+i) = 0x0E;
			if((*(buf_tmp+i) & 0x0F) == 6)
				*(buf_tmp+i) = 0x0F;
		}
		else
			return 0;
	}
	for(uint8_t j=0, i=1; i<12; i+=2, j++)
	{
		*(buf_tmp+i) = (*(buf_tmp+i)<<4)+*(buf_tmp+i+1);
		if (i == 1)
		{
			*(buf_id) = *(buf_tmp+i);
		} 
		else
		{
			*(buf_id + j) = *(buf_tmp+i);
		}
	}

	return 1;
}

void clear_buffer(uint8_t* buf, uint8_t len)
{
	for(uint8_t i=0; i<len; i++)
	{
		*(buf+i) = 0xFF;
	}
}
void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
	cli();
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address and Data Registers */
	EEARL = uiAddress;
	EEDR = ucData;
	/* Write logical one to EEMPE */
	EECR |= (1<<EEMPE);
	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEPE);
	sei();
}

unsigned char EEPROM_read(unsigned int uiAddress)
{
	cli();
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address register */
	EEARL = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	sei();
	/* Return data from Data Register */
	return EEDR;
}
