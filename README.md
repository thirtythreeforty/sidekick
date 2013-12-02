TI Sidekick is an embedded device capable of receiving, storing, and transmitting data from TI calculators.

The project is based around a Microchip dsPIC33EPXXXGP502 microcontroller.  It uses one or more EEPROM memories to 
store variable data, and implements the [TI "DBUS" link protocol](http://merthsoft.com/linkguide/) at the physical,
packet, and application layers, all in software.  The project can optionally use a Newhaven I2C display to provide 
feedback.  All the hardware is configurable at compile time; see `common.h` to customize the build for a particular 
setup.

Communication with a TI-89 is fully implemented and working.  Communication with other models, including the TI-83 
Plus and TI-84 Plus, should work with minor or no adjustments to the software; this has not been tested, however.

The code will, by default, build for the "[Bully 
Bootloader](http://www.reesemicro.com/Home/pic24-software-library-collection/pic24-bully-bootloader)."  This can be 
changed (building for a stock, blank microcontroller) by simply removing the linker file from the project.
