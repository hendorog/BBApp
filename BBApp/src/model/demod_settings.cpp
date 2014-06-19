#include "demod_settings.h"

struct iq_auto_bandwidth_entry {
    Frequency bandwidth;
    int decimationRate;
};

static iq_auto_bandwidth_entry iq_auto_bandwidth_lut[] = {
    { 100.0e3, 7 },
    { 200.0e3, 6 },
    { 500.0e3, 5 },
    { 1.0e6,   4 },
    { 2.0e6,   3 },
    { 5.0e6,   2 },
    { 10.0e6,  1 },
    { 27.0e6,  0 }
};
static int iq_auto_bandwidth_lut_sz =
        sizeof(iq_auto_bandwidth_lut) /
        sizeof(iq_auto_bandwidth_entry);

void get_next_iq_bandwidth(Frequency current,
                           Frequency &next,
                           int &decimationRate)
{
    int lutIx = 0;
    while(current < iq_auto_bandwidth_lut[lutIx].bandwidth &&
          lutIx < iq_auto_bandwidth_lut_sz)
        lutIx++;


}

DemodSettings::DemodSettings()
{

}

DemodSettings::~DemodSettings()
{

}
