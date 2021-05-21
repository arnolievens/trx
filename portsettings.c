/**
 * @author      : Arno Lievens (arnolievens@gmail.com)
 * @created     : 20/05/2021
 * @filename    : portsettings.c
 */

#include "portsettings.h"

#include <stdio.h>

void portsettings_print(const portsettings_t* portsettings)
{
    if (portsettings->port) printf("%-12s = %s\n", "port", portsettings->port);
    else printf("%-12s = none\n", "port");
    printf("%-12s = %lu\n", "baudrate", portsettings->baudrate);
    printf("%-12s = %f\n", "wait", portsettings->wait);
    printf("%-12s = %i\n", "count", portsettings->count);
}

