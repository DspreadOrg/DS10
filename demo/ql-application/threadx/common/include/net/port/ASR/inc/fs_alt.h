
#ifndef MBEDTLS_FS_ALT_H
#define MBEDTLS_FS_ALT_H

#include "ql_fs.h"

#ifdef FILE
#undef FILE
#endif
#define FILE QFILE

#ifdef fopen
#undef fopen
#endif
#define fopen ql_fopen

#ifdef fwrite
#undef fwrite
#endif
#define fwrite ql_fwrite

#ifdef fread
#undef fread
#endif
#define fread ql_fread

#ifdef fputc
#undef fputc
#endif
#define fputc ql_fputc

#ifdef fputs
#undef fputs
#endif
#define fputs ql_fputs

#ifdef fprintf
#undef fprintf
#endif
#define fprintf ql_fprintf

#ifdef fgetc
#undef fgetc
#endif
#define fgetc ql_fgetc

#ifdef fgets
#undef fgets
#endif
#define fgets ql_fgets

#ifdef fscanf
#undef fscanf
#endif
#define fscanf ql_fscanf

#ifdef fseek
#undef fseek
#endif
#define fseek ql_fseek

#ifdef ftell
#undef ftell
#endif
#define ftell ql_ftell

#ifdef frewind
#undef frewind
#endif
#define frewind ql_frewind

#ifdef fsize
#undef fsize
#endif
#define fsize ql_fsize

#ifdef ftruncate
#undef ftruncate
#endif
#define ftruncate ql_ftruncate


#ifdef fclose
#undef fclose
#endif
#define fclose ql_fclose

#endif

