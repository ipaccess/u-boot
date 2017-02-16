ifeq ($(CONFIG_T2200_ASIC),y)
DDR3_INIT = ddr3_asic_init.o
else
DDR3_INIT = ddr3_init.o
endif
ifeq ($(CONFIG_GT_BOARD),y)
obj-y := board.o i2c.o $(DDR3_INIT) clk_init.o serdes.o pcie.o clkrst.o spidrv.o spieprom.o spi_flash.o
else
obj-y := board.o i2c.o $(DDR3_INIT) clk_init.o serdes.o pcie.o mdio_bitbang.o atherosdrv.o ar8328.o rtl8363sb.o clkrst.o dejitter.o spidrv.o
endif
obj-y += lowlevel_init.o
obj-$(CONFIG_CMD_NORML) += norml.o
obj-$(CONFIG_CMD_KEY)   += key.o
obj-y += led.o
obj-y        += secboot.o
obj-y        += ipat2k_fuse.o  efuse.o i2c_flash.o
obj-$(CONFIG_CHARACTERISATION_IPAT2K) += characterisation.o damm.o
obj-$(CONFIG_FIT_REVOCATION) += revocation.o
