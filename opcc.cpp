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
    OPDARGTYPE_BYTE,
    OPDARGTYPE_WORD,
    OPDARGTYPE_WORD16,
    OPDARGTYPE_WORD32,
    OPDARGTYPE_FPU,         // FPU st(i)
    OPDARGTYPE_FPU32,       // 32-bit float
    OPDARGTYPE_FPU64,       // 64-bit float
    OPDARGTYPE_FPU80,       // 80-bit float
    OPDARGTYPE_FPUBCD       // floating point packed BCD
};

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
    OpByte() : isprefix(false), segoverride(OPSEG_NONE), more(false), modregrm(false), regswitch(false),
        suffix(OPSUFFIX_NONE), opmap_valid(false) {
    }
    ~OpByte() {
    }
public:
    // opbyte [mod/reg/rm [sib] [disp]] [immediate]
    string          name;       // JMP, etc.
    opccSeg         segoverride;// segment override
    bool            isprefix;   // opcode is prefix
    bool            more;       // this byte is not the last byte of the opcode, see map
    bool            modregrm;   // a MOD/REG/RM byte follows the opcode
    bool            regswitch;  // this opcode is not final, MOD/REG/RM REG field determines opcode, see map.
                                // if ALL opcodes in the map line up with matching REG field (byte & 0x38) then this code
                                // should reduce it down to switch (mrm.reg), else switch (mrm.byte).
                                // this is a signal to the opcc code below to emit code to load MOD/REG/RM then
                                // a switch statement for the next byte.
    enum opccSuffix suffix;     // suffix to name
    vector<enum opccArgs> immarg;
public:
    OpByte*         opmap[256];
    bool            opmap_valid;
};

OpByte*     opmap[256] = {NULL};

// C/C++ implementation of Perl's chomp function.
// static copy to differentiate from target system vs build system.
static void opcc_chomp(char *s) {
    char *t = s + strlen(s) - 1;
    while (t >= s && (*t == '\n' || *t == '\r')) *t-- = 0;
}

