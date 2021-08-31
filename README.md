# trx

Trx is a command-line utility that allows to simultaneously transmit and receive data to a serial device
You can specify the number of expected lines (-n) or use the timeout (-t) to detect the end of transmission
Device configuration is setup by means of options flags (-pbnt) or read from a device config file (-d) in one of the following directories:
- $XDG_CONFIG_HOME/trx
- $HOME/.trx
- /ect/trx
- absolute path './...' or '/...'

Commands can be read from stdin (TODO) arguments or from file (-i) in the same locations

### usage

[documentation](https://github.com/arnolievens/trx/blob/main/doc/trx.md)

### dependencies

-- pandoc: build man documentation

### install

```sh
make clean install
```
note: user should be in dial-out group

### todo
- read cmd from stdin
- segfault on first run? - tested on -d matrix
