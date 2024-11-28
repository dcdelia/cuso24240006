## CUSO seminar 24240006 - Dynamic Binary Instrumentation

This repository hosts the materials for a lecture on Dynamic Binary Instrumentation part of the [CUSO seminar 24240006](https://informatique.cuso.ch/?id=2283&L=0&tx_displaycontroller[showUid]=7625) *"Methodologies, Techniques, and Tools for Security: From Code Analysis to ML and Beyond"*. The seminar took place in Neuchâtel (CH) on November 28-29, 2024.

Please refer to the accompanying [presentation](slides.pdf) for using these materials.

The lecture has three hands-on exercises about writing Pin tools that:
- count executed instructions
- trace indirect call targets
- maintain a shadow stack and dump it upon exceptions

The examples come with a `Makefile` for Linux system. With enough patience, you can modify the default Pin tool for Windows to run the code also on Microsoft systems. The materials have been tested on Ubuntu 22.04 x64 using Pin 3.11 and gcc 11.4.0.

To download Intel Pin, go to [this page](https://www.intel.com/content/www/us/en/developer/articles/tool/pin-a-binary-instrumentation-tool-downloads.html) and download a recent version. Pin's distribution contains several exemplary tool to familiarize with increasingly complex APIs. From the same page, you can access also the instruction manual with the rich documentation for Pin APIs. If you end up facing challenges that you cannot figure out on your own, a good place where look is the [Pinheads](https://groups.io/g/pinheads) group. Or, if you attended this seminar, just drop me a line and I may be able to help you out ;-)

For the examples, we provide in the `assignments` folder an incomplete Pin tool, and the full code in the `solutions` folder. The folder `crash` contains instructions for reproducing a bug well-known among fuzzing practitioners under our third and last Pin tool.

Each `Makefile` we provide assumes that you created a symlink `$HOME/pin` to the folder containing Pin (e.g., `$HOME/pin-external-3.31-98869-gfa6f126a8-gcc-linux`). Please create one in your system or modify the `PIN_ROOT` variable in each `Makefile`.

Use `make profiler` to compile the DBI tools. Then, you can launch each tool with a command like this one:
```
$HOME/pin -t obj-intel/64 -- <program>
```

You are now all set, I guess. Enjoy!