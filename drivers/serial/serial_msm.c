/*
 * Copyright (C) 2013 ip.access Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * As a special exception, if other files instantiate templates or use macros
 * or inline functions from this file, or you compile this file and link it
 * with other works to produce a work based on this file, this file does not
 * by itself cause the resulting work to be covered by the GNU General Public
 * License. However the source code for this file must still be made available
 * in accordance with section (3) of the GNU General Public License.

 * This exception does not invalidate any other reasons why a work based on
 * this file might be covered by the GNU General Public License.
 */

#include <common.h>
#include <serial.h>

#define MR1_OFFSET    (0x0000)
#define MR2_OFFSET    (0x0004)
#define SR_OFFSET     (0x0008)
#define CSR_OFFSET    (0x0008)
#define CR_OFFSET     (0x0010)
#define IMR_OFFSET    (0x0014)
#define IPR_OFFSET    (0x0018)
#define TFWR_OFFSET   (0x001C)
#define RFWR_OFFSET   (0x0020)
#define HCR_OFFSET    (0x0024)
#define MREG_OFFSET   (0x0028)
#define NREG_OFFSET   (0x002C)
#define DREG_OFFSET   (0x0030)
#define MNDREG_OFFSET (0x0034)
#define IRDA_OFFSET   (0x0038)

#define RF_OFFSET     (0x000C)
#define TF_OFFSET     (0x000C)

#define CR_COMMAND_NULL             (0x00000000)
#define CR_COMMAND_RESET_RX         (0x00000010)
#define CR_COMMAND_RESET_TX         (0x00000020)
#define CR_COMMAND_RESET_ERROR      (0x00000030)
#define CR_COMMAND_RESET_BREAK      (0x00000040)
#define CR_COMMAND_START_BREAK      (0x00000050)
#define CR_COMMAND_STOP_BREAK       (0x00000060)
#define CR_COMMAND_RESET_CTS        (0x00000070)
#define CR_COMMAND_RESET_TX_ERROR   (0x00000080)
#define CR_COMMAND_PACKET_MODE      (0x00000090)
#define CR_COMMAND_MODE_RESET       (0x000000c0)
#define CR_COMMAND_SET_RFR          (0x000000d0)
#define CR_COMMAND_RESET_RFR        (0x000000e0)
#define CR_COMMAND_CLEAR_TX_DONE    (0x00000100)

/* masks */
#define UART_CR__UART_RX_EN___M (0x00000001)
#define UART_CR__UART_TX_EN___M (0x00000004)
#define UART_IPR__RXSTALE_LAST___M (0x00000020)
#define UART_SR__RXRDY___M (0x00000001)
#define UART_SR__TXRDY___M (0x00000004)

/* shifts */
#define UART_IPR__STALE_TIMEOUT_LSB___S (0)
#define UART_IPR__STALE_TIMEOUT_MSB___S (7)
#define UART_CSR__UART_TX_CLK_SEL___S (0)
#define UART_CSR__UART_RX_CLK_SEL___S (4)
#define UART_SR__RXRDY___S (0)
#define UART_SR__TXRDY___S (2)

/* combined values */
#define UART_SR_RX_READY (UART_SR__RXRDY___M << UART_SR__RXRDY___S)

/* UART configuration */
#define MR2_5_BITS                  ((0x00000000)<<4)
#define MR2_6_BITS                  ((0x00000001)<<4)
#define MR2_7_BITS                  ((0x00000002)<<4)
#define MR2_8_BITS                  ((0x00000003)<<4)
#define MR2_HALF_STOP               ((0x00000000)<<2)
#define MR2_1_STOP                  ((0x00000001)<<2)
#define MR2_1_HALF_STOP             ((0x00000002)<<2)
#define MR2_2_STOP                  ((0x00000003)<<2)
#define MR2_NO_PARITY               (0x00000000)
#define MR2_ODD_PARITY              (0x00000001)
#define MR2_EVEN_PARITY             (0x00000002)
#define MR2_SPACE_PARITY            (0x00000003)

