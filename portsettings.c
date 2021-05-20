/**
 * @author      : Arno Lievens (arnolievens@gmail.com)
 * @created     : 20/05/2021
 * @filename    : portsettings.c
 */

#include "portsettings.h"

#include <stdio.h>

void portsettings_print(const portsettings_t* portsettings)
{
    if (portsettings->port) printf("port     = %s\n", portsettings->port);
    else printf("port     = none\n");
    printf("baudrate = %lu\n", portsettings->baudrate);
    printf("wait     = %i\n", portsettings->wait);
    printf("count    = %i\n", portsettings->count);
}

