/*
 * Copyright (C) 2014 ip.access Ltd.
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

//NOTE
// - PERIPH_BLK_BLSP is set by make files for other platforms, but not for FSM90xx.

//U-boot is single threaded and doesn't support interrupts, so this isn't required.
#define enter_critical_section() {}
#define exit_critical_section() {}

// ************* from lk uart_dm.h START **************************************
#define MSM_BOOT_UART_DM_EXTR_BITS(value, start_pos, end_pos) \
                                             ((value << (32 - end_pos))\
                                              >> (32 - (end_pos - start_pos)))

/* UART Parity Mode */
enum MSM_BOOT_UART_DM_PARITY_MODE {
    MSM_BOOT_UART_DM_NO_PARITY,
    MSM_BOOT_UART_DM_ODD_PARITY,
    MSM_BOOT_UART_DM_EVEN_PARITY,
    MSM_BOOT_UART_DM_SPACE_PARITY
};

/* UART Stop Bit Length */
enum MSM_BOOT_UART_DM_STOP_BIT_LEN {
    MSM_BOOT_UART_DM_SBL_9_16,
    MSM_BOOT_UART_DM_SBL_1,
    MSM_BOOT_UART_DM_SBL_1_9_16,
    MSM_BOOT_UART_DM_SBL_2
};

/* UART Bits per Char */
enum MSM_BOOT_UART_DM_BITS_PER_CHAR {
    MSM_BOOT_UART_DM_5_BPS,
    MSM_BOOT_UART_DM_6_BPS,
    MSM_BOOT_UART_DM_7_BPS,
    MSM_BOOT_UART_DM_8_BPS
};

/* 8-N-1 Configuration */
#define MSM_BOOT_UART_DM_8_N_1_MODE          (MSM_BOOT_UART_DM_NO_PARITY | \
                                             (MSM_BOOT_UART_DM_SBL_1 << 2) | \
                                             (MSM_BOOT_UART_DM_8_BPS << 4))

/* UART_DM Registers */

/* UART Operational Mode Register */
#define MSM_BOOT_UART_DM_MR1(base)             ((base) + 0x00)
#define MSM_BOOT_UART_DM_MR2(base)             ((base) + 0x04)
#define MSM_BOOT_UART_DM_RXBRK_ZERO_CHAR_OFF (1 << 8)
#define MSM_BOOT_UART_DM_LOOPBACK            (1 << 7)

/* UART Clock Selection Register */
#if PERIPH_BLK_BLSP
#define MSM_BOOT_UART_DM_CSR(base)             ((base) + 0xA0)
#else
#define MSM_BOOT_UART_DM_CSR(base)             ((base) + 0x08)
#endif

/* UART DM TX FIFO Registers - 4 */
#if PERIPH_BLK_BLSP
#define MSM_BOOT_UART_DM_TF(base, x)         ((base) + 0x100+(4*(x)))
#else
#define MSM_BOOT_UART_DM_TF(base, x)         ((base) + 0x70+(4*(x)))
#endif

/* UART Command Register */
#if PERIPH_BLK_BLSP
#define MSM_BOOT_UART_DM_CR(base)              ((base) + 0xA8)
#else
#define MSM_BOOT_UART_DM_CR(base)              ((base) + 0x10)
#endif
#define MSM_BOOT_UART_DM_CR_RX_ENABLE        (1 << 0)
#define MSM_BOOT_UART_DM_CR_RX_DISABLE       (1 << 1)
#define MSM_BOOT_UART_DM_CR_TX_ENABLE        (1 << 2)
#define MSM_BOOT_UART_DM_CR_TX_DISABLE       (1 << 3)

/* UART Channel Command */
#define MSM_BOOT_UART_DM_CR_CH_CMD_LSB(x)    ((x & 0x0f) << 4)
#define MSM_BOOT_UART_DM_CR_CH_CMD_MSB(x)    ((x >> 4 ) << 11 )
#define MSM_BOOT_UART_DM_CR_CH_CMD(x)        (MSM_BOOT_UART_DM_CR_CH_CMD_LSB(x)\
                                             | MSM_BOOT_UART_DM_CR_CH_CMD_MSB(x))
