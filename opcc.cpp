#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>

using namespace std;

#include <algorithm>        // std::min
#include <string>
#include <vector>
#include <map>

enum opccMode {
    MOD_DECOMPILE=0,
    MOD_EXECUTE
};

enum opccSeg {
    OPSEG_NONE=-1,
    OPSEG_CS=0,
    OPSEG_DS,
    OPSEG_ES,
    OPSEG_FS,
    OPSEG_GS,
    OPSEG_SS
};

enum opccReg {
    OPREG_NONE=-1,

    // 8-bit
    OPREG_AL=0,
    OPREG_CL,
    OPREG_DL,
    OPREG_BL,
    OPREG_AH,
    OPREG_CH,
    OPREG_DH,
    OPREG_BH,

    // 16-bit
    OPREG_AX=0,
    OPREG_CX,
    OPREG_DX,
    OPREG_BX,
    OPREG_SP,
    OPREG_BP,
    OPREG_SI,
    OPREG_DI,

    // 32-bit
    OPREG_EAX=0,
    OPREG_ECX,
    OPREG_EDX,
    OPREG_EBX,
    OPREG_ESP,
    OPREG_EBP,
    OPREG_ESI,
    OPREG_EDI
};

enum opccSuffix {
    OPSUFFIX_NONE=0,
    OPSUFFIX_BYTE,
    OPSUFFIX_WORD,
    OPSUFFIX_WORD16,
    OPSUFFIX_WORD32
};

enum opccArgs {
    OPARG_NONE=0,
    OPARG_IB,           // imm8
    OPARG_IBS,          // imm8 sign extended
    OPARG_IW,           // imm<word>
    OPARG_IWS,          // imm<word> sign extended
    OPARG_IW16,         // imm16
    OPARG_IW16S,        // imm16 sign extended
    OPARG_IW32,         // imm32
    OPARG_IW32S         // imm32 sign extended
};

enum opccDispArg {
    OPDARG_NONE=0,
    OPDARG_rm,
    OPDARG_reg,
    OPDARG_imm
};

enum opccDispArgType {
    OPDARGTYPE_NONE=0,
    OPDARGTYPE_BYTE,        // 8-bit byte
    OPDARGTYPE_WORD,        // 16-bit (if 16-bit code),  32-bit (if 32-bit code)
    OPDARGTYPE_WORD16,      // 16-bit
    OPDARGTYPE_WORD32,      // 32-bit
    OPDARGTYPE_FPU,         // FPU st(i)
    OPDARGTYPE_FPU32,       // 32-bit float
    OPDARGTYPE_FPU64,       // 64-bit float
    OPDARGTYPE_FPU80,       // 80-bit float
    OPDARGTYPE_FPUBCD,      // floating point packed BCD
    OPDARGTYPE_SREG         // segment register (word size)
};

class OpCodeDisplayArg {
public:
    OpCodeDisplayArg() : arg(OPDARG_NONE), argtype(OPDARGTYPE_NONE), argreg(OPREG_NONE), suffix(OPSUFFIX_NONE), index(0), imm(0) { }
public:
    enum opccDispArg        arg;
    enum opccDispArgType    argtype;
    enum opccReg            argreg;
    enum opccSuffix         suffix;
    uint8_t                 index;
    uint64_t                imm;
public:
    inline bool operator==(const OpCodeDisplayArg &r) const {
        if (arg != r.arg) return false;
        if (argtype != r.argtype) return false;
        if (argreg != r.argreg) return false;
        if (suffix != r.suffix) return false;
        if (index != r.index) return false;
        if (imm != r.imm) return false;
        return true;
    }
};

const char *suffix_str(enum opccSuffix c,const unsigned int opsize) {
    switch (c) {
        case OPSUFFIX_BYTE:
            return "b";
        case OPSUFFIX_WORD:
            if (opsize >= 32) return "d";
            else return "w";
        case OPSUFFIX_WORD16:
            return "w";
        case OPSUFFIX_WORD32:
            return "d";
    }

    return "";
}

