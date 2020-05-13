#include "boot.h"
#include "BootFlash.h"
#include "memory_layout.h"
#include "xenium.h"

//See https://github.com/Ryzee119/OpenXenium/blob/master/Firmware/openxenium.vhd
//for Xenium CPLD details

//IO CONTROL INTERFACE
void xenium_set_bank(u8 bank){
    if(bank <= 10)
        IoOutputByte(XENIUM_REGISTER_BANKING, bank);
}

u8 xenium_get_bank(){
    return IoInputByte(XENIUM_REGISTER_BANKING) & 0x0F;
}

void xenium_set_led(u8 led){
    IoOutputByte(XENIUM_REGISTER_LED, led);
}

//FLASH MEMORY INTERFACE
static void lpc_send_byte(u32 address, u8 data){
    volatile u8 * volatile lpc_mem_map = (u8 *)LPCFlashadress;
    lpc_mem_map[address] = data;
}

static u8 xenium_flash_read_byte(u32 address){
    volatile u8 * volatile lpc_mem_map = (u8 *)LPCFlashadress;
    return lpc_mem_map[address];
}

static void xenium_flash_read_stream(u32 address, u8* data, u32 len){
    volatile u8 * volatile lpc_mem_map = (u8 *)LPCFlashadress;
    memcpy(data, (u8*)&lpc_mem_map[address], len);
}

static void xenium_flash_write_stream(u32 address, u8* data, u32 len){
    volatile u8 * volatile lpc_mem_map = (u8 *)LPCFlashadress;
    memcpy((u8*)&lpc_mem_map[address], data, len);
}

static void xenium_flash_reset(void){
    lpc_send_byte(0x00000000,0xF0);
}

static u8 xenium_flash_busy(void){
    return xenium_flash_read_byte(0) != xenium_flash_read_byte(0);
}

static u32 xenium_get_bank_size(u8 bank){
    u32 bank_size;
    switch (bank){
        case XENIUM_BANK_1:
        case XENIUM_BANK_2:
        case XENIUM_BANK_3:
        case XENIUM_BANK_4:
            bank_size = 0x40000;
            break;
        case XENIUM_BANK_1_512:
        case XENIUM_BANK_2_512:
            bank_size = 0x80000;
            break;
        case XENIUM_BANK_1_1024:
            bank_size = 0x100000;
            break;
        //Let's not erase any other banks!
        default:
            bank_size = 0;
            break;
    }
    return bank_size;
}

static void xenium_sector_erase(u32 sector_address){
    lpc_send_byte(0xAAAA, 0xAA);
    lpc_send_byte(0x5555, 0x55);
    lpc_send_byte(0xAAAA, 0x80);
    lpc_send_byte(0xAAAA, 0xAA);
    lpc_send_byte(0x5555, 0x55);
    lpc_send_byte(sector_address, 0x30);
    while(xenium_flash_busy());
	xenium_flash_reset();
}

static void xenium_flash_program_byte(u32 address, u8 data){
    lpc_send_byte(0xAAAA, 0xAA);
    lpc_send_byte(0x5555, 0x55);
    lpc_send_byte(0xAAAA, 0xA0);
    lpc_send_byte(address, data);
    while(xenium_flash_busy());
}

u8 xenium_is_detected(){
    xenium_flash_reset();
    lpc_send_byte(0xAAAA,0xAA);
    lpc_send_byte(0x5555,0x55);
    lpc_send_byte(0xAAAA,0x90);
    u8 manuf = xenium_flash_read_byte(0x00);

    xenium_flash_reset();
    lpc_send_byte(0xAAAA,0xAA);
    lpc_send_byte(0x5555,0x55);
    lpc_send_byte(0xAAAA,0x90);
    u8 devid = xenium_flash_read_byte(0x02);

    printk("manuf: %02x, devid: %02x\n", manuf, devid);
    if(manuf == XENIUM_MANUF_ID &&
       devid == XENIUM_DEVICE_ID){
        printk("Xenium is detected\n");
        return 1;   
    }
    printk("Xenium NOT detected\n");
    return 0;
}

void xenium_erase_bank(u8 bank){
    u32 bank_size = xenium_get_bank_size(bank);
    if (bank_size == 0)
        return;

    printk("Erasing Bank %u ", bank);
    u8 old_bank = xenium_get_bank();
    xenium_set_bank(bank);
    xenium_flash_reset();
    for(u32 i = 0; i < bank_size; i += XENIUM_FLASH_SECTOR_SIZE){
        printk(" . ");
        xenium_sector_erase(i);
    }
    xenium_set_bank(old_bank);
    printk("\n", bank);
}

void xenium_write_bank(u8 bank, u8* data){
    u32 bank_size = xenium_get_bank_size(bank);
    if (bank_size == 0)
        return;

    printk("Writing Bank %u ", bank);
    u8 old_bank = xenium_get_bank();
    xenium_set_bank(bank);
    xenium_flash_reset();
    for (u32 i = 0; i < bank_size; i++){
        xenium_flash_program_byte(i, data[i]);
    }
    xenium_set_bank(old_bank);
}