#define MSM_BOOT_UART_DM_CMD_NULL            MSM_BOOT_UART_DM_CR_CH_CMD(0)
#define MSM_BOOT_UART_DM_CMD_RESET_RX        MSM_BOOT_UART_DM_CR_CH_CMD(1)
#define MSM_BOOT_UART_DM_CMD_RESET_TX        MSM_BOOT_UART_DM_CR_CH_CMD(2)
#define MSM_BOOT_UART_DM_CMD_RESET_ERR_STAT  MSM_BOOT_UART_DM_CR_CH_CMD(3)
#define MSM_BOOT_UART_DM_CMD_RES_BRK_CHG_INT MSM_BOOT_UART_DM_CR_CH_CMD(4)
#define MSM_BOOT_UART_DM_CMD_START_BRK       MSM_BOOT_UART_DM_CR_CH_CMD(5)
#define MSM_BOOT_UART_DM_CMD_STOP_BRK        MSM_BOOT_UART_DM_CR_CH_CMD(6)
#define MSM_BOOT_UART_DM_CMD_RES_CTS_N       MSM_BOOT_UART_DM_CR_CH_CMD(7)
#define MSM_BOOT_UART_DM_CMD_RES_STALE_INT   MSM_BOOT_UART_DM_CR_CH_CMD(8)
#define MSM_BOOT_UART_DM_CMD_PACKET_MODE     MSM_BOOT_UART_DM_CR_CH_CMD(9)
#define MSM_BOOT_UART_DM_CMD_MODE_RESET      MSM_BOOT_UART_DM_CR_CH_CMD(C)
#define MSM_BOOT_UART_DM_CMD_SET_RFR_N       MSM_BOOT_UART_DM_CR_CH_CMD(D)
#define MSM_BOOT_UART_DM_CMD_RES_RFR_N       MSM_BOOT_UART_DM_CR_CH_CMD(E)
#define MSM_BOOT_UART_DM_CMD_RES_TX_ERR      MSM_BOOT_UART_DM_CR_CH_CMD(10)
#define MSM_BOOT_UART_DM_CMD_CLR_TX_DONE     MSM_BOOT_UART_DM_CR_CH_CMD(11)
#define MSM_BOOT_UART_DM_CMD_RES_BRKSTRT_INT MSM_BOOT_UART_DM_CR_CH_CMD(12)
#define MSM_BOOT_UART_DM_CMD_RES_BRKEND_INT  MSM_BOOT_UART_DM_CR_CH_CMD(13)
#define MSM_BOOT_UART_DM_CMD_RES_PER_FRM_INT MSM_BOOT_UART_DM_CR_CH_CMD(14)

/*UART General Command */
#define MSM_BOOT_UART_DM_CR_GENERAL_CMD(x)   ((x) << 8)

#define MSM_BOOT_UART_DM_GCMD_NULL            MSM_BOOT_UART_DM_CR_GENERAL_CMD(0)
#define MSM_BOOT_UART_DM_GCMD_CR_PROT_EN      MSM_BOOT_UART_DM_CR_GENERAL_CMD(1)
#define MSM_BOOT_UART_DM_GCMD_CR_PROT_DIS     MSM_BOOT_UART_DM_CR_GENERAL_CMD(2)
#define MSM_BOOT_UART_DM_GCMD_RES_TX_RDY_INT  MSM_BOOT_UART_DM_CR_GENERAL_CMD(3)
#define MSM_BOOT_UART_DM_GCMD_SW_FORCE_STALE  MSM_BOOT_UART_DM_CR_GENERAL_CMD(4)
#define MSM_BOOT_UART_DM_GCMD_ENA_STALE_EVT   MSM_BOOT_UART_DM_CR_GENERAL_CMD(5)
#define MSM_BOOT_UART_DM_GCMD_DIS_STALE_EVT   MSM_BOOT_UART_DM_CR_GENERAL_CMD(6)

