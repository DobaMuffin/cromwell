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

static void xenium_get_bank_info(u8 bank, u32* bank_size, u32* address_start){
    switch (bank){
        case XENIUM_BANK_1:
            *bank_size = 0x40000;
            *address_start = 0x000000;
            break;
        case XENIUM_BANK_2:
            *bank_size = 0x40000;
            *address_start = 0x040000;
            break;
        case XENIUM_BANK_3:
            *bank_size = 0x40000;
            *address_start = 0x080000;
            break;
        case XENIUM_BANK_4:
            *bank_size = 0x40000;
            *address_start = 0x0C0000;
            break;
        case XENIUM_BANK_RECOVERY:
            *bank_size = 0x40000;
            *address_start = 0x1C0000;
            break;
        case XENIUM_BANK_1_512:
            *bank_size = 0x80000;
            *address_start = 0x000000;
            break;
        case XENIUM_BANK_2_512:
            *bank_size = 0x80000;
            *address_start = 0x080000;
            break;
        case XENIUM_BANK_1_1024:
            *bank_size = 0x100000;
            *address_start = 0x000000;
            break;
        default:
            //Let's not erase the OS banks!
            *bank_size = 0;
            break;
    }
}

static void xenium_sector_erase(u32 sector){
    u32 SA;
    printk("Erasing sector %u...", sector);
    if(sector<=30){
        sector=sector<<3;
        SA=sector<<12;
    }
    else if(sector == 31){
        sector = 0b11111000;
        SA=sector<<12;
    }
    else if (sector == 32){
        sector = 0b11111100;
        SA=sector<<12;
    }
    else if (sector == 33){
        sector = 0b11111101;
        SA=sector<<12;
    }
    else if (sector == 34){
        sector = 0b11111110;
        SA=sector<<12;
    }
    else {
        return;
    }
    xenium_flash_reset();
    lpc_send_byte(0xAAAA, 0xAA);
    lpc_send_byte(0x5555, 0x55);
    lpc_send_byte(0xAAAA, 0x80);
    lpc_send_byte(0xAAAA, 0xAA);
    lpc_send_byte(0x5555, 0x55);
    lpc_send_byte(SA,    0x30);
    while(xenium_flash_busy());
    printk("complete!\n");
}

static void xenium_flash_program_byte(u32 address, u8 data){
    lpc_send_byte(0xAAAA, 0xAA);
    lpc_send_byte(0x5555, 0x55);
    lpc_send_byte(0xAAAA, 0xA0);
    lpc_send_byte(address, data);
    while(xenium_flash_busy());
}

u8 xenium_is_detected(){
    //Genuine xenium needs a read a 0x74 to get it going
    xenium_flash_read_byte(0x74);

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
    const u32 SECTOR_SIZE = 0x10000;
    u32 bank_size;
    u32 address_start, address_end; 
    u32 sector_start, sector_end;

    xenium_get_bank_info(bank, &bank_size, &address_start);
    address_end = address_start + bank_size;
    if (bank_size == 0)
        return;

    //Convert the address to the corresponding sectors
    sector_start = address_start / SECTOR_SIZE;
    if (bank == XENIUM_BANK_RECOVERY){
        //Recovery bank is at the end of the flash chip which has some
        //different sector sizes, instead of compensating for all that,
        //I just manually set the last sector for this case.
        sector_end = 34;
    } else {
        sector_end = address_end / SECTOR_SIZE;
    }
    printk("Erasing Bank %u (Sector %u to %u)\n", bank,
                                                 sector_start,
                                                 sector_end);
    
    xenium_flash_reset();
    for(u32 i = sector_start; i <= sector_end; i++){
        xenium_sector_erase(i);
    }  
}

void xenium_write_bank(u8 bank, u8* data){
    u32 bank_size;
    u32 address_start; 

    xenium_get_bank_info(bank, &bank_size, &address_start);
    if (bank_size == 0)
        return;

    xenium_flash_reset();
    //We assume the bank is already erased using xenium_erase_bank()
    xenium_flash_write_stream(address_start, data, bank_size);  
}