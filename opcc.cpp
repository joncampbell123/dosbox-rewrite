
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <list>
#include <map>

struct Token {
    enum TokenValue {
        None=0,
        Error,
        End,
        Enum,
        Opcode,
        Opname,         // string = opname
        Display,
        Reads,
        Writes,
        Comma,
        Opsize,         // opsize(reg), opsize(rm), opsize(cpureg), etc. token of param in value
        ModRegRm,
        Range,
        Ternary,
        OpByte,
        Byte,
        Word,
        Immediate,      // "i"
        Mod,
        Reg,
        Rm,
        Iop,
        Ibs,
        RegSpec,        // Intel-ism for value of reg field in combo opcodes i.e. reg == 0 spec by /0
        HexValue,
        Prefix,
        PrefixName,
        ParensOpen,
        ParensClose,
        Lock,
        And,
        Dash,
        Encoding,
        State,
        Symbol,
        AssignSign,     // =
        EqualsSign,     // ==
        NotEqualsSign,  // !=
        QuestionMark,
        ColonMark,
        SegOverride,
        CS,
        DS,
        ES,
        SS,
        Rep,
        Repz,
        Repnz,
        a,              // AL/AX/EAX/RAX
        b,              // BL/BX/EBX/RBX
        c,              // CL/CX/ECX/RCX
        d,              // DL/DX/EDX/RCX
        S,              //    SI/ESI/RSI
        D               //    DI/EDI/RDI
    };

    std::string         string,string2;
    std::string         error_source;
    uint64_t            value,value2,value3,value4,value5;
    std::vector<Token>  subtoken;
    enum TokenValue     token;
    long                line;
    unsigned int        pos;

    Token() : value(0), value2(0), value3(0), value4(0), value5(0), token(None), line(0), pos(0) {
    }

    void fprintf_debug(FILE *fp) {
        if (token == Opsize) {
            Token sv; sv.token = (enum TokenValue)value;

            fprintf(fp,"opsize(");
            sv.fprintf_debug(fp);
            fprintf(fp,")");
        }
        else if (token == Range) {
            fprintf(fp,"range(");
            fprintf(fp,"0x%llX-0x%llX",(unsigned long long)value,(unsigned long long)value2);
            fprintf(fp,")");
        }
        else if (token == CS) {
            fprintf(fp,"cs");
        }
        else if (token == DS) {
            fprintf(fp,"ds");
        }
        else if (token == ES) {
            fprintf(fp,"es");
        }
        else if (token == SS) {
            fprintf(fp,"ss");
        }
        else if (token == Rep) {
            fprintf(fp,"rep");
        }
        else if (token == Repz) {
            fprintf(fp,"repz");
        }
        else if (token == Repnz) {
            fprintf(fp,"repnz");
        }
        else if (token == SegOverride) {
            fprintf(fp,"segoverride");
        }
        else if (token == Lock) {
            fprintf(fp,"lock");
        }
        else if (token == Ternary) {
            fprintf(fp,"ternary(");
            {
                Token sv; sv.token = (enum TokenValue)value;
                sv.fprintf_debug(fp);
            }
            {
                Token sv; sv.token = (enum TokenValue)value2;
                sv.fprintf_debug(fp);
            }
            {
                Token sv; sv.token = HexValue; sv.value = value3;
                sv.fprintf_debug(fp);
            }
            fprintf(fp,"?");
            {
                Token sv; sv.token = (enum TokenValue)value4;
                sv.fprintf_debug(fp);
            }
            fprintf(fp,":");
            {
                Token sv; sv.token = (enum TokenValue)value5;
                sv.fprintf_debug(fp);
            }
            fprintf(fp,")");
        }
        else if (token == ModRegRm)
            fprintf(fp,"mod/reg/rm");
        else if (token == HexValue)
            fprintf(fp,"0x%llX",(unsigned long long)value);
        else if (token == Iop)
            fprintf(fp,"iop");
        else if (token == Ibs)
            fprintf(fp,"ibs");
        else if (token == QuestionMark)
            fprintf(fp,"?");
        else if (token == ColonMark)
            fprintf(fp,":");
        else if (token == And)
            fprintf(fp,"&");
        else if (token == Byte)
            fprintf(fp,"byte");
        else if (token == Word)
            fprintf(fp,"word");
        else if (token == OpByte)
            fprintf(fp,"opbyte");
        else if (token == Immediate)
            fprintf(fp,"i");
        else if (token == Reg)
            fprintf(fp,"reg");
        else if (token == Rm)
            fprintf(fp,"rm");
        else if (token == a)
            fprintf(fp,"a");
        else if (token == b)
            fprintf(fp,"b");
        else if (token == c)
            fprintf(fp,"c");
        else if (token == d)
            fprintf(fp,"d");
        else if (token == S)
            fprintf(fp,"S");
        else if (token == D)
            fprintf(fp,"D");
        else {
            fprintf(fp,"???");
        }
    }
};

std::string         source_file;
long                source_file_line=1;
bool                source_stdin=false;
FILE*               source_fp=NULL;

std::string         dest_file;
bool                dest_stdout=true;
FILE*               dest_fp=NULL;

std::string         symbol_file;

char                line[4096];

// symbols
const size_t        symbol_continue_prefix = 0;
const size_t        symbol_continue_main = 1;
const size_t        symbol_undefined = 2;

class OPCC_Symbol {
public:
    OPCC_Symbol() {
    }
    ~OPCC_Symbol() {
    }
public:
    void fprintf_debug(FILE *fp) {
        fprintf(fp,"/* %-24s %-20s */\n",
            enum_string.c_str(),
            (std::string("\"")+opname_string+"\"").c_str());

        if (!display.empty()) {
            fprintf(fp,"/*   display: ");
            for (size_t i=0;i < display.size();i++) {
                if (i != 0) fprintf(fp,", ");
                display[i].fprintf_debug(fp);
            }
            fprintf(fp," */\n");
        }

        if (!reads.empty()) {
            fprintf(fp,"/*   reads:   ");
            for (size_t i=0;i < reads.size();i++) {
                if (i != 0) fprintf(fp,", ");
                reads[i].fprintf_debug(fp);
            }
            fprintf(fp," */\n");
        }

        if (!writes.empty()) {
            fprintf(fp,"/*   writes:  ");
            for (size_t i=0;i < writes.size();i++) {
                if (i != 0) fprintf(fp,", ");
                writes[i].fprintf_debug(fp);
            }
            fprintf(fp," */\n");
        }
    }
public:
    std::string                 enum_string;                // enum         (name used in enum for C++ code)
    std::string                 opname_string;              // opname       (string displayed in debugger)
    std::vector<Token>          display,reads,writes;
};

std::vector<OPCC_Symbol>        Symbols;
std::map<std::string,size_t>    SymbolsByEnum;

static OPCC_Symbol              Symbol_none;

void fprintf_error_pos(FILE *fp,const Token &t,const char *msg...) {
    va_list va;

    va_start(va,msg);
    vfprintf(fp,msg,va);
    va_end(va);

    fprintf(fp,"line %ld, col %u: at %s\n",t.line,t.pos,t.error_source.c_str());
}

OPCC_Symbol &OPCC_Symbol_New(const char *enum_name) {
    const size_t index = Symbols.size();

    Symbols.resize(index+1);

    OPCC_Symbol &v = Symbols[index];
    v.enum_string = enum_name;

    assert(SymbolsByEnum.find(enum_name) == SymbolsByEnum.end());
    SymbolsByEnum[enum_name] = index;

    return v;
}

OPCC_Symbol &OPCC_Symbol_Create(const char *s) {
    auto i = SymbolsByEnum.find(s);

    if (i != SymbolsByEnum.end())
        return Symbol_none;

    return OPCC_Symbol_New(s);
}

OPCC_Symbol &OPCC_Symbol_Lookup(const char *s) {
    auto i = SymbolsByEnum.find(s);

    if (i != SymbolsByEnum.end()) {
        assert(i->second < Symbols.size());
        return Symbols[i->second];
    }

    return Symbol_none;
}

OPCC_Symbol &OPCC_Symbol_Lookup_By_Index(const size_t i) {
    if (i < Symbols.size())
        return Symbols[i];

    return Symbol_none;
}

size_t OPCC_SymbolToIndex(const OPCC_Symbol &s) {
    // apparently this works because C++ requires std::vector to allocate a contiguous block of memory for the array
    const size_t i = (size_t)((uintptr_t)((&s) - (&Symbols[0])));

    assert(i < Symbols.size());
    assert(&Symbols[i] == (&Symbols[0] + i));

    return i;
}

OPCC_Symbol &OPCC_IndexToSymbol(const size_t i) {
    assert(i < Symbols.size());
    return Symbols[i];
}

class OPCC_Opcode {
public:
    OPCC_Opcode() : reg_match(-1), Symbol_Index(~((size_t)0)) {
    }
    ~OPCC_Opcode() {
    }
public:
    void fprintf_debug(FILE *fp) {
        OPCC_Symbol &sym = OPCC_Symbol_Lookup_By_Index(Symbol_Index);

        fprintf(fp,"/* Opcode: %-24s %-20s */\n",
            sym.enum_string.c_str(),
            (std::string("\"")+sym.opname_string+"\"").c_str());

        if (opsize.token != Token::None) {
            fprintf(fp,"/*   Opsize: ");
            opsize.fprintf_debug(fp);
            fprintf(fp," */\n");
        }
        if (reg_source.token != Token::None) {
            fprintf(fp,"/*   Reg source: ");
            reg_source.fprintf_debug(fp);
            fprintf(fp," */\n");
        }
        if (!encoding.empty()) {
            fprintf(fp,"/*   Encoding: ");
            for (size_t ei=0;ei < encoding.size();ei++) {
                if (ei != 0) fprintf(fp," ");
                encoding[ei].fprintf_debug(fp);

                if (encoding[ei].token == Token::ModRegRm && reg_match >= 0)
                    fprintf(fp," /%d",reg_match);
            }
            fprintf(fp," */\n");
        }
    }
public:
    std::vector<Token>          encoding;   // encoding tokens (range, HexValue, mod/reg/rm, immediate)
    signed char                 reg_match;  // opcode matches reg value (group opcode)
    size_t                      Symbol_Index;
    Token                       reg_source;
    Token                       opsize;
};

std::vector<OPCC_Opcode>        Opcodes;

class OPCC_Prefix {
public:
    OPCC_Prefix() {
    }
    ~OPCC_Prefix() {
    }
public:
    void fprintf_debug(FILE *fp) {
        fprintf(fp,"/* Prefix: %-20s */\n",
            (std::string("\"")+prefix_name+"\"").c_str());

        if (!encoding.empty()) {
            fprintf(fp,"/*   Encoding: ");
            for (size_t ei=0;ei < encoding.size();ei++) {
                if (ei != 0) fprintf(fp," ");
                encoding[ei].fprintf_debug(fp);
            }
            fprintf(fp," */\n");
        }

        for (size_t si=0;si < state.size();si++) {
            auto &i = state[si];
            fprintf(fp,"/*   State change: ");
            i.first.fprintf_debug(fp);
            fprintf(fp," = ");
            i.second.fprintf_debug(fp);
            fprintf(fp," */\n");
        }
    }
public:
    std::string                 prefix_name;
    std::vector<Token>          encoding;   // encoding tokens (range, HexValue, mod/reg/rm, immediate)
    std::vector<std::pair<Token,Token> > state;
};

std::vector<OPCC_Prefix>        Prefixes;

void OPCC_Symbol_Init(void) {
    SymbolsByEnum.clear();
    Symbols.clear();

    // auto-create "continue_prefix" symbol. this tells the state machine to keep running (prefix opcode).
    assert(Symbols.size() == symbol_continue_prefix);
    assert(OPCC_SymbolToIndex(OPCC_Symbol_New("continue_prefix")) == symbol_continue_prefix);

    // auto-create "continue_main" symbol. this tells the state machine to keep running (main opcode).
    assert(Symbols.size() == symbol_continue_main);
    assert(OPCC_SymbolToIndex(OPCC_Symbol_New("continue_main")) == symbol_continue_main);

    // auto-create "undefined" symbol. disassembler can show "unknown", emulator can throw #UD exception unless 8088 emulation which is then a NO-OP
    assert(Symbols.size() == symbol_undefined);
    assert(OPCC_SymbolToIndex(OPCC_Symbol_New("undefined")) == symbol_undefined);
}

static void help(void) {
    fprintf(stderr,"opcc [options]\n");
    fprintf(stderr,"Opcode compiler, 2017 rewrite\n");
    fprintf(stderr," -i <file>              Read from file\n");
    fprintf(stderr," -i -                   Read from stdin\n");
    fprintf(stderr," -o <file>              Write to file\n");
    fprintf(stderr," -o -                   Write to stdout\n");
    fprintf(stderr," -s <file>              Write symbols to file\n");
}

// C/C++ implementation of Perl's chomp function.
// static copy to differentiate from target system vs build system.
static void opcc_chomp(char *s) {
    char *t = s + strlen(s) - 1;
    while (t >= s && (*t == '\n' || *t == '\r')) *t-- = 0;
}

static void opcc_eat_comments(char *s) {
    char *t = strchr(s,';');
    if (t == NULL) return;
    *t = 0; // SNIP
}

static char *line_parse = line;

static void line_skip_whitespace(void) {
    while (*line_parse == ' ' || *line_parse == '\t') line_parse++;
}

static bool line_get(void) {
    if (line == NULL) return false;
    if (fgets(line,sizeof(line)-1,source_fp) == NULL) return false;
    source_file_line++;
    line_parse = line;

    opcc_chomp(line);
    opcc_eat_comments(line);
    return true;
}

bool isunquotednamechar(char c) {
    return isalpha(c) || isdigit(c) || c == '_';
}

static std::string line_get_unquoted_name(void) {
    char *start;

    line_skip_whitespace();
    start = line_parse;

    while (*line_parse && isunquotednamechar(*line_parse))
        line_parse++;

    assert(start <= line_parse);
    return std::string(start,(size_t)(line_parse - start));
}

static std::string line_get_name(void) {
    line_skip_whitespace();

    if (*line_parse == '\"') {
        char *start = ++line_parse;
        while (*line_parse && *line_parse != '\"')
            line_parse++;

        char *end = line_parse;
        assert(start <= end);
        if (*line_parse == '\"') line_parse++;
        return std::string(start,(size_t)(end - start));
    }

    return line_get_unquoted_name();
}

