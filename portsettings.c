/**
 * @author      : Arno Lievens (arnolievens@gmail.com)
 * @created     : 20/05/2021
 * @filename    : portsettings.c
 */

#include "portsettings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int portsettings_set_baudrate(portsettings_t* portsettings, const char* str)
{
    speed_t baudrate = 0;
    if (!str || !*str) return -1;

    switch (atol(str)) {
        case 2400: baudrate = B2400; break;
        case 4800: baudrate = B4800; break;
        case 9600: baudrate = B9600; break;
        case 19200: baudrate = B19200; break;
        case 38400: baudrate = B38400; break;
        default: return -1;
    }
    portsettings->baudrate = baudrate;
    return 0;
}

int portsettings_set_port(portsettings_t* portsettings, const char* str)
{
    if (!str || !*str) return -1;

    portsettings->port = calloc(strlen(str), 1);
    strncpy(portsettings->port, str, strlen(str));

    return access(portsettings->port, F_OK);
}

void portsettings_print(const portsettings_t* portsettings)
{
    if (portsettings->port) printf("%-12s = %s\n", "port", portsettings->port);
    else printf("%-12s = none\n", "port");
    printf("%-12s = %i\n", "baudrate", portsettings->baudrate);
    printf("%-12s = %f\n", "wait", portsettings->wait);
    printf("%-12s = %i\n", "count", portsettings->count);
}

void portsettings_die(portsettings_t* portsettings)
{
    if (portsettings->port) free(portsettings->port);
    portsettings->port = NULL;
}
