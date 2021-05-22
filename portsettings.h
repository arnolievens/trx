/**
 * @author      : Arno Lievens (arnolievens@gmail.com)
 * @created     : 20/05/2021
 * @filename    : portsettings.h
 */

#ifndef PORTSETTINGS_H
#define PORTSETTINGS_H

#include <termios.h>
#include <sys/time.h>

/**
 * object containing all settings necessary for serial connection
 */
typedef struct {
   speed_t baudrate;      /**< baudrate qs defined in termios.h */
   char *port;            /**< serial device file */
   unsigned int count;    /**< amount of lines will be attempted to read */
   double timeout;        /**< msec passed when attempting to read line */
} portsettings_t;

/**
 * set defaults
 *
 * @return portsettings object with default values
 */
extern portsettings_t portsettings_default(void);

/**
 * fancy print all port settings
 *
 * @param[in] portsettings object
 */
extern void portsettings_print(const portsettings_t* portsettings);

/**
 * set baudrate
 *
 * @param[out] portsettings object in which baudrate will be updated
 * @param[in] baudrate to be parsed and validated
 * @return status 0 for succes, -1 for failure
 */
extern int portsettings_set_baudrate(portsettings_t* portsettings, const char* baudrate);

/**
 * set port
 *
 * @param[out] portsettings object in which port will be updated
 * @param[in] port serial device - validated before setting
 * @return status 0 for succes, -1 for failure
 */
extern int portsettings_set_port(portsettings_t* portsettings, const char* port);

/**
 * set timeout
 *
 * @param[out] portsettings object in which timeout will be updated
 * @param[in] timeout string containing a positive double value or 0
 * @return status 0 for succes, -1 for failure
 */
extern int portsettings_set_timeout(portsettings_t* portsettings, const char* timeout);

/**
 * free allocated memory
 *
 * @param[in] portsettings all dyn. allocated memory in this object to be freed
 */
extern void portsettings_die(portsettings_t* portsettings);

#endif

// vim:ft=c

