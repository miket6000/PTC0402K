# PTC0402K
A fully isolated (channel to channel and channel to input) four channel, two output programmable temperature controller and logger.

![3d_top_TemperatureLogger](https://github.com/miket6000/PTC0402K/assets/7330990/e91ebdf5-a59f-4036-9017-cadffed45999)

This project was built with a few target features in mind:
- Four input channels with at least 0.5C accuracy.
- SCPI interface to interface cleanly with the rest of my lab setup.
- Complete isolation of each channel (at least 230VAC) both between channels and between mains. This allows you to put a thermocouple directly on a live heatsink without risk of damaging the device or anything attached to the other thermocouples.
- A couple of relay outputs for potentially controlling heaters or coolers to allow temperature control.
- USB Type-C for both power and communications for simple setup.

## Status
Rev 1 hardware has arrived. There is a fault in the current schematic where the digital isolators all drive the DO lines, even when their CS line is high. This kills the entire SPI bus. In my Rev 1 hardware I have corrected this by adding a small N-Channel MOSFET between the isolator's output enable pin and ground which is switched by the CS line. This means that normally the output is disabled on all isolators, but is enabled when the CS line is pulled low. Rev 2 will include this fix and also add external flash to allow on board logging. I'm debating the merits of including a 32kHz crystal for the built in RTC (but then, do I also want a backup battery...) and might end up upgrading the STM32F072 to an STM32F078 for the additional ROM.

Other than the previously mentinoed issue all the hardware appears to work well. I've not measured the isolated 3V3 noise levels yet, but it's not enough to show up on a very quick scope check, and certainly doesn't cause issues with the temperature measurements. 

A basic test formware has been written which reads the hot and cold junction temperatures, as well as the thermocouple status and allows them to be read via SCPI. It also allows control of the output relays though no temperature controller or alarms have been written.

There was an annoying issue where libscpi attempted to use snprintf for the float to string conversion but by default the printf libraries used by Cube-MX have floating point support disabled (this was a PITA to debug). Enabling floating point support in the printf libraries caused the code size to explode and the image no longer fit within the flash so I'm currently using a custom and simplified float to string function hastily stuffed into the scpi-def.c file. I would like to update the entire code base to fixed point math, but that's a project for another day.


