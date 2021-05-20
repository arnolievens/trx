/**
 * @author      : Arno Lievens (arnolievens@gmail.com)
 * @created     : 20/05/2021
 * @filename    : serial.h
 */

#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>

#include "portsettings.h"

extern int serial_init(portsettings_t* portsettings);
extern int serial_tx(portsettings_t* portsettings, const char* cmd);
extern int serial_rx(portsettings_t* portsettings, char *buffer, size_t length);
extern void serial_die(void);

#endif

// vim:ft=c

