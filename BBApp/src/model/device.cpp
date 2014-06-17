#ifndef DEVICE_CPP
#define DEVICE_CPP

#include "device.h"

const char* device_type_to_string(int type)
{
    if(type == BB_DEVICE_NONE) {
        return "";
    } else if(type == BB_DEVICE_BB60A) {
        return "BB60A";
    } else if(type == BB_DEVICE_BB60C) {
        return "BB60C";
    } else {
        return "";
    }
}

void Device::UpdateDiagnostics()
{
    float temp_now, voltage_now, current_now;
    bbGetDeviceDiagnostics(id, &temp_now, &voltage_now, &current_now);

    if((temp_now != current_temp) || (voltage_now != voltage) || (current_now != current)) {
        update_diagnostics_string = true;
    }

    current_temp = temp_now;
    voltage = voltage_now;
    current = current_now;
}

#endif // DEVICE_CPP
