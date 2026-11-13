.file	"palproc.s"

#include "palproc.h"

PROCEDURE_ENTRY(MakeStaticPALCall)

        NESTED_SETUP (5,8,0,0)
        mov         loc3 = b5
        mov         loc4 = r2
        mov         loc7 = r1;;

        movl        loc6 = PAL_MC_CLEAR_LOG
        mov         r2 = psr;;
        mov         loc5 = r2

        cmp.eq      p6,p7 = r28,loc6;;
    (p7)movl        loc6 = PAL_MC_DYNAMIC_STATE;;
    (p7)cmp.eq      p6,p7 = r28,loc6;;

    (p7)movl        loc6 = PAL_MC_ERROR_INFO;;
    (p7)cmp.eq      p6,p7 = r28,loc6;;

    (p7)movl        loc6 = PAL_MC_RESUME;;
    (p7)cmp.eq      p6,p7 = r28,loc6

        mov         loc6 = 0x1;;
    (p7)dep         r2 = loc6,r2,13,1;;

        dep         r2 = r0,r2,14,1;;

        mov         psr.l = r2
        srlz.d;;
        srlz.i;;

StaticGetPALLocalIP:
        mov         loc2 = ip;;
        add         loc2 = StaticComeBackFromPALCall - StaticGetPALLocalIP,loc2;;
        mov         b0 = loc2
        mov         r28 = in1
        mov         r29 = in2
        mov         r30 = in3;;
        mov         r31 = in4
        mov         b5 =  in0;;
        br.sptk     b5

StaticComeBackFromPALCall:

        mov         psr.l = loc5;;
        srlz.d;;
        srlz.i;;

        mov         b5 = loc3
        mov         r2 = loc4
        mov         r1 = loc7

        NESTED_RETURN

PROCEDURE_EXIT(MakeStaticPALCall)

PROCEDURE_ENTRY(MakeStackedPALCall)

        NESTED_SETUP (5,8,4,0)
        mov         loc3 = b5
        mov         loc4 = r2
        mov         loc7 = r1
        mov         r2 = psr;;
        mov         loc5 = r2;;
        dep         r2 = r0,r2,14,1;;
        mov         psr.l = r2
        srlz.d;;
        srlz.i;;

StackedGetPALLocalIP:
        mov         r28 = in1
        mov         out0 = in1
        mov         out1 = in2;;
        mov         out2 = in3
        mov         out3 = in4
        mov         b5 =  in0;;
        br.call.dpnt b0=b5;;

StackedComeBackFromPALCall:

        mov         psr.l = loc5;;
        srlz.d;;
        srlz.i;;
        mov         b5 = loc3
        mov         r2 = loc4
        mov         r1 = loc7

        NESTED_RETURN

PROCEDURE_EXIT(MakeStackedPALCall)

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif
