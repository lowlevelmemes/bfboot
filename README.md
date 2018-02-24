# bfboot
### A full brainfuck to bootable OS image compiler.

How to build
============

As of now, you'll only be able to build and run bfboot from
within the build directory.
To build it, simply run 'make' in the root of the source tree.
bfboot depends on nasm at runtime, so make sure to have that
installed and in your $PATH.

Compiling brainfuck
===================

Simply run `./bfboot` **within the bfboot source tree**.
bfboot takes 2 arguments, namely the brainfuck source file and
the output name of the created bootable image.

Example: `./bfboot 99bottles.bf 99bottles.img`

Running the image
=================

The generated image is a flat disk image. It can be written to
a USB stick using utilities such as cat or dd, or it can be
run in an emulator such as QEMU.

To run in QEMU: `qemu-system-i386 -hda 99bottles.img`
