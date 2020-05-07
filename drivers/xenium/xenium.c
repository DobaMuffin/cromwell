#include "boot.h"
#include "BootFlash.h"
#include "memory_layout.h"
#include "xenium.h"

//See https://github.com/Ryzee119/OpenXenium/blob/master/Firmware/openxenium.vhd
//for Xenium CPLD details

//IO CONTROL INTERFACE
void xenium_set_bank(u8 bank){
    printk("Bank set to %u\n", bank);
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
    memcpy(data, &lpc_mem_map[address], len);
}

static void xenium_flash_write_stream(u32 address, u8* data, u32 len){
    volatile u8 * volatile lpc_mem_map = (u8 *)LPCFlashadress;
    memcpy(&lpc_mem_map[address], data, len);
}

static void xenium_flash_reset(){
    lpc_send_byte(0x00000000,0xF0);
}

static u8 xenium_flash_busy(){
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
    xenium_flash_reset();
    lpc_send_byte(0xAAAA, 0xAA);
    lpc_send_byte(0x5555, 0x55);
    lpc_send_byte(0xAAAA, 0x80);
    lpc_send_byte(0xAAAA, 0xAA);
    lpc_send_byte(0x5555, 0x55);
    lpc_send_byte(sector_address, 0x30);
    while(xenium_flash_busy());
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
	xenium_set_bank(bank);
    xenium_flash_reset();
    for(u32 i = 0; i <= bank_size; i += XENIUM_FLASH_SECTOR_SIZE){
        printk(" . ");
        xenium_sector_erase(i);
    }
    printk("\n", bank);
}

void xenium_write_bank(u8 bank, u8* data){
    u32 bank_size = xenium_get_bank_size(bank);
    if (bank_size == 0)
        return;

	printk("Writing Bank %u ", bank);
	xenium_set_bank(bank);
    xenium_flash_reset();
    xenium_flash_write_stream(0, data, bank_size);  
}