/* u-boot configuration */
#if !defined(CONFIG_SERIAL_MSM_UART_BASE)
#define CONFIG_SERIAL_MSM_UART_BASE 0x94000000
#endif
#define UART_BASE (CONFIG_SERIAL_MSM_UART_BASE)

#if !defined(CONFIG_SERIAL_MSM_UART_CONFIG)
#define CONFIG_SERIAL_MSM_UART_CONFIG (MR2_8_BITS|MR2_1_STOP|MR2_NO_PARITY)
#endif
#define UART_CONFIG CONFIG_SERIAL_MSM_UART_CONFIG

#if !defined(CONFIG_SERIAL_MSM_UART_BAUD)
#define CONFIG_SERIAL_MSM_UART_BAUD 115200
#endif
#define UART_BAUD (CONFIG_SERIAL_MSM_UART_BAUD)

#if !defined(CONFIG_SERIAL_MSM_TX_WATERMARK)
#define CONFIG_SERIAL_MSM_TX_WATERMARK 10
#endif
#define TX_WATERMARK (CONFIG_SERIAL_MSM_TX_WATERMARK)

#if !defined(CONFIG_SERIAL_MSM_RX_WATERMARK)
#define CONFIG_SERIAL_MSM_RX_WATERMARK 115
#endif
#define RX_WATERMARK (CONFIG_SERIAL_MSM_RX_WATERMARK)

#if !defined(CONFIG_SERIAL_MSM_TCXO_19200) && !defined(CONFIG_SERIAL_MSM_TCXO_4800)
#define CONFIG_SERIAL_MSM_TCXO_19200
#endif

#define uwr(v,a) writel(v, (UART_BASE) + (a))
#define urd(a) readl((UART_BASE) + (a))
#define reset_command(x) uwr(((x) | UART_CR__UART_TX_EN___M | UART_CR__UART_RX_EN___M), CR_OFFSET)





static int msm_serial_fini(void)
{
	reset_command(CR_COMMAND_RESET_RX);    /* reset rx */
	reset_command(CR_COMMAND_RESET_TX);    /* reset tx */
	reset_command(CR_COMMAND_RESET_ERROR); /* reset error status */
	reset_command(CR_COMMAND_RESET_BREAK); /* reset rx break interrupt */
	reset_command(CR_COMMAND_RESET_CTS);   /* reset cts */
	reset_command(CR_COMMAND_SET_RFR);     /* set rfr */
	uwr(CR_COMMAND_NULL, CR_OFFSET);       /* disable the uart */
	return 0;
}




static inline unsigned int _baud(unsigned int b)
{
	unsigned int ret;

	switch (b)
	{
		case 75:
		{
			ret = 0;
			break;
		}
		case 150:
		{
			ret = 1;
			break;
		}
		case 300:
		{
			ret = 2;
			break;
		}
		case 600:
		{
			ret = 3;
			break;
		}
		case 1200:
		{
			ret = 4;
			break;
		}
		case 2400:
		{
			ret = 5;
			break;
		}
		case 3600:
		{
			ret = 6;
			break;
		}
		case 4800:
		{
			ret = 7;
			break;
		}
		case 7200:
		{
			ret = 8;
			break;
		}
		case 9600:
		{
			ret = 9;
			break;
		}
		case 14400:
		{
			ret = 10;
			break;
		}
		case 19200:
		{
			ret = 11;
			break;
		}
		case 28800:
		{
			ret = 12;
			break;
		}
		case 38400:
		{
			ret = 13;
			break;
		}
		case 57600:
		{
			ret = 14;
			break;
		}
		case 115200:
		{
			ret = 15;
			break;
		}
		default: /* Default to 9600 */
		{
			ret = 9;
		}
	}

	return ret;
}




