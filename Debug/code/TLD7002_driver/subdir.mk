################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../code/TLD7002_driver/TLD7002FuncLayer.c \
../code/TLD7002_driver/TLD7002_ControlLayer.c \
../code/TLD7002_driver/TLD7002_ServiceLayer.c 

OBJS += \
./code/TLD7002_driver/TLD7002FuncLayer.o \
./code/TLD7002_driver/TLD7002_ControlLayer.o \
./code/TLD7002_driver/TLD7002_ServiceLayer.o 

COMPILED_SRCS += \
./code/TLD7002_driver/TLD7002FuncLayer.src \
./code/TLD7002_driver/TLD7002_ControlLayer.src \
./code/TLD7002_driver/TLD7002_ServiceLayer.src 

C_DEPS += \
./code/TLD7002_driver/TLD7002FuncLayer.d \
./code/TLD7002_driver/TLD7002_ControlLayer.d \
./code/TLD7002_driver/TLD7002_ServiceLayer.d 


# Each subdirectory must supply rules for building sources it contributes
code/TLD7002_driver/%.src: ../code/TLD7002_driver/%.c code/TLD7002_driver/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fD:/smart_car/workspace/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo 'Finished building: $<'
	@echo ' '

code/TLD7002_driver/%.o: ./code/TLD7002_driver/%.src code/TLD7002_driver/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo 'Finished building: $<'
	@echo ' '