FILE *in_fp = NULL;
string in_path;

FILE *out_fp = NULL;
string out_path;

enum opccMode cc_mode = MOD_DECOMPILE;

static void help() {
    fprintf(stderr,"opcc [options]\n");
    fprintf(stderr,"Take a DOSBox-XR opcode sheet and generate C++ code to handle it.\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"   -i <input>          Input file path or - for stdin\n");
    fprintf(stderr,"   -o <output>         Output file path or - for stdout\n");
    fprintf(stderr,"   -m <mode>           Generation mode:\n");
    fprintf(stderr,"        decompile        Generate code to decompile/disassemble\n");
    fprintf(stderr,"        execute          Generate code to execute instructions\n");
}

static bool parse(int argc,char **argv) {
    char *a;
    int i;

    for (i=1;i < argc;) {
        a = argv[i++];

        if (*a == '-') {
            do { a++; } while (*a == '-');

            if (!strcmp(a,"h") || !strcmp(a,"help")) {
                help();
                return false;
            }
            else if (!strcmp(a,"i")) {
                a = argv[i++];
                if (a == NULL) return false;
                in_path = a;
            }
            else if (!strcmp(a,"o")) {
                a = argv[i++];
                if (a == NULL) return false;
                out_path = a;
            }
            else if (!strcmp(a,"m")) {
                a = argv[i++];

                if (!strcmp(a,"decompile"))
                    cc_mode = MOD_DECOMPILE;
                else if (!strcmp(a,"execute"))
                    cc_mode = MOD_EXECUTE;
                else {
                    fprintf(stderr,"Unknown mode\n");
                    return false;
                }
            }
            else {
                fprintf(stderr,"Unexpected switch %s\n",a);
                return false;
            }
        }
        else {
            fprintf(stderr,"Unexpected arg %s\n",a);
            return false;
        }
    }

    if (in_path.empty() || out_path.empty()) {
        fprintf(stderr,"Must specify input and output file. See -h option for help\n");
        return false;
    }

    return true;
}

class OpByte {
public:
    OpByte() : isprefix(false), segoverride(OPSEG_NONE), modregrm(false), reg_from_opcode(false),
        already(false), suffix(OPSUFFIX_NONE), opmap_valid(false) {
    }
    ~OpByte() {
        free_opmap();
    }
    void free_opmap(void) {
        for (size_t i=0;i < 256;i++) free_opmap_code(i);
    }
    void free_opmap_code(size_t i) {
        if (opmap[i] != NULL) {
            delete opmap[i];
            opmap[i] = NULL;
        }
    }
    void init_opmap(void) {
        if (!opmap_valid) {
            opmap_valid = true;
            for (size_t i=0;i < 256;i++) opmap[i] = NULL;
        }
    }
    bool init_opcode(const uint8_t op) {
        if (!opmap_valid)
            init_opmap();

        if (opmap[op] == NULL) {
            opmap[op] = new OpByte();
            return true;
        }

        return false;
    }
    bool exist_opcode(const uint8_t op) {
        if (!opmap_valid) return false;
        return (opmap[op] != NULL)?true:false;
    }
public:
    inline bool operator==(const OpByte &r) const {
        if (format_str != r.format_str) return false;
        if (name != r.name) return false;
        if (segoverride != r.segoverride) return false;
        if (isprefix != r.isprefix) return false;
        if (modregrm != r.modregrm) return false;
        if (reg_from_opcode != r.reg_from_opcode) return false;
        if (suffix != r.suffix) return false;
        if (immarg != r.immarg) return false;
        if (disparg != r.disparg) return false;
        if (opmap_valid != r.opmap_valid) return false;

        if (opmap_valid) {
            assert(r.opmap_valid);
            for (unsigned int i=0;i < 256;i++) {
                OpByte *a = opmap[i];
                OpByte *b = r.opmap[i];

                if ((a != NULL) != (b != NULL))
                    return false;

                if (a != NULL) {
                    assert(b != NULL);
                    if (!(*a == *b))
                        return false;
                }
            }
        }

        return true;
    }
public:
    // opbyte [mod/reg/rm [sib] [disp]] [immediate]
    string          format_str;
    string          name;       // JMP, etc.
    opccSeg         segoverride;// segment override
    bool            isprefix;   // opcode is prefix
    bool            modregrm;   // a MOD/REG/RM byte follows the opcode.
                                // if more == false, this byte is final and mod/reg/rm decode the instruction as normal.
                                // if more == true, this byte is NOT final and the next byte is the mod/reg/rm byte where REG determines the opcode.
                                // In some cases (FPU 8087 opcodes) mod/reg/rm both determines the memory address or FPU register and in other cases
                                // the mod/reg/rm byte IS the second byte of the opcode (when mod == 3).
    bool            reg_from_opcode; // reg from opcode, reg = (opcode >> 3) & 7
    bool            already;
    enum opccSuffix suffix;     // suffix to name
    vector<enum opccArgs> immarg;
    vector<OpCodeDisplayArg> disparg;
public:
    OpByte*         opmap[256];
    bool            opmap_valid;
};

OpByte      opmap16,opmap32;

// C/C++ implementation of Perl's chomp function.
// static copy to differentiate from target system vs build system.
static void opcc_chomp(char *s) {
    char *t = s + strlen(s) - 1;
    while (t >= s && (*t == '\n' || *t == '\r')) *t-- = 0;
}

class parse_opcode_state {
public:
    parse_opcode_state() {
        oprange_min = -1;
        oprange_max = -1; // last byte of opcode takes the range (min,max) inclusive
        mrm_reg_match = -1; // /X style, to say that opcode is specified by REG field of mod/reg/rm
        reg_from_opcode = false; // if set, lowest 3 bits of opcode define REG field
        segoverride = OPSEG_NONE;
        suffix = OPSUFFIX_NONE;
        is_prefix = false; // opcode is prefix, changes decode state then starts another opcode
        modregrm = false;
        op = 0;
    }
public:
    bool                    reg_from_opcode; // if set, lowest 3 bits of opcode define REG field
    bool                    is_prefix; // opcode is prefix, changes decode state then starts another opcode
    bool                    modregrm;
    int                     oprange_min;
    int                     oprange_max; // last byte of opcode takes the range (min,max) inclusive
    int                     mrm_reg_match; // /X style, to say that opcode is specified by REG field of mod/reg/rm
    enum opccSeg            segoverride;
    enum opccSuffix         suffix;     // suffix to name
public:
    string                  format_str;
    string                  name;
    vector<enum opccArgs>   immarg;
    vector<OpCodeDisplayArg> disparg;
    unsigned char           ops[16];
    uint8_t                 op;
};

bool parse_opcode_def_gen1(parse_opcode_state &st,OpByte& maproot) {
    OpByte *map = &maproot,*submap;

    for (unsigned int i=0;i < st.op;i++) {
        const uint8_t opcode = st.ops[i];
        map->init_opcode(opcode);
        map = map->opmap[opcode];
        assert(map != NULL);
    }

    if (st.modregrm) {
        map->modregrm = st.modregrm;
        if (st.mrm_reg_match >= 0) {
            for (unsigned int mod=0;mod < 4;mod++) {//TODO: mod value conditionals
                for (unsigned int rm=0;rm < 8;rm++) {
                    const uint8_t mrm = (mod << 6) + (st.mrm_reg_match << 3) + rm;

                    map->init_opcode(mrm);
                    submap = map->opmap[mrm];
                    assert(submap != NULL);

                    submap->modregrm = false;
                    submap->disparg = st.disparg;
                    submap->format_str = st.format_str;
                    submap->segoverride = st.segoverride;
                    submap->isprefix = st.is_prefix;
                    submap->immarg = st.immarg;
                    submap->suffix = st.suffix;
                    submap->name = st.name;
                }
            }
        }
        else {
            map->segoverride = st.segoverride;
            map->format_str = st.format_str;
            map->isprefix = st.is_prefix;
            map->disparg = st.disparg;
            map->immarg = st.immarg;
            map->suffix = st.suffix;
            map->name = st.name;
        }
    }
    else {
        map->reg_from_opcode = st.reg_from_opcode;
        map->segoverride = st.segoverride;
        map->format_str = st.format_str;
        map->isprefix = st.is_prefix;
        map->disparg = st.disparg;
        map->immarg = st.immarg;
        map->suffix = st.suffix;
        map->name = st.name;
    }

    return true;
}

bool parse_opcode_def(char *line,unsigned long lineno,char *s) {
    parse_opcode_state st;
    bool allow_modregrm = true;
    bool allow_op = true; // once set to false, can't define more opcode bytes
    char *next = NULL;
    int argc = 0;

    next = strchr(s,':');
    if (next != NULL) *next++ = 0;

    while (*s != 0) {
        if (isblank(*s)) {
            s++;
            continue;
        }

        // ASCII chop next space
        char *ns = strchr(s,' ');
        if (ns != NULL) *ns++ = 0;

        if (!strncmp(s,"0x",2)) { // opcode byte
            if (!allow_op) {
                fprintf(stderr,"Opcode byte not allowed at this point\n");
                return false;
            }

            /* use long parsing to catch overlarge values */
            unsigned long b = strtoul(s,&s,0);

            if (b > 0xFFUL) {
                fprintf(stderr,"Opcode byte out of range\n");
                return false;
            }
            else if (*s != 0 && *s != ':' && !isblank(*s)) {
                fprintf(stderr,"Excess char follows opcode byte: %s\n",s);
                return false;
            }
            else if ((st.op+4) >= sizeof(st.ops)) {
                fprintf(stderr,"Opcode byte string too long\n");
                return false;
            }

            st.ops[st.op++] = (unsigned char)b;
        }
        else if (!strcmp(s,"mod/reg/rm")) {
            if (st.modregrm) {
                fprintf(stderr,"Cannot define mod/reg/rm field twice\n");
                return false;
            }
            else if (!allow_modregrm) {
                fprintf(stderr,"Cannot define mod/reg/rm field here\n");
                return false;
            }

            allow_op = false;
            st.modregrm = true;
        }
        else if (s[0] == '/' && isdigit(s[1])) { // /1, /3, etc. to match by REG value
            if (!st.modregrm) {
                fprintf(stderr,"Cannot use REG matching (/1 /3 etc) without mod/reg/rm\n");
                return false;
            }
            else if (st.mrm_reg_match >= 0) {
                fprintf(stderr,"Cannot match more than one REG value\n");
                return false;
            }

            /* use long parsing to catch overlarge values */
            unsigned long b = strtoul(s+1,&s,0);

            if (b > 7) {
                fprintf(stderr,"/REG match out of range\n");
                return false;
            }

            st.mrm_reg_match = b;
        }
        else if (!strcmp(s,"ib")) {
            st.immarg.push_back(OPARG_IB);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"ibs")) {
            st.immarg.push_back(OPARG_IBS);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"iw")) {
            st.immarg.push_back(OPARG_IW);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"iws")) {
            st.immarg.push_back(OPARG_IWS);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"iw16")) {
            st.immarg.push_back(OPARG_IW16);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"iw16s")) {
            st.immarg.push_back(OPARG_IW16S);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"iw32")) {
            st.immarg.push_back(OPARG_IW32);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"iw32s")) {
            st.immarg.push_back(OPARG_IW32S);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"prefix")) {
            st.is_prefix = true;
        }
        else if (!strncmp(s,"segoverride(",12)) {
            s += 12;
            while (isblank(*s)) s++;

            if (st.segoverride != OPSEG_NONE) {
                fprintf(stderr,"You already declared a segment override\n");
                return false;
            }
            else if (!st.is_prefix) {
                fprintf(stderr,"Segment override must be declared a prefix\n");
                return false;
            }

            if (!strncmp(s,"cs",2))
                st.segoverride = OPSEG_CS;
            else if (!strncmp(s,"ds",2))
                st.segoverride = OPSEG_DS;
            else if (!strncmp(s,"es",2))
                st.segoverride = OPSEG_ES;
            else if (!strncmp(s,"fs",2))
                st.segoverride = OPSEG_FS;
            else if (!strncmp(s,"gs",2))
                st.segoverride = OPSEG_GS;
            else if (!strncmp(s,"ss",2))
                st.segoverride = OPSEG_SS;
            else {
                fprintf(stderr,"Unknown segment override\n");
                return false;
            }
        }
        else if (!strncmp(s,"range(",6)) {
            if (st.oprange_min >= 0 || st.oprange_max >= 0) {
                fprintf(stderr,"Cannot define more than one opcode range\n");
                return false;
            }
            else if (!allow_op) {
                fprintf(stderr,"Opcode range not allowed here\n");
                return false;
            }

            s += 6;
            while (isblank(*s)) s++;
            st.oprange_min = strtol(s,&s,0);
            while (isblank(*s)) s++;
            if (*s != ',') {
                fprintf(stderr,"Syntax error in range()\n");
                return false;
            }
            s++;
            while (isblank(*s)) s++;
            st.oprange_max = strtol(s,&s,0);
            while (isblank(*s)) s++;
            if (*s != ')') {
                fprintf(stderr,"Syntax error in range(), no closing parenthesis\n");
                return false;
            }

            if (st.oprange_min < 0 || st.oprange_min > 0xFF || st.oprange_max < 0 || st.oprange_max > 0xFF || st.oprange_min > st.oprange_max) {
                fprintf(stderr,"Opcode range error. min=%d max=%d\n",st.oprange_min,st.oprange_max);
                return false;
            }

            allow_op = false;
        }
        else if (!strcmp(s,"reg=op02")) {
            if (st.modregrm) {
                fprintf(stderr,"Cannot declare reg = opcode[0:2] when reg already comes from mod/reg/rm\n");
                return false;
            }

            st.reg_from_opcode = true;
        }
        else {
            fprintf(stderr,"Syntax error: %s\n",s);
            return false;
        }

        if (ns != NULL) s = ns;
        else break;
    }

    if (st.op == 0 && st.oprange_min < 0) {
        fprintf(stderr,"Opcode bytes not defined\n");
        return false;
    }

    if (next == NULL) {
        fprintf(stderr,"Opcode does not define anything\n");
        return false;
    }
    while (isblank(*next)) next++;

    argc = 0;
    s = next;
    while (*s != 0) {
        if (isblank(*s)) {
            s++;
            continue;
        }

        // ASCII chop next space
        char *ns = strchr(s,' ');
        if (ns != NULL) *ns++ = 0;

        if (argc == 0) { // "NAME"b
            if (!st.name.empty()) {
                fprintf(stderr,"Syntax error: %s\n",s);
                return false;
            }

            if (*s != '\"') {
                fprintf(stderr,"Name must be kept in quotes\n");
                return false;
            }
            s++;
            while (*s != 0 && *s != '\"') st.name += *s++;
            if (*s != '\"') {
                fprintf(stderr,"Name must be kept in quotes\n");
                return false;
            }
            s++;

            if (!strcmp(s,"b"))
                st.suffix = OPSUFFIX_BYTE;
            else if (!strcmp(s,"w"))
                st.suffix = OPSUFFIX_WORD;
            else if (!strcmp(s,"w16"))
                st.suffix = OPSUFFIX_WORD16;
            else if (!strcmp(s,"w32"))
                st.suffix = OPSUFFIX_WORD32;
            else if (*s == 0)
                st.suffix = OPSUFFIX_NONE;
            else {
                fprintf(stderr,"Name has unknown suffix\n");
                return false;
            }
        }
        else if (argc == 1) { /* b(r/m),b(reg) */
            st.format_str = s;
            // TODO
        }
        else {
            fprintf(stderr,"Unexpected arg\n");
            return false;
        }

        if (ns != NULL) s = ns;
        else break;

        argc++;
    }

    if (st.oprange_min < 0) {
        if (true/*16-bit*/ && !parse_opcode_def_gen1(/*&*/st,/*&*/opmap16))
            return false;
        if (true/*32-bit*/ && !parse_opcode_def_gen1(/*&*/st,/*&*/opmap32))
            return false;
    }
    else {
        for (unsigned int i=st.oprange_min;i <= st.oprange_max;i++) {
            assert((st.op+1) < sizeof(st.ops));
            st.ops[st.op++] = i;

            if (true/*16-bit*/ && !parse_opcode_def_gen1(/*&*/st,/*&*/opmap16))
                return false;
            if (true/*32-bit*/ && !parse_opcode_def_gen1(/*&*/st,/*&*/opmap32))
                return false;

            st.op--;
        }
    }

    return true;
}

