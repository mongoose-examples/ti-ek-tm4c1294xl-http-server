// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#include <string.h>

#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/GPIO.h>

#include "Board.h"
#include "mongoose.h"

static const char *s_listening_address = "http://0.0.0.0:80";

void device_dashboard_fn(struct mg_connection *, int, void *, void *);

Void mgTask(UArg arg0, UArg arg1)
{
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    MG_INFO(("Mongoose version : v%s", MG_VERSION));
    MG_INFO(("Listening on     : %s", s_listening_address));

    if (mg_http_listen(&mgr, s_listening_address, device_dashboard_fn, &mgr) == NULL) {
      MG_ERROR(("Cannot listen on %s", s_listening_address));
      return;
    }

    while (1) mg_mgr_poll(&mgr, 1000);
}

void netOpenHook()
{
    Task_Handle taskHandle;
    Task_Params taskParams;
    Error_Block eb;

    Error_init(&eb);

    Task_Params_init(&taskParams);
    taskParams.stackSize = 4096;
    taskParams.priority = 1;
    taskHandle = Task_create((Task_FuncPtr)mgTask, &taskParams, &eb);
    if (taskHandle == NULL) {
        System_printf("netOpenHook: Failed to create tcpHandler Task\n");
    }

    System_flush();
}

int main(void)
{
    Board_initGeneral();
    Board_initGPIO();
    Board_initEMAC();

    System_flush();

    BIOS_start();

    return 0;
}
