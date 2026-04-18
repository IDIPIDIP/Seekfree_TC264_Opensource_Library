################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../code/GPS.c \
../code/encoder.c \
../code/fitting.c \
../code/flash.c \
../code/imu.c \
../code/init.c \
../code/integrate.c \
../code/meun.c \
../code/mode1.c \
../code/mode2.c \
../code/mode3.c \
../code/path_track.c \
../code/pid.c \
../code/point_collection.c \
../code/run.c \
../code/screen.c \
../code/speaker.c \
../code/zf_device_dot_matrix_screen.c \
../code/zf_device_tld7002.c 

OBJS += \
./code/GPS.o \
./code/encoder.o \
./code/fitting.o \
./code/flash.o \
./code/imu.o \
./code/init.o \
./code/integrate.o \
./code/meun.o \
./code/mode1.o \
./code/mode2.o \
./code/mode3.o \
./code/path_track.o \
./code/pid.o \
./code/point_collection.o \
./code/run.o \
./code/screen.o \
./code/speaker.o \
./code/zf_device_dot_matrix_screen.o \
./code/zf_device_tld7002.o 

COMPILED_SRCS += \
./code/GPS.src \
./code/encoder.src \
./code/fitting.src \
./code/flash.src \
./code/imu.src \
./code/init.src \
./code/integrate.src \
./code/meun.src \
./code/mode1.src \
./code/mode2.src \
./code/mode3.src \
./code/path_track.src \
./code/pid.src \
./code/point_collection.src \
./code/run.src \
./code/screen.src \
./code/speaker.src \
./code/zf_device_dot_matrix_screen.src \
./code/zf_device_tld7002.src 

C_DEPS += \
./code/GPS.d \
./code/encoder.d \
./code/fitting.d \
./code/flash.d \
./code/imu.d \
./code/init.d \
./code/integrate.d \
./code/meun.d \
./code/mode1.d \
./code/mode2.d \
./code/mode3.d \
./code/path_track.d \
./code/pid.d \
./code/point_collection.d \
./code/run.d \
./code/screen.d \
./code/speaker.d \
./code/zf_device_dot_matrix_screen.d \
./code/zf_device_tld7002.d 


# Each subdirectory must supply rules for building sources it contributes
code/%.src: ../code/%.c code/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fD:/smart_car/workspace/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo 'Finished building: $<'
	@echo ' '

code/%.o: ./code/%.src code/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo 'Finished building: $<'
	@echo ' '


