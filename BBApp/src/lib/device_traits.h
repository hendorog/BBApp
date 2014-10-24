#ifndef DEVICE_TRAITS_H
#define DEVICE_TRAITS_H

enum DeviceType {
    DeviceTypeSA44,
    DeviceTypeSA124,
    DeviceTypeBB60A,
    DeviceTypeBB60C
};

// BB60C values by default ?
class device_traits {
public:
    static void set_device_type(DeviceType new_type);

    static double min_span();
    static double min_frequency();
    static double max_frequency();
    static double min_real_time_rbw();
    static double max_real_time_rbw();
    static double min_real_time_span();
    static double max_real_time_span();
    static int max_atten();
    static int max_gain();
    static double sample_rate();

private:
    static DeviceType type;
};

#endif // DEVICE_TRAITS_H
