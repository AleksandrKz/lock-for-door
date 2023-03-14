#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#define TRUE 1
#define FALSE 0

#define ONEWIRE_PORT PORTB
#define ONEWIRE_DDR DDRB
#define ONEWIRE_PIN PINB
#define ONEWIRE_PIN_NUM PB7

#define SKIP_ROM 0xCC
#define READ_ROM 0x33
#define MATCH_ROM 0x55
#define SEARCH_ROM 0xF0
#define READ_SCRPD 0xBE //ds18b20
#define CONVERT_T 0x44 //ds18b20

uint8_t crc;
int8_t LastDiscrepancy;
int8_t LastFamilyDiscrepancy;
_Bool LastDeviceFlag;
extern unsigned char rom[8];
extern unsigned char scratchpad[9];

// ����������� ����� ������ ���� ��������� "�������" ������������� ��������
/*
#define ONEWIRE_STRONG_DDR DDRB
#define ONEWIRE_STRONG_PORT PORTB
#define ONEWIRE_STRONG_PIN_NUM PB1*/

// ������������� 1-wire
void onewire_init();

// ������������� ������ ������� �� ���� 1-wire
void onewire_low();

// "���������" ���� 1-wire
void onewire_high();

// ������ �������� ������ �� ���� 1-wire
uint8_t onewire_level();

// ����� ������� ������ (reset), ������� �������� ������� �����������.
uint8_t onewire_reset();

// ���������� ���� ���
void onewire_send_bit(uint8_t bit);

// ���������� ���� ����, ������ ������ ���, ������� ��� �����
void onewire_send(uint8_t b);

// ������ �������� ����, ������������ ������������.
uint8_t onewire_read_bit();

// ������ ���� ����, ���������� �����������, ������� ��� �����, ���������� ����������� ��������
uint8_t onewire_read();

// ���������� ���������� �������� ����������� �����
uint8_t onewire_crc_update(uint8_t crc, uint8_t b);

// ��������� �������� ���������� ����� crc ����������� ���� ��� ����� b.
//uint8_t onewire_crc_update(uint8_t crc, uint8_t b);

//SKIP ROM
uint8_t onewire_skip();

//������� 0x33 READ ROM
uint8_t onewire_read_rom(uint8_t * buf);

//������� 0x55 MATCH ROM (���������� ���)
uint8_t onewire_match(uint8_t * data);

// ������������ ��������� �� ����
uint8_t search_rom();
void reset_search();

//������� 0xBE read_scratchpad ds18b20
void onewire_read_scratchpad(uint8_t * buf);