bool parse_opcodelist(void) {
    char line[1024],*s;
    char parse_line[1024];
    unsigned long lineno = 0;

    memset(line,0,sizeof(line));
    while (!feof(in_fp)) {
        if (ferror(in_fp)) {
            fprintf(stderr,"File I/O error on stdin\n");
            return false;
        }

        lineno++;
        if (fgets(line,sizeof(line)-1,in_fp) == NULL)
            break;

        opcc_chomp(line);
        s = line;
        while (isblank(*s)) s++;

        /* eat comment field */
        {
            char *semi = strchr(s,';');
            if (semi != NULL) *semi = 0;
        }

        /* skip blank lines */
        if (*s == 0) continue;

        strcpy(parse_line,line);
        if (!strncmp(s,"opcode ",7)) {
            s += 7;
            while (isblank(*s)) s++;

            if (!parse_opcode_def(line,lineno,s)) {
                fprintf(stderr," (%u): %s\n",lineno,parse_line);
                return false;
            }
        }
        else {
            fprintf(stderr,"Unknown input:\n");
            fprintf(stderr," (%u): %s\n",lineno,parse_line);
            return false;
        }
    }

    return true;
}

void outcode_gen(const unsigned int codewidth,const unsigned int addrwidth,const uint8_t *opbase,const size_t opbaselen,OpByte &map,const unsigned int indent=0);

