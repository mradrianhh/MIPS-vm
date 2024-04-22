#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vmemory_controller.h"
#include "vmemory/vmemorybus.h"
#include "device_table/device_table.h"

int vmemory_controller_init(vMEMORY_CONTROLLER_t *controller, REGISTER_t *MAR, REGISTER_t *MDR)
{
    controller->device_info = device_table_add(DEVICE_TYPE_MEMORY_CONTROLLER);

    controller->logger.device_info = controller->device_info;
    controller->logger.file_name = "../logs/vmemory_controller.txt";
    logger_init(&controller->logger);

    log_info(&controller->logger, "Initializing.\n");
    
    controller->MAR = MAR;
    controller->MDR = MDR;
    vmemorybus_init();

    return 0;
}

int vmemory_controller_shutdown(vMEMORY_CONTROLLER_t *controller)
{
    log_info(&controller->logger, "Shutting down.\n");
    logger_shutdown(&controller->logger);

    return 0;
}

int vmemory_controller_fetch_page(vMEMORY_CONTROLLER_t *controller)
{
    int rc = 0;

    vMEMORYBUS_PACKETS_t packets = {
        .data = 0,
        .access = *controller->MAR,
        .control.access_op = vMEMORYBUS_CONTROL_READ,
        .control.unit_used = vMEMORYBUS_CONTROL_USED,
    };

    rc = vmemorybus_write(vMEMORYBUS_SEL_IN, &packets);
    if (rc)
    {
        return 1;
    }
    log_trace(&controller->logger, "Package {Data: 0x%02x Access: 0x%02x Control: 0x%02x} written to vMEMORYBUS_IN.\n",
              packets.data, packets.access, packets.control.control_field);

    packets.control.unit_used = vMEMORYBUS_CONTROL_NOT_USED;
    while (!packets.control.unit_used)
    {
        vmemorybus_read(vMEMORYBUS_SEL_OUT, &packets);
    }
    log_trace(&controller->logger, "Package {Data: 0x%02x Access: 0x%02x Control: 0x%02x} read from vMEMORYBUS_OUT.\n",
              packets.data, packets.access, packets.control.control_field);

    *controller->MDR = packets.data;
    return 0;
}
