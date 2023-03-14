#include "1wire.h"

unsigned char rom[8];
unsigned char scratchpad[9];


const uint8_t onewire_crc_table[] PROGMEM = {
  0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83, 0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41, 
  0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e, 0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc, 
  0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0, 0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62, 
  0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d, 0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff, 
  0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5, 0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07, 
  0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58, 0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a, 
  0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6, 0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24, 
  0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b, 0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9, 
  0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f, 0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd, 
  0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92, 0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50, 
  0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c, 0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee, 
  0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1, 0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73, 
  0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49, 0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b, 
  0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4, 0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16, 
  0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a, 0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8, 
  0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7, 0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35  
};

/*
// Обновляет значение контольной суммы crc применением всех бит байта b.
// Возвращает обновлённое значение контрольной суммы
inline uint8_t onewire_crc_update(uint8_t crc, uint8_t b) {
  return pgm_read_byte(&onewire_crc_table[crc ^ b]);
}*/

// Обновляет значение контольной суммы crc применением всех бит байта b.
// Возвращает обновлённое значение контрольной суммы

uint8_t onewire_crc_update(uint8_t crc, uint8_t b) {
  return pgm_read_byte(&onewire_crc_table[crc ^ b]);
/*
  for (uint8_t p = 8; p; p--) {
    crc = ((crc ^ b) & 1) ? (crc >> 1) ^ 0b10001100 : (crc >> 1);
    b >>= 1;
  }
  return crc;*/
}

// Инициализация 1-wire
void onewire_init(){
	ONEWIRE_PORT &= ~_BV(ONEWIRE_PIN_NUM);
    onewire_high();
}

// Устанавливает низкий уровень на шине 1-wire
inline void onewire_low() {
  ONEWIRE_DDR |= _BV(ONEWIRE_PIN_NUM);
}

// "Отпускает" шину 1-wire
inline void onewire_high() {
  ONEWIRE_DDR &= ~_BV(ONEWIRE_PIN_NUM);
}

// Читает значение уровня на шине 1-wire
inline uint8_t onewire_level() {
  return ONEWIRE_PIN & _BV(ONEWIRE_PIN_NUM);
}

// Выдаёт импульс сброса (reset), ожидает ответный импульс присутствия.
// Если импульс присутствия получен, дожидается его завершения и возвращает 1, иначе возвращает 0 
uint8_t onewire_reset() 
{
  onewire_low();
  _delay_us(480); // Пауза 480..960 мкс
  onewire_high();
  //_delay_us(2); // Время необходимое подтягивающему резистору, чтобы вернуть высокий уровень на шине
  // Ждём не менее 60 мс до появления импульса присутствия;
	_delay_us(70);
    if (!onewire_level()) {
    // Если обнаружен импульс присутствия, ждём его окончания
    while (!onewire_level()) { } // Ждём конца сигнала присутствия
	_delay_us(1);
    return 1;
  }
  _delay_us(1);
  return 0;
}

// Отправляет один бит
// bit отправляемое значение, 0 - ноль, любое другое значение - единица
void onewire_send_bit(uint8_t bit) {
  onewire_low();
  if (bit) {
    _delay_us(10); // Низкий импульс, от 1 до 15 мкс (с учётом времени восстановления уровня)
    onewire_high();
    _delay_us(55); // Ожидание до завершения таймслота (не менее 60 мкс)
  } else {
    _delay_us(65); // Низкий уровень на весь таймслот (не менее 60 мкс, не более 120 мкс)
    onewire_high();
    _delay_us(5); // Время восстановления высокого уровеня на шине + 1 мс (минимум)
  }
}

// Отправляет один байт, восемь подряд бит, младший бит вперёд
// b - отправляемое значение
void onewire_send(uint8_t b) {
  for (uint8_t p = 8; p; p--) {
    onewire_send_bit(b & 1);
    b >>= 1;
  }
}

// читает значение бита, передаваемое уйстройством.
// Возвращает 0 - если передан 0, отличное от нуля значение - если передана единица
uint8_t onewire_read_bit() {
  onewire_low();
  _delay_us(3); // Длительность низкого уровня, минимум 1 мкс
  onewire_high();
  _delay_us(10); // Пауза до момента сэмплирования, всего не более 15 мкс
  uint8_t r = onewire_level();
  _delay_us(53); // Ожидание до следующего тайм-слота, минимум 60 мкс с начала низкого уровня
  return r;
}

// Читает один байт, переданный устройством, младший бит вперёд, возвращает прочитанное значение
uint8_t onewire_read() {
  uint8_t r = 0;
  for (uint8_t p = 8; p; p--) {
    r >>= 1;
    if (onewire_read_bit())
      r |= 0x80;
  }
  return r;
}

// Выполняет последовательность инициализации (reset + ожидает импульс присутствия)
// Если импульс присутствия получен, выполняет команду SKIP ROM
// Возвращает 1, если импульс присутствия получен, 0 - если нет
uint8_t onewire_skip() {
  if (!onewire_reset())
    return 0;
  onewire_send(0xCC);
  return 1;
}