void opcode_gen_case_statement(const unsigned int codewidth,const unsigned int addrwidth,const uint8_t *opbase,const size_t opbaselen,OpByte &map,const unsigned int indent,const string &indent_str,OpByte *submap,const uint8_t op) {
    unsigned int immc = 0;

    if (submap->modregrm)
        fprintf(out_fp,"%s        IPFB_mrm_sib_disp_a%u_read(mrm,sib,disp);\n",indent_str.c_str(),addrwidth);

    /* then fetch other args */
    for (size_t i=0;i < submap->immarg.size();i++) {
        enum opccArgs a = submap->immarg[i];

        switch (a) {
            case OPARG_IB:
                if (immc != 0)  fprintf(out_fp,"%s        imm%u=IPFB();\n",indent_str.c_str(),immc+1);
                else            fprintf(out_fp,"%s        imm=IPFB();\n",indent_str.c_str());
                immc++;
                break;
            case OPARG_IBS:
                if (immc != 0)  fprintf(out_fp,"%s        imm%u=IPFBsigned();\n",indent_str.c_str(),immc+1);
                else            fprintf(out_fp,"%s        imm=IPFBsigned();\n",indent_str.c_str());
                immc++;
                break;
            case OPARG_IW:
                if (immc != 0)  fprintf(out_fp,"%s        imm%u=IPFcodeW();\n",indent_str.c_str(),immc+1);
                else            fprintf(out_fp,"%s        imm=IPFcodeW();\n",indent_str.c_str());
                immc++;
                break;
            case OPARG_IWS:
                if (immc != 0)  fprintf(out_fp,"%s        imm%u=IPFcodeWsigned();\n",indent_str.c_str(),immc+1);
                else            fprintf(out_fp,"%s        imm=IPFcodeWsigned();\n",indent_str.c_str());
                immc++;
                break;
            case OPARG_IW16:
                if (immc != 0)  fprintf(out_fp,"%s        imm%u=IPFW();\n",indent_str.c_str(),immc+1);
                else            fprintf(out_fp,"%s        imm=IPFW();\n",indent_str.c_str());
                immc++;
                break;
            case OPARG_IW16S:
                if (immc != 0)  fprintf(out_fp,"%s        imm%u=IPFWsigned();\n",indent_str.c_str(),immc+1);
                else            fprintf(out_fp,"%s        imm=IPFWsigned();\n",indent_str.c_str());
                immc++;
                break;
            case OPARG_IW32:
                if (immc != 0)  fprintf(out_fp,"%s        imm%u=IPFDW();\n",indent_str.c_str(),immc+1);
                else            fprintf(out_fp,"%s        imm=IPFDW();\n",indent_str.c_str());
                immc++;
                break;
            case OPARG_IW32S:
                if (immc != 0)  fprintf(out_fp,"%s        imm%u=IPFDWsigned();\n",indent_str.c_str(),immc+1);
                else            fprintf(out_fp,"%s        imm=IPFDWsigned();\n",indent_str.c_str());
                immc++;
                break;
        };
    }

    if (submap->opmap_valid) {
        uint8_t tmp[16];

        assert(opbaselen < 15);
        if (opbaselen != 0) memcpy(tmp,opbase,opbaselen);
        tmp[opbaselen] = op;

        outcode_gen(codewidth,addrwidth,tmp,opbaselen+1,*submap,indent+2U);
    }

    if (submap->isprefix)
        fprintf(out_fp,"%s        goto _x86decode_after_prefix_code%u_addr%u;\n",indent_str.c_str(),codewidth,addrwidth);
    else
        fprintf(out_fp,"%s        break;\n",indent_str.c_str());
}

