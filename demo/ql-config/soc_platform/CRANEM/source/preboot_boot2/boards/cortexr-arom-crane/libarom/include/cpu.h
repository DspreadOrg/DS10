#ifndef _CPU_H_
#define _CPU_H_

#define CRANE_MFPR_BASE                      0xD401E000
#define M_ICU_BASE                           0xD4282000
#define PMU_BASE                             0xD4282800
#define PMU_MAIN_BASE                        0xD4050000
#define QSPI_IPB_BASE                        0xD420B000
#define DMA_BASE                             0xD4000000
#define CIU_BASE                             0xD4282C00
#define GEU_BASE                             0xD4201000

/* apb clock */
#define APBC_BASE                            0xD4015000
#define APBC_TWSI0_CLK_RST                   0x2C /* CI2C 0 */
#define APBC_TWSI1_CLK_RST                   0x60 /* CI2C 1 */
#define APBC_TWSI2_CLK_RST                   0x78 /* PI2C */
#define APBC_TWSI3_CLK_RST                   0x7C /* CI2C 2 */

/* timer */
#define CRANE_APB_TIMER0_BASE                0xD4014000
#define TIMER_BASE                           CRANE_APB_TIMER0_BASE
#define TIMER_MAX_DEV_NUM                    3   /* counter */
#define TIMER_MAX_CHAN_NUM                   3   /* match */
#define TIMER_DEV0_IRQ                       13
#define TIMER_DEV1_IRQ                       14
#define TIMER_DEV2_IRQ                       14

/* uart */
#define UART0_BASE                           0xD4017000
#define UART1_BASE                           0xD4018000
#define UART0_IRQ                            27
#define UART1_IRQ                            59
#define HAS_UART_1                           1
#define APBC_CLK_FPGA                        13000000
#define APBC_CLK_VDK                         24000000

/* usb */
#define CRANE_USB_BASE                       0xD4208000
#define USB_IRQ_NUM                          44

/* TWSI */
#define TWSI0_BASE 0xD4011000
#define TWSI1_BASE 0xD4010800
#define TWSI2_BASE 0xD4037000
#define TWSI3_BASE 0xD403E000

#define TWSI_BMR        0x0
#define TWSI_DBR        0x8
#define TWSI_CR         0x10
#define TWSI_SR         0x18
#define TWSI_SAR        0x20
#define TWSI_LCR        0x28
#define TWSI_WCR        0x30
#define TWSI_WFIFO      0x40
#define TWSI_WFIFO_WPTR 0x44
#define TWSI_WFIFO_RPTR 0x48
#define TWSI_RFIFO      0x50
#define TWSI_RFIFO_WPTR 0x54
#define TWSI_RFIFO_RPTR 0x58
#define TWSI_RST_CYC    0x60

/* chip id */
#define CHIP_ID(x)                           ((x) & 0xFFFF)
#define REV_ID(x)                            (((x) >> 16) & 0xFF)

#define CHIP_ID_CRANE                        (0x6731)
#define CHIP_ID_CRANEG                       (0x3603) /*craneG A0*/
#define CHIP_ID_CRANEM                       (0x1603) /*craneM A0*/
#define CHIP_ID_FALCON                       (0x1803)
#define CHIP_ID_CRANEG_Z2                    (0x3602)
#define CHIP_ID_CRANEL                       (0x1606)

/* ----- Control register bits ---------------------------------------- */

#define CR_START                (1 << 0)                /* start bit */
#define CR_STOP                 (1 << 1)                /* stop bit */
#define CR_ACKNAK               (1 << 2)                /* send ACK(0) or NAK(1) */
#define CR_TB                   (1 << 3)                /* transfer byte bit */
#define CR_MA                   (1 << 4)                /* master abort */
#define CR_SCLE                 (1 << 5)                /* master clock enable, mona SCLEA */
#define CR_IUE                  (1 << 6)                /* unit enable */
#define CR_GCD                  (1 << 7)                /* general call disable */
#define CR_ITEIE                (1 << 8)                /* enable tx interrupts */
#define CR_DRFIE                (1 << 9)                /* enable rx interrupts, mona: DRFIE */
#define CR_BEIE                 (1 << 10)               /* enable bus error ints */
#define CR_SSDIE                (1 << 11)               /* slave STOP detected int enable */
#define CR_ALDIE                (1 << 12)               /* enable arbitration interrupt */
#define CR_SADIE                (1 << 13)               /* slave address detected int enable */
#define CR_UR                   (1 << 14)               /* unit reset */
#define CR_FM                   (1 << 15)               /* Bus Mode Fast */
#define CR_HS                   (1 << 16)               /* Bus Mode HIGH */
#define CR_MSDIE                (1 << 17)               /* master stop detected interrupt enable */
#define CR_MSDE                 (1 << 18)               /* master stop detected enable */
#define CR_GPIOEN               (1 << 19)               /* GPIO mode enable for SCL during HS mode */
#define CR_FIFOEN               (1 << 20)               /* fifo mode enable */
#define CR_TXBEGIN              (1 << 21)               /* transaction begin */
#define CR_TXDONE_IE            (1 << 22)               /* transaction done interrupt enable */
#define CR_RXHF_IE              (1 << 23)               /* rx fifo half full interrupt enable */
#define CR_TXE_IE               (1 << 24)               /* tx fifo empty interrupt enable */
#define CR_RXF_IE               (1 << 25)               /* rx fifo full interrupt enable */
#define CR_RXOV_IE              (1 << 26)               /* rx fifo overrun intterrupt enable */
#define CR_DMA_EN               (1 << 27)               /* dma enable for both rx and tx */
#define CR_I2C_BUS_RESET_REQ    (1 << 28)               /* bus reset request*/

/* ----- Status register bits ----------------------------------------- */

#define SR_RWM                  (1 << 0)                /* read/write mode */
#define SR_ACKNAK               (1 << 1)                /* ack/nak status */
#define SR_UB                   (1 << 2)                /* unit busy */
#define SR_IBB                  (1 << 3)                /* bus busy */
#define SR_SSD                  (1 << 4)                /* slave stop detected */
#define SR_ALD                  (1 << 5)                /* arbitration loss detected */
#define SR_ITE                  (1 << 6)                /* tx buffer empty */
#define SR_IRF                  (1 << 7)                /* rx buffer full */
#define SR_GCAD                 (1 << 8)                /* general call address detected */
#define SR_SAD                  (1 << 9)                /* slave address detected */
#define SR_BED                  (1 << 10)               /* bus error no ACK/NAK */
#define SR_EBB                  (1 << 11)               /* early bus busy */
#define SR_MSD                  (1 << 12)               /* master stop detected */
#define SR_TXDONE               (1 << 13)               /* transaction done */
#define SR_RXHF                 (1 << 14)               /* rx fifo half full */
#define SR_TXE                  (1 << 15)               /* tx fifo empty */
#define SR_RXF                  (1 << 16)               /* rx fifo full */
#define SR_RXOV                 (1 << 17)               /* rx fifo overrun */

#define HW_PLATFORM_TYPE_SILICON             0
#define HW_PLATFORM_TYPE_FPGA                1
#define HW_PLATFORM_TYPE_ZEBU_Z1             2
#define HW_PLATFORM_TYPE_VDK                 3
unsigned int hw_platform_type(void);
unsigned int hw_chip_id(void);

#endif /* _CPU_H_ */