bool parse_opcode_def(char *line,unsigned long lineno,char *s) {
    bool allow_modregrm = true;
    vector<enum opccArgs> immarg;
    int oprange_min = -1,oprange_max = -1; // last byte of opcode takes the range (min,max) inclusive
    int mrm_reg_match = -1; // /X style, to say that opcode is specified by REG field of mod/reg/rm
    bool reg_from_opcode = false; // if set, lowest 3 bits of opcode define REG field
    enum opccSeg segoverride = OPSEG_NONE;
    bool is_prefix=false; // opcode is prefix, changes decode state then starts another opcode
    bool modregrm=false;
    bool allow_op=true; // once set to false, can't define more opcode bytes
    char *next=NULL;
    unsigned char ops[16];
    uint8_t op=0;

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
            else if (op >= sizeof(ops)) {
                fprintf(stderr,"Opcode byte string too long\n");
                return false;
            }

            ops[op++] = (unsigned char)b;
        }
        else if (!strcmp(s,"mod/reg/rm")) {
            if (modregrm) {
                fprintf(stderr,"Cannot define mod/reg/rm field twice\n");
                return false;
            }
            else if (!allow_modregrm) {
                fprintf(stderr,"Cannot define mod/reg/rm field here\n");
                return false;
            }

            modregrm = true;
            allow_op = false;
        }
        else if (s[0] == '/' && isdigit(s[1])) { // /1, /3, etc. to match by REG value
            if (!modregrm) {
                fprintf(stderr,"Cannot use REG matching (/1 /3 etc) without mod/reg/rm\n");
                return false;
            }
            else if (mrm_reg_match >= 0) {
                fprintf(stderr,"Cannot match more than one REG value\n");
                return false;
            }

            /* use long parsing to catch overlarge values */
            unsigned long b = strtoul(s,&s,0);

            if (b > 7) {
                fprintf(stderr,"/REG match out of range\n");
                return false;
            }
        }
        else if (!strcmp(s,"ib")) {
            immarg.push_back(OPARG_IB);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"ibs")) {
            immarg.push_back(OPARG_IBS);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"iw")) {
            immarg.push_back(OPARG_IW);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"iws")) {
            immarg.push_back(OPARG_IWS);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"iw16")) {
            immarg.push_back(OPARG_IW16);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"iw16s")) {
            immarg.push_back(OPARG_IW16S);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"iw32")) {
            immarg.push_back(OPARG_IW32);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"iw32s")) {
            immarg.push_back(OPARG_IW32S);
            allow_modregrm = false;
            allow_op = false;
        }
        else if (!strcmp(s,"prefix")) {
            is_prefix = true;
        }
        else if (!strncmp(s,"segoverride(",12)) {
            s += 12;
            while (isblank(*s)) s++;

            if (segoverride != OPSEG_NONE) {
                fprintf(stderr,"You already declared a segment override\n");
                return false;
            }
            else if (!is_prefix) {
                fprintf(stderr,"Segment override must be declared a prefix\n");
                return false;
            }

            if (!strncmp(s,"cs",2))
                segoverride = OPSEG_CS;
            else if (!strncmp(s,"ds",2))
                segoverride = OPSEG_DS;
            else if (!strncmp(s,"es",2))
                segoverride = OPSEG_ES;
            else if (!strncmp(s,"fs",2))
                segoverride = OPSEG_FS;
            else if (!strncmp(s,"gs",2))
                segoverride = OPSEG_GS;
            else if (!strncmp(s,"ss",2))
                segoverride = OPSEG_SS;
            else {
                fprintf(stderr,"Unknown segment override\n");
                return false;
            }
        }
        else if (!strncmp(s,"range(",6)) {
            if (oprange_min >= 0 || oprange_max >= 0) {
                fprintf(stderr,"Cannot define more than one opcode range\n");
                return false;
            }
            else if (!allow_op) {
                fprintf(stderr,"Opcode range not allowed here\n");
                return false;
            }

            s += 6;
            while (isblank(*s)) s++;
            oprange_min = strtol(s,&s,0);
            while (isblank(*s)) s++;
            if (*s != ',') {
                fprintf(stderr,"Syntax error in range()\n");
                return false;
            }
            s++;
            while (isblank(*s)) s++;
            oprange_max = strtol(s,&s,0);
            while (isblank(*s)) s++;
            if (*s != ')') {
                fprintf(stderr,"Syntax error in range(), no closing parenthesis\n");
                return false;
            }

            if (oprange_min < 0 || oprange_min > 0xFF || oprange_max < 0 || oprange_max > 0xFF || oprange_min > oprange_max) {
                fprintf(stderr,"Opcode range error. min=%d max=%d\n",oprange_min,oprange_max);
                return false;
            }

            allow_op = false;
        }
        else if (!strcmp(s,"reg=op02")) {
            if (modregrm) {
                fprintf(stderr,"Cannot declare reg = opcode[0:2] when reg already comes from mod/reg/rm\n");
                return false;
            }

            reg_from_opcode = true;
        }
        else {
            fprintf(stderr,"Syntax error: %s\n",s);
            return false;
        }

        if (ns != NULL) s = ns;
        else break;
    }

    return true;
}

bool parse_opcodelist(void) {
    char line[1024],*s;
    unsigned long lineno = 0;

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

        if (!strncmp(s,"opcode ",7)) {
            s += 7;
            while (isblank(*s)) s++;

            if (!parse_opcode_def(line,lineno,s)) {
                fprintf(stderr," (%u): %s\n",lineno,line);
                return false;
            }
        }
        else {
            fprintf(stderr,"Unknown input:\n");
            fprintf(stderr," (%u): %s\n",lineno,line);
            return false;
        }
    }

    return true;
}

int main(int argc,char **argv) {
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

    return 0;
}

