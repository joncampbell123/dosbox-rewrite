
// utility struct to store bit position, width, and mask of pixels.
// T is either uint16_t or uint32_t. This is ONLY for use with RGB
// pixel formats.
template <class T> struct rgbchannelinfo {
    uint8_t         shift;              // raw value = pixel << shift
    uint8_t         bwidth;             // width of the pixel value, in bits
    T               bmask;              // pmask = (1U << bwidth) - 1
    T               mask;               // mask = pmask << shift
public:
    void setByMask(const T m) { // initialize this struct by mask (i.e. rgb mask provided by X windows)
        shift = bitscan_forward(m,0);
        bwidth = bitscan_count(m,shift) - shift;
        bmask = (uint32_t(1U) << uint32_t(bwidth)) - uint32_t(1U);
        mask = bmask << shift;
    }
};

template <class T> struct rgbinfo {
    struct rgbchannelinfo<T>    r,g,b,a;
};

