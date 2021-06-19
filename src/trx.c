/**
 * @author      : Arno Lievens (arnolievens@gmail.com)
 * @created     : 17/05/2021
 * @filename    : trx.c
 */

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include "portsettings.h"
#include "serial.h"

#define CMD_LEN 80

/**
 * length of array
 *
 * @param[in] a array
 * @return length
 */
#define LENGTH(a) sizeof(a)/sizeof(a[0])

#define UNUSED(x) (void)(x)

extern char **environ;

portsettings_t portsettings;

volatile sig_atomic_t killed = 0;

/**
 * represent a file, it's absolute path while preserving the indicated name
 */
typedef struct file_t {
    char* name; /**< either abs path or file with extension in eg ~/.trx/ */
    char* path; /**< absolute path or relative to ./ */
    FILE* stream; /**< FILE pointer */
} file_t;

/**
 * conatins all program settings set by arguments
 */
struct settings {
    file_t device; /**< device config file */
    file_t input; /**< commands to be transmitted */
    file_t output; /**< responses will be written to this file */
    int verbose; /**< increase verbosity */
    int quiet; /**< mute stdout */
} settings;

/**
 * arg options
 */
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


////////////////////////////////////////////////////////////////////////////////
//                            function prototypes                             //
////////////////////////////////////////////////////////////////////////////////

/**
 * print the help section to stdout
 */
static void print_help(void);

/**
 * print all settings
 */
static void print_settings(void);

/**
 * find file in various locations (~/.config/trx, ~/.trx, /etc/trx)
 *
 * @param[in,out] file name of file or absolute path
 * @param[in] ext extension
 * @return pointer to allocated string with absolute path or NULL if failed
 */
static char* find_file(const char* file, const char* ext);

/**
 * parse a device config file and set properties
 *
 * @param[out] portsettings writes settings in this struct
 * @param[in] file reads from this file
 * @return status 0 for succes, -1 for failure
 */
static int parse_config(file_t *file);

/**
 * controll transmit and receive
 *
 * @param[in] portsettings uses these settings for timeout and count
 * @param[in] cmd command message string
 * @return status 0 for succes, -1 for failure
 */
static int run(const char* cmd);

static void die(void);

////////////////////////////////////////////////////////////////////////////////
//                            function definition                             //
////////////////////////////////////////////////////////////////////////////////

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
    if (settings.input.name)
        printf("%-12s = %s\n", "input", settings.input.name);
    if (settings.output.name)
        printf("%-12s = %s\n", "output", settings.output.name);
    if (settings.device.name)
        printf("%-12s = %s\n", "device", settings.device.name);
    if (1) {
        printf("%-12s = %i\n", "verbose", settings.verbose);
        printf("%-12s = %i\n", "quiet", settings.quiet);
    }
}

char* find_file(const char* file, const char* ext)
{
    char* path = NULL;
    char buf[CMD_LEN+1];
    struct stat file_stat;

    if (*file != '~') {
        strcpy(buf, file);

    /* expand tilde */
    } else {
        strcpy(buf, getenv("HOME"));
        strcat(buf, "/");
        strcat(buf, file+1);
    }

    /* absolute path */
    if (access(buf, F_OK) == -1) {

        /* ~/.config/trx/{}.ext */
        strcpy(buf, getenv("XDG_CONFIG_HOME"));
        strcat(buf, "/trx/");
        strcat(buf, file);
        strcat(buf, ext);
        if (access(buf, F_OK) == -1) {

            /* ~/.trx/{}.ext */
            strcpy(buf, getenv("HOME"));
            strcat(buf, "/.trx/");
            strcat(buf, file);
            strcat(buf, ext);
            if (access(buf, F_OK) == -1) {

                /* /etc/trx/{}.ext */
                strcpy(buf, "/etc");
                strcat(buf, "/trx/");
                strcat(buf, file);
                strcat(buf, ext);

                if (access(buf, F_OK) == -1) {
                    return NULL;
                }
            }
        }
    }

    path = malloc(strlen(buf)+1);
    strcpy(path, buf);

    stat(path, &file_stat);

    /* check if file is regular file */
    if (!S_ISREG(file_stat.st_mode)) {
        fprintf(stderr, "\"%s\" is not a regular file\n", path);
        return NULL;

    /* check if file is empty */
    } else if (!file_stat.st_size) {
        fprintf(stderr, "file \"%s\" is empty\n", path);
        return NULL;

    } else {
        return path;
    }
}