static int msm_serial_init(void)
{
	if (0 != msm_serial_fini()) {
		return -1;
	}

	/* UART clock setup */
#if defined(CONFIG_SERIAL_MSM_TCXO_19200)
	/* TCXO = 19.2MHz */
	uwr(0x0006, MREG_OFFSET);
	uwr(0x00F1, NREG_OFFSET);
	uwr(0x000F, DREG_OFFSET);
	uwr(0x001A, MNDREG_OFFSET);
#elif defined(CONFIG_SERIAL_MSM_TCXO_4800)
	/* TCXO/4 = 4.8MHz */
	uwr(0x00C0, MREG_OFFSET);
	uwr(0x00AF, NREG_OFFSET);
	uwr(0x0080, DREG_OFFSET);
	uwr(0x0019, MNDREG_OFFSET);
#else
# error One of CONFIG_SERIAL_MSM_TCXO_19200 or CONFIG_SERIAL_MSM_TCXO_4800 must be defined
#endif

	reset_command(CR_COMMAND_RESET_RX);    /* reset rx */
	reset_command(CR_COMMAND_RESET_TX);    /* reset tx */
	reset_command(CR_COMMAND_RESET_ERROR); /* reset error status */
	reset_command(CR_COMMAND_RESET_BREAK); /* reset rx break interrupt */
	reset_command(CR_COMMAND_RESET_CTS);   /* reset cts */
	reset_command(CR_COMMAND_SET_RFR);     /* set rfr */

	/* stale timeout = 630 * bitrate */
	uwr((UART_IPR__RXSTALE_LAST___M |
		(0x0000000f << UART_IPR__STALE_TIMEOUT_MSB___S) |
		(0x0000001f << UART_IPR__STALE_TIMEOUT_LSB___S)), IPR_OFFSET);

	uwr(0x00000000,   IMR_OFFSET);         /* disable all interrupts */
	uwr(RX_WATERMARK, RFWR_OFFSET);        /* RX watermark = 58 * 2 - 1 (in the ABL this is 0) */
	uwr(TX_WATERMARK, TFWR_OFFSET);        /* TX watermark (in the ABL this is 8) */
	uwr(0x00000000,   IRDA_OFFSET);        /* set RS-232 (disables IRDA) */
	uwr(0x0000001e,   HCR_OFFSET);         /* Set hunt character */
	uwr(0x00000001,   MR1_OFFSET);         /* Set RFR and CTS off and 1 character RFR - lk uses 0x10 (16) */
	uwr(UART_CONFIG,  MR2_OFFSET);         /* UART is configured as 8N1, but allow this to be overridden */

	/* configure the BAUD rate */
	uwr((_baud(UART_BAUD) << UART_CSR__UART_RX_CLK_SEL___S) |
		(_baud(UART_BAUD) << UART_CSR__UART_TX_CLK_SEL___S), CSR_OFFSET);

	/* enable the UART */
	uwr((UART_CR__UART_TX_EN___M | UART_CR__UART_RX_EN___M), CR_OFFSET);

	/*
         * We have seen a spurious character from the uart after being enabled when
	 * there is no debug header attached, this can cause our boot process
	 * problems. To work around this problem with the UART, wait for a bit then
	 * flush out any characters.
         */
	udelay(1000*100);
	while (urd(SR_OFFSET) & UART_SR_RX_READY)
		urd(RF_OFFSET);

	return 0;
}




static int msm_serial_getc(void)
{
	while (!(urd(SR_OFFSET) & UART_SR_RX_READY)) ;
	return urd(RF_OFFSET);
}




static void msm_serial_putc(char ch)
{
	if ('\n' == ch) {
		while (!(urd(SR_OFFSET) & UART_SR__TXRDY___M)) ;
		uwr('\r', TF_OFFSET);
	}

	while (!(urd(SR_OFFSET) & UART_SR__TXRDY___M)) ;
	uwr(ch, TF_OFFSET);
}




static int msm_serial_tstc(void)
{
	return urd(SR_OFFSET) & UART_SR_RX_READY;
}




static void msm_serial_setbrg(void)
{

}




static struct serial_device msm_serial_drv = {
	.name	= "arm_msm",
	.start	= msm_serial_init,
	.stop	= msm_serial_fini,
	.setbrg	= msm_serial_setbrg,
	.putc	= msm_serial_putc,
	.puts	= default_serial_puts,
	.getc	= msm_serial_getc,
	.tstc	= msm_serial_tstc,
};




void msm_serial_initialize(void)
{
	serial_register(&msm_serial_drv);
}




__weak struct serial_device *default_serial_console(void)
{
	return &msm_serial_drv;
}
