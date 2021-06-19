# NAME
trx command-line serial transmit-receive utility

# SYNOPSIS
**trx** \[options\] \[file|baudrate|timeout|count\] \[cmd1\] ...

# DESCRIPTION
Trx is a command-line utiliy that allows to simultaneously transmit and receive data to a serial device<br>
You can specify the number of expected lines **(-n)** or use the timeout **(-t)** to detect the end of transmission.<br>
Device configuration is setup by means of options flags **(-pbnt)** or read from a device config file **(-d)**<br>
Multiple commands can be given in the command arguments or alternatively, a file can be specified (**-i**)<br>
When an input file is provided as wel as one or more argument commands, the latter will be sent prior to reading the input file<br>
The device config or command file may be either an absolue path or a file with respectively ".conf" and ".cmd" extensions in one of the following directories:<br>
"\~/.trx", "\~/.config/trx" or "/etc/trx/"\
Command-line options override settings in the config file.

# OPTIONS

## General

**-i**, **\--input** **\<filename\>**
: read commands from file, line-by-line

**-o**, **\--output** **\<filename\>**
: write response to file instead of stdout TODO: not yet implemented

**-v**, **\--verbose**
: verbose output, returns info about serial port and general config options

**-q**, **\--quiet**
: suppress stdout, does not mute stderr

**-h**, **\--help**
: print help menu

## Serial device

**-b**, **\--baudrate** **\<baudrate\>**
: port baudrate setting - must be a typical baudrate value - eg 9600, 19200, ...

**-p**, **\--port** **\<port-name\>**
: port device file name - eg /dev/ttyS0

**-t**, **\--timeout** **\<timeout\>**
: receiver will wait <--timeout> msec for new input unless <--count> has been reached

**-n**, **\--count** **\<count\>**
: max number of lines to be read per command

**-d**, **\--device** **\<filename\>**
: device config file, absolute path or the name of a unique file in $XDG\_CONFIG\_HOME $HOME/.trx or /etc/trx

# EXAMPLES
**trx -d someDevice --timeout 0.2 -n 1 \"some command"\"**
: use \"someDevice\" config file in eg /etc/trx, set timeout to 200ms, read one line after sending "some command" and do not wait any longer

**trx -p /dev/ttyS0 -i transmit.txt -q**
: use device ttyS0 and read commands from ./transmit.txt, don't print output

**trx -d ./dev.conf \"cmd1\" \"cmd2\"**
: use \"dev.conf\" device config file and send two commands

# EXIT VALUES
**0**
: Succes, data was successfully transmitted - even if receive timed-out

**1**
: Fail, invalid options, connection was not established or aborted

# BUGS
plenty

# COPYRIGHT
Copyright © 2021 Arno Lievens. License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.<br>
This is free software: you are free to change and redistribute it.  There  is  NO
WARRANTY, to the extent permitted by law.
