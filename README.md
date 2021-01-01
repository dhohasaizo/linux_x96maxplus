HOW TO BUILD KERNEL X96MAX+:

this is kernel for official roms x96max+ android 9.0 (32bit rom)

Download toolchain from official website;
https://releases.linaro.org/components/toolchain/binaries/6.3-2017.02/arm-linux-gnueabihf/gcc-linaro-6.3.1-2017.02-x86_64_arm-linux-gnueabihf.tar.xz

configuration your toolchain patch..see "buildx96max" script

1.  cd {kernel source)
2.  ./buildx96max

note:
this is original kernel no include vfd driver. 
i use vfd driver from " https://github.com/anpaza/linux_vfd "
vfd driver run in daemon mode.