void outcode_gen(const unsigned int codewidth,const unsigned int addrwidth,const uint8_t *opbase,const size_t opbaselen,OpByte &map,const unsigned int indent) {
    OpByte *submap,*submap2;
    string indent_str;

    for (unsigned int ind=0;ind < indent;ind++)
        indent_str += "    ";

    if (opbaselen == 0) {
        fprintf(out_fp,"/* BEGIN AUTOGENERATED CODE */\n");
        fprintf(out_fp,"/* code width = %u     addr width = %u */\n",codewidth,addrwidth);
        fprintf(out_fp,"/* expect host code to provide: */\n");
        fprintf(out_fp,"/*   uint8_t op;               opcode byte */\n");
        fprintf(out_fp,"/*   uint%u_t imm,imm2,imm3,imm4; immediate %u-bit */\n",codewidth,codewidth);
        fprintf(out_fp,"/*   struct x86ModRegRm mrm;   mod/reg/rm byte */\n");
        fprintf(out_fp,"/*   struct x86ScaleIndexBase sib; scalar/index/base byte */\n");
        fprintf(out_fp,"/*   uint%u_t disp;            displacement */\n",addrwidth);
        fprintf(out_fp,"/* expect host code to provide: */\n");
        fprintf(out_fp,"/*   uint8_t IPFB();           fetch byte at instruction pointer */\n");
        fprintf(out_fp,"/*   uint16_t IPFW();          fetch word at instruction pointer */\n");
        fprintf(out_fp,"/*   uint32_t IPFDW();         fetch dword at instruction pointer */\n");
        fprintf(out_fp,"/*   uint%u_t IPFcodeW();      fetch %u-bit word at instruction pointer */\n",codewidth,codewidth);
        fprintf(out_fp,"/*   void IPFB_mrm_sib_disp_a%u_read(mrm,sib,disp);   read mod/reg/rm, sib, displacement */\n",codewidth,addrwidth);
        fprintf(out_fp,"_x86decode_begin_code%u_addr%u:\n",codewidth,addrwidth);
        fprintf(out_fp,"_x86decode_after_prefix_code%u_addr%u:\n",codewidth,addrwidth);
    }
    else {
        fprintf(out_fp,"/* Opcodes starting with ");
        for (size_t i=0;i < opbaselen;i++) fprintf(out_fp,"%02Xh ",opbase[i]);
        fprintf(out_fp,"*/\n");
    }

    /* TODO: If modregrm opcode byte and opcodes in this level have a consistent reg == /X pattern
     *       then generate a switch statement by mrm.reg instead of byte value. */

    if (map.modregrm)
        fprintf(out_fp,"%sswitch (mrm.byte) {\n",indent_str.c_str());
    else
        fprintf(out_fp,"%sswitch (op=IPFB()) {\n",indent_str.c_str());

    for (unsigned int op=0;op < 0x100;op++) {
        submap = map.opmap[op];
        if (submap != NULL && !submap->already) {
            for (unsigned int op2=op;op2 < 0x100;op2++) {
                submap2 = map.opmap[op2];

                if (submap2 != NULL && !submap2->already && (op == op2 || *submap == *submap2)) {
                    fprintf(out_fp,"%s    case 0x%02X: /* ",indent_str.c_str(),op2);
                    for (size_t i=0;i < opbaselen;i++) fprintf(out_fp,"%02Xh ",opbase[i]);
                    fprintf(out_fp,"%02Xh ",op2);
                    fprintf(out_fp,"%s%s %s ",submap2->name.c_str(),suffix_str(submap2->suffix,codewidth),submap2->format_str.c_str());
                    if (map.modregrm) fprintf(out_fp,"mod=%u reg=%u rm=%u ",op2>>6,(op2>>3)&7,op2&7);
                    else if (submap2->reg_from_opcode) fprintf(out_fp,"reg=%u ",op2&7);
                    fprintf(out_fp,"*/\n");
                    submap2->already = true;
                }
            }

            opcode_gen_case_statement(codewidth,addrwidth,opbase,opbaselen,map,indent,indent_str,submap,op);
        }
    }
    fprintf(out_fp,"%s    default:\n",indent_str.c_str());
    fprintf(out_fp,"%s        goto _x86decode_illegal_opcode;\n",indent_str.c_str());
    fprintf(out_fp,"%s};\n",indent_str.c_str());

    if (opbaselen != 0) {
        fprintf(out_fp,"/* End of opcodes starting with ");
        for (size_t i=0;i < opbaselen;i++) fprintf(out_fp,"%02Xh ",opbase[i]);
        fprintf(out_fp,"*/\n");
    }

    for (unsigned int op=0;op < 0x100;op++) {
        submap = map.opmap[op];
        if (submap != NULL) submap->already = false;
    }
}

