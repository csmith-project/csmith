/*
 * This is a RANDOMLY GENERATED PROGRAM.
 *
 * Generator: 2.3.0
 * Options:   --no-math64 --paranoid --longlong --no-pointers --arrays --no-jumps --no-consts --volatiles --volatile-pointers --checksum --no-divs --no-muls --compound-assignment --no-structs --no-packed-struct --bitfields --no-argc --unions -s 2694622287 --output tmp.c
 * Seed:      2694622287
 */

#include "csmith.h"


static long __undefined;

/* --- Struct/Union Declarations --- */
union U0 {
   volatile int16_t  f0;
};

/* --- GLOBAL VARIABLES --- */
static int16_t g_2 = (-1L);
static unsigned long g_3[3][6] = {{0x506D4441L,0x506D4441L,0x506D4441L,0x506D4441L,0x506D4441L,0x506D4441L},{0x506D4441L,0x506D4441L,0x506D4441L,0x506D4441L,0x506D4441L,0x506D4441L},{0x506D4441L,0x506D4441L,0x506D4441L,0x506D4441L,0x506D4441L,0x506D4441L}};
static volatile int16_t g_4 = 0L;/* VOLATILE GLOBAL g_4 */
static union U0 g_5 = {0xF09CL};/* VOLATILE GLOBAL g_5 */


/* --- FORWARD DECLARATIONS --- */
static union U0  func_1(void);


/* --- FUNCTIONS --- */
/* ------------------------------------------ */
/* 
 * reads : g_2 g_5
 * writes: g_3 g_4
 */
static union U0  func_1(void)
{ /* block id: 0 */
    g_3[1][2] = g_2;
    g_4 = (-4L);
    return g_5;
    /* statement id: 3 */
    }




/* ---------------------------------------- */
int main (void)
{
    int i, j;
    int print_hash_value = 0;
    platform_main_begin();
    crc32_gentab();
    func_1();
    transparent_crc_bytes (&g_2, sizeof(g_2), "g_2", print_hash_value);
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 6; j++)
        {
            transparent_crc(g_3[i][j], "g_3[i][j]", print_hash_value);
            if (print_hash_value) printf("index = [%d][%d]\n", i, j);

        }
    }
    transparent_crc_bytes (&g_4, sizeof(g_4), "g_4", print_hash_value);
    transparent_crc_bytes (&g_5.f0, sizeof(g_5.f0), "g_5.f0", print_hash_value);
    platform_main_end(crc32_context ^ 0xFFFFFFFFUL, print_hash_value);
    return 0;
}

/************************ statistics *************************
XXX max struct depth: 0
breakdown:
   depth: 0, occurrence: 4
XXX total union variables: 1

XXX non-zero bitfields defined in structs: 0
XXX zero bitfields defined in structs: 0
XXX const bitfields defined in structs: 0
XXX volatile bitfields defined in structs: 0
XXX structs with bitfields in the program: 0
breakdown:
XXX full-bitfields structs in the program: 0
breakdown:
XXX times a bitfields struct's address is taken: 0
XXX times a bitfields struct on LHS: 0
XXX times a bitfields struct on RHS: 0
XXX times a single bitfield on LHS: 0
XXX times a single bitfield on RHS: 0

XXX max expression depth: 1
breakdown:
   depth: 1, occurrence: 5

XXX total number of pointers: 0

XXX times a non-volatile is read: 2
XXX times a non-volatile is write: 1
XXX times a volatile is read: 0
XXX    times read thru a pointer: 0
XXX times a volatile is write: 1
XXX    times written thru a pointer: 0
XXX times a volatile is available for access: 0
XXX percentage of non-volatile access: 75

XXX forward jumps: 0
XXX backward jumps: 0

XXX stmts: 3
XXX max block depth: 0
breakdown:
   depth: 0, occurrence: 3

XXX percentage a fresh-made variable is used: 100
XXX percentage an existing variable is used: 0
********************* end of statistics **********************/