// WARNING: recursive
static Token line_get_token(void) {
    char *start;
    Token r;

    r.line = source_file_line;
    line_skip_whitespace();
    r.pos = (unsigned int)(line_parse + 1 - line);
    if (*line_parse == 0)
        return r;

    start = line_parse;
    if (isalpha(*line_parse)) {
        /* read until whitespace or open parens */
        while (*line_parse &&
          !(*line_parse == ' ' || *line_parse == '\t' || *line_parse == '(' ||
            *line_parse == ')' || *line_parse == ','  || *line_parse == '&' ||
            *line_parse == '?' || *line_parse == ':'  || *line_parse == '=')) line_parse++;

        if (*line_parse == '(') {
            /* this is where recursion comes in! */
            bool closeparens = false;
            Token st;

            std::string funcname = std::string(start,(size_t)(line_parse - start));

            line_parse++;
            line_skip_whitespace();
            r.token = Token::Error;

            if (funcname == "opsize") {
                /* opsize(reg), opsize(rm), etc */
                st = line_get_token();
                if (st.token == Token::Error) {
                    fprintf_error_pos(stderr,st,"Error in parenthesis ");
                }
                else if (
                    st.token == Token::Reg ||
                    st.token == Token::Rm ||
                    st.token == Token::Immediate) {
                    r.token = Token::Opsize;
                    r.value = (uint64_t)st.token;
                }
                else {
                    fprintf_error_pos(stderr,st,"Unexpected token ");
                }

                /* should close with parenthesis */
                if (!closeparens && r.token != Token::Error) {
                    st = line_get_token();
                    if (st.token != Token::ParensClose) {
                        fprintf_error_pos(stderr,st,"Failure to close parenthesis ");
                        r.token = Token::Error;
                    }
                    else {
                        closeparens = true;
                    }
                }
            }
            else if (funcname == "range") {
                /* range(min-max), etc */
                st = line_get_token();
                if (st.token == Token::Error) {
                    fprintf_error_pos(stderr,st,"Error in parenthesis ");
                }
                else if (st.token == Token::HexValue) {
                    r.value = st.value;

                    st = line_get_token();
                    if (st.token == Token::Dash) {
                        st = line_get_token();
                        if (st.token == Token::HexValue) {
                            r.value2 = st.value;

                            r.token = Token::Range;
                        }
                        else {
                            fprintf_error_pos(stderr,st,"Unexpected token ");
                        }
                    }
                    else {
                        fprintf_error_pos(stderr,st,"Unexpected token ");
                    }
                }
                else {
                    fprintf_error_pos(stderr,st,"Unexpected token ");
                }

                /* should close with parenthesis */
                if (!closeparens && r.token != Token::Error) {
                    st = line_get_token();
                    if (st.token != Token::ParensClose) {
                        fprintf_error_pos(stderr,st,"Failure to close parenthesis ");
                        r.token = Token::Error;
                    }
                    else {
                        closeparens = true;
                    }
                }
            }
            else if (funcname == "ternary") {
                /* ternary(opbyte&1?truecond:falsecond) */
                st = line_get_token();
                if (st.token == Token::Error) {
                    fprintf_error_pos(stderr,st,"Error in ternary test ");
                    r.token = Token::Error;
                }
                else if (st.token == Token::OpByte) {
                    r.value = (uint64_t)st.token;

                    st = line_get_token();
                    if (st.token == Token::And) {
                        r.value2 = (uint64_t)st.token;

                        st = line_get_token();
                        if (st.token == Token::HexValue) {
                            r.value3 = (uint64_t)st.value;

                            st = line_get_token();
                            if (st.token == Token::QuestionMark) {
                                st = line_get_token();
                                if (st.token == Token::Word ||
                                    st.token == Token::Byte) {
                                    r.value4 = (uint64_t)st.token;

                                    st = line_get_token();
                                    if (st.token == Token::ColonMark) {
                                        st = line_get_token();
                                        if (st.token == Token::Word ||
                                            st.token == Token::Byte) {
                                            r.value5 = (uint64_t)st.token;
                                            r.token = Token::Ternary;
                                        }
                                        else {
                                            fprintf_error_pos(stderr,st,"Invalid ternary test ");
                                            r.token = Token::Error;
                                        }
                                    }
                                    else {
                                        fprintf_error_pos(stderr,st,"Invalid ternary test ");
                                        r.token = Token::Error;
                                    }
                                }
                                else {
                                    fprintf_error_pos(stderr,st,"Invalid ternary test ");
                                    r.token = Token::Error;
                                }
                            }
                            else {
                                fprintf_error_pos(stderr,st,"Invalid ternary test ");
                                r.token = Token::Error;
                            }
                        }
                        else {
                            fprintf_error_pos(stderr,st,"Invalid ternary test ");
                            r.token = Token::Error;
                        }
                    }
                    else {
                        fprintf_error_pos(stderr,st,"Invalid ternary test ");
                        r.token = Token::Error;
                    }
                }
                else {
                    fprintf_error_pos(stderr,st,"Invalid ternary test ");
                    r.token = Token::Error;
                }

                /* should close with parenthesis */
                if (!closeparens && r.token != Token::Error) {
                    st = line_get_token();
                    if (st.token != Token::ParensClose) {
                        fprintf_error_pos(stderr,st,"Failure to close parenthesis ");
                        r.token = Token::Error;
                    }
                    else {
                        closeparens = true;
                    }
                }
            }
            else {
                fprintf_error_pos(stderr,r,"Unknown function '%s' ",start);
            }

            if (r.token != Token::Error && !closeparens) {
                fprintf_error_pos(stderr,st,"Failure to close parenthesis ");
                r.token = Token::Error;
            }

            if (r.token == Token::Error)
                r.error_source = start;
        }
        else {
            std::string tokenname = std::string(start,(size_t)(line_parse - start));

            line_skip_whitespace();

//            fprintf(stderr,"token '%s'\n",tokenname.c_str());

            // sorry, I'm not going out of my way to make it case insensitive
            if (tokenname == "display") {
                r.token = Token::Display;
            }
            else if (tokenname == "encoding") {
                r.token = Token::Encoding;
            }
            else if (tokenname == "end") {
                r.token = Token::End;
            }
            else if (tokenname == "opbyte") {
                r.token = Token::OpByte;
            }
            else if (tokenname == "word") {
                r.token = Token::Word;
            }
            else if (tokenname == "byte") {
                r.token = Token::Byte;
            }
            else if (tokenname == "enum") {
                r.token = Token::Enum;
                /* what follows is an enum name */
                r.string = line_get_unquoted_name();
                if (r.string.empty()) {
                    r.token = Token::Error;
                    r.error_source = start;
                    return r;
                }
            }
            else if (tokenname == "prefixname") {
                r.token = Token::PrefixName;
                /* what follows is an enum name */
                r.string = line_get_name();
                if (r.string.empty()) {
                    r.token = Token::Error;
                    r.error_source = start;
                    return r;
                }
            }
            else if (tokenname == "opcode") {
                r.token = Token::Opcode;
            }
            else if (tokenname == "state") {
                Token st;

                /* state name=value */
                r.token = Token::State;

                st = line_get_token();
                if (!(st.token == Token::None || st.token == Token::Error)) {
                    r.subtoken.push_back(st);

                    st = line_get_token();
                    if (st.token == Token::EqualsSign) {
                        st = line_get_token();
                        if (!(st.token == Token::None || st.token == Token::Error)) {
                            r.subtoken.push_back(st);
                        }
                    }
                }

                if (r.subtoken.size() < 2) {
                    if (st.token == Token::None || st.token == Token::Error) {
                        fprintf_error_pos(stderr,st,"State unexpected token ");
                        r.token = Token::Error;
                        r.error_source = start;
                        return r;
                    }
                }
            }
            else if (tokenname == "opname") {
                r.token = Token::Opname;
                r.string = line_get_name(); // quoted or unquoted
                if (r.string.empty()) {
                    r.token = Token::Error;
                    r.error_source = start;
                    return r;
                }
            }
            else if (tokenname == "ibs") {
                r.token = Token::Ibs;
            }
            else if (tokenname == "iop") {
                r.token = Token::Iop;
            }
            else if (tokenname == "mod/reg/rm") {
                r.token = Token::ModRegRm;
            }
            else if (tokenname == "opsize") {
                r.token = Token::Opsize;
                r.value = (uint64_t)Token::None;
            }
            else if (tokenname == "reads") {
                r.token = Token::Reads;
            }
            else if (tokenname == "symbol") {
                r.token = Token::Symbol;
            }
            else if (tokenname == "rm") {
                r.token = Token::Rm;
            }
            else if (tokenname == "reg") {
                r.token = Token::Reg;
            }
            else if (tokenname == "rep") {
                r.token = Token::Rep;
            }
            else if (tokenname == "repz") {
                r.token = Token::Repz;
            }
            else if (tokenname == "repnz") {
                r.token = Token::Repnz;
            }
            else if (tokenname == "segoverride") {
                r.token = Token::SegOverride;
            }
            else if (tokenname == "prefix") {
                r.token = Token::Prefix;
            }
            else if (tokenname == "cs") {
                r.token = Token::CS;
            }
            else if (tokenname == "ds") {
                r.token = Token::DS;
            }
            else if (tokenname == "es") {
                r.token = Token::ES;
            }
            else if (tokenname == "ss") {
                r.token = Token::SS;
            }
            else if (tokenname == "i") {
                r.token = Token::Immediate;
            }
            else if (tokenname == "a") {
                r.token = Token::a;
            }
            else if (tokenname == "b") {
                r.token = Token::b;
            }
            else if (tokenname == "c") {
                r.token = Token::c;
            }
            else if (tokenname == "c") {
                r.token = Token::d;
            }
            else if (tokenname == "S") {
                r.token = Token::S;
            }
            else if (tokenname == "D") {
                r.token = Token::D;
            }
            else if (tokenname == "lock") {
                r.token = Token::Lock;
            }
            else if (tokenname == "writes") {
                r.token = Token::Writes;
            }
            else {
                r.token = Token::Error;
                r.error_source = start;
                return r;
            }
        }
    }
    else if (isdigit(*line_parse)) {
        r.token = Token::HexValue;
        r.value = (uint64_t)strtoull(line_parse,&line_parse,0);
    }
    else if (*line_parse == '/') { // Intel-ism for specifying the value of reg in mod/reg/rm   /0 /1 /2 /3 /4 /5 /6 /7
        line_parse++;
        if (!isdigit(*line_parse)) {
            r.token = Token::Error;
            r.error_source = start;
            return r;
        }
        r.token = Token::RegSpec;
        r.value = (uint64_t)strtoull(line_parse,&line_parse,0);
    }
    else if (*line_parse == ',') {
        r.token = Token::Comma;
        line_parse++;
    }
    else if (*line_parse == '(') {
        r.token = Token::ParensOpen;
        line_parse++;
    }
    else if (*line_parse == ')') {
        r.token = Token::ParensClose;
        line_parse++;
    }
    else if (*line_parse == '=') {
        r.token = Token::EqualsSign;
        line_parse++;
    }
    else if (*line_parse == '&') {
        r.token = Token::And;
        line_parse++;
    }
    else if (*line_parse == '-') {
        r.token = Token::Dash;
        line_parse++;
    }
    else if (*line_parse == '?') {
        r.token = Token::QuestionMark;
        line_parse++;
    }
    else if (*line_parse == ':') {
        r.token = Token::ColonMark;
        line_parse++;
    }
    else {
        r.token = Token::Error;
        r.error_source = start;
        return r;
    }

    line_skip_whitespace();
    return r;
}

