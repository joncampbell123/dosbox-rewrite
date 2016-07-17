
struct HostCPUCaps {
/*---------------- x86/x86_64 ---------------------*/
#if defined(__i386__) || defined(__x86_64__)
    bool                        mmx;
    bool                        sse;
    bool                        sse2;
    bool                        sse3;
    bool                        ssse3;
    bool                        avx;
    bool                        avx2;

    HostCPUCaps() : mmx(0), sse(0), sse2(0), sse3(0), ssse3(0), avx(0), avx2(0) { }
#else
/*---------------- unknown ------------------------*/
    bool                        _dummy;

    HostCPUCaps() : _dummy(0) { }
#endif
/*-------------------------------------------------*/
    void                        detect(void);
};

extern struct HostCPUCaps hostCPUcaps;