/* UART Interrupt Mask Register */
#if PERIPH_BLK_BLSP
#define MSM_BOOT_UART_DM_IMR(base)             ((base) + 0xB0)
#else
#define MSM_BOOT_UART_DM_IMR(base)             ((base) + 0x14)
#endif

#define MSM_BOOT_UART_DM_TXLEV               (1 << 0)
#define MSM_BOOT_UART_DM_RXHUNT              (1 << 1)
#define MSM_BOOT_UART_DM_RXBRK_CHNG          (1 << 2)
#define MSM_BOOT_UART_DM_RXSTALE             (1 << 3)
#define MSM_BOOT_UART_DM_RXLEV               (1 << 4)
#define MSM_BOOT_UART_DM_DELTA_CTS           (1 << 5)
#define MSM_BOOT_UART_DM_CURRENT_CTS         (1 << 6)
#define MSM_BOOT_UART_DM_TX_READY            (1 << 7)
#define MSM_BOOT_UART_DM_TX_ERROR            (1 << 8)
#define MSM_BOOT_UART_DM_TX_DONE             (1 << 9)
#define MSM_BOOT_UART_DM_RXBREAK_START       (1 << 10)
#define MSM_BOOT_UART_DM_RXBREAK_END         (1 << 11)
#define MSM_BOOT_UART_DM_PAR_FRAME_ERR_IRQ   (1 << 12)

#define MSM_BOOT_UART_DM_IMR_ENABLED         (MSM_BOOT_UART_DM_TX_READY | \
                                              MSM_BOOT_UART_DM_TXLEV    | \
                                              MSM_BOOT_UART_DM_RXLEV    | \
                                              MSM_BOOT_UART_DM_RXSTALE)

/* UART Interrupt Programming Register */
#define MSM_BOOT_UART_DM_IPR(base)             ((base) + 0x18)
#define MSM_BOOT_UART_DM_STALE_TIMEOUT_LSB   0x0f
#define MSM_BOOT_UART_DM_STALE_TIMEOUT_MSB   0	/* Not used currently */

/* UART Transmit/Receive FIFO Watermark Register */
#define MSM_BOOT_UART_DM_TFWR(base)            ((base) + 0x1C)
/* Interrupt is generated when FIFO level is less than or equal to this value */
#define MSM_BOOT_UART_DM_TFW_VALUE           0

#define MSM_BOOT_UART_DM_RFWR(base)            ((base) + 0x20)
/*Interrupt generated when no of words in RX FIFO is greater than this value */
#define MSM_BOOT_UART_DM_RFW_VALUE           0

/* UART Hunt Character Register */
#define MSM_BOOT_UART_DM_HCR(base)             ((base) + 0x24)

/* Used for RX transfer initialization */
#define MSM_BOOT_UART_DM_DMRX(base)            ((base) + 0x34)

/* Default DMRX value - any value bigger than FIFO size would be fine */
#define MSM_BOOT_UART_DM_DMRX_DEF_VALUE    0x220

/* Register to enable IRDA function */
#if PERIPH_BLK_BLSP
#define MSM_BOOT_UART_DM_IRDA(base)            ((base) + 0xB8)
#else
#define MSM_BOOT_UART_DM_IRDA(base)            ((base) + 0x38)
#endif

/* UART Data Mover Enable Register */
#define MSM_BOOT_UART_DM_DMEN(base)            ((base) + 0x3C)

/* Number of characters for Transmission */
#define MSM_BOOT_UART_DM_NO_CHARS_FOR_TX(base) ((base) + 0x040)

/* UART RX FIFO Base Address */
#define MSM_BOOT_UART_DM_BADR(base)            ((base) + 0x44)