static int parse_argv(int argc,char **argv) {
    char *a;
    int i;

    for (i=1;i < argc;) {
        a = argv[i++];

        if (*a == '-') {
            do { a++; } while (*a == '-');

            if (!strcmp(a,"h") || !strcmp(a,"help")) {
                help();
                return 1;
            }
            else if (!strcmp(a,"s")) {
                a = argv[i++];
                if (a == NULL) return 1;

                symbol_file = a;
            }
            else if (!strcmp(a,"o")) {
                a = argv[i++];
                if (a == NULL) return 1;

                if (!strcmp(a,"-")) {
                    dest_stdout = true;
                    dest_file.clear();
                }
                else {
                    dest_stdout = false;
                    dest_file = a;
                }
            }
            else if (!strcmp(a,"i")) {
                // TODO: allow multiple -i files
                a = argv[i++];
                if (a == NULL) return 1;

                if (!strcmp(a,"-")) {
                    source_stdin = true;
                    source_file.clear();
                }
                else {
                    source_stdin = false;
                    source_file = a;
                }
            }
            else {
                fprintf(stderr,"Unknown switch %s\n",a);
                return 1;
            }
        }
        else {
            fprintf(stderr,"Unknown arg %s\n",a);
            return 1;
        }
    }

    if (!source_stdin && source_file.empty()) {
        help();
        return 1;
    }

    return 0;
}

