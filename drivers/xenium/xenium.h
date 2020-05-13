#ifndef _XENIUM_H_
#define _XENIUM_H_

#include "cromwell_types.h"

#define XENIUM_REGISTER_BANKING 0x00EF
#define XENIUM_REGISTER_LED 0x00EE

#define XENIUM_LED_OFF    0
#define XENIUM_LED_RED    1
#define XENIUM_LED_GREEN  2
#define XENIUM_LED_AMBER  3
#define XENIUM_LED_BLUE   4
#define XENIUM_LED_PURPLE 5
#define XENIUM_LED_TEAL   6
#define XENIUM_LED_WHITE  7

#define XENIUM_MANUF_ID  0x01
#define XENIUM_DEVICE_ID 0xC4
#define XENIUM_FLASH_SECTOR_SIZE 0x10000

#define XENIUM_BANK_TSOP        0
#define XENIUM_BANK_BOOTLOADER  1
#define XENIUM_BANK_XENIUMOS    2
#define XENIUM_BANK_1           3
#define XENIUM_BANK_2           4
#define XENIUM_BANK_3           5
#define XENIUM_BANK_4           6
#define XENIUM_BANK_1_512       7
#define XENIUM_BANK_2_512       8
#define XENIUM_BANK_1_1024      9
#define XENIUM_BANK_RECOVERY    10

//OpenXOS uses Sector 33 in the Flash chip for non-voltatile
//settings and eeprom backup. This sits at absolute address 0x1FA000
//which is in Bank 10 (0x1C0000). To get the relative address against Bank 10
//I subtract the difference.
//A genuine Xenium uses Sector 34 which I wont touch.
#define XENIUM_SETTINGS_OFFSET (0x1FA000-0x1C0000)
#define XENIUM_SETTINGS_SECTOR_SIZE 0x2000 //8kbytes
#define XENIUM_MAX_BIOS_NAME_LENGTH 32
#define XENIUM_MAX_BANKS 4

typedef struct __attribute__((__packed__)) {
    u8 bank_used; //0 if not used, other to actual xenium cpld bank number
    u8 bios_led_colour;
    u8 bios_name[XENIUM_MAX_BIOS_NAME_LENGTH];
	u32 bios_size;
} flash_bank;

typedef struct __attribute__((__packed__)) {
    u8 default_bank;
    u8 instant_boot;
    u8 quick_boot;
    flash_bank flash_bank[4];
    u8 eeprom[256];
	u8 checksum; //checksum of the settings struct  excluding itself
} xenium_settings;

void xenium_set_bank(u8 bank);
void xenium_set_led(u8 led);
u8 xenium_get_bank(void);
u8 xenium_is_detected(void);
void xenium_erase_bank(u8 bank);
void xenium_write_bank(u8 bank, u8* data);
void xenium_read_settings(xenium_settings* settings);
void xenium_update_settings(xenium_settings* settings);

#endif