/* UART Status Register */
#if PERIPH_BLK_BLSP
#define MSM_BOOT_UART_DM_SR(base)              ((base) + 0x0A4)
#else
#define MSM_BOOT_UART_DM_SR(base)              ((base) + 0x008)
#endif
#define MSM_BOOT_UART_DM_SR_RXRDY            (1 << 0)
#define MSM_BOOT_UART_DM_SR_RXFULL           (1 << 1)
#define MSM_BOOT_UART_DM_SR_TXRDY            (1 << 2)
#define MSM_BOOT_UART_DM_SR_TXEMT            (1 << 3)
#define MSM_BOOT_UART_DM_SR_UART_OVERRUN     (1 << 4)
#define MSM_BOOT_UART_DM_SR_PAR_FRAME_ERR    (1 << 5)
#define MSM_BOOT_UART_DM_RX_BREAK            (1 << 6)
#define MSM_BOOT_UART_DM_HUNT_CHAR           (1 << 7)
#define MSM_BOOT_UART_DM_RX_BRK_START_LAST   (1 << 8)

/* UART Receive FIFO Registers - 4 in numbers */
#if PERIPH_BLK_BLSP
#define MSM_BOOT_UART_DM_RF(base, x)      ((base) + 0x140 + (4*(x)))
#else
#define MSM_BOOT_UART_DM_RF(base, x)      ((base) + 0x70 + (4*(x)))
#endif

/* UART Masked Interrupt Status Register */
#if PERIPH_BLK_BLSP
#define MSM_BOOT_UART_DM_MISR(base)         ((base) + 0xAC)
#else
#define MSM_BOOT_UART_DM_MISR(base)         ((base) + 0x10)
#endif

/* UART Interrupt Status Register */
#if PERIPH_BLK_BLSP
#define MSM_BOOT_UART_DM_ISR(base)          ((base) + 0xB4)
#else
#define MSM_BOOT_UART_DM_ISR(base)          ((base) + 0x14)
#endif

/* Number of characters received since the end of last RX transfer */
#if PERIPH_BLK_BLSP
#define MSM_BOOT_UART_DM_RX_TOTAL_SNAP(base)  ((base) + 0xBC)
#else
#define MSM_BOOT_UART_DM_RX_TOTAL_SNAP(base)  ((base) + 0x38)
#endif

/* UART TX FIFO Status Register */
#define MSM_BOOT_UART_DM_TXFS(base)           ((base) + 0x4C)
#define MSM_BOOT_UART_DM_TXFS_STATE_LSB(x)   MSM_BOOT_UART_DM_EXTR_BITS(x,0,6)
#define MSM_BOOT_UART_DM_TXFS_STATE_MSB(x)   MSM_BOOT_UART_DM_EXTR_BITS(x,14,31)
#define MSM_BOOT_UART_DM_TXFS_BUF_STATE(x)   MSM_BOOT_UART_DM_EXTR_BITS(x,7,9)
#define MSM_BOOT_UART_DM_TXFS_ASYNC_STATE(x) MSM_BOOT_UART_DM_EXTR_BITS(x,10,13)

/* UART RX FIFO Status Register */
#define MSM_BOOT_UART_DM_RXFS(base)           ((base) + 0x50)
#define MSM_BOOT_UART_DM_RXFS_STATE_LSB(x)   MSM_BOOT_UART_DM_EXTR_BITS(x,0,6)
#define MSM_BOOT_UART_DM_RXFS_STATE_MSB(x)   MSM_BOOT_UART_DM_EXTR_BITS(x,14,31)
#define MSM_BOOT_UART_DM_RXFS_BUF_STATE(x)   MSM_BOOT_UART_DM_EXTR_BITS(x,7,9)
#define MSM_BOOT_UART_DM_RXFS_ASYNC_STATE(x) MSM_BOOT_UART_DM_EXTR_BITS(x,10,13)

/* Macros for Common Errors */
#define MSM_BOOT_UART_DM_E_SUCCESS           0
#define MSM_BOOT_UART_DM_E_FAILURE           1
#define MSM_BOOT_UART_DM_E_TIMEOUT           2
#define MSM_BOOT_UART_DM_E_INVAL             3
#define MSM_BOOT_UART_DM_E_MALLOC_FAIL       4
#define MSM_BOOT_UART_DM_E_RX_NOT_READY      5
// ************* from lk uart_dm.h END ****************************************

// ************* from lk iomap.h START ****************************************
#define PERIPH_SS_BASE              0xF9800000
#define BLSP1_UART3_BASE            (PERIPH_SS_BASE + 0x00120000)
// ************* from lk iomap.h END ******************************************

// ************* from lk uart_dm.c START **************************************
static unsigned int msm_boot_uart_dm_reset(uint32_t base)
{
    writel(MSM_BOOT_UART_DM_CMD_RESET_RX, MSM_BOOT_UART_DM_CR(base));
    writel(MSM_BOOT_UART_DM_CMD_RESET_TX, MSM_BOOT_UART_DM_CR(base));
    writel(MSM_BOOT_UART_DM_CMD_RESET_ERR_STAT, MSM_BOOT_UART_DM_CR(base));
    writel(MSM_BOOT_UART_DM_CMD_RES_TX_ERR, MSM_BOOT_UART_DM_CR(base));
    writel(MSM_BOOT_UART_DM_CMD_RES_STALE_INT, MSM_BOOT_UART_DM_CR(base));

    return MSM_BOOT_UART_DM_E_SUCCESS;
}

static unsigned int msm_boot_uart_dm_init_rx_transfer(uint32_t uart_dm_base)
{
    writel(MSM_BOOT_UART_DM_GCMD_DIS_STALE_EVT, MSM_BOOT_UART_DM_CR(uart_dm_base));
    writel(MSM_BOOT_UART_DM_CMD_RES_STALE_INT, MSM_BOOT_UART_DM_CR(uart_dm_base));
    writel(MSM_BOOT_UART_DM_DMRX_DEF_VALUE, MSM_BOOT_UART_DM_DMRX(uart_dm_base));
    writel(MSM_BOOT_UART_DM_GCMD_ENA_STALE_EVT, MSM_BOOT_UART_DM_CR(uart_dm_base));

    return MSM_BOOT_UART_DM_E_SUCCESS;
}

static unsigned int msm_boot_uart_dm_init(uint32_t uart_dm_base)
{
    /* Configure UART mode registers MR1 and MR2 */
    /* Hardware flow control isn't supported */
    writel(0x0, MSM_BOOT_UART_DM_MR1(uart_dm_base));

    /* 8-N-1 configuration: 8 data bits - No parity - 1 stop bit */
    writel(MSM_BOOT_UART_DM_8_N_1_MODE, MSM_BOOT_UART_DM_MR2(uart_dm_base));

    /* Configure Interrupt Mask register IMR */
    writel(MSM_BOOT_UART_DM_IMR_ENABLED, MSM_BOOT_UART_DM_IMR(uart_dm_base));

    /* Configure Tx and Rx watermarks configuration registers */
    /* TX watermark value is set to 0 - interrupt is generated when
     * FIFO level is less than or equal to 0 */
    writel(MSM_BOOT_UART_DM_TFW_VALUE, MSM_BOOT_UART_DM_TFWR(uart_dm_base));

    /* RX watermark value */
    writel(MSM_BOOT_UART_DM_RFW_VALUE, MSM_BOOT_UART_DM_RFWR(uart_dm_base));

    /* Configure Interrupt Programming Register */
    /* Set initial Stale timeout value */
    writel(MSM_BOOT_UART_DM_STALE_TIMEOUT_LSB, MSM_BOOT_UART_DM_IPR(uart_dm_base));

    /* Configure IRDA if required */
    /* Disabling IRDA mode */
    writel(0x0, MSM_BOOT_UART_DM_IRDA(uart_dm_base));

    /* Configure and enable sim interface if required */

    /* Configure hunt character value in HCR register */
    /* Keep it in reset state */
    writel(0x0, MSM_BOOT_UART_DM_HCR(uart_dm_base));

    /* Configure Rx FIFO base address */
    /* Both TX/RX shares same SRAM and default is half-n-half.
     * Sticking with default value now.
     * As such RAM size is (2^RAM_ADDR_WIDTH, 32-bit entries).
     * We have found RAM_ADDR_WIDTH = 0x7f */

    /* Issue soft reset command */
    msm_boot_uart_dm_reset(uart_dm_base);

    /* Enable/Disable Rx/Tx DM interfaces */
    /* Data Mover not currently utilized. */
    writel(0x0, MSM_BOOT_UART_DM_DMEN(uart_dm_base));

    /* Enable transmitter and receiver */
    writel(MSM_BOOT_UART_DM_CR_RX_ENABLE, MSM_BOOT_UART_DM_CR(uart_dm_base));
    writel(MSM_BOOT_UART_DM_CR_TX_ENABLE, MSM_BOOT_UART_DM_CR(uart_dm_base));

    /* Initialize Receive Path */
    msm_boot_uart_dm_init_rx_transfer(uart_dm_base);

    return MSM_BOOT_UART_DM_E_SUCCESS;
}

#define NON_PRINTABLE_ASCII_CHAR      128

static uint8_t pack_chars_into_words(uint8_t *buffer, uint8_t cnt, uint32_t *word)
{
    uint8_t num_chars_writtten = 0;
    int j;

    *word = 0;

     for(j=0; j < cnt; j++)
     {
         if (buffer[num_chars_writtten] == '\n')
         {
            /* replace '\n' by the NON_PRINTABLE_ASCII_CHAR and print '\r'.
             * While printing the NON_PRINTABLE_ASCII_CHAR, we will print '\n'.
             * Thus successfully replacing '\n' by '\r' '\n'.
             */
            *word |= ('\r' & 0xff) << (j * 8);
            buffer[num_chars_writtten] = NON_PRINTABLE_ASCII_CHAR;
         }
         else
         {
            if (buffer[num_chars_writtten] == NON_PRINTABLE_ASCII_CHAR)
            {
                buffer[num_chars_writtten] = '\n';
            }

             *word |= (buffer[num_chars_writtten] & 0xff) << (j * 8);

             num_chars_writtten++;
         }
     }

     return num_chars_writtten;
}

static unsigned int
msm_boot_uart_calculate_num_chars_to_write(char *data_in,
                 uint32_t *num_of_chars)
{
    int i = 0, j = 0;

    if ((data_in == NULL) || (*num_of_chars < 0)) {
        return MSM_BOOT_UART_DM_E_INVAL;
    }

    for (i = 0, j = 0; i < *num_of_chars; i++, j++) {
        if (data_in[i] == '\n') {
            j++;
        }

    }

    *num_of_chars = j;

    return MSM_BOOT_UART_DM_E_SUCCESS;
}

