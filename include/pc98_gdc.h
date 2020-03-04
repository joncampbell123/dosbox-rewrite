
enum {
    GDC_MASTER=0,
    GDC_SLAVE=1
};

struct PC98_GDC_state {
    PC98_GDC_state();
    void reset_fifo(void);
    void reset_rfifo(void);
    void flush_fifo_old(void);
    bool write_fifo(const uint16_t c);
    bool write_rfifo(const uint8_t c);
    bool write_fifo_command(const unsigned char c);
    bool write_fifo_param(const unsigned char c);
    bool rfifo_has_content(void);
    uint8_t read_status(void);
    uint8_t rfifo_read_data(void);
    void idle_proc(void);

    void force_fifo_complete(void);
    void take_cursor_char_setup(unsigned char bi);
    void take_cursor_pos(unsigned char bi);
    void take_reset_sync_parameters(void);
    void cursor_advance(void);

    void begin_frame(void);
    void next_line(void);

    void load_display_partition(void);
    void next_display_partition(void);

    size_t fifo_can_read(void);
    bool fifo_empty(void);
    Bit16u read_fifo(void);
};