int parse_config(file_t *file)
{
    char line[CMD_LEN+2];

    file->stream = fopen(file->path, "r");

    if (!file->stream) {
        fprintf(stderr, "%s \"%s\"\n", strerror(errno), settings.device.path);
        return -1;
    }

    while (fgets(line, CMD_LEN+1, file->stream)) {

        /* validate max line length */
        if (strlen(line) >= CMD_LEN) {
            fprintf(stderr,
                    "maximum line length exceeded: %i characters\n",
                    CMD_LEN);
            goto fail;
        }

        /* skip comments */
        if (*line == '#') continue;

        char *p;

        p = strtok(line, "= \r\n");

        if (!portsettings.port && (strcmp(p, "port") == 0)) {
            p = strtok(NULL, "= \r\n");
            if (portsettings_set_port(&portsettings, p) == -1) {
                fprintf(stderr, "invalid serial port: %s\n", p);
                goto fail;
            }

        } else if (!portsettings.baudrate && (strcmp(p, "baudrate") == 0)) {
            p = strtok(NULL, "= \r\n");
            if (portsettings_set_baudrate(&portsettings, p) == -1) {
                fprintf(stderr, "invalid baudrate: %s\n", p);
                goto fail;
            }

        } else if (!portsettings.timeout && (strcmp(p, "timeout") == 0)) {
            p = strtok(NULL, "= \r\n");
            if (portsettings_set_timeout(&portsettings, p) == -1) {
                fprintf(stderr, "invalid timeout: %s\n", p);
                goto fail;
            }

        } else if (portsettings.count == UINT_MAX && (strcmp(p, "count") == 0)) {
            p = strtok(NULL, "= \r\n");
            if (portsettings_set_count(&portsettings, p) == -1) {
                fprintf(stderr, "invalid count: %s\n", p);
                goto fail;
            }
        }
    }

    fclose(file->stream);
    return 0;
fail:
    fclose(file->stream);
    fprintf(stderr, "error parsing config file: %s\n", file->name);
    return -1;
}

int run(const char* cmd)
{
    serial_tx(&portsettings, cmd);

    char buf[81];
    unsigned int n = 0;

    while (portsettings.count == UINT_MAX || n++ < portsettings.count) {

        if (killed) die();

        if (serial_rx(&portsettings, buf, 80) != -1) {
            if (!settings.quiet) {
                if (settings.verbose) printf("%-12s = ", "response");
                if (*buf) {
                    printf("%s\n", buf);
                } else {
                    if (settings.verbose) printf("<timeout>\n");
                    break;
                }
            }
        }
        else break;
    }
    return 0;
}

void term(int signum)
{
    UNUSED(signum);
    killed = 1;
}


void die(void)
{
    serial_die();
    portsettings_die(&portsettings);
    if (settings.device.path) free(settings.device.path);
    if (settings.input.path) free(settings.input.path);
    if (settings.output.path) free(settings.output.path);
    if (settings.output.stream) fclose(settings.output.stream);
    /* if (output_file) fclose(output_file); */
    exit(EXIT_SUCCESS);
}


////////////////////////////////////////////////////////////////////////////////
//                                    main                                    //
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    struct sigaction action;


    portsettings = portsettings_default();

    /* parse options */
    int oc;
    int oi = 0;
    while ((oc = getopt_long(argc, argv, "d:i:o:b:p:t:n:vqh",
                    long_options, &oi)) != -1) {
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
                if (portsettings_set_count(&portsettings, optarg) != -1) {
                    break;
                } else {
                    fprintf(stderr, "invalid count: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }

            /* program settings */
            case 'd':
                settings.device.name = optarg;
                break;

            case 'i':
                settings.input.name = optarg;
                break;

            case 'o':
                fprintf(stderr, "--output option not yet implemented ...\n");
                exit(EXIT_FAILURE);
                /* settings.output.name = optarg; */
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

    /* validate device config file */
    if (settings.device.name) {
        settings.device.path = find_file(settings.device.name, ".conf");
        if (!settings.device.path) {
            fprintf(stderr, "%s \"%s\"\n", strerror(errno),
                    settings.device.name);
            exit(EXIT_FAILURE);
        }
    }

    /* validate input file */
    if (settings.input.name) {
        settings.input.path = find_file(settings.input.name, ".cmd");
        if (!settings.input.path) {
            fprintf(stderr, "%s \"%s\"\n", strerror(errno),
                    settings.input.name);
            exit(EXIT_FAILURE);
        }
    }

    /* read config file (device) */
    if (settings.device.path) {
        if (parse_config(&settings.device)!= -1);
        else exit(EXIT_FAILURE);
    }

    /* verbose print */
    if (settings.verbose) {
        print_settings();
        portsettings_print(&portsettings);
    }

    /* catch sig */
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction(SIGINT, &action, NULL);

    /* init serial port */
    if (serial_init(&portsettings) == -1) {
        exit(EXIT_FAILURE);
    }

    /* run arg commands */
    for (int i = optind; i < argc; i++) {
        if (killed) die();
        if (settings.verbose) printf("%-12s = %s\n", "command", argv[i]);
        run(argv[i]);
    }


    /* run input file */
    if (settings.input.name) {
        settings.input.stream = fopen(settings.input.path, "r");
        if (settings.input.stream) {

            char line[CMD_LEN+2];

            if (settings.verbose) printf("using input file \'%s\"\n",
                    settings.input.path);

            while (fgets(line, CMD_LEN+1, settings.input.stream)) {

                if (killed) die();

                /*filter comments and empty lines*/
                if (*line == '#' || *line == '\n') continue;

                /*validate max line length*/
                if (strlen(line) >= CMD_LEN) {
                    fprintf(stderr,
                            "maximum line length exceeded: %i characters\n",
                            CMD_LEN);
                    return -1;
                }

                /*trim trailing newlines*/
                if (line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';

                if (settings.verbose) printf("%-12s = %s\n", "command", line);
                run(line);
            }

            fclose(settings.input.stream);

        } else {
            fprintf(stderr, "%s \"%s\"\n",
                    strerror(errno), settings.input.name);
            exit(EXIT_FAILURE);
        }
    }
    die();
}