/* symbol
 *    ...
 * end
 *
 * at entry to this function, the line containing "symbol" has been parsed.
 * it's up to us to fetch lines until we hit "end" */
bool parse_symbol_block(void) {
    bool ret = true,end = false;
    OPCC_Symbol sym;
    Token t;

    t.pos = 1;
    t.line = source_file_line;
    while (line_get()) {
        t = line_get_token();

        if (t.token == Token::Error) {
            fprintf_error_pos(stderr,t,"Error in ");
            ret = false;
            break;
        }
        else if (t.token == Token::End) {
            end = true;
            break;
        }
        else if (t.token == Token::Enum) {
            /* enum   <name> */
            /* name is given in "string" */
            assert(!t.string.empty());

            if (sym.enum_string.empty())
                sym.enum_string = t.string;
            else {
                fprintf_error_pos(stderr,t,"Enum can only be specified once ");
                ret = false;
                break;
            }
        }
        else if (t.token == Token::Opname) {
            /* enum   <name> */
            /* name is given in "string" */
            assert(!t.string.empty());

            if (sym.opname_string.empty())
                sym.opname_string = t.string;
            else {
                fprintf_error_pos(stderr,t,"Opname can only be specified once ");
                ret = false;
                break;
            }
        }
        else if (t.token == Token::Display ||
                 t.token == Token::Writes ||
                 t.token == Token::Reads) {
            std::vector<Token> dsp;
            Token st;

            /* display    (opcode spec) */
            /* writes     (opcode spec) */
            /* reads      (opcode spec) */
            do {
                st = line_get_token();
                if (st.token == Token::Error || st.token == Token::None)
                    break;
                else if (st.token == Token::Opsize) { /* opsize(reg) */
                    assert(st.value != (uint64_t)Token::None);

                         if (t.token == Token::Display)
                        sym.display.push_back(st);
                    else if (t.token == Token::Writes)
                        sym.writes.push_back(st);
                    else if (t.token == Token::Reads)
                        sym.reads.push_back(st);
                    else
                        abort();
                }
                else {
                    st.token = Token::Error;
                    break;
                }

                /* after the spec, I require a comma, or end of line */
                st = line_get_token();
                if (st.token == Token::Error || st.token == Token::None)
                    break;
                if (st.token != Token::Comma) {
                    st.token = Token::Error;
                    break;
                }
            } while (1);

            if (st.token == Token::Error) {
                fprintf_error_pos(stderr,t,"Error in ");
                ret = false;
                break;
            }
        }
        else if (t.token == Token::None) {
            continue;
        }
        else {
            fprintf_error_pos(stderr,t,"Unexpected token at ");
            ret = false;
            break;
        }
    }

    if (ret && !end) {
        fprintf_error_pos(stderr,t,"No End token, stopping parsing at ");
        ret = false;
    }

    if (sym.enum_string.empty()) {
        fprintf_error_pos(stderr,t,"No enum string provided ");
        ret = false;
    }
    else if (sym.opname_string.empty()) {
        fprintf_error_pos(stderr,t,"No opname string provided ");
        ret = false;
    }
    else {
        OPCC_Symbol &x = OPCC_Symbol_Lookup(sym.enum_string.c_str());
        if (&x != &Symbol_none) {
            fprintf_error_pos(stderr,t,"Enum string already exists ");
            ret = false;
            return ret;
        }

        if (ret)
            /*OPCC_Symbol&*/OPCC_Symbol_New(sym.enum_string.c_str()) = sym;
    }

    return ret;
}

/* opcode
 *    ...
 * end
 *
 * at entry to this function, the line containing "opcode" has been parsed.
 * it's up to us to fetch lines until we hit "end" */
