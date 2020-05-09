/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "include/config.h"
#include "TextMenu.h"
#include "ResetMenuActions.h"
#include "VideoInitialization.h"
#include "xenium.h"

TEXTMENU *LaunchMenuInit(void) {

    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;
    
    xenium_settings settings;
    xenium_read_settings(&settings);
    
    //Create the root menu - MANDATORY
    menuPtr = malloc(sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "");
    menuPtr->firstMenuItem=NULL;
    
    for(u32 i = 0; i < XENIUM_MAX_BANKS; i ++){
        if(settings.flash_bank[i].bank_used != 0){
            itemPtr = malloc(sizeof(TEXTMENUITEM));
            memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
            strcpy(itemPtr->szCaption, settings.flash_bank[i].bios_name);
            //itemPtr->functionPtr = XeniumBootBank;
            //itemPtr->functionDataPtr = (void*)(&settings.flash_bank[i].bank_used);
            itemPtr->functionPtr = NULL;
            itemPtr->functionDataPtr = NULL;
			TextMenuAddItem(menuPtr, itemPtr);
        }
    }

    itemPtr = malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "--------------------");
    itemPtr->functionPtr = NULL;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Add A New Item");
    itemPtr->functionPtr = NULL;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Remove An Item");
    itemPtr->functionPtr = NULL;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Rename An Item");
    itemPtr->functionPtr = NULL;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);
    
    itemPtr = malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Set A Default Item");
    itemPtr->functionPtr = NULL;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);


    return menuPtr;
}
