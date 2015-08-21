



ifeq ($(CONFIG_T2200_ASIC),y)
DDR3_INIT = ddr3_asic_init.o
else
DDR3_INIT = ddr3_init.o
endif

obj-y := board.o i2c.o $(DDR3_INIT) clk_init.o serdes.o pcie.o mdio_bitbang.o atherosdrv.o ar8328.o rtl8363sb.o clkrst.o dejitter.o spidrv.o
obj-y += lowlevel_init.o
obj-$(CONFIG_CMD_NORML) += norml.o