// Выполняет последовательность инициализации (reset + ожидает импульс присутствия)
// Если импульс присутствия получен, выполняет команду READ ROM, затем читает 8-байтовый код устройства
//    и сохраняет его в буфер по указателю buf, начиная с младшего байта
// Возвращает 1, если код прочитан, 0 - если импульс присутствия не получен
uint8_t onewire_read_rom(uint8_t * buf) {
  if (!onewire_reset())
    return 0; 
  onewire_send(0x33);
  
  for (uint8_t p = 0; p < 8; p++) {
    *(buf++) = onewire_read();
  }
  return 1;
}

void onewire_read_scratchpad(uint8_t * buf) {
  onewire_send(READ_SCRPD);
  for (uint8_t p = 0; p < 9; p++) {
    *(buf+p) = onewire_read();
  }
}

// Выполняет последовательность инициализации (reset + ожидает импульс присутствия)
// Если импульс присутствия получен, выполняет команду MATCH ROM, и пересылает 8-байтовый код 
//   по указателю data (младший байт вперёд)
// Возвращает 1, если импульс присутствия получен, 0 - если нет
uint8_t onewire_match(uint8_t * data) {
  if (!onewire_reset())
    return 0;
  onewire_send(0x55);
  for (uint8_t p = 0; p < 8; p++) {
    onewire_send(*(data++));
  }
  return 1;
}

// сканирование устройств на шине
uint8_t search_rom() {
	crc = 0;
	int8_t id_bit_number;
	int8_t last_zero, rom_byte_number, search_result;
	int8_t id_bit, cmp_id_bit;
    unsigned char rom_byte_mask, search_direction;

   // initialize for search
    id_bit_number = 1;
    last_zero = 0;
    rom_byte_number = 0;
    rom_byte_mask = 1;
    search_result = 0;

	
	if (!LastDeviceFlag)
	{
		if (!onewire_reset())
		{
			LastDiscrepancy = 0;
			LastDeviceFlag = FALSE;
			LastFamilyDiscrepancy = 0;
			return FALSE;
		}
		onewire_send(SEARCH_ROM);
	do 
	{
			id_bit = onewire_read_bit();
			cmp_id_bit = onewire_read_bit();
			
			if (id_bit == 1 && cmp_id_bit == 1)
			{
				return 0;
			}
			if (id_bit != cmp_id_bit)
			{
				search_direction = id_bit;
			} 
			else 
			{
				if (id_bit_number < LastDiscrepancy)
                  search_direction = ((rom[rom_byte_number] & rom_byte_mask) > 0);
               else
                  // if equal to last pick 1, if not then pick 0
                  search_direction = (id_bit_number == LastDiscrepancy);

               // if 0 was picked then record its position in LastZero
               if (search_direction == 0)
               {
                  last_zero = id_bit_number;

                  // check for Last discrepancy in family
                  if (last_zero < 9)
                     LastFamilyDiscrepancy = last_zero;
               }
			}
			
			if (search_direction == 1)
				rom[rom_byte_number] |= rom_byte_mask;
            else
				rom[rom_byte_number] &= ~rom_byte_mask;
			
			onewire_send_bit(search_direction);
			
			id_bit_number++;
			rom_byte_mask <<= 1;
			
			if (rom_byte_mask == 0)
            {
                onewire_crc_update(crc, rom[rom_byte_number]);  // accumulate the CRC
                rom_byte_number++;
                rom_byte_mask = 1;
            }
	} while (rom_byte_number < 8);
	
	if (!((id_bit_number < 65) || (crc != 0)))
	{
		LastDiscrepancy = last_zero;
		if (LastDiscrepancy == 0)
            LastDeviceFlag = TRUE;
		search_result = TRUE;
	}
	}	
	if (!search_result || !rom[0])
   {
      LastDiscrepancy = 0;
      LastDeviceFlag = FALSE;
      LastFamilyDiscrepancy = 0;
      search_result = FALSE;
   }

   return search_result;
}

void reset_search()
{
   // reset the search state
   LastDiscrepancy = 0;
   LastDeviceFlag = FALSE;
   LastFamilyDiscrepancy = 0;
}


// функции ниже нужны только если требуется "сильный" подтягивающий резистор
// включение "сильной" подтяжки
/*
void onewire_strong_enable() {
  // Для исключения низкого уровня на шине, сначала изменяется регистр значения
  ONEWIRE_STRONG_PORT |= _BV(ONEWIRE_STRONG_PIN_NUM);
  // затем - регистр направления
  ONEWIRE_STRONG_DDR |= _BV(ONEWIRE_STRONG_PIN_NUM); 
}

// отключение "сильной" подтяжки
void onewire_strong_disable() {
  // Для исключения низкого уровня на шине, сначала изменяется регистр направления
  ONEWIRE_STRONG_DDR &= ~_BV(ONEWIRE_STRONG_PIN_NUM); 
  // затем - регистр значения
  ONEWIRE_STRONG_PORT &= ~_BV(ONEWIRE_STRONG_PIN_NUM);
}*/