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
RTOSCONFIGDIR := $(INCDIR)/rtos
RTOSHEAPCONFIG ?= 4
# Add additional files if necessary
RTOSSRCS := $(RTOSDIR)/tasks.c $(RTOSDIR)/list.c $(RTOSDIR)/queue.c $(RTOSDIR)/timers.c
RTOSSRCS += $(RTOSDEVDIR)/port.c
RTOSSRCS += $(RTOSDIR)/portable/MemMang/heap_$(RTOSHEAPCONFIG).c
RTOSOBJS := $(RTOSSRCS:%.c=$(OBJDIR)/%.o)


COMMON_CFLAGS = -Wall -Wextra -std=c11 -g3 -Os
CMSIS_CPPFLAGS := -DUSE_HAL_DRIVER -DUSE_NUCLEO_32 -DSTM32G431xx
CMSIS_CPPFLAGS += -I $(STMHALINC) -I $(STMCMSISINC) -I $(ARMCMSISINC) -I $(BSPDIR)
RTOSCPPFLAGS := -I $(RTOSINCDIR) -I $(RTOSINCDIR)/portable -I $(RTOSCONFIGDIR) -I $(RTOSDEVDIR)
ARMDSPCPPFLAGS := -DDISABLEFLOAT16 -I $(ARMDSPINC) -I $(ARMDSPINC)/dsp

CPUFLAGS = -mcpu=cortex-m4 -mthumb
FPUFLAGS = -mfloat-abi=hard -mfpu=fpv4-sp-d16

AFLAGS := -D --warn $(CPUFLAGS) -g
CPPFLAGS := -I $(INCDIR) -I $(INCDIR)/mcu -I $(INCDIR)/rtos
CPPFLAGS += $(CMSIS_CPPFLAGS) -I $(RTOSINCDIR) -I $(RTOSDEVDIR) $(ARMDSPCPPFLAGS)
CFLAGS := $(CPUFLAGS) $(FPUFLAGS) $(COMMON_CFLAGS)
LDSCRIPT := STM32G431KBTX_FLASH.ld
LDFLAGS := -T $(LDSCRIPT) -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group
LDFLAGS += -Wl,-Map=main.map,--cref,-gc-sections
LDLIBS :=
DEPFLAGS = -MT $@ -MMD -MP -MF $(@:$(OBJDIR)/%.o=$(DEPDIR)/%.d)

