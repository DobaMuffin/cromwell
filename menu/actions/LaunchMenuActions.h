/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "boot.h"
#include "BootIde.h"
#include "video.h"
#include "xenium.h"

#define LAUNCHMENU_EXIT 2
#define LAUNCHMENU_BACK 1

void BiosList(void *ignored);