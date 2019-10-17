/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

static option_t Option[] = {
    { TRUE, "Hash", "32", "spin", "min 1 max 4096", NULL },
    { TRUE, "Pawn Hash", "16", "spin", "min 1 max 32", NULL },
    { TRUE, "Ponder", "false", "check", "", NULL },
    { TRUE, "OwnBook", "false", "check", "", NULL },
    { TRUE, "Book File", "twistedbook.bin", "string", "", NULL },
    { TRUE, "Book Move Limit", "128", "spin", "min 1 max 256", NULL },
    { TRUE, "Display Book Moves", "false", "check", "", NULL },
    { TRUE, "Time Buffer", "1000", "spin", "min 0 max 10000", NULL },
    { TRUE, "Transposition Table", "true", "check", "", NULL },
    { TRUE, "Null Move Pruning", "true", "check", "", NULL },
    { TRUE, "Internal Iterative Deepening", "true", "check", "", NULL },
    { TRUE, "Internal Iterative Deepening Depth", "5", "spin", "min 1 max 15", NULL },
    { TRUE, "Delta Pruning", "true", "check", "", NULL },
    { TRUE, "Delta Pruning Threshold", "50", "spin", "min 0 max 500", NULL },
    { TRUE, "Quiescence Check Depth", "5", "spin", "min 1 max 15", NULL },
    { TRUE, "Zero Pruning", "true", "check", "", NULL },
    { TRUE, "Zero Pruning Threshold", "25", "spin", "min 0 max 500", NULL },
    { TRUE, "Zero Pruning Multiplier", "25", "spin", "min 0 max 500", NULL },
    { TRUE, "Razor Pruning", "false", "check", "", NULL },
    { TRUE, "Speculation PV", "true", "check", "", NULL },
    { TRUE, "Speculation PV Level", "12", "spin", "min 0 max 30", NULL },
    { TRUE, "Speculation", "true", "check", "", NULL },
    { TRUE, "Speculation Base", "3", "spin", "min 2 max 15", NULL },
    { TRUE, "Speculation Level", "4", "spin", "min 2 max 30", NULL },
    { TRUE, "Extended Speculation", "true", "check", "", NULL },
    { TRUE, "Extended Speculation Base", "4", "spin", "min 3 max 15", NULL },
    { TRUE, "Extended Speculation Level", "4", "spin", "min 2 max 30", NULL },
    { TRUE, "King Attacks Aggression Level", "5", "spin", "min 0 max 20", NULL },
    { TRUE, "Piece Attacks Midgame", "2", "spin", "min 0 max 20", NULL },
    { TRUE, "Piece Attacks Endgame", "2", "spin", "min 0 max 20", NULL },
    { TRUE, "UCI_ShowCurrLine", "false", "check", "", NULL },
    { TRUE, "UCI_EngineAbout", "Twisted Logic by Edsel Apostol", "string", "", NULL },
    { FALSE, NULL, NULL, NULL, NULL, NULL },
};

/* the precomputed static piece 64 bit attacks */
static uint64 KnightMoves[64];
static uint64 KingMoves[64];
static uint64 PawnCaps[64][2];
static uint64 PawnMoves[64][2];
static uint64 PawnMoves2[64][2];

/* used by the Olithink style magic move generation */
static uint64 RayMagic[0x10000];

/* the precomputed material values table and the flags table */
static material_info_t MaterialTable[MAX_MATERIAL][MAX_MATERIAL];

/* the rank and file mask indexed by square */
static uint64 RankMask[64];
static uint64 FileMask[64];

/* the 64 bit attacks on 8 different ray directions */
static uint64 DirA[8][64];

/* the 64 bit attacks between two squares of a valid ray direction */
static uint64 InBetween[64][64];

/* this is used in lazy move generation */
static int MoveGenPhase[128];
static int MoveGenPhaseEvasion;
static int MoveGenPhaseStandard;
static int MoveGenPhaseSelective;
static int MoveGenPhaseSelectiveTactical;
static int MoveGenPhaseQuiescence;
static int MoveGenPhaseQuiescenceAndChecks;

/* contains the delta of possible piece moves between two squares,
zero otherwise */
static int Direction[64][64];

/* used for pre-computed piece-square table */
static int PcSqTb[2048];

/* used in updating the castle status of the position */
static int CastleMask[64];

/* used as initial king penalty */
static int KingPosPenalty[2][64];

/* this is used for pawn shelter and pawn storm */
static uint64 PassedMask[2][64];
static uint64 PawnShelterMask1[2][3];
static uint64 PawnShelterMask2[2][3];
static uint64 PawnShelterMask3[2][3];

/* used in debugging, etc.. */
FILE *logfile;
FILE *errfile;
FILE *dumpfile;