SRCS := $(wildcard $(SRCDIR)/*.c)
SRCS += $(wildcard $(SRCDIR)/mcu/*.c)
SRCS += $(wildcard $(SRCDIR)/rtos/*.c)
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
ARMDSPSRCS += $(ARMDSPDIR)/Source/BasicMathFunctions/arm_mult_f32.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/ComplexMathFunctions/arm_cmplx_mag_f32.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/StatisticsFunctions/arm_mean_f32.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/WindowFunctions/arm_blackman_harris_92db_f32.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/CommonTables/arm_const_structs.c
ARMDSPSRCS += $(ARMDSPDIR)/Source/CommonTables/arm_common_tables.c
ARMDSPOBJS := $(ARMDSPSRCS:%.c=$(OBJDIR)/%.o)

TARGET = spekky_matrix
DACTESTTARGET = dac_tests
LEDSTESTTARGET = leds_tests
SPITESTTARGET = spi_tests
MICTESTTARGET = mic_tests
FFTPROCESSINGTESTTARGET = fft_processing_tests

TESTCC := gcc
TESTSIZE := size
ASAN ?= 0


TESTDIR = tests
MOCKLIBDIR = lib/fff
TESTLIBDIR = lib/greatest
TESTOBJDIR := $(OBJDIR)/$(TESTDIR)
TESTCPPFLAGS := -I $(INCDIR) -I $(INCDIR)/mcu -I $(INCDIR)/rtos -I $(TESTLIBDIR) -I $(TESTDIR) -I $(MOCKLIBDIR)
TESTCFLAGS := $(COMMON_CFLAGS) $(CMSIS_CPPFLAGS)
TESTLDFLAGS :=

ifeq ($(ASAN), 1)
	TESTCFLAGS += -fsanitize=address,undefined
	TESTLDFLAGS += -fsanitize=address,undefined
endif

DAC_TESTSRCS := $(TESTDIR)/dac_suite.c $(TESTDIR)/dac_main.c
DAC_TESTSRCS += $(SRCDIR)/dac.c
DAC_TESTOBJS := $(DAC_TESTSRCS:%.c=$(TESTOBJDIR)/%.o)
LEDS_TESTSRCS := $(TESTDIR)/leds_suite.c $(TESTDIR)/leds_main.c
LEDS_TESTSRCS += $(SRCDIR)/led_matrix.c
LEDS_TESTOBJS := $(LEDS_TESTSRCS:%.c=$(TESTOBJDIR)/%.o)
SPI_TESTSRCS := $(TESTDIR)/spi_suite.c $(TESTDIR)/spi_main.c
SPI_TESTSRCS += $(SRCDIR)/spi.c
SPI_TESTOBJS := $(SPI_TESTSRCS:%.c=$(TESTOBJDIR)/%.o)
MIC_TESTSRCS := $(TESTDIR)/mic_data_suite.c $(TESTDIR)/mic_data_main.c
MIC_TESTSRCS += $(SRCDIR)/mic_data_processing.c
MIC_TESTOBJS := $(MIC_TESTSRCS:%.c=$(TESTOBJDIR)/%.o)
FFTPROCESSING_TESTSRCS := $(TESTDIR)/fft_processing_suite.c $(TESTDIR)/fft_processing_main.c
FFTPROCESSING_TESTSRCS += $(SRCDIR)/fft_processing.c
FFTPROCESSING_TESTOBJS := $(FFTPROCESSING_TESTSRCS:%.c=$(TESTOBJDIR)/%.o)

PRINT_PREFIX = "\#\#\#\#\#"


.PHONY: all clean tests srcdepdir freertos_update cmsis_update_all sigploti2s_update \
unit_test_update_all update_all flash-erase flash-write flash-backup
all: $(TARGET).elf $(TARGET).bin
tests: $(DACTESTTARGET).elf $(LEDSTESTTARGET).elf $(SPITESTTARGET).elf $(MICTESTTARGET).elf \
$(FFTPROCESSINGTESTTARGET).elf

flash-backup:
	$(FLASH) read BIN_BACKUP.bin 0x08000000 0x20000

flash-write: $(TARGET).bin
	$(FLASH) --flash=128k write $< 0x08000000

flash-erase:
	$(FLASH) erase

freertos_update:
	@echo "$(PRINT_PREFIX) Initializing/updating FreeRTOS submodule"
	git submodule update --init --remote $(LIBDIR)/FreeRTOS-Kernel

cmsis_update_all:
	@echo "$(PRINT_PREFIX) Initializing/updating cmsis submodules"
	git submodule update --init --remote $(CMSISMODULES)

sigploti2s_update:
	@echo "$(PRINT_PREFIX) Initializing/updating SigPlotI2S submodules"
	git submodule update --init --remote $(LIBDIR)/SigPlotI2S

unit_test_update_all:
	@echo "$(PRINT_PREFIX) Initializing/updating unit test submodules"
	git submodule update --init --remote $(LIBDIR)/greatest $(LIBDIR)/fff

update_all: freertos_update cmsis_update_all sigploti2s_update unit_test_update_all


$(OBJDIR)/$(RTOSDIR)/%.o: $(RTOSDIR)/%.c
	@echo "$(PRINT_PREFIX) Creating RTOS objects"
	@mkdir -p $(@D)
	$(CC) $(RTOSCPPFLAGS) $(CFLAGS) -c $< -o $@


$(SYSOBJ): $(SYSFILE)
	@echo "$(PRINT_PREFIX) Creating system object"
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(STARTUPOBJ): $(STARTUPFILE)
	@echo "$(PRINT_PREFIX) Creating startup object"
	@mkdir -p $(@D)
	$(AS) $(AFLAGS) $< -o $@

# Satisfy make, no rule needed for target, is only a prerequisite
$(STARTUPFILE):
$(SYSFILE):

$(OBJDIR)/$(ARMDSPDIR)/%.o: $(ARMDSPDIR)/%.c
	@echo "$(PRINT_PREFIX) Creating DSP objects"
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffunction-sections -fdata-sections -c $< -o $@

$(OBJDIR)/$(STMHALDIR)/%.o: $(STMHALDIR)/%.c
	@echo "$(PRINT_PREFIX) Creating HAL objects"
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


$(TARGET).bin: $(TARGET).elf
	@echo "$(PRINT_PREFIX) Creating binary image"
	$(OBJCOPY) -O binary $^ $@

$(TARGET).elf: $(SRCOBJS) $(STARTUPOBJ) $(SYSOBJ) $(STMHALOBJS) $(RTOSOBJS) $(ARMDSPOBJS)
	@echo "$(PRINT_PREFIX) Linking objects"
	$(CC) $(LDFLAGS) $(LDLIBS) $(CPUFLAGS) $(FPUFLAGS) $^ -o $@
	$(SIZE) $@

$(OBJDIR)/$(SRCDIR)/%.o: $(SRCDIR)/%.c | srcdepdir
	@echo "$(PRINT_PREFIX) Creating objects"
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

srcdepdir :
	@mkdir -p $(DEPDIR)/$(SRCDIR) $(DEPDIR)/$(SRCDIR)/mcu $(DEPDIR)/$(SRCDIR)/rtos

$(SRCDEPS):


# Unit test builds
$(DACTESTTARGET).elf: $(DAC_TESTOBJS)
	@echo "$(PRINT_PREFIX) Linking test objects"
	$(TESTCC) $(TESTLDFLAGS) $(TESTLDLIBS) $^ -o $@
	$(TESTSIZE) $@

$(LEDSTESTTARGET).elf: $(LEDS_TESTOBJS)
	@echo "$(PRINT_PREFIX) Linking test objects"
	$(TESTCC) $(TESTLDFLAGS) $(TESTLDLIBS) $^ -o $@
	$(TESTSIZE) $@

$(SPITESTTARGET).elf: $(SPI_TESTOBJS)
	@echo "$(PRINT_PREFIX) Linking test objects"
	$(TESTCC) $(TESTLDFLAGS) $(TESTLDLIBS) $^ -o $@
	$(TESTSIZE) $@

$(MICTESTTARGET).elf: $(MIC_TESTOBJS)
	@echo "$(PRINT_PREFIX) Linking test objects"
	$(TESTCC) $(TESTLDFLAGS) $(TESTLDLIBS) $^ -o $@
	$(TESTSIZE) $@

$(FFTPROCESSINGTESTTARGET).elf: TESTLDLIBS = -lm
$(FFTPROCESSINGTESTTARGET).elf: $(FFTPROCESSING_TESTOBJS)
	@echo "$(PRINT_PREFIX) Linking test objects"
	$(TESTCC) $(TESTLDFLAGS) $(TESTLDLIBS) $^ -o $@
	$(TESTSIZE) $@

$(TESTOBJDIR)/%.o: %.c
	@echo "$(PRINT_PREFIX) Creating test objects"
	@mkdir -p $(@D)
	$(TESTCC) $(TESTCPPFLAGS) $(TESTCFLAGS) -c $< -o $@


clean:
	@echo "$(PRINT_PREFIX) Cleaning build"
	-$(RM) $(TARGET).{elf,bin} $(DACTESTTARGET).elf $(LEDSTESTTARGET).elf $(SPITESTTARGET).elf
	-$(RM) -rf $(OBJDIR) $(DEPDIR)

-include $(wildcard $(SRCDEPS))
