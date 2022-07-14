PORT 		=
UPLOAD_PROTOCOL =
BUILD_MCU 	=
BUILD_VARIANT 	=
UPLOAD_SPEED 	=
BUILD_F_CPU 	=

CXX 	= g++
AR 	= ar
SIZE 	= size
OBJCOPY = objcopy
AVRDUDE =

DUDEFLAGS 	=
FUSES		=
SIZEFLAGS 	=

PRJCXXFLAGS = -Os -g \
	-fmerge-all-constants \
	-fno-inline-small-functions -fshort-enums \
	-fno-exceptions -std=c++14 \
	-W -Wall -pedantic

LDFLAGS := $(LDFLAGS) -Wl,--gc-sections -Wl,--relax

#FORMAT = ihex binary
FORMAT = elf64-x86-64
