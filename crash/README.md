### Crash analysis

We will use the `callstack` Pin tool to inspect the stack trace at a crash site. As working example, we will use the `guetzli` program from the Google Fuzzer Test suite, which contains a known bug causing an assert to fail.

To compile `guetzli`, make sure to have `git pkg-config libpng-dev` installed in your system.

Fetch its source and the specific commit in use to FTS, then compile it as a static library with debug symbol generation enabled:

```
git clone https://github.com/google/guetzli.git
cd guetzli/
git checkout 9afd0bbb7db0bd3a50226845f0f6c36f14933b6b
CFLAGS="-g" CXXFLAGS="-g" make guetzli_static
```

Copy the `driver.c` file in this folder into the source tree of `guetzli`. This file is a stripped-down version of the AFL++'s driver for running test cases in a fuzzer. We will use the fuzzing harness `LLVMFuzzerTestOneInput()` present in the program and invoke it on an input read from disk. The driver will materialize the file into a buffer and pass it to the harness.

```
g++ -g -std=c++11 fuzz_target.cc -I . bin/Release/libguetzli_static.a driver.c -o testprogram
```

To test the correct execution of the program, we can use two seeds from FTS: an image and a partially ill-formed input. Download both from the [seeds folder](https://github.com/google/fuzzer-test-suite/tree/master/guetzli-2017-3-30/seeds) of the `guetzli` project and save them into a folder called `inputs`. Download there also the [crashing test case](https://github.com/google/fuzzer-test-suite/raw/refs/heads/master/guetzli-2017-3-30/crash-5737651426557952) from FTS that we will use with our Pin tool.

The expected output with the three files is:

```
$ ./testprogram inputs/not_kitty.jpg 
Reading 413 bytes from inputs/not_kitty.jpg
Execution successful.

$ ./testprogram inputs/semiseed
Reading 131 bytes from inputs/semiseed
Unexpected end of scan.
Can't read jpg data from input file
Execution successful.

$ ./testprogram inputs/crash-5737651426557952 
Reading 135 bytes from inputs/crash-5737651426557952
testprogram: guetzli/output_image.cc:398: void guetzli::OutputImage::SaveToJpegData(guetzli::JPEGData*) const: Assertion `coeff % quant == 0' failed.
Aborted (core dumped)
```

And then we run the program with the crashing test case under our Pin tool:

```
$ $HOME/pin/pin -t ../obj-intel64/callstack.so -- ./testprogram inputs/crash-5737651426557952 
Reading 135 bytes from inputs/crash-5737651426557952
testprogram: guetzli/output_image.cc:398: void guetzli::OutputImage::SaveToJpegData(guetzli::JPEGData*) const: Assertion `coeff % quant == 0' failed.
Unrecoverable error at instruction pointer: 0x7f54e25989fc
Shadow stack:
[0] 7f5af31269fc pthread_kill (:0) 
[1] 7f5af30b87ee abort (:0) 
[2] 7f5af30b8716 __assert_fail_base.cold (:0) 
[3] 7f5af30c9e91 __assert_fail (:0) 
[4] 55ee72ffe2ba guetzli::OutputImage::SaveToJpegData (/home/sc/shared/cuso/guetzli/guetzli/output_image.cc:398) 
[5] 55ee72fe95e2 guetzli::(anonymous namespace)::Processor::TryQuantMatrix(guetzli::JPEGData const&, float, int (*) [64], guetzli::OutputImage*)  (/home/sc/shared/cuso/guetzli/guetzli/processor.cc:296) 
[6] 55ee72feb48e guetzli::(anonymous namespace)::Processor::ProcessJpegData(guetzli::Params const&, guetzli::JPEGData const&, guetzli::Comparator*, guetzli::GuetzliOutput*, guetzli::ProcessStats*)  (/home/sc/shared/cuso/guetzli/guetzli/processor.cc:332) 
[7] 55ee72fed389 guetzli::Process (/home/sc/shared/cuso/guetzli/guetzli/processor.cc:866) 
[8] 55ee72fdbb2d LLVMFuzzerTestOneInput (/home/sc/shared/cuso/guetzli/fuzz_target.cc:22) 
[9] 55ee73012abc ExecuteFilesOnyByOne (/home/sc/shared/cuso/guetzli/driver.c:50) 
[10] 55ee73012b57 LLVMFuzzerRunDriver (/home/sc/shared/cuso/guetzli/driver.c:71) 
[11] 55ee73012c53 main (/home/sc/shared/cuso/guetzli/driver.c:97) 
[12] 7f5af30b9d8e __libc_start_call_main (:0) 
[13] 7f5af30b9e3b __libc_start_main (:0) 
[14] 55ee72fdb91f _start (:0) 
```

The output will be very similar to what you can obtain with a backtrace in GDB.