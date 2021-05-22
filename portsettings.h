/**
 * @author      : Arno Lievens (arnolievens@gmail.com)
 * @created     : 20/05/2021
 * @filename    : portsettings.h
 */

#ifndef PORTSETTINGS_H
#define PORTSETTINGS_H

#include <termios.h>

typedef struct {
    speed_t baudrate;
    char *port;
    double wait;
    unsigned int count;
} portsettings_t;

extern void portsettings_print(const portsettings_t* portsettings);
extern int portsettings_set_baudrate(portsettings_t* portsettings, const char* str);
extern int portsettings_set_port(portsettings_t* portsettings, const char* str);
extern void portsettings_die(portsettings_t* portsettings);


#endif

// vim:ft=c

