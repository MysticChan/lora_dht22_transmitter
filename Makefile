PERIPHS_INC_DIR=./STM8S_StdPeriph_Lib/Libraries/STM8S_StdPeriph_Driver/inc
PERIPHS_SRC_DIR=./STM8S_StdPeriph_Lib/Libraries/STM8S_StdPeriph_Driver/src
APP_DIR=./app

CC=sdcc
ASM=sdasstm8
LINK=sdcc

# CPU part number
PART=STM8S105


# Directory for built objects
BUILD_DIR=build-sdcc
#dht22.rel led.rel 
# Port/application object files sx1278.rel
APP_OBJECTS = uart.rel led.rel timer.rel dht22.rel sx1278.rel main.rel

# STM8S Peripheral driver object files stm8s_tim4.rel  stm8s_spi.rel
PERIPH_OBJECTS = stm8s_gpio.rel stm8s_tim1.rel stm8s_clk.rel stm8s_uart2.rel stm8s_tim2.rel stm8s_tim4.rel stm8s_spi.rel

# Collection of built objects (excluding test applications)
ALL_OBJECTS = $(APP_OBJECTS) $(PERIPH_OBJECTS)
BUILT_OBJECTS = $(patsubst %,$(BUILD_DIR)/%,$(ALL_OBJECTS))

# Test object files (dealt with separately as only one per application build)
#TEST_OBJECTS = $(notdir $(patsubst %.c,%.rel,$(wildcard $(TESTS_DIR)/*.c)))

# Target application filenames (.elf) for each test object
#TEST_HEXS = $(patsubst %.rel,%.ihx,$(TEST_OBJECTS))
#TEST_ELFS = $(patsubst %.rel,%.elf,$(TEST_OBJECTS))
OUTPUT_OBJECTS=Lora_Transmitter.ihx
# stm8s105_tempelate.elf
# Search build/output directory for dependencies
vpath %.rel .\$(BUILD_DIR)
vpath %.elf .\$(BUILD_DIR)
vpath %.hex .\$(BUILD_DIR)

# Compiler/Assembler flags
CFLAGS= -mstm8 -c -D $(PART) -D __SDCC__ --opt-code-size
#DBG_CFLAGS= -mstm8 -c -D $(PART) -D __SDCC__ --opt-code-size
ASMFLAGS= -loff
#DBG_ASMFLAGS= -loff
LINKFLAGS= -mstm8
#DBG_LINKFLAGS= --debug -mstm8


#################
# Build targets #
#################

# All tests
all: $(BUILD_DIR) $(OUTPUT_OBJECTS) pack

# Make build/output directory
$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(OUTPUT_OBJECTS): $(APP_ASM_OBJECTS) $(KERNEL_OBJECTS) $(PERIPH_OBJECTS) $(APP_OBJECTS)
	$(LINK) $(BUILT_OBJECTS) $(LINKFLAGS) -o $(BUILD_DIR)/$@


# Peripheral objects builder
$(PERIPH_OBJECTS): %.rel: $(PERIPHS_SRC_DIR)/%.c
	$(CC) $< $(CFLAGS) -I $(PERIPHS_INC_DIR) -I $(APP_DIR) -o $(BUILD_DIR)/$*.rel

# Application C objects builder
$(APP_OBJECTS): %.rel: ./$(APP_DIR)/%.c
	$(CC) $< $(CFLAGS) -I $(APP_DIR) -I $(PERIPHS_INC_DIR) -o $(BUILD_DIR)/$*.rel

pack: ./$(BUILD_DIR)/$(OUTPUT_OBJECTS)
	packihx ./$(BUILD_DIR)/$(OUTPUT_OBJECTS) >./$(BUILD_DIR)/$(patsubst %.ihx,%.hex,$(OUTPUT_OBJECTS))
	

# Clean
clean:
	rm -f *.o *.elf *.map *.hex *.bin *.lst *.stm8 *.s19
#	rm -rf doxygen-kernel
#	rm -rf doxygen-stm8
	rm -rf build-sdcc

rebuild: clean all

#doxygen:
#	doxygen $(KERNEL_DIR)/Doxyfile
#	doxygen ./Doxyfile

