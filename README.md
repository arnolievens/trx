# trx

Trx is a command-line utility that allows to simultaneously transmit and receive data to a serial device
You can specify the number of expected lines (-n) or use the timeout (-t) to detect the end of transmission
Device configuration is setup by means of options flags (-pbnt) or read from a device config file (-i)

### usage

[documentation](https://github.com/arnolievens/trx/blob/main/trx.md)

### dependencies

-- pandoc: build man documentation

### install

```sh
make clean install
```
note: user should be in dailout group
