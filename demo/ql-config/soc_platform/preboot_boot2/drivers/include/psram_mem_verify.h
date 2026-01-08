#ifndef PSRAM_MEM_VERIFY_H
#define PSRAM_MEM_VERIFY_H

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "pt.h"
#include "process.h"
#include "contiki.h"
#include "flash.h"

#define PSRAM_MEM_TEST_ALL 0

/*
 * crane_flasehr and psram.c will use it
 * If scan psram memory to test psram, enable it
 */
/*#define PSRAM_MEM_TEST_ALL 1 */

/*---------------------------------------------------------------------------*/
typedef unsigned long ul;
typedef unsigned long long ull;
typedef unsigned long volatile ulv;
typedef unsigned char volatile u8v;
typedef unsigned short volatile u16v;
/*---------------------------------------------------------------------------*/
typedef struct {
  /* Containers for the State */
  struct pt pt;
  /* Input Variables */
  process_event_t ev;
  union {
    unsigned long volatile *bufa;
    unsigned long start;
  };
  union {
    unsigned long volatile *bufb;
    unsigned long end;
  };
  union {
    size_t count;
    size_t size;
  };

  unsigned int id;
  /* Output Variables */
  int result;               /**< Result Code */
} pmem_state_t;
/*---------------------------------------------------------------------------*/
/* start a protothread to do thread_fun(uint64_t address, const void *buf, size_t sz).
 * state: supply parent pt for thread_fun.
 * child_state: pt for thread_fun.(if multi pt threads, child_state may be custom defined)
 * state->result: return result of thread_fun.
 *
 * eg: if thread_fun is test_xor_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count),
 * invoke PMEM_BASE_TASK(state, test_xor_comparison_state, test_xor_comparison, bufa, bufb, count);
 */
#define PMEM_BASE_TASK(state, child_state, thread_fun, buf_a, buf_b, sz, case_id) \
  { \
    static pmem_state_t child_state; \
    memset(&child_state, 0, sizeof(child_state)); \
    child_state.ev = state->ev; \
    child_state.bufa = buf_a; \
    child_state.bufb = buf_b; \
    child_state.count = sz; \
    child_state.id = case_id; \
    PT_SPAWN(&state->pt, &(child_state.pt), thread_fun(&child_state)); \
    state->result = child_state.result; \
  }
/*---------------------------------------------------------------------------*/
#define ADD_FAIL_NUM(state) \
  { \
    if(!state->result) { \
      LOG_INFO(" ok\n"); \
    } else { \
      LOG_INFO(" fail\n"); \
      pmem_fail_num++; \
    } \
    LOG_INFO("\n\r"); \
  }
/*---------------------------------------------------------------------------*/
typedef enum {
  PMEM_ID_TEST_RANDOM,                 /*0*/
  PMEM_ID_TEST_XOR,                    /*1*/
  PMEM_ID_TEST_SUB,                    /*2*/
  PMEM_ID_TEST_MUL,                    /*3*/
  PMEM_ID_TEST_DIV,                    /*4*/
  PMEM_ID_TEST_OR,                     /*5*/
  PMEM_ID_TEST_AND,                    /*6*/
  PMEM_ID_TEST_SEQINC,                 /*7*/
  PMEM_ID_TEST_SOLIDBITS,              /*8*/
  PMEM_ID_TEST_BLOCKSEQ,               /*9*/
  PMEM_ID_TEST_CHECHERBORD,            /*10*/
  PMEM_ID_TEST_BITSPREAD,              /*11*/
  PMEM_ID_TEST_BITFLIP,                /*12*/
  PMEM_ID_TEST_WALKBITS1,              /*13*/
  PMEM_ID_TEST_WALKBITS0,              /*14*/
  PMEM_ID_TEST_8BIT_WIDE_RANDOM,       /*15*/
  PMEM_ID_TEST_16BIT_WIDE_RANDOM,      /*16*/
  PMEM_ID_TEST_DDR_PATTERN_ALL_0,      /*17*/
  PMEM_ID_TEST_DDR_PATTERN_ALL_1,      /*18*/
  PMEM_ID_TEST_DDR_PATTERN_SPLIT_1,    /*19*/

  PMEM_ID_MAX
} pmem_id_t;

PT_THREAD(psram_mem_test_all(pmem_state_t * state));

#endif
