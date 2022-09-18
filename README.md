# energydisplay

![Finished display](https://i.imgur.com/8EXR7QX.png)

This is an energy status display that uses a 400x300 e-ink screen by Waveshare and a standard ESP32 with integrated WiFi.
The data is fetched from a Raspberry Pi or Beaglebone or any http web server for that matter. Getting the data into the relevant files is done elsewhere, this is just a display. There's one file for the core values, and then two text files for the histogram data for PV energy production and domestic power usage.

To extend the lifetime of the e-ink display, the image is refreshed only once every 5 minutes. Your SSID, wifi password and URL to fetch the files has to be filled into the relevant variables in the source.

The GxEPD library is needed to compile. It can be found at https://github.com/ZinggJM/GxEPD, make sure you use v2.x and not v3.x
The used pins are 4, 15, 16 and 17 as per the sourcecode, but this can be changed as needed. Wiring is up to personal preference. 

A color LCD version with animations for use with the M5Stack line of modules can be found on my github as well.

