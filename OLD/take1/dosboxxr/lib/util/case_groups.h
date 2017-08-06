
#define case_span_2(x) \
    case (x)+0: case (x)+1
#define case_span_4(x) \
    case_span_2(x): case_span_2((x)+2)
#define case_span_8(x) \
    case_span_4(x): case_span_4((x)+4)
#define case_span_16(x) \
    case_span_8(x): case_span_8((x)+8)

// this macro covers one whole 8-value block where in MOD/REG/RM MOD == mod and REG == reg for any value of R/M
#define case_span_by_mod_reg(mod,reg) \
        case_span_8(((mod) << 6) + ((reg) << 3))

