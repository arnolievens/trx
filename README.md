# trx

Trx  is  a  commandline  serial port transceiver.\
Transmit a command to a serial port and wait for response

### usage

```sh
trx [options] [command] [command] [...]

  -b  --baudrate  serial port baudrate setting (must be valid)
                  eg 9600, 19200, ...

  -p  --port      serial port device file

  -t  --timeout   receiver will wait <timeout> msec for new input
                  unless count is fulfilled

  -n  --count     max number of lines to be read

  -d  --device    device config file
                  search in $XDG_CONFIG_HOME when no abs path given

  -i  --input     contents of this file will be transmitted per line
                  as if they are given as separate arguments

  -o  --output    response is written to file instead of stdout

  -v  --verbose   verbose output

  -q  --quiet     suppress writing response to stdout
                  does not mute stderr

  -h  --help      this menu
```

### install

```sh
make clean install
```