bool parse_opcode_block(void) {
    bool ret = true,end = false;
    OPCC_Opcode opcode;
    Token t;

    t.pos = 1;
    t.line = source_file_line;
    while (line_get()) {
        t = line_get_token();

        if (t.token == Token::Error) {
            fprintf_error_pos(stderr,t,"Error in ");
            ret = false;
            break;
        }
        else if (t.token == Token::End) {
            end = true;
            break;
        }
        else if (t.token == Token::Enum) {
            /* enum   <name> */
            /* name is given in "string" */
            assert(!t.string.empty());

            OPCC_Symbol &x = OPCC_Symbol_Lookup(t.string.c_str());
            if (&x == &Symbol_none) {
                fprintf_error_pos(stderr,t,"Symbol '%s' does not exist ",t.string.c_str());
                ret = false;
                break;
            }

            opcode.Symbol_Index = OPCC_SymbolToIndex(x);
        }
        else if (t.token == Token::Encoding) {
            enum {
                PREFIX=0,
                MAIN,
                MODREGRM,
                MODREGRM_CLARIFY,
                MAIN_TAIL,      // only for 3dnow! opcode encoding (0x0F 0x0F mod/reg/rm OPCODE)
                IMMEDIATE
            };
            int state = PREFIX;

            do {
                Token st = line_get_token();

                if (st.token == Token::Error) {
                    fprintf_error_pos(stderr,st,"Error in encoding token ");
                    ret = false;
                    break;
                }
                else if (st.token == Token::None) {
                    break;
                }
                else if ((st.token == Token::Range || st.token == Token::HexValue) && state <= MAIN) {
                    opcode.encoding.push_back(st);
                    state = MAIN;
                }
                else if ((st.token == Token::Range || st.token == Token::HexValue) && state >= MODREGRM && state <= MAIN_TAIL) {
                    opcode.encoding.push_back(st);
                    state = MAIN_TAIL;
                }
                else if (st.token == Token::ModRegRm && state >= MAIN && state < MODREGRM) {
                    opcode.encoding.push_back(st);
                    state = MODREGRM;
                }
                else if (st.token == Token::RegSpec && state >= MODREGRM && state <= MODREGRM_CLARIFY && opcode.reg_match < 0) {
                    opcode.reg_match = (signed char)st.value;
                    state = MODREGRM_CLARIFY;
                }
                else if (
                   (st.token == Token::Iop || st.token == Token::Ibs
                    ) && state >= MAIN && state <= IMMEDIATE) {
                    opcode.encoding.push_back(st);
                    state = IMMEDIATE;
                }
                else {
                    fprintf_error_pos(stderr,st,"Unexpected encoding token ");
                    ret = false;
                    break;
                }
            } while (1);

            if (!ret) break;
        }
        else if (t.token == Token::Opsize) {
            Token st = line_get_token();

            if (st.token == Token::Error) {
                fprintf_error_pos(stderr,st,"Error in opsize token ");
                ret = false;
                break;
            }
            else if (st.token == Token::Ternary) {
                opcode.opsize = st;
            }
            else {
                fprintf_error_pos(stderr,st,"Unexpected opsize token ");
                ret = false;
                break;
            }
        }
        else if (t.token == Token::Reg) {
            // reg <reg>
            if (opcode.reg_source.token == Token::None) {
                Token st = line_get_token();

                if (st.token == Token::a || st.token == Token::b ||
                    st.token == Token::c || st.token == Token::d ||
                    st.token == Token::S || st.token == Token::D ||
                    st.token == Token::Reg) {
                    opcode.reg_source = st;
                }
                else {
                    fprintf_error_pos(stderr,st,"Reg source not valid");
                }
            }
            else {
                fprintf_error_pos(stderr,t,"Reg source already defined");
            }
        }
        else if (t.token == Token::None) {
            continue;
        }
        else {
            fprintf_error_pos(stderr,t,"Unexpected token at ");
            ret = false;
            break;
        }
    }

    // if mod/reg/rm was specified, reg_source should be mod/reg/rm
    if (ret) {
        for (size_t si=0;si < opcode.encoding.size();si++) {
            if (opcode.encoding[si].token == Token::ModRegRm) {
                if (opcode.reg_source.token == Token::None)
                    opcode.reg_source = opcode.encoding[si];
                else {
                    fprintf_error_pos(stderr,t,"Reg source already specified ");
                    ret = false;
                    break;
                }
            }
        }
    }

    if (ret && !end) {
        fprintf_error_pos(stderr,t,"No End token, stopping parsing at ");
        ret = false;
    }
    else if (opcode.encoding.empty()) {
        fprintf_error_pos(stderr,t,"No encoding specified ");
        ret = false;
    }
    else if (opcode.Symbol_Index == ~((size_t)0)) {
        fprintf_error_pos(stderr,t,"No symbol specified ");
        ret = false;
    }
    else {
        if (ret) Opcodes.push_back(opcode);
    }

    return ret;
}

/* prefix
 *    ...
 * end
 *
 * at entry to this function, the line containing "prefix" has been parsed.
 * it's up to us to fetch lines until we hit "end" */
bool parse_prefix_block(void) {
    bool ret = true,end = false;
    OPCC_Prefix prefix;
    Token t;

    t.pos = 1;
    t.line = source_file_line;
    while (line_get()) {
        t = line_get_token();

        if (t.token == Token::Error) {
            fprintf_error_pos(stderr,t,"Error in ");
            ret = false;
            break;
        }
        else if (t.token == Token::End) {
            end = true;
            break;
        }
        else if (t.token == Token::Encoding) {
            do {
                Token st = line_get_token();

                if (st.token == Token::Error) {
                    fprintf_error_pos(stderr,st,"Error in encoding token ");
                    ret = false;
                    break;
                }
                else if (st.token == Token::None) {
                    break;
                }
                else if (st.token == Token::Range || st.token == Token::HexValue) {
                    prefix.encoding.push_back(st);
                }
                else {
                    fprintf_error_pos(stderr,st,"Unexpected encoding token ");
                    ret = false;
                    break;
                }
            } while (1);

            if (!ret) break;
        }
        else if (t.token == Token::PrefixName) {
            assert(!t.string.empty());

            if (prefix.prefix_name.empty()) {
                prefix.prefix_name = t.string;
            }
            else {
                fprintf_error_pos(stderr,t,"PrefixName already set ");
                ret = false;
                break;
            }
        }
        else if (t.token == Token::State) {
            /* state name=val */
            assert(t.subtoken.size() >= 2);
            prefix.state.push_back(std::pair<Token,Token>(t.subtoken[0],t.subtoken[1]));
        }
        else if (t.token == Token::None) {
            continue;
        }
        else {
            fprintf_error_pos(stderr,t,"Unexpected token at ");
            ret = false;
            break;
        }
    }

    if (ret && !end) {
        fprintf_error_pos(stderr,t,"No End token, stopping parsing at ");
        ret = false;
    }
    else if (prefix.encoding.empty()) {
        fprintf_error_pos(stderr,t,"No encoding specified ");
        ret = false;
    }
    else {
        if (ret) Prefixes.push_back(prefix);
    }

    return ret;
}

