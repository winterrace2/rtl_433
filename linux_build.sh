# This is an ugly 1st version of a librtl_433 build script for linux
# I successfully tested this on my Raspberry Pi 3b but it surely should be replaced by something more professional
# You have to compile librtl_433 beforehand...
gcc -L../librtl_433 -Iinclude -I../librtl_433/include -D RTLSDR src/compat_paths.c src/configure.c src/confparse.c src/getopt.c src/optparse.c src/rtl_433.c -lm -lrtlsdr -lrtl_433 -o rtl_433