/*
 * Sequences the necessary post-reset actions from as soon as we are able to run C
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
 */

#include "boot.h"
#include "BootEEPROM.h"
#include "BootFlash.h"
#include "BootFATX.h"
#include "xbox.h"
#include "cpu.h"
#include "config.h"
#include "video.h"
#include "memory_layout.h"
#include "xenium.h"

#define SILENT_MODE

JPEG jpegBackdrop;

int nTempCursorMbrX, nTempCursorMbrY;

extern volatile int nInteruptable;

volatile CURRENT_VIDEO_MODE_DETAILS vmode;
extern KNOWN_FLASH_TYPE aknownflashtypesDefault[];

volatile xenium_settings settings;

//////////////////////////////////////////////////////////////////////
//
//  BootResetAction()

extern void BootResetAction ( void ) {
    bool fMbrPresent=false;
    bool fSeenActive=false;
    int nFATXPresent=false;
    int nTempCursorX, nTempCursorY;
    int n, nx;

    memcpy(&cromwell_config,(void*)(0x03A00000+0x20),4);
    memcpy(&cromwell_retryload,(void*)(0x03A00000+0x24),4);
    memcpy(&cromwell_loadbank,(void*)(0x03A00000+0x28),4);
    memcpy(&cromwell_Biostype,(void*)(0x03A00000+0x2C),4);

    VIDEO_CURSOR_POSX= 150;
    VIDEO_CURSOR_POSY= 150;

    VIDEO_AV_MODE = 0xff;
    nInteruptable = 0;

    // prep our BIOS console print state
    VIDEO_ATTR=0xffffffff;

    // init malloc() and free() structures
    MemoryManagementInitialization((void *)MEMORYMANAGERSTART, MEMORYMANAGERSIZE);

    BootInterruptsWriteIdt();

    // initialize the PCI devices
    //bprintf("BOOT: starting PCI init\n");
    BootPciPeripheralInitialization();
    // Reset the AGP bus and start with good condition
    BootAGPBUSInitialization();

    // We disable The CPU Cache
    cache_disable();
    // We Update the Microcode of the CPU
    display_cpuid_update_microcode();
    // We Enable The CPU Cache
    cache_enable();
    //setup_ioapic();
    // We look how much memory we have ..
    BootDetectMemorySize();

    BootEepromReadEntireEEPROM();

    // Load and Init the Background image
    // clear the Video Ram
    memset((void *)FB_START,0x00,0x400000);

    BootVgaInitializationKernelNG((CURRENT_VIDEO_MODE_DETAILS *)&vmode);

    {
        // decode and malloc backdrop bitmap
        extern int _start_backdrop;
        BootVideoJpegUnpackAsRgb(
            (u8 *)&_start_backdrop,
            &jpegBackdrop
        );
    }
    // display solid red frontpanel LED while we start up
    xenium_set_led(XENIUM_LED_RED);
    setLED("rrrr");
    // paint the backdrop
#ifndef DEBUG_MODE
    BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
#endif

    I2CTransmitWord(0x10, 0x1a01); // unknown, done immediately after reading out eeprom data
    I2CTransmitWord(0x10, 0x1b04); // unknown

    /* Here, the interrupts are Switched on now */
    BootPciInterruptEnable();
    /* We allow interrupts */
    nInteruptable = 1;

    I2CTransmitWord(0x10, 0x1901); // no reset on eject

    VIDEO_CURSOR_POSY=vmode.ymargin;
    VIDEO_CURSOR_POSX=(vmode.xmargin/*+64*/)*4;

#ifndef SILENT_MODE
    if (cromwell_config==XROMWELL) 	printk("\2Xromwell " VERSION "\2\n" );
    if (cromwell_config==CROMWELL)	printk("\2Cromwell BIOS " VERSION "\2\n" );
    VIDEO_CURSOR_POSY=vmode.ymargin+32;
    VIDEO_CURSOR_POSX=(vmode.xmargin/*+64*/)*4;
    printk( __DATE__ " (rev. %s) -  https://github.com/XboxDev/cromwell\n", GITREV);
    VIDEO_CURSOR_POSX=(vmode.xmargin/*+64*/)*4;
    printk("Available RAM: %d MB\n",xbox_ram);

    // capture title area
    VIDEO_ATTR=0xffc8c8c8;
    printk("Encoder: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%s  ", VideoEncoderName());
    VIDEO_ATTR=0xffc8c8c8;
    printk("Cable: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%s  ", AvCableName());

    if (I2CGetTemperature(&n, &nx)) {
        VIDEO_ATTR=0xffc8c8c8;
        printk("CPU Temp: ");
        VIDEO_ATTR=0xffc8c800;
        printk("%doC  ", n);
        VIDEO_ATTR=0xffc8c8c8;
        printk("M/b Temp: ");
        VIDEO_ATTR=0xffc8c800;
        printk("%doC  ", nx);
    }

    printk("\n");
#endif
    xenium_set_led(XENIUM_LED_BLUE);
    setLED("rrrr");

    VIDEO_ATTR=0xffffffff;

    // gggb while waiting for Ethernet & Hdd

    setLED("gggx");

    // set Ethernet MAC address from EEPROM
    {
        volatile u8 * pb=(u8 *)0xfef000a8;  // Ethernet MMIO base + MAC register offset (<--thanks to Anders Gustafsson)
        int n;
        for(n=5; n>=0; n--) {
            *pb++=    eeprom.MACAddress[n];    // send it in backwards, its reversed by the driver
        }
    }
#ifndef SILENT_MODE
    BootEepromPrintInfo();
#endif
    /*
    #ifdef FLASH
    	{
    		OBJECT_FLASH of;
    		memset(&of,0x00,sizeof(of));
    		of.m_pbMemoryMappedStartAddress=(u8 *)LPCFlashadress;
    		BootFlashGetDescriptor(&of, (KNOWN_FLASH_TYPE *)&aknownflashtypesDefault[0]);
    		VIDEO_ATTR=0xffc8c8c8;
    		printk("Flash type: ");
    		VIDEO_ATTR=0xffc8c800;
    		printk("%s\n", of.m_szFlashDescription);
    	}
    #endif
    */
    nTempCursorX=VIDEO_CURSOR_POSX;
    nTempCursorY=VIDEO_CURSOR_POSY;
#ifndef SILENT_MODE
    printk("BOOT: start USB init\n");
#endif
    BootStartUSB();

    // init the IDE devices
#ifndef SILENT_MODE
    VIDEO_ATTR=0xffc8c8c8;
    printk("Initializing IDE Controller\n");
#endif
    BootIdeWaitNotBusy(0x1f0);
    wait_ms(200);
#ifndef SILENT_MODE
    printk("Ready\n");
#endif
    // reuse BIOS status area

#ifndef DEBUG_MODE
    BootVideoClearScreen(&jpegBackdrop, nTempCursorY, VIDEO_CURSOR_POSY+1);  // blank out volatile data area
#endif
    VIDEO_CURSOR_POSX=nTempCursorX;
    VIDEO_CURSOR_POSY=nTempCursorY;

    BootIdeInit();
    printk("\n");

    nTempCursorMbrX=VIDEO_CURSOR_POSX;
    nTempCursorMbrY=VIDEO_CURSOR_POSY;

    // if we made it this far, lets have a solid green LED to celebrate
    setLED("gggg");
    xenium_set_led(XENIUM_LED_OFF);

    u8 xenium_present = xenium_is_detected();
    printk("Xenium detected: %u, Currently on bank %u\n",xenium_present, 
                                                         xenium_get_bank());

    if(!xenium_present){
        printf("\2ERROR XENIUM NOT DETECTED\n\n");
        //while(1);
    }
    u8 boot_bank = xenium_get_bank();

    memset(&settings,0x00,sizeof(xenium_settings));
    xenium_read_settings(&settings);
	    
    if(boot_bank != XENIUM_BANK_BOOTLOADER){
        printk("You booted into this BIOS from XeniumOS\n");
        printk("Setting bank %u as used\n", boot_bank);
        settings.flash_bank[boot_bank - 3].bank_used = boot_bank;
        settings.flash_bank[boot_bank - 3].bios_size = 0x40000;
        strcpy(settings.flash_bank[0].bios_name,"OpenXeniumOS");
        //xenium_update_settings(&settings);
        //xenium_read_settings(&settings);
    }
   // wait_ms(20000);

	//Populate dummy data for testing
	
    settings.flash_bank[0].bank_used = 3;
    settings.flash_bank[0].bios_size = 0x40000;
    settings.flash_bank[0].bios_led_colour = XENIUM_LED_PURPLE;
    strcpy(settings.flash_bank[0].bios_name,"M8Plus");
	
    settings.flash_bank[1].bank_used = 4;
    settings.flash_bank[1].bios_size = 0x40000;
    settings.flash_bank[1].bios_led_colour = XENIUM_LED_BLUE;
    strcpy(settings.flash_bank[1].bios_name,"OpenXeniumOS");
	
    settings.flash_bank[2].bank_used = 5;
    settings.flash_bank[2].bios_size = 0x40000;
    settings.flash_bank[2].bios_led_colour = XENIUM_LED_RED;
    strcpy(settings.flash_bank[2].bios_name,"IND-BIOS");

    settings.flash_bank[3].bank_used = 0;

    //xenium_update_settings(&settings);
    //xenium_read_settings(&settings);
	
    while(1) {
        TextMenu(TextMenuInit(),NULL);
    }
}
