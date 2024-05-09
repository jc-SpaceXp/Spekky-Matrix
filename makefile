TOOLSET = arm-none-eabi
CC := $(TOOLSET)-gcc
AS := $(TOOLSET)-as
GDB := $(TOOLSET)-gdb
SIZE := $(TOOLSET)-size
OBJCOPY := $(TOOLSET)-objcopy

FLASH := st-flash

SRCDIR = src
INCDIR = inc
LIBDIR = lib
OBJDIR = .obj
DEPDIR = .deps

BASECMSISDIR := $(LIBDIR)/cmsis
STMHALDIR := $(BASECMSISDIR)/stm32g4xx_hal_driver
STMHALINC := $(STMHALDIR)/Inc
STMCMSISDIR := $(BASECMSISDIR)/cmsis_device_g4
STMCMSISINC := $(STMCMSISDIR)/Include
ARMCMSISDIR := $(BASECMSISDIR)/CMSIS_6/CMSIS/Core
ARMCMSISINC := $(ARMCMSISDIR)/Include
BSPDIR := $(BASECMSISDIR)/stm32g4xx-nucleo-bsp
ARMDSPDIR := $(BASECMSISDIR)/CMSIS-DSP
ARMDSPINC := $(ARMDSPDIR)/Include
CMSISMODULES := $(STMHALDIR) $(STMCMSISDIR) $(BSPDIR) $(BASECMSISDIR)/CMSIS_6 $(ARMDSPDIR)

RTOSDIR := $(LIBDIR)/FreeRTOS-Kernel
RTOSINCDIR := $(RTOSDIR)/include
RTOSDEVDIR := $(RTOSDIR)/portable/GCC/ARM_CM4F
RTOSCONFIGDIR := $(INCDIR)
RTOSHEAPCONFIG ?= 4
# Add additional files if necessary
RTOSSRCS := $(RTOSDIR)/tasks.c $(RTOSDIR)/list.c $(RTOSDIR)/queue.c $(RTOSDIR)/timers.c
RTOSSRCS += $(RTOSDEVDIR)/port.c
RTOSSRCS += $(RTOSDIR)/portable/MemMang/heap_$(RTOSHEAPCONFIG).c
RTOSOBJS := $(RTOSSRCS:%.c=$(OBJDIR)/%.o)


COMMON_CFLAGS = -Wall -Wextra -std=c11 -g3 -Os
CMSIS_CPPFLAGS := -DUSE_HAL_DRIVER -DUSE_NUCLEO_32 -DSTM32G431xx
CMSIS_CPPFLAGS += -I $(STMHALINC) -I $(STMCMSISINC) -I $(ARMCMSISINC) -I $(BSPDIR)
RTOSCPPFLAGS := -I $(RTOSINCDIR) -I $(RTOSINCDIR)/portable -I $(INCDIR) -I $(RTOSDEVDIR)
ARMDSPCPPFLAGS := -DDISABLEFLOAT16 -I $(ARMDSPINC) -I $(ARMDSPINC)/dsp

CPUFLAGS = -mcpu=cortex-m4 -mthumb
FPUFLAGS = -mfloat-abi=hard -mfpu=fpv4-sp-d16

AFLAGS := -D --warn $(CPUFLAGS) -g
CPPFLAGS := -I $(INCDIR) -I $(INCDIR)/mcu $(CMSIS_CPPFLAGS) -I $(RTOSINCDIR) -I $(RTOSDEVDIR) $(ARMDSPCPPFLAGS)
CFLAGS := $(CPUFLAGS) $(FPUFLAGS) $(COMMON_CFLAGS)
LDSCRIPT := STM32G431KBTX_FLASH.ld
LDFLAGS := -T $(LDSCRIPT) -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group
LDFLAGS += -Wl,-Map=main.map,--cref,-gc-sections
LDLIBS :=
DEPFLAGS = -MT $@ -MMD -MP -MF $(@:$(OBJDIR)/%.o=$(DEPDIR)/%.d)

