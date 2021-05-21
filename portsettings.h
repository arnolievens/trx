/**
 * @author      : Arno Lievens (arnolievens@gmail.com)
 * @created     : 20/05/2021
 * @filename    : portsettings.h
 */

#ifndef PORTSETTINGS_H
#define PORTSETTINGS_H

typedef struct {
    unsigned long baudrate;
    char *port;
    double wait;
    unsigned int count;
} portsettings_t;

extern void portsettings_print(const portsettings_t* portsettings);

#endif

// vim:ft=c

