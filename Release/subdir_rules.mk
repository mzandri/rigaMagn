################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Giroscopio.obj: ../Giroscopio.cpp $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti6/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --opt_for_speed=0 --include_path="C:/ti6/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="C:/ti6/TivaWare_C_Series-2.1.0.12573" --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RB1 --define=_DEBUG_ --define=PART_TM4C123GH6PM --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="Giroscopio.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

distMis.obj: ../distMis.cpp $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti6/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --opt_for_speed=0 --include_path="C:/ti6/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="C:/ti6/TivaWare_C_Series-2.1.0.12573" --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RB1 --define=_DEBUG_ --define=PART_TM4C123GH6PM --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="distMis.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

encQuad.obj: ../encQuad.cpp $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti6/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --opt_for_speed=0 --include_path="C:/ti6/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="C:/ti6/TivaWare_C_Series-2.1.0.12573" --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RB1 --define=_DEBUG_ --define=PART_TM4C123GH6PM --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="encQuad.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

gyro_f.obj: ../gyro_f.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti6/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --opt_for_speed=0 --include_path="C:/ti6/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="C:/ti6/TivaWare_C_Series-2.1.0.12573" --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RB1 --define=_DEBUG_ --define=PART_TM4C123GH6PM --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="gyro_f.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

gyro_init.obj: ../gyro_init.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti6/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --opt_for_speed=0 --include_path="C:/ti6/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="C:/ti6/TivaWare_C_Series-2.1.0.12573" --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RB1 --define=_DEBUG_ --define=PART_TM4C123GH6PM --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="gyro_init.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

init.obj: ../init.cpp $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti6/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --opt_for_speed=0 --include_path="C:/ti6/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="C:/ti6/TivaWare_C_Series-2.1.0.12573" --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RB1 --define=_DEBUG_ --define=PART_TM4C123GH6PM --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="init.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

interrupt.obj: ../interrupt.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti6/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --opt_for_speed=0 --include_path="C:/ti6/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="C:/ti6/TivaWare_C_Series-2.1.0.12573" --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RB1 --define=_DEBUG_ --define=PART_TM4C123GH6PM --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="interrupt.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.cpp $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti6/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --opt_for_speed=0 --include_path="C:/ti6/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="C:/ti6/TivaWare_C_Series-2.1.0.12573" --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RB1 --define=_DEBUG_ --define=PART_TM4C123GH6PM --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

parse.obj: ../parse.cpp $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti6/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --opt_for_speed=0 --include_path="C:/ti6/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="C:/ti6/TivaWare_C_Series-2.1.0.12573" --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RB1 --define=_DEBUG_ --define=PART_TM4C123GH6PM --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="parse.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

pid.obj: ../pid.cpp $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti6/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --opt_for_speed=0 --include_path="C:/ti6/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="C:/ti6/TivaWare_C_Series-2.1.0.12573" --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RB1 --define=_DEBUG_ --define=PART_TM4C123GH6PM --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="pid.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

qei.obj: ../qei.cpp $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti6/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --opt_for_speed=0 --include_path="C:/ti6/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="C:/ti6/TivaWare_C_Series-2.1.0.12573" --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RB1 --define=_DEBUG_ --define=PART_TM4C123GH6PM --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="qei.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

tiva_timer.obj: ../tiva_timer.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti6/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --opt_for_speed=0 --include_path="C:/ti6/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="C:/ti6/TivaWare_C_Series-2.1.0.12573" --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RB1 --define=_DEBUG_ --define=PART_TM4C123GH6PM --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="tiva_timer.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

tm4c123gh6pm_startup_ccs.obj: ../tm4c123gh6pm_startup_ccs.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti6/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --opt_for_speed=0 --include_path="C:/ti6/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="C:/ti6/TivaWare_C_Series-2.1.0.12573" --gcc --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RB1 --define=_DEBUG_ --define=PART_TM4C123GH6PM --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="tm4c123gh6pm_startup_ccs.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


