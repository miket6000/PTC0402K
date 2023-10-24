# PTC0402K
A fully isolated (channel to channel and channel to input) four channel, two output programmable temperature controller and logger.

![3d_top_TemperatureLogger](https://github.com/miket6000/PTC0402K/assets/7330990/e91ebdf5-a59f-4036-9017-cadffed45999)

This project was built with a few target features in mind:
- Four input channels with at least 0.5C accuracy.
- SCPI interface to interface cleanly with the rest of my lab setup.
- Complete isolation of each channel (at least 230VAC) both between channels and between mains. This allows you to put a thermocouple directly on a live heatsink without risk of damaging the device or anything attached to the other thermocouples.
- A couple of relay outputs for potentially controlling heaters or coolers to allow temperature control.
- USB Type-C for both power and communications for simple setup.

A lot of inspiration came from the very nice project by Jana Marie at ![https://github.com/Jana-Marie/temperature-logger](https://github.com/Jana-Marie/temperature-logger) 

## Status
### Hardware
Rev 1 hardware has arrived. There is a fault in the current schematic where the digital isolators all drive the DO lines, even when their CS line is high. This kills the entire SPI bus. In my Rev 1 hardware I have corrected this by adding a small N-Channel MOSFET between the isolator's output enable pin and ground which is switched by the CS line. This means that normally the output is disabled on all isolators, but is enabled when the CS line is pulled low.

A bigger issue appears to be the temperature error in measureing the cold side junction. Because of the proximity of the MAX31855 to the isolated DCDC converter (which gets warm) and the distance to the thermocouple connector, there can be an error in the reported cold side junction temperature. As thermocouples are differnetial temperature devices, this directly adds to the temperature of the measurement. It looks like this is somewhere in the vicinty of 2-3 degrees C, however this depends on the ambient temperature and how long the entire device is powered which makes it difficult to calibrate out. 

Rev 2 will include the CS line fix and also add external flash to allow on board logging. I'm debating the merits of including a 32kHz crystal for the built in RTC (but then, do I also want a backup battery...) and might end up upgrading the STM32F072 to an STM32F078 for the additional ROM. For the cold junction offset issue I'm still debating options. I could simply move the MAX31855 closer to the junction and further from the DCDC converter, or I could add an additional temperature sensor to the junction itself and provide proper automatic calibration in SW. If I go that path however I end up with a highly integrated solution where I'm effectively not relying on any of the integration and I'd probably be better off with a 16-24bit DAC and discrete voltage reference and an analogue muxer, however this would take up more board space and likely end up costing more.

### Software
A basic test firmware has been written which reads the hot and cold junction temperatures, as well as the thermocouple status and allows them to be read via SCPI. It also allows control of the output relays though no temperature controller or alarms have been written.

The math for computing the corrected NIST polynomial based temperatures is all calculated using fixed point math as there is no FPU in the STM32F072. Rather than computing the polynomials directly on the uC this was done offline to create a lookup table. This is both faster than using floats and more accurate than using fixed point the whole way through as we can use true double precision during intermediate steps of the calculations and only drop to the 16.16 fixed precision format for the final lookup.
