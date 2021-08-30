/**
 * @author      : Arno Lievens (arnolievens@gmail.com)
 * @created     : 20/05/2021
 * @filename    : serial.h
 */

#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>

#include "../include/portsettings.h"

/**
 * initialize serial port, open file and set connection properties
 *
 * @param[in] portsettings struct containing all settings
 * @return status 0 for succes, -1 for failure
 */
extern int serial_init(const portsettings_t* portsettings);

/*
 * transmit string on intialized port
 *
 * blocks until entire command has been sent
 *
 * @param[in] portsettings struct containing all settings
 * @param[in] command this string will be transmitted
 * @return status 0 for succes, -1 for failure
 */
extern int serial_tx(const portsettings_t* portsettings, const char* command);

/*
 * receive line on serial port
 *
 * blocks until line is read or timeout has passed
 * line delimiter is excluded and string is null-terminated
 * a timeout returns empty string with status 0
 *
 * @param[in] portsettings struct containing all settings
 * @param[out] buffer buffer used to write received line
 * @return status 0 for succes, -1 for failure
 */
extern int serial_rx(const portsettings_t* portsettings, char *buffer, size_t length);

/*
 * free used resources
 * reset serial port settings
 *
 * @return status 0 for succes, -1 for failure
 */
extern int serial_die(void);

#endif

// vim:ft=c

