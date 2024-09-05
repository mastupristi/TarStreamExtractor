# TAR Stream Extractor

A few times in my work, I have needed a sw component that could extract an archive on the fly as it is received.<br>
It doesn't matter what kind of archive it is (but better if it's a standard format). <br>
I searched without finding anything useful though, so I wrote it by myself.<br>
I chose the TAR format. I tried to make the sw component as system-independent as possible (in fact in my work I use 32-bit microcontrollers, but for testing I use my 64-bit PC), and so no system calls, no dynamic memory allocation, no library dependencies, etc.

## Usage

Most tar processors found on the net *pull* the bytes from the tar file by themselves.
This is not good for our requirement to extract the contents of a tar transmitted in a stream, then incrementally.<br>
This is why in my implementation the data must be *pushed* into the extraction engine. There is no constraint on how many bytes at a time should be pushed.

The user will have to implement the call backs to be provided to the extraction engine through the init function. You can take a look at the examples

## Supported Features and Limitations

Although the *TAR Stream Extractor* core should support all types of tar, the example provided supports only tar containing files and not directories. In other words, the example requires tar not containing directory structures. The files that the tar contains must therefore be pathless.

## Further readings and related projects

Much of the specification of the tar format was found in the following places:

- [Tape Archive (tar) File Format Family](https://www.loc.gov/preservation/digital/formats/fdd/fdd000531.shtml)
- [Basic Tar Format](https://www.gnu.org/software/tar/manual/html_node/Standard.html)
- [Wikipedia](https://en.wikipedia.org/wiki/Tar_(computing)#File_format)

Another resource has been the [microtar](https://github.com/rxi/microtar) project

## License

This project is covered by the Apache 2.0 license
