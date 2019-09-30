#include "proc.h"
#include "mm.h"
#include "exe.h"
#include "sysconst.h"
#include "global.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

struct message exe_msg;

void TaskEXE()
{
    printf("\n---------{TaskEXE} Page Dir Base: 0x%.8x---------", getcr3());

    for (;;) {
        sendrecv(RECEIVE, ANY, &exe_msg);

        switch (exe_msg.value) {
        case FORK:
            exe_msg.RETVAL = do_fork();
            break;
        default:
            printf("\nEXE-{unknown message value}");
            dump_msg(&exe_msg);
        }

        sendrecv(SEND, exe_msg.source, &exe_msg);
    }
}