static unsigned int
msm_boot_uart_dm_write(uint32_t base, char *data, unsigned int num_of_chars)
{
    unsigned int tx_word_count = 0;
    unsigned int tx_char_left = 0, tx_char = 0;
    unsigned int tx_word = 0;
    int i = 0;
    char *tx_data = NULL;
    uint8_t num_chars_written;

    if ((data == NULL) || (num_of_chars <= 0)) {
        return MSM_BOOT_UART_DM_E_INVAL;
    }

    msm_boot_uart_calculate_num_chars_to_write(data, &num_of_chars);

    tx_data = data;

    /* Write to NO_CHARS_FOR_TX register number of characters
     * to be transmitted. However, before writing TX_FIFO must
     * be empty as indicated by TX_READY interrupt in IMR register
     */

    /* Check if transmit FIFO is empty.
     * If not we'll wait for TX_READY interrupt. */
    if (!(readl(MSM_BOOT_UART_DM_SR(base)) & MSM_BOOT_UART_DM_SR_TXEMT)) {
        while (!(readl(MSM_BOOT_UART_DM_ISR(base)) & MSM_BOOT_UART_DM_TX_READY)) {
            udelay(1);
            /* Kick watchdog? */
        }
    }

    //We need to make sure the DM_NO_CHARS_FOR_TX&DM_TF are are programmed atmoically.
    enter_critical_section();
    /* We are here. FIFO is ready to be written. */
    /* Write number of characters to be written */
    writel(num_of_chars, MSM_BOOT_UART_DM_NO_CHARS_FOR_TX(base));

    /* Clear TX_READY interrupt */
    writel(MSM_BOOT_UART_DM_GCMD_RES_TX_RDY_INT, MSM_BOOT_UART_DM_CR(base));

    /* We use four-character word FIFO. So we need to divide data into
     * four characters and write in UART_DM_TF register */
    tx_word_count = (num_of_chars % 4) ? ((num_of_chars / 4) + 1) :
        (num_of_chars / 4);
    tx_char_left = num_of_chars;

    for (i = 0; i < (int)tx_word_count; i++) {
        tx_char = (tx_char_left < 4) ? tx_char_left : 4;
        num_chars_written = pack_chars_into_words((uint8_t*)tx_data, tx_char, &tx_word);

        /* Wait till TX FIFO has space */
        while (!(readl(MSM_BOOT_UART_DM_SR(base)) & MSM_BOOT_UART_DM_SR_TXRDY)) {
            udelay(1);
        }

        /* TX FIFO has space. Write the chars */
        writel(tx_word, MSM_BOOT_UART_DM_TF(base, 0));
        tx_char_left = num_of_chars - (i + 1) * 4;
        tx_data = tx_data + num_chars_written;
    }
    exit_critical_section();

    return MSM_BOOT_UART_DM_E_SUCCESS;
}

static unsigned int
msm_boot_uart_dm_read(uint32_t base, unsigned int *data, int wait)
{
    static int rx_last_snap_count = 0;
    static int rx_chars_read_since_last_xfer = 0;

    if (data == NULL) {
        return MSM_BOOT_UART_DM_E_INVAL;
    }

    /* We will be polling RXRDY status bit */
    while (!(readl(MSM_BOOT_UART_DM_SR(base)) & MSM_BOOT_UART_DM_SR_RXRDY)) {
        /* if this is not a blocking call, we'll just return */
        if (!wait) {
            return MSM_BOOT_UART_DM_E_RX_NOT_READY;
        }
    }

    /* Check for Overrun error. We'll just reset Error Status */
    if (readl(MSM_BOOT_UART_DM_SR(base)) & MSM_BOOT_UART_DM_SR_UART_OVERRUN) {
        writel(MSM_BOOT_UART_DM_CMD_RESET_ERR_STAT, MSM_BOOT_UART_DM_CR(base));
    }

    /* RX FIFO is ready; read a word. */
    *data = readl(MSM_BOOT_UART_DM_RF(base, 0));

    /* increment the total count of chars we've read so far */
    rx_chars_read_since_last_xfer += 4;

    /* Rx transfer ends when one of the conditions is met:
     * - The number of characters received since the end of the previous
     *   xfer equals the value written to DMRX at Transfer Initialization
     * - A stale event occurred
     */

    /* If RX transfer has not ended yet */
    if (rx_last_snap_count == 0) {
        /* Check if we've received stale event */
        if (readl(MSM_BOOT_UART_DM_MISR(base)) & MSM_BOOT_UART_DM_RXSTALE) {
            /* Send command to reset stale interrupt */
            writel(MSM_BOOT_UART_DM_CMD_RES_STALE_INT, MSM_BOOT_UART_DM_CR(base));
        }

        /* Check if we haven't read more than DMRX value */
        else if ((unsigned int)rx_chars_read_since_last_xfer <
            readl(MSM_BOOT_UART_DM_DMRX(base))) {
            /* We can still continue reading before initializing RX transfer */
            return MSM_BOOT_UART_DM_E_SUCCESS;
        }

        /* If we've reached here it means RX
         * xfer end conditions been met
         */

        /* Read UART_DM_RX_TOTAL_SNAP register
         * to know how many valid chars
         * we've read so far since last transfer
         */
        rx_last_snap_count = readl(MSM_BOOT_UART_DM_RX_TOTAL_SNAP(base));

    }

    /* If there are still data left in FIFO we'll read them before
     * initializing RX Transfer again */
    if ((rx_last_snap_count - rx_chars_read_since_last_xfer) >= 0) {
        return MSM_BOOT_UART_DM_E_SUCCESS;
    }

    msm_boot_uart_dm_init_rx_transfer(base);
    rx_last_snap_count = 0;
    rx_chars_read_since_last_xfer = 0;

    return MSM_BOOT_UART_DM_E_SUCCESS;
}
// ************* from lk uart_dm.c END ****************************************

