#ifndef CINIT_H
#define CINIT_H

#ifdef __cplusplus
extern "C" {
#endif

void cinit_clear_bss(void);
void cinit_call_constructors(void);

#ifdef __cplusplus
}
#endif

#endif /* CINIT_H */
