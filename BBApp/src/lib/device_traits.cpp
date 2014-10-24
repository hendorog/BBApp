#include "device_traits.h"

#include "sa_api.h"
#include "bb_api.h"

DeviceType device_traits::type = DeviceTypeBB60C;

void device_traits::set_device_type(DeviceType new_type)
{
    type = new_type;
}

double device_traits::min_span()
{
    switch(type) {
    case DeviceTypeSA44: case DeviceTypeSA124: return 10.0;
    case DeviceTypeBB60A: case DeviceTypeBB60C: return BB_MIN_SPAN;
    }
    return 100.0;
}

double device_traits::min_frequency()
{
    switch(type) {
    case DeviceTypeSA44: case DeviceTypeSA124: return SA_MIN_FREQ;
    case DeviceTypeBB60A: case DeviceTypeBB60C: return BB60_MIN_FREQ;
    }
    return BB60_MIN_FREQ;
}

double device_traits::max_frequency()
{
    switch(type) {
    case DeviceTypeSA44: return SA_MAX_FREQ;
    case DeviceTypeSA124: return 12.4e9;
    case DeviceTypeBB60A: case DeviceTypeBB60C: return BB60_MAX_FREQ;
    }
    return BB60_MAX_FREQ;
}

double device_traits::min_real_time_rbw()
{
    switch(type) {
    case DeviceTypeSA44: case DeviceTypeSA124: return 1.0e3;
    case DeviceTypeBB60A: case DeviceTypeBB60C: return BB_MIN_RT_RBW;
    }
    return BB_MIN_RT_RBW;
}

double device_traits::max_real_time_rbw()
{
    switch(type) {
    case DeviceTypeSA44: case DeviceTypeSA124: return 100.0e3;
    case DeviceTypeBB60A: case DeviceTypeBB60C: return BB_MAX_RT_RBW;
    }
    return BB_MAX_RT_RBW;
}

double device_traits::min_real_time_span()
{
    switch(type) {
    case DeviceTypeSA44: case DeviceTypeSA124: return 100.0e3;
    case DeviceTypeBB60A: case DeviceTypeBB60C: return BB_MIN_RT_SPAN;
    }
    return BB_MIN_RT_SPAN;
}

double device_traits::max_real_time_span()
{
    switch(type) {
    case DeviceTypeSA44: case DeviceTypeSA124: return 250.0e3;
    case DeviceTypeBB60A: return BB60A_MAX_RT_SPAN;
    case DeviceTypeBB60C: return BB60C_MAX_RT_SPAN;
    }
    return BB_MIN_RT_SPAN;
}

int device_traits::max_atten()
{
    switch(type) {
    case DeviceTypeSA44: case DeviceTypeSA124: return 2;
    case DeviceTypeBB60A: case DeviceTypeBB60C: return 3;
    }
    return 3;
}

int device_traits::max_gain()
{
    switch(type) {
    case DeviceTypeSA44: case DeviceTypeSA124: return 2;
    case DeviceTypeBB60A: case DeviceTypeBB60C: return 3;
    }
    return 3;
}

double device_traits::sample_rate()
{
    switch(type) {
    case DeviceTypeSA44: case DeviceTypeSA124: return 486.111e3;
    case DeviceTypeBB60A: case DeviceTypeBB60C: return 40.0e6;
    }
    return BB_SAMPLERATE;
}
