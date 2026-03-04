################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../code/asr/asr_init.c \
../code/asr/base64.c \
../code/asr/hmac_sha256.c \
../code/asr/mic.c \
../code/asr/sha1.c \
../code/asr/url_code.c \
../code/asr/websocket_client.c 

OBJS += \
./code/asr/asr_init.o \
./code/asr/base64.o \
./code/asr/hmac_sha256.o \
./code/asr/mic.o \
./code/asr/sha1.o \
./code/asr/url_code.o \
./code/asr/websocket_client.o 

COMPILED_SRCS += \
./code/asr/asr_init.src \
./code/asr/base64.src \
./code/asr/hmac_sha256.src \
./code/asr/mic.src \
./code/asr/sha1.src \
./code/asr/url_code.src \
./code/asr/websocket_client.src 

C_DEPS += \
./code/asr/asr_init.d \
./code/asr/base64.d \
./code/asr/hmac_sha256.d \
./code/asr/mic.d \
./code/asr/sha1.d \
./code/asr/url_code.d \
./code/asr/websocket_client.d 


# Each subdirectory must supply rules for building sources it contributes
code/asr/%.src: ../code/asr/%.c code/asr/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fD:/smart_car/workspace/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo 'Finished building: $<'
	@echo ' '

code/asr/%.o: ./code/asr/%.src code/asr/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo 'Finished building: $<'
	@echo ' '


