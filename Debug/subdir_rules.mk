################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs901/ccs/tools/compiler/ti-cgt-msp430_18.12.2.LTS/bin/cl430" -vmsp --use_hw_mpy=none --include_path="C:/ti/ccs901/ccs/ccs_base/msp430/include" --include_path="C:/Users/Donnager/workspace/ED1_MiniProject" --include_path="C:/ti/ccs901/ccs/tools/compiler/ti-cgt-msp430_18.12.2.LTS/include" --advice:power=all --define=__MSP430G2553__ -g --printf_support=nofloat --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


