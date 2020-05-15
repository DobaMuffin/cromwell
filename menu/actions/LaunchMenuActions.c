/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "LaunchMenuActions.h"
#include "xenium.h"
#include "TextMenu.h"
#include "BootVideo.h"

u8 exit = 0;
extern volatile CURRENT_VIDEO_MODE_DETAILS vmode;
extern volatile xenium_settings settings;

static void bios_launch(flash_bank* bios){
    //See https://xboxdevwiki.net/PIC
    int pic_scratch = I2CTransmitByteGetReturn(0x10, 0x1B);
    I2CTransmitWord(0x10, 0x1B00 | ((u8)pic_scratch & ~0x04)); //Clear flag at 0x04 to force boot animation on reboot.
    
    xenium_set_bank(bios->bank_used);
    xenium_set_led(bios->bios_led_colour);
    BootStopUSB();

    I2CRebootQuick();
}

static void bios_delete(flash_bank* bios){
	//VIDEO_ATTR=0xFFFF0000;
	//printk("Width: %u Height: %u\n",vmode.width, vmode.height);
	//VIDEO_CURSOR_POSX = vmode.width/2;
    //VIDEO_CURSOR_POSY = vmode.height/2;
	//printk("\2Deleting %s...",bios->bios_name);
	bios->bank_used = 0;
	//VIDEO_ATTR=0xffffffff;
	breakOutOfMenu = 1;
	exit = LAUNCHMENU_EXIT;
}

static void bios_rename(flash_bank* bios){
	printk("Rename %s\n",bios->bios_name);
	wait_ms(1000);
	breakOutOfMenu = 1;
	exit = LAUNCHMENU_EXIT;
}

static void bios_set_default(flash_bank* bios){
	printk("Set Default %s\n",bios->bios_name);
	settings.default_bank = bios->bank_used;
	breakOutOfMenu = 1;
	exit = LAUNCHMENU_EXIT;
}

static void exit_menu(__attribute__((unused)) void * stub){
	exit = LAUNCHMENU_BACK;
	breakOutOfMenu = 1;
}

static int _BiosAddItem() {
	
	return 0;
}

static int _BiosSelectItem(flash_bank* bios) {
    TEXTMENU menu;
    TEXTMENUITEM LaunchItem, DeleteItem, RenameItem, SetDefaultItem, BackItem;
    memset(&menu, 0x00, sizeof(TEXTMENU));
    memset(&LaunchItem, 0x00, sizeof(TEXTMENUITEM));
    memset(&DeleteItem, 0x00, sizeof(TEXTMENUITEM));
    memset(&RenameItem, 0x00, sizeof(TEXTMENUITEM));
    memset(&SetDefaultItem, 0x00, sizeof(TEXTMENUITEM));
    memset(&BackItem, 0x00, sizeof(TEXTMENUITEM));
    

	//Create the root menu - MANDATORY
    strncpy(menu.szCaption, bios->bios_name, MENUCAPTIONSIZE);

    strcpy(LaunchItem.szCaption, "Launch BIOS");
    LaunchItem.functionPtr = bios_launch;
    LaunchItem.functionDataPtr = bios;
    TextMenuAddItem(&menu, &LaunchItem);
	
	strcpy(DeleteItem.szCaption, "Delete BIOS");
    DeleteItem.functionPtr = bios_delete;
    DeleteItem.functionDataPtr = bios;
    TextMenuAddItem(&menu, &DeleteItem);
	
	strcpy(RenameItem.szCaption, "Rename BIOS");
    RenameItem.functionPtr = bios_rename;
    RenameItem.functionDataPtr = bios;
    TextMenuAddItem(&menu, &RenameItem);
	
	strcpy(SetDefaultItem.szCaption, "Set as Instant Boot Default");
    SetDefaultItem.functionPtr = bios_set_default;
    SetDefaultItem.functionDataPtr = bios;
    TextMenuAddItem(&menu, &SetDefaultItem);
	
	strcpy(BackItem.szCaption, "Return");
    BackItem.functionPtr = exit_menu;
    BackItem.functionDataPtr = NULL;
    TextMenuAddItem(&menu, &BackItem);
	
    //Draw the menu
    TextMenu(&menu, &LaunchItem);
	
	if(exit == 2){
		breakOutOfMenu = 1;
		xenium_update_settings(&settings);
	}	
	if (exit)
		return 0;
    return 1;
}

static int _BiosListItems(void) {
	TEXTMENU menu;
	TEXTMENUITEM bank[XENIUM_MAX_BANKS];
	TEXTMENUITEM Spacer, AddItem;
	exit = 0;
	memset(&menu,0x00,sizeof(TEXTMENU));
	memset(&bank[0],0x00,sizeof(TEXTMENUITEM) * XENIUM_MAX_BANKS);
    memset(&Spacer,0x00,sizeof(TEXTMENUITEM));
    memset(&AddItem,0x00,sizeof(TEXTMENUITEM));
    
    //Create the root menu - MANDATORY
    strcpy(menu.szCaption, "Launch Menu");
	
    for(u32 i = 0; i < XENIUM_MAX_BANKS; i ++){
        if(settings.flash_bank[i].bank_used != 0){
            strcpy(bank[i].szCaption, settings.flash_bank[i].bios_name);
            bank[i].functionPtr = _BiosSelectItem;
            bank[i].functionDataPtr = &settings.flash_bank[i];
			TextMenuAddItem(&menu, &bank[i]);
        }
    }

    strcpy(Spacer.szCaption, "--------------------");
    Spacer.functionPtr = NULL;
    Spacer.functionDataPtr = NULL;
    TextMenuAddItem(&menu, &Spacer);

    strcpy(AddItem.szCaption, "Add A New Item");
    AddItem.functionPtr = NULL;
    AddItem.functionDataPtr = NULL;
    TextMenuAddItem(&menu, &AddItem);
	
    //Draw the menu
    TextMenu(&menu, menu.firstMenuItem);
    return 0;
}

void BiosList(void *ignored){
	if (!_BiosListItems())
		return;

    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(100);
}