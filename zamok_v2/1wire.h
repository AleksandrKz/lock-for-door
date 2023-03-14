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

// Определения нужны только если требуется "сильный" подтягивающий резистор
/*
#define ONEWIRE_STRONG_DDR DDRB
#define ONEWIRE_STRONG_PORT PORTB
#define ONEWIRE_STRONG_PIN_NUM PB1*/

// Инициализация 1-wire
void onewire_init();

// Устанавливает низкий уровень на шине 1-wire
void onewire_low();

// "Отпускает" шину 1-wire
void onewire_high();

// Читает значение уровня на шине 1-wire
uint8_t onewire_level();

// Выдаёт импульс сброса (reset), ожидает ответный импульс присутствия.
uint8_t onewire_reset();

// Отправляет один бит
void onewire_send_bit(uint8_t bit);

// Отправляет один байт, восемь подряд бит, младший бит вперёд
void onewire_send(uint8_t b);

// читает значение бита, передаваемое уйстройством.
uint8_t onewire_read_bit();

// Читает один байт, переданный устройством, младший бит вперёд, возвращает прочитанное значение
uint8_t onewire_read();

// Возвращает обновлённое значение контрольной суммы
uint8_t onewire_crc_update(uint8_t crc, uint8_t b);

// Обновляет значение контольной суммы crc применением всех бит байта b.
//uint8_t onewire_crc_update(uint8_t crc, uint8_t b);

//SKIP ROM
uint8_t onewire_skip();

//Команда 0x33 READ ROM
uint8_t onewire_read_rom(uint8_t * buf);

//Команда 0x55 MATCH ROM (Совпадение ПЗУ)
uint8_t onewire_match(uint8_t * data);

// сканирование устройств на шине
uint8_t search_rom();
void reset_search();

//Команда 0xBE read_scratchpad ds18b20
void onewire_read_scratchpad(uint8_t * buf);