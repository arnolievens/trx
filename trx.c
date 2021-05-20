/**
 * @author      : Arno Lievens (arnolievens@gmail.com)
 * @created     : 17/05/2021
 * @filename    : trx.c
 */

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "portsettings.h"
#include "serial.h"

#define CMD_LEN 80
#define DEFAULT_WAIT 500

extern char **environ;
extern int errno ;

struct settings {
    char *device;
    char *input;
    char *output;
    int verbose;
    int quiet;
    int help;
} settings;

struct command {
    char *v;
    size_t len;
} command;


/* arg options */
struct option long_options[] = {
    {"device",    required_argument,  NULL,  'd'},
    {"input",     required_argument,  NULL,  'i'},
    {"output",    required_argument,  NULL,  'o'},
    {"baudrate",  required_argument,  NULL,  'b'},
    {"port",      required_argument,  NULL,  'p'},
    {"wait",      required_argument,  NULL,  'w'},
    {"count",     required_argument,  NULL,  'n'},
    {"verbose",   no_argument,        NULL,  'v'},
    {"quiet",     no_argument,        NULL,  'q'},
    {"help",      no_argument,        NULL,  'h'},
    {NULL,        0,                  NULL,  0}
};

void print_settings()
{
    if (settings.input)
        printf("input    = %s\n", settings.input);
    if (settings.output)
        printf("output   = %s\n", settings.output);
    if (settings.device)
        printf("device   = %s\n", settings.device);
    if (1) {
        printf("verbose  = %i\n", settings.verbose);
        printf("quiet    = %i\n", settings.quiet);
        printf("help     = %i\n", settings.help);
    }
}

int parse_config(portsettings_t* portsettings, FILE *file)
{
    char line[CMD_LEN+2];

    while (fgets(line, CMD_LEN+1, file)) {

        /* validate max line length */
        if (strlen(line) >= CMD_LEN) {
            fprintf(stderr,
                    "maximum line length exceeded: %i characters\n",
                    CMD_LEN);
            return -1;
        }

        /* skip comments */
        if (*line == '#') continue;

        char *p;

        p = strtok(line, "= \r\n");

        if (!portsettings->port && (strcmp(p, "port") == 0)) {
            p = strtok(NULL, "= \r\n");
            if (p) {
                portsettings->port = calloc(81, 1);
                strcpy(portsettings->port, p);
            }

        } else if (!portsettings->baudrate && (strcmp(p, "baudrate") == 0)) {
            p = strtok(NULL, "= \r\n");
            if (p) {
                portsettings->baudrate = atol(p);
                if (!portsettings->baudrate) {
                    fprintf(stderr,
                            "illegal baudrate setting: %lu\n",
                            portsettings->baudrate);
                    return -1;
                }
            }

        } else if (!portsettings->wait && (strcmp(p, "wait") == 0)) {
            p = strtok(NULL, "= \r\n");
            if (p) {
                portsettings->wait = atoi(p);
                if (!portsettings->wait) {
                    fprintf(stderr,
                            "invalid \"wait\" value: %i\n",
                            portsettings->wait);
                    return -1;
                }
            }

        } else if (!portsettings->count && (strcmp(p, "count") == 0)) {
            p = strtok(NULL, "= \r\n");
            if (p) {
                portsettings->count = atoi(p);
                if (!portsettings->count) {
                    fprintf(stderr,
                            "invalid \"count\" value: %i\n",
                            portsettings->count);
                    return -1;
                }
            }
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    portsettings_t portsettings = { 0 };
    FILE *input_file = NULL;
    FILE *output_file = NULL;

    command.v = calloc(CMD_LEN, 1);
    command.len = CMD_LEN;

    /* defaults */
    portsettings.wait = DEFAULT_WAIT;
    portsettings.count = -1;

    /* parse options */
    int oc;
    int oi = 0;
    while ((oc = getopt_long(argc, argv, "d:i:o:b:p:w:n:vqh", long_options, &oi)) != -1) {
        switch (oc) {
            case 'd':
                settings.device = optarg;
                break;

            case 'i':
                settings.input = optarg;
                break;

            case 'o':
                settings.output = optarg;
                break;

            case 'b':
                portsettings.baudrate = atol(optarg);
                break;

            case 'p':
                portsettings.port = calloc(strlen(optarg)+1, 1);
                strcpy(portsettings.port, optarg);
                break;

            case 'w':
                portsettings.wait = atoi(optarg);
                break;

            case 'n':
                portsettings.count = atoi(optarg);
                break;

            case 'v':
                settings.verbose = 1;
                break;

            case 'q':
                settings.quiet = 1;
                break;

            case 'h':
                settings.help = 1;
                break;

            default:
                fprintf(stderr, "getopts error - unknown option: %c\n", oc);
                exit(EXIT_FAILURE);
        }
    }

    /* first remaining arg is command */
    while (optind < argc) {
        if (strlen(argv[optind]) > command.len - strlen(command.v)) {
            command.v = realloc(command.v, command.len*2);
            command.len *= 2;
        }
        strcat(command.v, argv[optind]);
        optind++;
    }

    /* read config file (device) */
    if (settings.device) {

        FILE *device_file;
        device_file = fopen(settings.device, "r");

        if (device_file) {
            int e = parse_config(&portsettings, device_file);
            fclose(device_file);
            if (e == -1) exit(EXIT_FAILURE);

        } else {
            fprintf(stderr, "%s \"%s\"\n", strerror(errno), settings.device);
            exit(EXIT_FAILURE);
        }
    }

    /* input file */
    if (settings.input) {
        input_file = fopen(settings.input, "r");
        if (input_file) {
            printf("using input file \'%s\"\n", settings.input);
        } else {
            fprintf(stderr, "%s \"%s\"\n", strerror(errno), settings.device);
            exit(EXIT_FAILURE);
        }
    }

    print_settings();
    printf("\n");
    portsettings_print(&portsettings);
    if (command.v && *command.v) printf("command  = %s\n", command.v);

    /* init serial port */
    if (serial_init(&portsettings) == -1) {
        exit(EXIT_FAILURE);
    }

    char buf[81];
    serial_tx(&portsettings, command.v);

    int counter = 0;
    unsigned int n = 0;

    while (counter < portsettings.wait && n < portsettings.count) {
        if (serial_rx(&portsettings, buf, 80) != -1) {
            printf("response: %s\n", buf);
            counter = 0;
            n++;
        } else {
            printf("counter: %i\n", counter);
            usleep(1000);
            counter++;
        }
    }


    /* die */
    serial_die();
    if (input_file) fclose(input_file);
    if (output_file) fclose(output_file);
    /* if (output_file) fclose(output_file); */
    if (command.v) free(command.v);
    if (portsettings.port) free(portsettings.port);
    exit(EXIT_SUCCESS);
}
