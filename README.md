## CUSO seminar 24240006 - Dynamic Binary Instrumentation

This repository hosts the materials for a lecture on Dynamic Binary Instrumentation part of the [CUSO seminar 24240006](https://informatique.cuso.ch/?id=2283&L=0&tx_displaycontroller[showUid]=7625) *"Methodologies, Techniques, and Tools for Security: From Code Analysis to ML and Beyond"*. The seminar took place in Neuchâtel (CH) on November 28-29, 2024.

Please refer to the accompanying [presentation](slides.pdf) for these materials.

The lecture has three hands-on exercises about writing Pin tools that:
1. count executed instructions
1. trace indirect call targets
1. maintain a shadow stack and dump it upon exceptions

The examples come with a `Makefile` for Linux. With enough patience, you can modify the default Pin tool for Windows to run the code also on such systems. The materials have been tested on Ubuntu 22.04 x64 using Pin 3.11 and gcc 11.4.0.

To download Intel Pin, go to [this page](https://www.intel.com/content/www/us/en/developer/articles/tool/pin-a-binary-instrumentation-tool-downloads.html) and download a recent version. From there, you can access also the rich instruction manual for Pin APIs. Pin's distribution contains several exemplary tool to familiarize with the APIs. Another good place where to look for answers is the [Pinheads](https://groups.io/g/pinheads) group. Or, if you attended this seminar, just drop me a line and I may be able to help you out ;-)

For the hands-on, we provide in `assignments/` an incomplete sketch of each tool and a full implementation in `solutions/`. The folder `crash` contains instructions for reproducing a bug well-known among fuzzing researchers using our third Pin tool.

The `Makefile` we provide assumes that you created a symlink `$HOME/pin` to the install folder where you decompress Pin's archive (e.g., `$HOME/pin-external-3.31-98869-gfa6f126a8-gcc-linux`). Please create one in your system or modify the `PIN_ROOT` variable in the `Makefile`.

Just use `make profiler` to compile the DBI tools. Then, you can launch a tool as follows:
```
$HOME/pin -t obj-intel64/inscounter.so -- <program>
```

You are now all set. Enjoy!