#define UART_BASE BLSP1_UART3_BASE

// We would normally set this to zero, but this puts it in the bss section,
// when u-boot relocates this variable will be set back to zero after msm_serial_init
// has been called.
static int uart_init_flag = 0xff;

static int msm_serial_init(void)
{
    // LK has already performed the following initialisation in uart_dm_init()
    // - uart clock
    // - GPIO
    // - GSBI
    if(MSM_BOOT_UART_DM_E_SUCCESS == msm_boot_uart_dm_init(UART_BASE))
    {
        uart_init_flag = 1;
    }
    return 0;
}

static int msm_serial_fini(void)
{
    writel(MSM_BOOT_UART_DM_CMD_RESET_RX, MSM_BOOT_UART_DM_CR(UART_BASE));
    writel(MSM_BOOT_UART_DM_CMD_RESET_TX, MSM_BOOT_UART_DM_CR(UART_BASE));
    writel(MSM_BOOT_UART_DM_CMD_RESET_ERR_STAT, MSM_BOOT_UART_DM_CR(UART_BASE));
    writel(MSM_BOOT_UART_DM_CMD_RES_TX_ERR, MSM_BOOT_UART_DM_CR(UART_BASE));
    writel(MSM_BOOT_UART_DM_CMD_RES_STALE_INT, MSM_BOOT_UART_DM_CR(UART_BASE));

    return 0;
}

static void msm_serial_setbrg(void)
{
    //Not implemented
}

static void msm_serial_putc(char ch)
{
    if(uart_init_flag != 1)
        return;

    msm_boot_uart_dm_write(UART_BASE, &ch, 1);
}

static int msm_serial_getc(void)
{
    int byte;
    static unsigned int word = 0;
    int wait = 1; //blocking read
    if(uart_init_flag != 1)
        return -1;

    if (!word)
    {
        /* Read from FIFO only if it's a first read or all the four
         * characters out of a word have been read */
        if (msm_boot_uart_dm_read(UART_BASE, &word, wait) != MSM_BOOT_UART_DM_E_SUCCESS)
            return -1;
    }

    byte = (int)word & 0xff;
    word = word >> 8;

    return byte;
}

static int msm_serial_tstc(void)
{
    return (readl(MSM_BOOT_UART_DM_SR(UART_BASE)) & MSM_BOOT_UART_DM_SR_RXRDY);
}

static struct serial_device msm_serial_drv = {
    .name    = "arm_msm",
    .start   = msm_serial_init,
    .stop    = msm_serial_fini,
    .setbrg  = msm_serial_setbrg,
    .putc    = msm_serial_putc,
    .puts    = default_serial_puts,
    .getc    = msm_serial_getc,
    .tstc    = msm_serial_tstc,
};

void msm_serial_initialize(void)
{
    serial_register(&msm_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
    return &msm_serial_drv;
}
