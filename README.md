# TAR Stream Extractor

A few times in my work, I have needed a sw component that could extract an archive on the fly as it is received.<br>
It doesn't matter what kind of archive it is (but better if it's a standard format). <br>
I searched without finding anything useful though, so I wrote it by myself.<br>
I chose the TAR format. I tried to make the sw component as system-independent as possible (in fact in my work I use 32-bit microcontrollers, but for testing I use my 64-bit PC), and so no system calls, no dynamic memory allocation, no library dependencies, etc.

## Usage

The user will have to implement the call backs to be provided to the extraction engine. You can take a look at the examples

## License

This project is covered by the Apache 2.0 license
