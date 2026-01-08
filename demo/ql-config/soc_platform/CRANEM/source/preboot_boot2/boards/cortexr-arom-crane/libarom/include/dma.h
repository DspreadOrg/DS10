#ifndef _DMA_H_
#define _DMA_H_

#include "cpu.h"

/*
 * DMA register address offset definitions
 */
#define DMA_CSR0_31         0x0
#define DMA_ALGN            0xA0
#define DMA_PCSR            0xA4
#define DMA_RQSR0           0xE0
#define DMA_RQSR1           0xE4
#define DMA_RQSR2           0xE8
#define DMA_INT             0xF0
#define DMA_RCMR0_63        0x100
#define DMA_RCMR64_99       0x1100
#define DMA_DADR0           0x200
#define DMA_SADR0_31        0x204
#define DMA_TADR0_31        0x208
#define DMA_CMD0_31         0x20C

#define DCMD_INCSRCADDR     (1 << 31)   /* Source Address Increment Setting. */
#define DCMD_INCTRGADDR     (1 << 30)   /* Target Address Increment Setting. */
#define DCMD_FLOWSRC        (1 << 29)   /* Flow Control by the source. */
#define DCMD_FLOWTRG        (1 << 28)   /* Flow Control by the target. */
#define DCMD_STARTIRQEN     (1 << 22)   /* Start Interrupt Enable */
#define DCMD_ENDIRQEN       (1 << 21)   /* End Interrupt Enable */
#define DCMD_ENDIAN         (1 << 18)   /* Device Endian-ness. */
#define DCMD_BURST8         (1 << 16)   /* 8 byte burst */
#define DCMD_BURST16        (2 << 16)   /* 16 byte burst */
#define DCMD_BURST32        (3 << 16)   /* 32 byte burst */
#define DCMD_BURST64        (4 << 16)   /* 64 byte burst */
#define DCMD_WIDTH1         (1 << 14)   /* 1 byte width */
#define DCMD_WIDTH2         (2 << 14)   /* 2 byte width (HalfWord) */
#define DCMD_WIDTH4         (3 << 14)   /* 4 byte width (Word) */
#define DCMD_LENGTH         0x01fff     /* length mask (max = 8K - 1) */

#define DCSR_RUN            (1 << 31)   /* Run Bit (read / write) */
#define DCSR_NODESC         (1 << 30)   /* No-Descriptor Fetch (read/write) */
#define DCSR_STOPIRQEN      (1 << 29)   /* Stop INT Enable (read / write) */
#define DCSR_REQPEND        (1 << 8)    /* Request Pending (read-only) */
#define DCSR_STOPSTATE      (1 << 3)    /* Stop State (read-only) */
#define DCSR_ENDINTR        (1 << 2)    /* End Interrupt (read / write) */
#define DCSR_STARTINTR      (1 << 1)    /* Start Interrupt (read / write) */
#define DCSR_BUSERR         (1 << 0)    /* Bus Error Interrupt (read/write) */

#define DRCMR_MAPVLD        (1 << 7)    /* Map Valid (read / write) */
#define DRCMR_CHLNUM        0x1f        /* mask for Chan Number (read/write) */

#define DCMD_INCSRCADDR_SHIFT       31  /* Source address increment */
#define DCMD_INCTRGADDR_SHIFT       30  /* Target address increment */
#define DCMD_FLOWSRC_SHIFT          29  /* Source flow control */
#define DCMD_FLOWTRG_SHIFT          28  /* Target flow control */
#define DCMD_MAX_BURST_SIZE_SHIFT   16  /* Max burst size */
#define DCMD_WIDTH_SHIFT            14  /* Width of the on-chip peripheral */

#define DCSR(ch)        (DMA_BASE + DMA_CSR0_31 + 4 * (ch))
#define DSADR(ch)       (DMA_BASE + DMA_SADR0_31 + 4 * 4 * (ch))
#define DTADR(ch)       (DMA_BASE + DMA_TADR0_31 + 4 * 4 * (ch))
#define DCMD(ch)        (DMA_BASE + DMA_CMD0_31 + 4 * 4 * (ch))

#define DMA_BURST_SIZE_8_BYTE   1
#define DMA_BURST_SIZE_16_BYTE  2
#define DMA_BURST_SIZE_32_BYTE  3

#define DMA_DEVICE_ID_AES_TX    69
#define DMA_DEVICE_ID_AES_RX    68
#define DMA_DEVICE_ID_QSIP_TX   99
#define DMA_DEVICE_ID_QSPI_RX   97

#define DMA_CHANNEL_AES_TX      1
#define DMA_CHANNEL_AES_RX      0
#define DMA_CHANNEL_QSPI_TX     28
#define DMA_CHANNEL_QSPI_RX     27

struct dma_op_t {
    unsigned channel;
    unsigned src;
    unsigned dest;
    unsigned len;           /* Length of transfer in bytes */
    unsigned width;         /* Width of on-chip peripheral */
    unsigned burst_size;    /* Max burst size of each data */
    unsigned flow_dest;     /* Flow control of the target. */
    unsigned flow_src;      /* Flow control of the source */
    unsigned inc_dest_addr; /* Target address increment setting*/
    unsigned inc_src_addr;  /* Source address increment setting*/
};

void dmac_map_device_to_channel(int device_num, int channel);
int dma_xfer(struct dma_op_t *op);
int dma_channel_running(int channel);
unsigned dma_mapped_addr(unsigned addr);

#endif
