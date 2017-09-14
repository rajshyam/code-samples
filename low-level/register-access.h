/**
 */

#ifndef _REGISTER_ACCESS_SAMPLE_H_
#define _REGISTER_ACCESS_SAMPLE_H_

/* REG_1[0:15] */
#define REG_1_1_BIT_POSITION                     ( 0 )
#define REG_1_1_BIT_DEPTH                        ( 16 )

/* REG_1[16:31] */
#define REG_1_2_BIT_POSITION                     ( 16 )
#define REG_1_2_BIT_DEPTH                        ( 16 )

/* REG_2[0:23] */
#define REG_2_1_BIT_POSITION                     ( 0 )
#define REG_2_1_BIT_DEPTH                        ( 24 )

/* REG_1[24:31] */
#define REG_2_2_BIT_POSITION                     ( 24 )
#define REG_2_2_BIT_DEPTH                        ( 8 )


/** This structure is initialized to point to
 * memory mapped h/w
 */
typedef struct __attribute__((__packed__)) {
    uint32_t volatile REG_1;
    uint32_t volatile _UNUSED[4];
    uint32_t volatile REG_2;
} device_regs_t;


#endif
