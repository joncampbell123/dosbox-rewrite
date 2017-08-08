
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <map>

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

OPCC_Symbol &OPCC_Symbol_New(const char *enum_name) {
    const size_t index = Symbols.size();

    Symbols.resize(index+1);

    OPCC_Symbol &v = Symbols[index];
    v.enum_string = enum_name;

    assert(SymbolsByEnum.find(enum_name) == SymbolsByEnum.end());
    SymbolsByEnum[enum_name] = index;

    return v;
}

OPCC_Symbol &OPCC_Symbol_LookupOrCreate(const char *s) {
    auto i = SymbolsByEnum.find(s);

    if (i != SymbolsByEnum.end()) {
        assert(i->second < Symbols.size());
        return Symbols[i->second];
    }

    return OPCC_Symbol_New(s);
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
    char *t = strrchr(s,';');
    if (t == NULL) return;
    *t = 0; // SNIP
}

static bool line_get(void) {
    if (line == NULL) return false;
    if (fgets(line,sizeof(line)-1,source_fp) == NULL) return false;
    source_file_line++;

    opcc_chomp(line);
    opcc_eat_comments(line);
    return true;
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

int main(int argc,char **argv) {
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
    }

    fclose(source_fp);
    fclose(dest_fp);
    return 0;
}

