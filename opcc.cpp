
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
        Encoding,
        State,
        Symbol,
        AssignSign,     // =
        EqualsSign,     // ==
        NotEqualsSign   // !=
    };

    std::string         string,string2;
    std::string         error_source;
    uint64_t            value;
    enum TokenValue     token;
    long                line;
    unsigned int        pos;

    Token() : value(0), token(None), line(0), pos(0) {
    }
};

std::string         source_file;
long                source_file_line=1;
bool                source_stdin=false;
FILE*               source_fp=NULL;

std::string         dest_file;
bool                dest_stdout=true;
FILE*               dest_fp=NULL;

char                line[4096];

// symbols
const size_t        symbol_stop = 0;
const size_t        symbol_undefined = 1;

class OPCC_Symbol {
public:
    OPCC_Symbol() {
    }
    ~OPCC_Symbol() {
    }
public:
    std::string         enum_string;                // enum         (name used in enum for C++ code)
    std::string         opname_string;              // opname       (string displayed in debugger)
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

void OPCC_Symbol_Init(void) {
    SymbolsByEnum.clear();
    Symbols.clear();

    // auto-create "stop" symbol. this tells the state machine to stop.
    assert(Symbols.size() == symbol_stop);
    assert(OPCC_SymbolToIndex(OPCC_Symbol_New("stop")) == symbol_stop);

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
        while (*line_parse && !(*line_parse == ' ' || *line_parse == '\t' || *line_parse == '(' || *line_parse == ')' || *line_parse == ',')) line_parse++;

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
                r.token = Token::State;
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
            else if (tokenname == "prefix") {
                r.token = Token::Prefix;
            }
            else if (tokenname == "i") {
                r.token = Token::Immediate;
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
        r.value = (uint64_t)strtoull(line_parse,&line_parse,0);
    }
    else if (*line_parse == '/') { // Intel-ism for specifying the value of reg in mod/reg/rm   /0 /1 /2 /3 /4 /5 /6 /7
        line_parse++;
        if (!isdigit(*line_parse)) {
            r.token = Token::Error;
            r.error_source = start;
            return r;
        }
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

            fprintf(stderr,"enum: '%s'\n",t.string.c_str());
        }
        else if (t.token == Token::Opname) {
            /* enum   <name> */
            /* name is given in "string" */
            assert(!t.string.empty());

            fprintf(stderr,"opname: '%s'\n",t.string.c_str());
        }
        else if (t.token == Token::Display ||
                 t.token == Token::Writes ||
                 t.token == Token::Reads) {
            std::vector<Token> dsp;
            Token st;

            /* display    (opcode spec) */
            do {
                st = line_get_token();
                if (st.token == Token::Error || st.token == Token::None)
                    break;
                else if (st.token == Token::Opsize) { /* opsize(reg) */
                    fprintf(stderr,"opsize(token=%llu)\n",(unsigned long long)st.value);
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

            fprintf(stderr,"enum: '%s'\n",t.string.c_str());
        }
        else if (t.token == Token::Encoding) {
            // TODO
        }
        else if (t.token == Token::Opsize) {
            // TODO
        }
        else if (t.token == Token::Reg) {
            // TODO
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
            // TODO
        }
        else if (t.token == Token::PrefixName) {
            // TODO
        }
        else if (t.token == Token::State) {
            // TODO
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

    fclose(source_fp);
    fclose(dest_fp);
    return ret;
}

