/*
 *   Offset = 0xD8010
 *
 *        00            0D            80            10
 * Bits 31-24         23-16         15-8           7-0
 * 0000 0000     0000 1101     1000 0000     0001 0000
 *
 * Bits 31-25      24 23 22       21-12          11-2         10
 *      0000000     0  0  0     0011011000     0000000100     00
 *                  S I1  I2         H              L
 *
 * J1 = ~I1 ^ S = 1
 * J2 = ~I2 ^ S = 1
 * https://web.eecs.umich.edu/~prabal/teaching/eecs373-f10/readings/ARMv7-M_ARM.pdf
 * A.6.7.18
 */

#include <assert.h>

/*---------------------------------------------------------------------------*/
typedef union {
  struct {
    unsigned last1 : 1;
    unsigned imm11l : 11;
    unsigned imm10h : 10;
    unsigned i2 : 1;
    unsigned i1 : 1;
    unsigned s : 1;
    unsigned msb7 : 7;
  } o;
  int v;
} bl_offset_t;

typedef union {
  struct {
    unsigned imm11l : 11;
    unsigned j2 : 1;
    unsigned fixed1 : 1;
    unsigned j1 : 1;
    unsigned fixed2 : 2;
    unsigned imm10h : 10;
    unsigned s : 1;
    unsigned msb5 : 5;
  } i;
  unsigned v;
} bl_instruction_t;
/*---------------------------------------------------------------------------*/
/*
 * Provide the address shown in the disassembler for the instruction to
 * calculate for. When the code executes this will be PC.
 * Provide the address of the function (branch traget / btarget) the code
 * should branch to.
 * The return value will be an encode instruction ready to be an instruction
 * that can be patched into an ARM binary with a hex editor.
 * Why would you use this?
 * If you want to branch to a different function with the same (or at least
 * comptabile) signature, you can replace the branch instruction and
 * alter the code. This particular instruction is a little trickier than
 * direct branch instructions.
 */
unsigned
bl_encode(unsigned pc, unsigned target)
{
  bl_offset_t offset;
  bl_instruction_t final_instr;

  pc += 4;          /* Due to arm pipeline */
  pc &= 0xfffffffe; /* 2 byte alignment */

  offset.v = target - pc;
  assert(offset.v < 16 * 1024 * 1024 && offset.v > -16 * 1024 * 1024);
  final_instr.v = 0;

  /* bits 27-31 = 0b11110 */
  final_instr.i.msb5 = 0x1e;

  /* bit 26 = S */
  final_instr.i.s = offset.o.s;

  /* imm10h, bits 16-25 */
  final_instr.i.imm10h = offset.o.imm10h;

  /* static */
  final_instr.i.fixed2 = 0x3;

  /* j1 */
  final_instr.i.j1 = (offset.o.i1 ^ 0x1) ^ offset.o.s;

  /* static */
  final_instr.i.fixed1 = 0x1;

  /* j2 */
  final_instr.i.j2 = (offset.o.i2 ^ 0x1) ^ offset.o.s;

  /* imm11l */
  final_instr.i.imm11l = offset.o.imm11l;

  unsigned v = final_instr.v;
  v = v >> 16 | v << 16;

  return v;
}
/*---------------------------------------------------------------------------*/
