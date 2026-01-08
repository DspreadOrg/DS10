include $(RIOTMAKE)/arch/riscv.inc.mk

export CFLAGS_CPU  = -march=rv32imc_zicsr -mabi=ilp32

