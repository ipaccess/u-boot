include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS	:= \
	ipafsm92xx.o \
	bootcount_ipafsm92xx.o \
	tlmm.o \
	timer.o \
	cpu.o \
	ssbi.o \
	pm8058.o \
	fuse.o \
	secboot.o \
	key.o \
	hwinfo.o \
	revocation.o \
	ipafsm92xx_nand.o \
	ipafsm92xx_led.o \
	k_msm_nand.o \
	k_dma.o \
	nand_partitions.o

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