int main(int argc,char **argv) {
    static const uint8_t empty_opcode[1] = {0};
    bool res;

    if (!parse(argc,argv))
        return 1;

    if (in_path == "-")
        in_fp = fdopen(dup(0/*STDIN*/),"r");
    else
        in_fp = fopen(in_path.c_str(),"r");

    res = parse_opcodelist();
    fclose(in_fp);
    if (!res) return 1;

    if (out_path == "-")
        out_fp = fdopen(dup(1/*STDOUT*/),"w");
    else
        out_fp = fopen(out_path.c_str(),"w");

    /* TODO: 16-bit and 32-bit modes if opcode list says the CPU supports the 32-bit operand overrides.
     *       If that is the case, we generate 16-bit and 32-bit cases in a "figure 8" goto configuration,
     *       so that there is one while() loop for executing 16-bit code and one while() loop for executing 32-bit code,
     *       and opcode prefixes jump from one loop to another. Another plan is to have 4 while loops, one for each
     *       permutation of 16-bit/32-bit code and 16-bit/32-bit addressing.
     *
     *       Sound stupid? Maybe, but the idea is that most code runs in the CPU mode it's designed for and that
     *       the operator overrides are uncommon, therefore the opcode decoding could gain a speedup by not having
     *       conditional jumps per instruction with regard to opcode size and address size decoding. */
    outcode_gen(16/*-bit code*/,16/*-bit addressing*/,empty_opcode/*opcode base*/,0/*opcode base len*/,/*&*/opmap16);

    fclose(out_fp);
    return 0;
}

