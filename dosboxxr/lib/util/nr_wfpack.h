
#include <stdint.h>

// wftype: must match the size of a CPU register to do fixed point math without shifting.
// the rationale is that, from experience with x86 processors past the Pentium III,
// processors are faster at addition, subtraction, and multiplication than they are with
// logical operators like AND, OR, and shift. therefore, we can speed up horizontal
// interpolation by managing the whole + fractional parts in a way that eliminates
// the shift operation when reading the whole part. But to accomplish this, we have to
// define the base datatype as one the exact width of a CPU register.
#if defined(__i386__) // x86
typedef uint32_t                        nr_wftype;	// datatype the size of a CPU register
#elif defined(__x86_64__) // x86_64
typedef uint64_t                        nr_wftype;	// datatype the size of a CPU register
#else
typedef unsigned int                    nr_wftype;	// datatype the size of a CPU register
#endif

// number of bits in wftype
static const unsigned char				nr_wfbits = (sizeof(nr_wftype) * 8);

// what to mask the wftype by to obtain the upper 8 bits (i.e. checking whether the fractional part is effectively zero)
static const nr_wftype					nr_top8bitmask = ((nr_wftype)0xFF << (nr_wftype)(nr_wfbits - 8));

// what to mask the fraction by if comparing to "close enough" to 1:1
static const nr_wftype					nr_wfscsigbits = ((nr_wftype)0xFFFF << (nr_wftype)(nr_wfbits - 16));
static const nr_wftype					nr_wfscsigbits_add = ((nr_wftype)1 << (nr_wftype)(nr_wfbits - (16 + 1)));
static const nr_wftype					nr_wfhalfmax = ((nr_wftype)1 << (nr_wftype)(nr_wfbits - 1));

// wftype pack to contain whole & fractional parts
struct nr_wfpack {
public:
	nr_wftype			                w,f;
public:
	// add to THIS struct another struct. inline for performance!
	inline void add(const struct nr_wfpack &b) {
#if defined(__x86_64__) && defined(HAVE_GCC_ASM) // x86_64 optimized version because GCC can't figure out what we're trying to do with carry
        // f += b.f
        // w += b.w + carry
        __asm__ (   "addq	%2,%0\n"
                    "adcq	%3,%1\n" :
                    /* outputs */ "+rme" (f), "+rme" (w) : /* +rme to mean we read/modify/write the outputs */
                    /* inputs */ "ri" (b.f), "ri" (b.w) : /* make input params registers, add cannot take two memory operands */
                    /* clobbered */ "cc" /* modifies flags */);
#elif defined(__i386__) && defined(HAVE_GCC_ASM) // i686 optimized version because GCC can't figure out what we're trying to do with carry
        // f += b.f
        // w += b.w + carry
        __asm__ (   "addl	%2,%0\n"
                    "adcl	%3,%1\n" :
                    /* outputs */ "+rme" (f), "+rme" (w) : /* +rme to mean we read/modify/write the outputs */
                    /* inputs */ "ri" (b.f), "ri" (b.w) : /* make input params registers, add cannot take two memory operands */
                    /* clobbered */ "cc" /* modifies flags */);
#else // generic version. may be slower than optimized because GCC probably won't figure out we want i686 equiv. of add eax,ebx followed by adc ecx,edx, the carry part)
		nr_wftype ov = f;
		w += b.w+(((f += b.f) < ov)?1:0);
		// what we want: add fractions. add whole part, with carry from fractions.
#endif
	}
	// C++ convenience for add()
	inline struct nr_wfpack& operator+=(const struct nr_wfpack &b) {
		add(b);
		return *this;
	}
	inline struct nr_wfpack operator+(const struct nr_wfpack &b) {
		struct nr_wfpack r = *this;
		return (r += b);
	}

	// inline for performance!
	inline unsigned int mul_h(const unsigned int b,const unsigned int a) {
		// NTS: Despite the fact modern CPUs are faster at ADD than SHR, this is faster
		//      than typecasting to long long then shifting down. By quite a bit.
		return a + (unsigned int)((((int)b - (int)a) * (f >> (nr_wfbits - 8))) >> 8);
	}
};

