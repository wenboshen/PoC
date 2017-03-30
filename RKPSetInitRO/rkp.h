#ifndef __RKP_H__
#define __RKP_H__

//RKP definitions
#define __AC(X,Y) (X##Y)
#define _AC(X,Y) __AC(X,Y)
#define _AT(T,X) ((T)(X))
#define UL(x) _AC(x, UL)
#define RKP_PREFIX  UL(0x83800000)
#define RKP_CMDID(CMD_ID)  ((UL(CMD_ID) << 12 ) | RKP_PREFIX)
#define KASLR_MEM_RESERVE  RKP_CMDID(0x70)
#define RKP_KDP_INIT RKP_CMDID(0x40)
#define RKP_PGD_SET  RKP_CMDID(0x21)
#define RKP_PMD_SET  RKP_CMDID(0x22)
#define RKP_PTE_SET  RKP_CMDID(0x23)
#define RKP_PGD_FREE RKP_CMDID(0x24)
#define RKP_PGD_NEW  RKP_CMDID(0x25)
#define RKP_SET_TTBR0 RKP_CMDID(0x10)
#define CFP_ROPP_NEW_KEY  RKP_CMDID(0x91)
#define CFP_ROPP_NEW_KEY_REENC  RKP_CMDID(0x92)
#define CFP_ROPP_KEY_DEC  RKP_CMDID(0x93)
#define CFP_ROPP_RET_KEY  RKP_CMDID(0x94)
#define CFP_TEST    RKP_CMDID(0x99)

#endif
