# rpi\_infrared\_reader

## Overview

This is meant to run on a Raspberry Pi with GPIO. I use a 4B but it'll probably work
on any Pi.

This can record the 38 KHz IR timing of a standard remote control. After recording the
timing of a remote button, the button can then be simulated with an IR LED.

This can be used with my picow\_infrared\_relay and picow\_infrared\_helper to send
IR codes over wifi to a Pico W with an IR LED.

## Hardware required

I've only tested it with a Pi 4B but it should work with any Pi.

You'll need an IR Sensor. I use a TSOP38238, $2 at Adafruit (product 157). 
I wire Pin 1 on the sensor to GPIO17 (actual pin 11). You can set PIN\_IRSENSOR
in irreader.c to change that.

With a sensor attached correctly, you can press buttons on a remote control and
save the timings read from the button.

## Software required

This is written in C and uses the pigpio library.

For a Raspbian/Raspberry Pi OS system, you can install pigpio with:
```bash
apt-get install libpigpio-dev
```

After that's installed, you can do:
```bash
git clone https://github.com/sanjayrao77/rpi_infrared_reader
cd rpi_infrared_reader
make
```

## Usage

After it's compiled, you can run ./irreader.

The timing is reset every 2 seconds. After a reset, it prints "Ready for button.."
to stderr. At this point, you can press a button on a remote. It will then print
the timing to stdout and save it to /tmp/irreader.log.

## Reading timing

There's going to be stray signals before and after your remote codes. You can
spot these by looking for large delays (large numbers).

Here's an example of output:

```
230,
230,
255,361590,9045,4465,605,530,605,530,600,535,600,1640,600,1645,600,1640,600,1640,605,530,605,1640,596,1644,605,1640,605,530,605,530,605,530,605,525,660,1585,600,535,650,485,600,1640,605,1635,600,1645,605,530,605,530,605,1635,600,1645,600,1640,600,535,600,535,595,540,595,1645,605,1635,600,536,599,39760,9045,2220,655,
225,
```

The first 230,230,255,361590 is leading garbage. This
can be spotted by the 361590 which is far too long to be part of a code.

The last 39760,9045,2220,655,225 is trailing garbage. This can be spotted since the 39760
is also too long to be part of a code.

Thus, we can trim it to:

```
9045,4465,605,530,605,530,600,535,600,1640,600,1645,600,1640,600,1640,605,530,605,1640,596,1644,605,1640,605,530,605,530,605,530,605,525,660,1585,600,535,650,485,600,1640,605,1635,600,1645,605,530,605,530,605,1635,600,1645,600,1640,600,535,600,535,595,540,595,1645,605,1635,600,536,599,
```

You could save this to data.txt and then run ./parsedata to get some C code version of
the timing compatible with picow\_infrared\_helper.

## Averaging timing

Included is a parsedata program. It reads timing from data.txt and averages multiple lines
to smooth out measurement error. It checks for aberrant numbers and will error if one sample
is too different from others.

In practice, this isn't very useful. IR devices are very lenient for timing discrepencies
and there might not be any need to average multiple samples.
