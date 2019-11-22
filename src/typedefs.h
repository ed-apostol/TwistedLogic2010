/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

/* some basic definitions */
#ifndef INLINE
#ifdef _MSC_VER
#define INLINE __forceinline
#elif defined(__GNUC__)
#define INLINE __inline__ __attribute__((always_inline))
#else
#define INLINE inline
#endif
#endif

#ifndef _MSC_VER
#include <inttypes.h>
typedef int8_t		int8;
typedef uint8_t		uint8;
typedef int16_t		int16;
typedef uint16_t	uint16;
typedef int32_t		int32;
typedef uint32_t	uint32;
typedef int64_t		int64;
typedef uint64_t	uint64;
#else
typedef __int8				int8;
typedef unsigned __int8		uint8;
typedef __int16				int16;
typedef unsigned __int16	uint16;
typedef __int32				int32;
typedef unsigned __int32	uint32;
typedef __int64				int64;
typedef unsigned __int64	uint64;
#endif

/* the move structure */
typedef struct move_t {
    uint32 m;
    int32 s;
}move_t;

/* the book entry type */
typedef struct book_entry_t {
    uint64 key;
    uint16 move;
    uint16 weight;
    uint32 learn;
}book_entry_t;

/* the book type */
typedef struct book_t {
    FILE *bookfile;
    int64 size;
}book_t;

/* the pawn hash table entry type */
typedef struct pawn_entry_t {
    uint64 passedbits;
    uint32 hashlock;
    int32 opn;
    int32 end;
    int8 shield[2][3];
    int8 storm[2][3];
}pawn_entry_t;

/* the pawn hash table type */
typedef struct pawntable_t {
    pawn_entry_t *table;
    int64 size;
    uint64 mask;
}pawntable_t;

/* the trans table entry type */
typedef struct trans_entry_t {
    uint32 hashlock;
    uint32 data;
    int16 maxvalue;
    int16 minvalue;
    int8 depth;
    int8 movedepth;
    int8 maxdepth;
    int8 mindepth;
}trans_entry_t;

/* the trans table type */
typedef struct transtable_t {
    trans_entry_t *table;
    int64 size;
    uint64 mask;
    int32 date;
    uint32 used;
    int32 age[DATESIZE];
}transtable_t;

typedef struct uci_option_t {
    int32 try_hash;
    int32 try_book;
    int32 book_limit;
    int32 try_null;
    int32 try_iid;
    int32 iid_depth;
    int32 try_delta;
    int32 delta;
    int32 qdepth;
    int32 try_raz;
    int32 raz_margin;
    int32 raz_mul;
    int32 try_prunerazor;
    int32 try_lmr1;
    int32 lmr1_depth;
    int32 lmr1_num;
    int32 try_lmr2;
    int32 lmr2_depth;
    int32 lmr2_num;
    int32 try_pvlmr1;
    int32 pvlmr_num;
    int32 king_atks_mul;
    int32 pc_atks_mid_mul;
    int32 pc_atks_end_mul;
    int32 showcurrline;
    int32 time_buffer;
}uci_option_t;

/* the undo structure */
typedef struct undo_t {
    uint32 lastmove;
    uint32 castle;
    uint32 fifty;
    int32 epsq;
    int32 open[2];
    int32 end[2];
    int32 mat_summ[2];
    uint64 hash;
    uint64 phash;
}undo_t;

/* the eval info structure */
typedef struct eval_info_t {
    uint64 atkall[2];
    uint64 atkpawns[2];
    uint64 atkknights[2];
    uint64 atkbishops[2];
    uint64 atkrooks[2];
    uint64 atkqueens[2];
    uint64 atkkings[2];
    uint64 kingzone[2];
    uint64 kingadj[2];
    uint64 pawnattacks[2];
    uint64 pawns[2];
    int32 mid_score[2];
    int32 end_score[2];
    int32 atkcntpcs[2];
    int32 phase;
    pawn_entry_t *pawn_entry;
}eval_info_t;

/* the position structure */
typedef struct position_t {
    uint64 pawns;
    uint64 knights;
    uint64 bishops;
    uint64 rooks;
    uint64 queens;
    uint64 kings;
    uint64 color[2];
    uint64 occupied;
    int32 pieces[64];
    int32 kpos[2];
    int32 side;
    int32 ply;
    uint32 lastmove;
    int32 castle;
    int32 fifty;
    int32 epsq;
    int32 open[2];
    int32 end[2];
    int32 mat_summ[2];
    uint64 hash;
    uint64 phash;
    int32 sp;
    uint64 stack[MAXDATA];
    book_t book;
    pawntable_t pawn_table;
    transtable_t trans_table;
    uci_option_t *uci_options;
}position_t;

