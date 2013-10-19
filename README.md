U23_2013_examples
=================

The U23 is the educational youth project organized by the Chaos Computer Club Colonge e.V.

This years topic is based on ARM Cortex-M4 microcontrollers. As a development plaform the STM32F4-Discovery boards are used because they are cheap and offer a lot of cool features

This repository contains a make-based buildsystem and a few libraries for abstracting basic system startup and libc-specific stuff as well as a few examples on how to use different on- and off-chip peripherals.

Folder contents
---------------

The following markups are supported.  The dependencies listed are required if
you wish to run the library.

* bare_metal/ -- Bare metal examples without any libs except CMSIS and the STM Peripheral Lib, just here for the sake of completeness
* build/ -- Files for the buildsystem
* docs/ -- Documentation (Datasheets and Schematics)
* examples/ -- Examples which use the libs in libs/ for several peripherals
* libs/ -- Libraries which abstract the startup and libc-specifics a bit
* tools/ -- Extra tools like stlink-trace for talking to the trace macrocell

Dependencies
------------

* The ARM toolchain: [gcc-arm-embedded](https://launchpad.net/gcc-arm-embedded)
* For flashing and debugging: [OpenOCD](http://openocd.sourceforge.net/)
* For flashing and debugging: [stlink](https://github.com/texane/stlink)


Usage
-----

If you want to build everything, just use

    make

To flash a built firmware using OpenOCD, you can run
 
    make upload FIRMWARE=06_servo

There is another make target called `upload-fast` which uses stlink for flashing. It is called fast because it only erases and writes the flash pages which are actually used by the new firmware.

    make upload-fast FIRMWARE=06_servo

For getting debug output via the trace macrocell, you can use the `trace` makefile target like this

    make trace
Make sure you enabled the debug output!

If you want to change the default FIRMWARE to be uploaded, you can do so in the `config.mak` file in the root folder.

udev-rules
----------

To be able to talk to the JTAG debugger, you need to install a udev-rule to your system. To do this, create file ` /etc/udev/rules.d/50-stm32f4.rules` like this

    gksudo gedit /etc/udev/rules.d/50-stm32f4.rules

Paste the following rule into the file and save and close it:

    ATTR{idVendor}=="0483", ATTR{idProduct}=="3748", SYMLINK+="stm32-%k", MODE="666", GROUP="plugdev"

After that you can restart your computer or execute

    sudo udevadm control --reload-rules
to reload the udev rules. After replugging the board, you should be able to access it as a normal user.