int main(int argc,char **argv) {
    int ret = 0;

    if (parse_argv(argc,argv))
        return 1;

    // open input file/stdin
    if (source_stdin)
        source_fp = fdopen(dup(0/*stdin*/),"r");
    else
        source_fp = fopen(source_file.c_str(),"r");

    if (source_fp == NULL) {
        fprintf(stderr,"Unable to open input\n");
        return 1;
    }
    source_file_line = 0;

    // open output file/stdout
    if (dest_stdout)
        dest_fp = fdopen(dup(1/*stdout*/),"w");
    else
        dest_fp = fopen(dest_file.c_str(),"w");

    if (dest_fp == NULL) {
        fprintf(stderr,"Unable to open output\n");
        return 1;
    }
    fprintf(dest_fp,"/* auto-generated by opcc */\n");

    // begin
    OPCC_Symbol_Init();

    // parse!
    memset(line,0,sizeof(line));
    while (line_get()) {
        Token t = line_get_token();

        if (t.token == Token::Error) {
            fprintf_error_pos(stderr,t,"Error in ");
            ret = 1;
            break;
        }
        else if (t.token == Token::None) {
            continue;
        }
        else if (t.token == Token::Symbol) {
            /* no additional tokens on the same line should exist */
            t = line_get_token();
            if (t.token != Token::None)
                fprintf_error_pos(stderr,t,"Extra tokens ignored at ");

            if (!parse_symbol_block()) {
                ret = 1;
                break;
            }
        }
        else if (t.token == Token::Opcode) {
            /* no additional tokens on the same line should exist */
            t = line_get_token();
            if (t.token != Token::None)
                fprintf_error_pos(stderr,t,"Extra tokens ignored at ");

            if (!parse_opcode_block()) {
                ret = 1;
                break;
            }
        }
        else if (t.token == Token::Prefix) {
            /* no additional tokens on the same line should exist */
            t = line_get_token();
            if (t.token != Token::None)
                fprintf_error_pos(stderr,t,"Extra tokens ignored at ");

            if (!parse_prefix_block()) {
                ret = 1;
                break;
            }
        }
        else {
            fprintf_error_pos(stderr,t,"Unexpected token at ");
            ret = 1;
            break;
        }
    }

    // show symbols
    fprintf(dest_fp,"/* Symbols parsed: */\n");
    for (size_t si=0;si < Symbols.size();si++) Symbols[si].fprintf_debug(dest_fp);
    fprintf(dest_fp,"/* -------------------------------------- */\n");
    fprintf(dest_fp,"\n");

    // export symbols
    if (!symbol_file.empty()) {
        FILE *fp = fopen(symbol_file.c_str(),"w");
        if (fp == NULL) return 1;

        fprintf(fp,"/* auto-generated */\n");
        fprintf(fp,"\n");

        fprintf(fp,"enum {\n");
        for (size_t si=0;si < Symbols.size();si++) {
            fprintf(fp,"\tOPCODE_%s,",Symbols[si].enum_string.c_str());
            if ((si%5) == 0) fprintf(fp," /* =%lu */",(unsigned long)si);
            fprintf(fp,"\n");
        }
        fprintf(fp,"\tOPCODE__END /* =%lu */\n",(unsigned long)Symbols.size());
        fprintf(fp,"};\n");
        fprintf(fp,"\n");

        fprintf(fp,"static const char *OPCODE_display_names[OPCODE__END] = {\n");
        for (size_t si=0;si < Symbols.size();si++) {
            fprintf(fp,"\t\"%s\"",Symbols[si].opname_string.c_str());
            if ((si+1) != Symbols.size()) fprintf(fp,",");
            fprintf(fp," /* %s */",Symbols[si].enum_string.c_str());
            fprintf(fp,"\n");
        }
        fprintf(fp,"};\n");
        fprintf(fp,"\n");

        fclose(fp);
    }

    // show opcodes
    fprintf(dest_fp,"/* Opcodes parsed: */\n");
    for (size_t si=0;si < Opcodes.size();si++) Opcodes[si].fprintf_debug(dest_fp);
    fprintf(dest_fp,"/* -------------------------------------- */\n");
    fprintf(dest_fp,"\n");

    // show prefixes
    fprintf(dest_fp,"/* Prefixes parsed: */\n");
    for (size_t si=0;si < Prefixes.size();si++) Prefixes[si].fprintf_debug(dest_fp);
    fprintf(dest_fp,"/* -------------------------------------- */\n");
    fprintf(dest_fp,"\n");

    fclose(source_fp);
    fclose(dest_fp);
    return ret;
}

