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
        printf("%-12s = %s\n", "input", settings.input);
    if (settings.output)
        printf("%-12s = %s\n", "output", settings.output);
    if (settings.device)
        printf("%-12s = %s\n", "device", settings.device);
    if (1) {
        printf("%-12s = %i\n", "verbose", settings.verbose);
        printf("%-12s = %i\n", "quiet", settings.quiet);
        printf("%-12s = %i\n", "help", settings.help);
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
            if (portsettings_set_port(portsettings, p) == -1) {
                fprintf(stderr, "invalid serial port: %s\n", p);
                exit(EXIT_FAILURE);
            }

        } else if (!portsettings->baudrate && (strcmp(p, "baudrate") == 0)) {
            p = strtok(NULL, "= \r\n");
            if (portsettings_set_baudrate(portsettings, p) == -1) {
                fprintf(stderr, "invalid baudrate: %s\n", p);
                return -1;
            }

        } else if (!portsettings->wait && (strcmp(p, "wait") == 0)) {
            p = strtok(NULL, "= \r\n");
            if (p) {
                portsettings->wait = atof(p);
                if (!portsettings->wait) {
                    fprintf(stderr,
                            "invalid \"wait\" value: %f\n",
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

int run(const portsettings_t* portsettings, const char* cmd)
{
    serial_tx(portsettings, cmd);

    char buf[81];
    unsigned int n = 0;

    while (n < portsettings->count) {
        if (serial_rx(portsettings, buf, 80) != -1) {
            if (!settings.quiet) {
                if (settings.verbose) printf("%-12s = ", "response");
                if (*buf) {
                    printf("%s\n", buf);
                } else {
                    if (settings.verbose) printf("<timeout>\n");
                    break;
                }
            }
            n++;
        }
        else break;
    }
    return 0;
}

int main(int argc, char **argv)
{
    portsettings_t portsettings = { 0 };
    FILE *input_file = NULL;
    FILE *output_file = NULL;

    /* command.v = calloc(CMD_LEN, 1); */
    /* command.len = CMD_LEN; */

    /* defaults */
    portsettings.wait = 1;
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
                if (portsettings_set_baudrate(&portsettings, optarg) != -1) {
                    break;
                } else {
                    fprintf(stderr, "invalid baudrate: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }

            case 'p':
                if (portsettings_set_port(&portsettings, optarg) != -1) {
                    break;
                } else {
                    fprintf(stderr, "invalid serial port: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }

            case 'w':
                portsettings.wait = atof(optarg);
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

    /* verbose print */
    if (settings.verbose) {
        print_settings();
        portsettings_print(&portsettings);
    }

    /* init serial port */
    if (serial_init(&portsettings) == -1) {
        exit(EXIT_FAILURE);
    }

    /* run arg commands */
    for (size_t i = optind; i < argc; i++) {
        if (settings.verbose) printf("%-12s = %s\n", "command", argv[i]);
        run(&portsettings, argv[i]);
    }

    /* run input file */
    if (settings.input) {
        input_file = fopen(settings.input, "r");
        if (input_file) {

            char line[CMD_LEN+2];

            if (settings.verbose) printf("using input file \'%s\"\n", settings.input);

            while (fgets(line, CMD_LEN+1, input_file)) {

                /* filter comments and empty lines */
                if (*line == '#' || *line == '\n') continue;

                /* validate max line length */
                if (strlen(line) >= CMD_LEN) {
                    fprintf(stderr,
                            "maximum line length exceeded: %i characters\n",
                            CMD_LEN);
                    return -1;
                }

                /* trim trailing newlines */
                if (line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';

                if (settings.verbose) printf("%-12s = %s\n", "command", line);
                run(&portsettings, line);
            }

            fclose(input_file);

        } else {
            fprintf(stderr, "%s \"%s\"\n", strerror(errno), settings.device);
            exit(EXIT_FAILURE);
        }
    }

    /* die */
    serial_die();
    portsettings_die(&portsettings);
    if (output_file) fclose(output_file);
    /* if (output_file) fclose(output_file); */
    exit(EXIT_SUCCESS);
}
