# Cam

Simple ESP32 based streaming webcam.

## Build

Make sure you have [Platformio](https://platformio.org/) installed.

Then copy `include/config-template.h` to `include/config.h` and edit
it to add your WiFi network settings.  **Do not check in
`include/config.h` or you will leak your password settings to the
interwebs**.

When you have done this you can build the project by issuing:

    pio run
	
## Uploading

When uploading the firmware to the device, make sure you have put the
device in programming mode.  This is accomplished by **shorting `IO0` to
ground during boot**.  The device will then enter programming mode.

You should also check `platoformio.ini` to ensure that the correct
serial port has been set up.

Then issue the command:

    pio run -v -t upload -t monitor
	
This will upload the firmware to the ESP32 and start a serial monitor
once uploading is done.  

Once the upload is complete you have to reboot the device.  (Ensure
that you do **not** have `IO0` shorted to ground as you do this or the
device will enter programming mode again).

The serial monitor can be stopped any time by pressing `Ctrl-C`.

