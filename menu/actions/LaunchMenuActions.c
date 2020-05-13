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
extern xenium_settings settings;

#define EXIT_

static void bios_delete(flash_bank* bios){
	VIDEO_ATTR=0xFFFF0000;
	printk("Width: %u Height: %u\n",vmode.width, vmode.height);
	VIDEO_CURSOR_POSX = vmode.width/2;
    VIDEO_CURSOR_POSY = vmode.height/2;
	printk("\2Deleting %s...",bios->bios_name);
	bios->bank_used = 0;
	VIDEO_ATTR=0xffffffff;
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

int BiosAddItem() {
	
	return 0;
}

int BiosListItems() {
	TEXTMENU menu;
	TEXTMENUITEM *itemPtr;
	TEXTMENUITEM bank[XENIUM_MAX_BANKS];
	TEXTMENUITEM Spacer, AddItem;
	exit = 0;
	memset(&menu,0x00,sizeof(TEXTMENU));
	
    //Create the root menu - MANDATORY
    strcpy(menu.szCaption, "");
	
    for(u32 i = 0; i < XENIUM_MAX_BANKS; i ++){
        if(settings.flash_bank[i].bank_used != 0){
            memset(&bank[i],0x00,sizeof(TEXTMENUITEM));
            strcpy(bank[i].szCaption, settings.flash_bank[i].bios_name);
            bank[i].functionPtr = BiosSelectItem;
            bank[i].functionDataPtr = &settings.flash_bank[i];
			TextMenuAddItem(&menu, &bank[i]);
        }
    }

    itemPtr = &Spacer;
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "--------------------");
    itemPtr->functionPtr = NULL;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(&menu, itemPtr);

    itemPtr = &AddItem;
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Add A New Item");
    itemPtr->functionPtr = NULL;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(&menu, itemPtr);
	
    //Draw the menu
    TextMenu(&menu, menu.firstMenuItem);
    return 0;
}

int BiosSelectItem(flash_bank* bios) {
    TEXTMENU menu;
	TEXTMENUITEM* itemPtr;
    TEXTMENUITEM DeleteItem, RenameItem, SetDefaultItem, BackItem;

	//Create the root menu - MANDATORY
    strncpy(menu.szCaption, bios->bios_name, MENUCAPTIONSIZE);
	
	itemPtr = &DeleteItem;
    memset(itemPtr, 0x00, sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Delete BIOS from Flash");
    itemPtr->functionPtr = bios_delete;
    itemPtr->functionDataPtr = bios;
    TextMenuAddItem(&menu, itemPtr);
	
	itemPtr = &RenameItem;
    memset(itemPtr, 0x00, sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Rename BIOS");
    itemPtr->functionPtr = bios_rename;
    itemPtr->functionDataPtr = bios;
    TextMenuAddItem(&menu, itemPtr);
	
	itemPtr = &SetDefaultItem;
    memset(itemPtr, 0x00, sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Set as Default BIOS");
    itemPtr->functionPtr = bios_set_default;
    itemPtr->functionDataPtr = bios;
    TextMenuAddItem(&menu, itemPtr);
	
	itemPtr = &BackItem;
    memset(itemPtr, 0x00, sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Return");
    itemPtr->functionPtr = exit_menu;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(&menu, itemPtr);
	
    //Draw the menu
    TextMenu(&menu, &BackItem);
	
	if(exit == 2){
		breakOutOfMenu = 1;
		xenium_update_settings(&settings);
	}	
	if (exit)
		return 0;
    return 1;
}

void BiosList(){
	if (!BiosListItems())
		return;

    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(100);
}

void BiosSelected(flash_bank* bios) {
	if (!BiosSelectItem(bios))
		return;

    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(100);
}

void BiosAdd(){
	if (!BiosAddItem())
		return;

    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(100);
}