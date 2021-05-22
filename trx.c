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

#define LENGTH(a) sizeof(a)/sizeof(a[0])

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
    {"timeout",   required_argument,  NULL,  'w'},
    {"count",     required_argument,  NULL,  'n'},
    {"verbose",   no_argument,        NULL,  'v'},
    {"quiet",     no_argument,        NULL,  'q'},
    {"help",      no_argument,        NULL,  'h'},
    {NULL,        0,                  NULL,  0}
};

void print_help()
{
    const char* help[] = {
        "usage: trx [options] [command] [command] [...]",
        "",
        "  -b  --baudrate  serial port baudrate setting (must be valid)",
        "                  eg 9600, 19200, ...",
        "",
        "  -p  --port      serial port device file",
        "",
        "  -t  --timeout   receiver will wait <timeout> msec for new input",
        "                  unless count is fulfilled",
        "",
        "  -n  --count     max number of lines to be read",
        "",
        "  -d  --device    device config file",
        "                  search in $XDG_CONFIG_HOME when no abs path given",
        "",
        "  -i  --input     contents of this file will be transmitted per line",
        "                  as if they are given as separate arguments",
        "",
        "  -o  --output    response is written to file instead of stdout",
        "",
        "  -v  --verbose   verbose output",
        "",
        "  -q  --quiet     suppress writing response to stdout",
        "                  does not mute stderr",
        "",
        "  -h  --help      this menu",
        "",
        "examples:",
        "  trx -d someDevice --timeout 0.2 -n 1 \"some command\"",
        "  trx -p /dev/ttyS0 -i transmit.txt -q",
        "  trx -d ./dev.config \"cmd1\" \"cmd2\"",
        "",
    };
    for (size_t i = 0; i < LENGTH(help); i++) printf("%s\n", help[i]);
}

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

        } else if (!portsettings->timeout && (strcmp(p, "timeout") == 0)) {
            p = strtok(NULL, "= \r\n");
            if (portsettings_set_timeout(portsettings, p) == -1) {
                fprintf(stderr, "invalid timeout: %s\n", p);
                return -1;
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
    FILE *input_file = NULL;
    FILE *output_file = NULL;
    portsettings_t portsettings = portsettings_default();

    /* parse options */
    int oc;
    int oi = 0;
    while ((oc = getopt_long(argc, argv, "d:i:o:b:p:t:n:vqh", long_options, &oi)) != -1) {
        switch (oc) {

            /* port settings */
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

            case 't':
                if (portsettings_set_timeout(&portsettings, optarg) != -1) {
                    break;
                } else {
                    fprintf(stderr, "invalid timeout: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }

            case 'n':
                portsettings.count = atoi(optarg);
                break;

            /* program settings */
            case 'd':
                settings.device = optarg;
                break;

            case 'i':
                settings.input = optarg;
                break;

            case 'o':
                settings.output = optarg;
                break;

            case 'v':
                settings.verbose = 1;
                break;

            case 'q':
                settings.quiet = 1;
                break;

            case 'h':
                print_help();
                exit(EXIT_SUCCESS);

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
