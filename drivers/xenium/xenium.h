#ifndef _XENIUM_H_
#define _XENIUM_H_

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

#define XENIUM_BANK_TSOP        0
#define XENIUM_BANK_CROMWELL    1
#define XENIUM_BANK_XENIUMOS    2
#define XENIUM_BANK_1           3
#define XENIUM_BANK_2           4
#define XENIUM_BANK_3           5
#define XENIUM_BANK_4           6
#define XENIUM_BANK_1_512       7
#define XENIUM_BANK_2_512       8
#define XENIUM_BANK_1_1024      9
#define XENIUM_BANK_RECOVERY    10

void xenium_set_bank(u8 bank);
void xenium_set_led(u8 led);
u8 xenium_get_bank(void);
u8 xenium_is_detected();
void xenium_erase_bank(u8 bank);


#endif