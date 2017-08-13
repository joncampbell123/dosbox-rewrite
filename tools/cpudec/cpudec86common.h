
extern x86_offset_t            exe_ip;
extern unsigned char*          exe_ip_ptr;
extern unsigned char*          exe_image;
extern unsigned char*          exe_image_fence;
extern bool                    exe_code32;

// include header core requires this
static inline bool IPcontinue(void) {
    return (exe_ip_ptr < exe_image_fence);
}

// include header core requires this
static inline x86_offset_t IPval(void) {
    return exe_ip;
}

// include header core requires this
static inline uint8_t IPFB(void) {
    const uint8_t r = *((const uint8_t*)exe_ip_ptr);
    exe_ip_ptr += 1;
    exe_ip += 1;
    return r;
}

// include header core requires this
static inline uint16_t IPFW(void) {
    const uint16_t r = le16toh(*((const uint16_t*)exe_ip_ptr));
    exe_ip_ptr += 2;
    exe_ip += 2;
    return r;
}

// include header core requires this
static inline uint32_t IPFDW(void) {
    const uint32_t r = le32toh(*((const uint32_t*)exe_ip_ptr));
    exe_ip_ptr += 4;
    exe_ip += 4;
    return r;
}

