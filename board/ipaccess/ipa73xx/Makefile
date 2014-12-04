#/*****************************************************************************
# * $picoChipHeaderSubst$
# *****************************************************************************/

#/*!
#* \file Makefile
#* \brief Used during the build process.
#*
#* Copyright (c) 2006-2011 Picochip Ltd
#*
#* This program is free software; you can redistribute it and/or modify
#* it under the terms of the GNU General Public License version 2 as
#* published by the Free Software Foundation.
#*
#* All enquiries to support@picochip.com
#*/

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

SOBJS-y := lowlevel_init.o

COBJS-y := ipa73xx.o
COBJS-y += nand.o
COBJS-y += ipa73xx_led.o
COBJS-y += secboot.o
COBJS-y += key.o
COBJS-y += fuse.o
COBJS-y += ipa73xx_bootcount.o
COBJS-y += ipa73xx_fuse.o
COBJS-y += ipa73xx_characterise_hw.o
COBJS-y += damm.o

SRCS    := $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS    := $(addprefix $(obj),$(COBJS-y))
SOBJS	:= $(addprefix $(obj),$(SOBJS-y))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak $(obj).depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
