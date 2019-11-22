/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

#ifdef VERSION64BIT
#ifdef _MSC_VER
#if defined(_WIN64) || defined(__LP64__)
#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse64)
#define USING_INTRINSICS
#endif
#elif defined(__GNUC__) && defined(__LP64__)
static INLINE unsigned char _BitScanForward64(unsigned int* const Index, const U64 Mask)
{
    U64 Ret;
    __asm__
    (
        "bsfq %[Mask], %[Ret]"
        :[Ret] "=r" (Ret)
        : [Mask] "mr" (Mask)
    );
    *Index = (unsigned int)Ret;
    return Mask ? 1 : 0;
}
static INLINE unsigned char _BitScanReverse64(unsigned int* const Index, const U64 Mask)
{
    U64 Ret;
    __asm__
    (
        "bsrq %[Mask], %[Ret]"
        :[Ret] "=r" (Ret)
        : [Mask] "mr" (Mask)
    );
    *Index = (unsigned int)Ret;
    return Mask ? 1 : 0;
}
#define USING_INTRINSICS
#endif
/* returns the least significant square from the 64 bitfield */
INLINE uint32 getFirstBit(uint64 bb) {
    int index = 0;
    _BitScanForward64(&index, bb);
    return index;
}

/* returns the least significant square and clears it from the 64 bitfield */
INLINE uint32 popFirstBit(uint64 *b) {
    int index = 0;
    _BitScanForward64(&index, *b);
    *b &= (*b - 1);
    return index;
}
/* this count the number of bits in a given bitfield,
it is using a SWAR algorithm I think */
INLINE uint32 bitCnt(uint64 x) {
    x -= (x >> 1) & 0x5555555555555555ULL;
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
    return (x * 0x0101010101010101ULL) >> 56;
}
#else
/* this count the number of bits in a given bitfield,
it is using a SWAR algorithm I think */
INLINE uint32 bitCnt(uint64 bb) {
    uint32 w = (uint32)(bb >> 32);
    uint32 v = (uint32)bb;

    v = v - ((v >> 1) & 0x55555555);
    w = w - ((w >> 1) & 0x55555555);
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
    w = (w & 0x33333333) + ((w >> 2) & 0x33333333);
    v = (v + (v >> 4)) & 0x0F0F0F0F;
    w = (w + (w >> 4)) & 0x0F0F0F0F;
    v = ((v + w) * 0x01010101) >> 24;
    return v;
}
/* returns the least significant square from the 64 bitfield */
INLINE uint32 getFirstBit(uint64 b) {
    uint32 folded;
    b ^= (b - 1);
    folded = ((uint32)b) ^ ((uint32)(b >> 32));
    return FoldedTable[(folded * 0x78291ACF) >> 26];
}

/* returns the least significant square and clears it from the 64 bitfield */
INLINE uint32 popFirstBit(uint64 *b) {
    uint64 bb = *b ^ (*b - 1);
    uint32 folded = ((uint32)bb ^ (uint32)(bb >> 32));
    *b &= (*b - 1);
    return FoldedTable[(folded * 0x78291ACF) >> 26];
}
#endif

/* returns a bitboard with all bits above b filled up (excluding b) */
uint64 fillUp(uint64 b) {
    b |= b << 8;
    b |= b << 16;
    return (b | (b << 32)) << 8;
}

/* returns a bitboard with all bits below b filled down (excluding b) */
uint64 fillDown(uint64 b) {
    b |= b >> 8;
    b |= b >> 16;
    return (b | (b >> 32)) >> 8;
}

/* returns a bitboard with all bits above b filled up (including b) */
uint64 fillUp2(uint64 b) {
    b |= b << 8;
    b |= b << 16;
    return (b | (b << 32));
}

/* returns a bitboard with all bits below b filled down (including b) */
uint64 fillDown2(uint64 b) {
    b |= b >> 8;
    b |= b >> 16;
    return (b | (b >> 32));
}

// shift the parameter b with i places to the left
uint64 shiftLeft(uint64 b, uint32 i) { return (b << i); }

// shift the parameter b with i places to the right
uint64 shiftRight(uint64 b, uint32 i) { return (b >> i); }