/* the material info structure */
typedef struct material_info_t {
    int16 value;
    uint16 flags;
}material_info_t;

/* the search data structure */
typedef struct search_info_t {
    int32 thinking_status;
    int32 depth_is_limited;
    int32 depth_limit;
    int32 moves_is_limited;
    int32 time_is_limited;
    int64 time_limit_max;
    int64 time_limit_abs;
    int32 node_is_limited;
    int64 node_limit;

    int64 nodes_since_poll;
    int64 nodes_between_polls;
    int64 nodes;

    int64 start_time;
    int64 last_time;
    int64 alloc_time;

    int32 last_last_value;
    int32 last_value;
    int32 best_value;
    int32 mate_found;
    int32 currmovenumber;
    int32 change;
    int32 research;
    int32 iteration;
    int32 maxplysearched;
    int32 legalmoves;

    uint32 bestmove;
    uint32 pondermove;

    uint32 singularExtension;

    uint32 history[1024];
    uint32 moves[MAXMOVES];
    uint32 transmove[MAXPLY];
    uint32 killer1[MAXPLY];
    uint32 killer2[MAXPLY];
    uint32 currline[MAXPLY];
}search_info_t;

/* uci options data structure */
typedef struct option_t {
    const char show;
    const char * var;
    const char * init;
    const char * type;
    const char * extra;
    const char * val;
}option_t;

/* the squares */
enum squarenames {
    a1 = 0, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

/* the phases used for lazy move generation */
enum movegen_phases {
    PH_NONE = 0,
    PH_EVASION,
    PH_TRANS,
    PH_ALL_CAPTURES,
    PH_ALL_CAPTURES_PURE,
    PH_GOOD_CAPTURES,
    PH_GOOD_CAPTURES_PURE,
    PH_BAD_CAPTURES,
    PH_KILLER_MOVES,
    PH_QUIET_MOVES,
    PH_QUIET_PASSEDPAWNS,
    PH_QUIET_PASSEDPAWNS_PURE,
    PH_NONTACTICAL_CHECKS,
    PH_NONTACTICAL_CHECKS_WIN,
    PH_NONTACTICAL_CHECKS_PURE,
    PH_END
};

/* the structure used in generating moves */
typedef struct sort_t {
    uint32 phase;
    uint32 transmove;
    uint32 killer1;
    uint32 killer2;
    int32 ply;
    int32 depth;
    int32 side;
    int32 pos;
    int32 size;
    int32 sizebad;
    uint64 pinned;
    uint64 target;
    move_t list[MAXMOVES];
    move_t bad[MAXMOVES / 2];
}sort_t;

//#  ifndef _MSC_VER
//typedef pthread_mutex_t mutex_t;
//#    define mutex_init(x, y) pthread_mutex_init(x, y)
//#    define mutex_lock(x) pthread_mutex_lock(x)
//#    define mutex_unlock(x) pthread_mutex_unlock(x)
//#    define mutex_destroy(x) pthread_mutex_destroy(x)
//#  else
//typedef CRITICAL_SECTION mutex_t;
//#    define mutex_init(x, y) InitializeCriticalSection(x)
//#    define mutex_lock(x) EnterCriticalSection(x)
//#    define mutex_unlock(x) LeaveCriticalSection(x)
//#    define mutex_destroy(x) DeleteCriticalSection(x)
//#  endif
//
//struct split_point_t {
//  position_t pos[MaxNumOfThreads];
//  search_stack_t sstack[MaxNumOfThreads][MAX_DEPTH];
//  search_stack_t *parent_sstack;
//  move_stack_t mstack[MaxNumOfThreads][MOVE_STACK_SIZE];
//  int ply, depth;
//  volatile int alpha, beta, bestvalue;
//  bool pvnode;
//  int master, slaves[MaxNumOfThreads];
//  mutex_t lock[1];
//  move_stack_t *current, *end;
//  volatile int moves;
//  volatile int cpus;
//};
//
//struct thread_t {
//  split_point_t *split_point;
//  volatile uint64 nodes;
//  volatile bool stop;
//  volatile bool running;
//  volatile bool idle;
//  volatile bool work_is_waiting;
//  volatile bool print_currline;
//};