SRCS := $(wildcard $(SRCDIR)/*.c)
SRCS += $(wildcard $(SRCDIR)/mcu/*.c)
SRCOBJS := $(SRCS:%.c=$(OBJDIR)/%.o)
SRCDEPS := $(SRCS:%.c=$(DEPDIR)/%.d)
STARTUPFILE := $(STMCMSISDIR)/Source/Templates/gcc/startup_stm32g431xx.s
STARTUPOBJ := $(STARTUPFILE:%.s=$(OBJDIR)/%.o)
SYSFILE := $(STMCMSISDIR)/Source/Templates/system_stm32g4xx.c
SYSOBJ := $(SYSFILE:%.c=$(OBJDIR)/%.o)
STMHALSRCS := $(STMHALDIR)/Src/stm32g4xx_hal.c
STMHALSRCS += $(STMHALDIR)/Src/stm32g4xx_hal_cortex.c
STMHALSRCS += $(STMHALDIR)/Src/stm32g4xx_hal_gpio.c
STMHALOBJS := $(STMHALSRCS:%.c=$(OBJDIR)/%.o)
# Add additional files if necessary
ARMDSPSRCS := $(ARMDSPDIR)/Source/TransformFunctions/arm_rfft_fast_init_f32.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/TransformFunctions/arm_rfft_fast_f32.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/TransformFunctions/arm_cfft_init_f32.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/TransformFunctions/arm_cfft_f32.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/TransformFunctions/arm_cfft_radix8_f32.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/TransformFunctions/arm_bitreversal2.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/BasicMathFunctions/arm_abs_f32.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/BasicMathFunctions/arm_abs_q31.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/ComplexMathFunctions/arm_cmplx_mag_f32.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/StatisticsFunctions/arm_max_f32.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/CommonTables/arm_const_structs.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/CommonTables/arm_common_tables.c
ARMDSPOBJS := $(ARMDSPSRCS:%.c=$(OBJDIR)/%.o)

TARGET = spekky_matrix
DACTESTTARGET = dac_tests
LEDSTESTTARGET = leds_tests
SPITESTTARGET = spi_tests

TESTCC := gcc
TESTSIZE := size

TESTDIR = tests
MOCKLIBDIR = lib/fff
TESTLIBDIR = lib/greatest
TESTOBJDIR := $(OBJDIR)/$(TESTDIR)
TESTCPPFLAGS := -I $(INCDIR) -I $(TESTLIBDIR) -I $(TESTDIR) -I $(MOCKLIBDIR)
TESTCFLAGS := $(COMMON_CFLAGS) $(CMSIS_CPPFLAGS)

DAC_TESTSRCS := $(TESTDIR)/dac_suite.c $(TESTDIR)/dac_main.c
DAC_TESTSRCS += $(SRCDIR)/dac.c
DAC_TESTOBJS := $(DAC_TESTSRCS:%.c=$(TESTOBJDIR)/%.o)
LEDS_TESTSRCS := $(TESTDIR)/leds_suite.c $(TESTDIR)/leds_main.c
LEDS_TESTSRCS += $(SRCDIR)/led_matrix.c
LEDS_TESTOBJS := $(LEDS_TESTSRCS:%.c=$(TESTOBJDIR)/%.o)
SPI_TESTSRCS := $(TESTDIR)/spi_suite.c $(TESTDIR)/spi_main.c
SPI_TESTSRCS += $(SRCDIR)/spi.c
SPI_TESTOBJS := $(SPI_TESTSRCS:%.c=$(TESTOBJDIR)/%.o)


.PHONY: all clean tests srcdepdir cmsis_modules_git_update freertos_git_update \
test_modules_git_update flash-erase flash-write flash-backup
all: $(TARGET).elf $(TARGET).bin
tests: $(DACTESTTARGET).elf $(LEDSTESTTARGET).elf $(SPITESTTARGET).elf

flash-backup:
	$(FLASH) read BIN_BACKUP.bin 0x08000000 0x20000

flash-write: $(TARGET).bin
	$(FLASH) --flash=128k write $< 0x08000000

flash-erase:
	$(FLASH) erase


freertos_git_update:
	@echo "Initializing/updating FreeRTOS submodule"
	git submodule update --init --remote $(LIBDIR)/FreeRTOS-Kernel

$(OBJDIR)/$(RTOSDIR)/%.o: $(RTOSDIR)/%.c | freertos_git_update
	@echo "Creating RTOS objects"
	@mkdir -p $(@D)
	$(CC) $(RTOSCPPFLAGS) $(CFLAGS) -c $< -o $@


$(SYSOBJ): $(SYSFILE)
	@echo "Creating system object"
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(STARTUPOBJ): $(STARTUPFILE)
	@echo "Creating startup object"
	@mkdir -p $(@D)
	$(AS) $(AFLAGS) $< -o $@

# Satisfy make, no rule needed for target, is only a prerequisite
$(STARTUPFILE):
$(SYSFILE):

$(OBJDIR)/$(ARMDSPDIR)/%.o: $(ARMDSPDIR)/%.c
	@echo "Creating DSP objects"
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffunction-sections -fdata-sections -c $< -o $@

$(OBJDIR)/$(STMHALDIR)/%.o: $(STMHALDIR)/%.c
	@echo "Creating HAL objects"
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

cmsis_modules_git_update:
	@echo "Initializing/updating cmsis submodules"
	git submodule update --init --remote $(CMSISMODULES)


$(TARGET).bin: $(TARGET).elf
	@echo "Creating binary image"
	$(OBJCOPY) -O binary $^ $@

$(TARGET).elf: $(SRCOBJS) $(STARTUPOBJ) $(SYSOBJ) $(STMHALOBJS) $(RTOSOBJS) $(ARMDSPOBJS) \
| cmsis_modules_git_update
	@echo "Linking objects"
	$(CC) $(LDFLAGS) $(LDLIBS) $(CPUFLAGS) $(FPUFLAGS) $^ -o $@
	$(SIZE) $@

$(OBJDIR)/$(SRCDIR)/%.o: $(SRCDIR)/%.c | srcdepdir
	@echo "Creating objects"
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

srcdepdir :
	@mkdir -p $(DEPDIR)/$(SRCDIR) $(DEPDIR)/$(SRCDIR)/mcu

$(SRCDEPS):

test_modules_git_update:
	@echo "Initializing/updating greatest submodule"
	git submodule update --init --remote $(LIBDIR)/greatest

# Unit test builds
$(DACTESTTARGET).elf: $(DAC_TESTOBJS) | test_modules_git_update
	@echo "Linking test objects"
	$(TESTCC) $(TESTLDFLAGS) $(TESTLDLIBS) $^ -o $@
	$(TESTSIZE) $@

$(LEDSTESTTARGET).elf: $(LEDS_TESTOBJS) | test_modules_git_update
	@echo "Linking test objects"
	$(TESTCC) $(TESTLDFLAGS) $(TESTLDLIBS) $^ -o $@
	$(TESTSIZE) $@

$(SPITESTTARGET).elf: $(SPI_TESTOBJS) | test_modules_git_update
	@echo "Linking test objects"
	$(TESTCC) $(TESTLDFLAGS) $(TESTLDLIBS) $^ -o $@
	$(TESTSIZE) $@

$(TESTOBJDIR)/%.o: %.c
	@echo "Creating test objects"
	@mkdir -p $(@D)
	$(TESTCC) $(TESTCPPFLAGS) $(TESTCFLAGS) -c $< -o $@


clean:
	@echo "Cleaning build"
	-$(RM) $(TARGET).{elf,bin} $(DACTESTTARGET).elf $(LEDSTESTTARGET).elf $(SPITESTTARGET).elf
	-$(RM) -rf $(OBJDIR) $(DEPDIR)

-include $(wildcard $(SRCDEPS))
