# spinnaker-adapters
Adapters for communicating with SpiNNaker hardware

## Dependencies

This package has the following dependencies:

* The SpiNNaker external devices library

* MUSIC https://github.com/INCF/MUSIC

## Building
The build system used is GNU autotools (aclocal, autoconf, automake, libtool).

To build and install, do:

```bash
./autogen.sh # generate autotools files
./configure
make
make install
```

## Examples

For examples of communication between SpiNNaker hardware and a host,
see the subdirectory 'examples'.
