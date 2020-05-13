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
#include "LaunchMenuActions.h"
#include "xenium.h"

extern xenium_settings settings;

TEXTMENU *TextMenuInit(void) {

    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;

    //Create the root menu - MANDATORY
    menuPtr = malloc(sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "");
    menuPtr->firstMenuItem=NULL;

    itemPtr = malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Launch Menu");
    itemPtr->functionPtr = BiosList;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Disk Tools");
    itemPtr->functionPtr=DrawChildTextMenu;
    itemPtr->functionDataPtr = (void *)PhyMenuInit();
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "EEprom Tools");
    itemPtr->functionPtr=DrawChildTextMenu;
    itemPtr->functionDataPtr = (void *)PhyMenuInit();
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Settings");
    itemPtr->functionPtr=DrawChildTextMenu;
    itemPtr->functionDataPtr = (void *)PhyMenuInit();
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Reboot (Slow)");
    itemPtr->functionPtr=SlowReboot;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Reboot (Fast)");
    itemPtr->functionPtr=QuickReboot;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Power Off");
    itemPtr->functionPtr=PowerOff;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    return menuPtr;
}