static u8 calc_checksum(u8* data, u32 len){
	//Generator lookup table for poly 0x85
	static const u8 crctable[256] = {
			0x00, 0x85, 0x8F, 0x0A, 0x9B, 0x1E, 0x14, 0x91, 0xB3, 0x36, 0x3C,
			0xB9, 0x28, 0xAD, 0xA7, 0x22, 0xE3, 0x66, 0x6C, 0xE9, 0x78, 0xFD,
			0xF7, 0x72, 0x50, 0xD5, 0xDF, 0x5A, 0xCB, 0x4E, 0x44, 0xC1,	0x43,
			0xC6, 0xCC, 0x49, 0xD8, 0x5D, 0x57, 0xD2, 0xF0, 0x75, 0x7F, 0xFA,
			0x6B, 0xEE, 0xE4, 0x61,	0xA0, 0x25, 0x2F, 0xAA, 0x3B, 0xBE, 0xB4,
			0x31, 0x13, 0x96, 0x9C, 0x19, 0x88, 0x0D, 0x07, 0x82, 0x86, 0x03,
			0x09, 0x8C, 0x1D, 0x98, 0x92, 0x17, 0x35, 0xB0, 0xBA, 0x3F, 0xAE,
			0x2B, 0x21, 0xA4, 0x65, 0xE0, 0xEA, 0x6F, 0xFE, 0x7B, 0x71, 0xF4,
			0xD6, 0x53, 0x59, 0xDC, 0x4D, 0xC8, 0xC2, 0x47,	0xC5, 0x40, 0x4A,
			0xCF, 0x5E, 0xDB, 0xD1, 0x54, 0x76, 0xF3, 0xF9, 0x7C, 0xED, 0x68,
			0x62, 0xE7,	0x26, 0xA3, 0xA9, 0x2C, 0xBD, 0x38, 0x32, 0xB7, 0x95,
			0x10, 0x1A, 0x9F, 0x0E, 0x8B, 0x81, 0x04, 0x89, 0x0C, 0x06, 0x83,
			0x12, 0x97, 0x9D, 0x18, 0x3A, 0xBF, 0xB5, 0x30, 0xA1, 0x24, 0x2E,
			0xAB, 0x6A, 0xEF, 0xE5, 0x60, 0xF1, 0x74, 0x7E, 0xFB, 0xD9, 0x5C,
			0x56, 0xD3, 0x42, 0xC7, 0xCD, 0x48, 0xCA, 0x4F, 0x45, 0xC0, 0x51,
			0xD4, 0xDE, 0x5B, 0x79, 0xFC, 0xF6, 0x73, 0xE2, 0x67, 0x6D, 0xE8,
			0x29, 0xAC, 0xA6, 0x23, 0xB2, 0x37, 0x3D, 0xB8, 0x9A, 0x1F, 0x15,
			0x90, 0x01, 0x84, 0x8E, 0x0B, 0x0F, 0x8A, 0x80, 0x05, 0x94, 0x11,
			0x1B, 0x9E, 0xBC, 0x39, 0x33, 0xB6, 0x27, 0xA2, 0xA8, 0x2D,	0xEC,
			0x69, 0x63, 0xE6, 0x77, 0xF2, 0xF8, 0x7D, 0x5F, 0xDA, 0xD0, 0x55,
			0xC4, 0x41, 0x4B, 0xCE,	0x4C, 0xC9, 0xC3, 0x46, 0xD7, 0x52, 0x58,
			0xDD, 0xFF, 0x7A, 0x70, 0xF5, 0x64, 0xE1, 0xEB, 0x6E, 0xAF, 0x2A,
			0x20, 0xA5, 0x34, 0xB1, 0xBB, 0x3E, 0x1C, 0x99, 0x93, 0x16, 0x87,
			0x02, 0x08, 0x8D
	};
	u8 crc = 0;
	for (u32 byte = 0; byte < len; byte++) {
		crc = (u8) (crctable[(u8) (data[byte] ^ crc)]);
	}
	printk("CRC is %02x\n",crc);
	return crc;
}

void xenium_read_settings(xenium_settings* settings){
    u8 old_bank = xenium_get_bank();
    xenium_set_bank(XENIUM_BANK_RECOVERY);
	xenium_flash_reset();
    xenium_flash_read_stream(XENIUM_SETTINGS_OFFSET,
                             (u8*)settings, sizeof(xenium_settings));

    u8 cs = calc_checksum((u8*)settings,sizeof(xenium_settings)-1);
    if (settings->checksum != cs){
        printk("Settings appear corrupt! reset to default\n");
        memset(settings,0x00,sizeof(xenium_settings));
		settings->checksum = cs;
    }
    xenium_set_bank(old_bank);
}

void xenium_update_settings(xenium_settings* new_settings){
    u8 checksum = calc_checksum((u8*)new_settings, sizeof(xenium_settings)-1);
    new_settings->checksum = checksum;

    u8 old_bank = xenium_get_bank();
    xenium_set_bank(XENIUM_BANK_RECOVERY);
    xenium_sector_erase(XENIUM_SETTINGS_OFFSET);
    for (u32 i = 0; i < sizeof(xenium_settings); i++){
        xenium_flash_program_byte(XENIUM_SETTINGS_OFFSET + i, ((u8*)(new_settings))[i]);
    }
    xenium_set_bank(old_bank);
}