
struct stretchblt_mode {
    const char*         name;
    void                (*render)(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);
    bool                (*can_do)(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);
};

extern struct stretchblt_mode stretchblt_modes[];

size_t stretchblt_mode_count();

