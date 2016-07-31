
#if HAVE_ENDIAN_H
/* nothing to do */
#else
# ifndef le16toh
#  define le16toh(x) (x)
# endif

# ifndef le32toh
#  define le32toh(x) (x)
# endif
#endif

