include Makefile.inc

LIBS = app console utils mcu avr

CXXFLAGS = $(PRJCXXFLAGS) -I.

SRCDIR = .
DEPDIR = .dep
OBJDIR = obj

SOURCE = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)%, $(OBJDIR)%, $(SOURCE:.cpp=.o))
DEPS = $(patsubst $(OBJDIR)%, $(DEPDIR)%, $(OBJS:.o=.d))
LIBA = $(patsubst %, $(OBJDIR)/lib%.a, $(LIBS))
LIBL = -L$(OBJDIR) $(patsubst %, -l%, $(LIBS))

TARGET 	= $(notdir $(CURDIR)).hex
ELF 	= $(notdir $(CURDIR)).elf

all: $(TARGET)

upload: $(TARGET)
	@echo [UPL] $<
	stty -F /dev/ttyUSB0 115200
	echo -e "reboot\\r\\n" > /dev/ttyUSB0
	sleep 0.8
	$(AVRDUDE) $(DUDEFLAGS) -U flash:w:$<
#	echo -e "reboot\\r\\n\\nreboot\\r\\n" > /dev/ttyUSB0

fuses:
	@echo [FUS] Settings fuses: $(FUSES)
	@$(AVRDUDE) $(DUDEFLAGS) $(FUSES)

clean:
	-@for lib in $(LIBS); do (cd $$lib; $(MAKE) clean); done
	-@rm -rf obj
	-@rm -rf .dep
	-@rm $(TARGET)

test: force_look
	@cd test ; $(MAKE) ; ./test

$(DEPDIR)/%.d: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -MM $< > $@
	@sed -i -e "s,$*.o:,$(OBJDIR)/$*.o $@: ,g" $@

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo [C++] $(notdir $<)
	@$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJDIR)/%.a: force_look
	@cd $(patsubst lib%, %, $(basename $(notdir $@)) ; $(MAKE) $(MFLAGS))

$(OBJDIR)/$(ELF): $(LIBA) $(OBJS)
	@echo [LNK] $(notdir $@)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LIBL) -o $@
	@echo
	@$(SIZE) $(SIZEFLAGS) $@

%.hex: $(OBJDIR)/%.elf
	@echo [HEX] $@
	@$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

force_look:
	@true

-include $(DEPS)
