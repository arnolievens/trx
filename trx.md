% TRX(1) trx 1.0
% Arno Lievens
% May 2021

# NAME
trx command-line serial transmit-receive utility

# SYNOPSIS
**trx** \[options\] \[file|baudrate|timeout|count\] \[cmd1\] ...

# DESCRIPTION
Trx is a command-line utiliy that allows to simultaneously transmit and receive data to a serial device\
You can specify the number of expected lines **(-n)** or use the timeout **(-t)** to detect the end of transmission\
Device configuration is setup by means of options flags **(-pbnt)** or read from a device config file **(-i)**

# OPTIONS

## General

**-i**, **\--input** **\<filename\>**
: read commands from file, line-by-line

**-o**, **\--output** **\<filename\>**
: write response to file instead of stdout

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
: max number of lines to be read

**-d**, **\--device** **\<filename\>**
: device config file, absolute path or the name of a unique file in $XDG\_CONFIG\_HONE $HOME/.trx or /etc/trx

# EXAMPLES
**trx -d someDevice --timeout 0.2 -n 1 \"some command"\"**
: use \"someDevice\" config file in eg /etc/trx, set timeout to 200ms, read one line after sending "some command" and do not wait any longer

**trx -p /dev/ttyS0 -i transmit.txt -q**
: use device ttyS0 and read commands from ./transmit.txt, don't print output

**trx -d ./dev.conf \"cmd1\" \"cmd2\"**
: use \"dev.conf\" device config file and send two commands

# EXIT VALUES
**0**
: Succes, data was succesfully transmitted - even if receive timed-out

**1**
: Fail, invalid options, connection was not established or aborted

# BUGS
plenty

# COPYRIGHT
Copyright  © 2021 Arno Lievens. License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\
This is free software: you are free to change and redistribute it.  There  is  NO
WARRANTY, to the extent permitted by law.
