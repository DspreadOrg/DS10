#ifndef _DMA_COMMON_H_
#define _DMA_COMMON_H_

struct dma_op_t {
  unsigned channel;
  unsigned src;
  unsigned dest;
  unsigned len;             /* Length of transfer in bytes */
  unsigned width;           /* Width of on-chip peripheral */
  unsigned burst_size;      /* Max burst size of each data */
  unsigned flow_dest;       /* Flow control of the target. */
  unsigned flow_src;        /* Flow control of the source */
  unsigned inc_dest_addr;   /* Target address increment setting*/
  unsigned inc_src_addr;    /* Source address increment setting*/
};

void dmac_map_device_to_channel(int device_num, int channel);
int dma_xfer(struct dma_op_t *op);
int dma_channel_running(int channel);
unsigned dma_mapped_addr(unsigned addr);

#endif
