/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

#ifdef DEBUG
#  define ASSERT(a) { if (!(a)) \
        Print(4, "file \"%s\", line %d, assertion \"" #a "\" failed\n",__FILE__,__LINE__);}
#else
#  define ASSERT(a)
#endif

#define BOOL                unsigned int
#define TRUE                1
#define FALSE               0

#define MAXPLY              128
#define MAXDATA             1024
#define MAXMOVES            256
#define INF                 10000
#define MAXHIST             16384

#define WHITE               0
#define BLACK               1
#define LEFT                0
#define RIGHT               1

#define EMPTY               0
#define PAWN                1
#define KNIGHT              2
#define BISHOP              3
#define ROOK                4
#define QUEEN               5
#define KING                6

#define CASTLE              14
#define TWOFORWARD          17
#define PROMOTE             33
#define ENPASSANT           9

#define WCKS                1
#define WCQS                2
#define BCKS                4
#define BCQS                8

#define NOMASK              0

#define STARTPOS  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define SQRANK(x)                ((x) >> 3)
#define SQFILE(x)                ((x) & 7)
#define MAX(a,b)                 (((a)>(b))?(a):(b))
#define MIN(a,b)                 (((a)<(b))?(a):(b))
#define DISTANCE(a,b)             MAX((abs((SQRANK(a))-(SQRANK(b)))),(abs((SQFILE(a))-(SQFILE(b)))))
#define PAWN_RANK(f,c)           (((c)==BLACK)?(7-SQRANK(f)):(SQRANK(f)))
#define PAWN_MOVE_INC(c)         ((c)?-8:8)
#define PAWN_PROMOTE(sq,c)       (SQFILE(sq) + ((c==BLACK)?0:56))
#define SQUARE_COLOUR(square)    (((square)^((square)>>3))&1)
#define FILE_MIRROR(square)      ((square)^07)
#define RANK_MIRROR(square)      ((square)^070)
#define COLOUR_IS_WHITE(a)       ((a) == WHITE)
#define COLOUR_IS_BLACK(a)       ((a) == BLACK)
#define MAX_MATERIAL              2*3*3*3*9
#define                           U64(u) (u##ULL)

#define PST(c,p,s,l)    (PcSqTb[(((c)<<10)|((p)<<7)|((s)<<1)|(l))])
#define MIDGAME 0
#define ENDGAME 1

#define ABORTED             0
#define THINKING            1
#define PONDERING           2
#define ANALYSING           3

#define DATESIZE            16

#define REDUCED             4

#define LOCK(x)             (uint32)((x)>>32)
#define KEY(x)              (uint32)(x)
#define SETHASH(l,k)        (uint64)(((l)<<32)|(k))

#define TEST(f, b) (BitMask[f] & (b))
#define RAYMACRO { \
if (TEST(i, board)) { \
if (b) { setBit(i, &xray); break; } \
    else { setBit(i, &occ); b = 1; } } \
if (!b) setBit(i, &free); \
                 }
