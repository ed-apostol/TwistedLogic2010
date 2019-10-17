/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

/* init.c */
extern void initPST(void);
extern void initArr(void);

/* utils.c */
extern int strncasecmp(const char *s1, const char *s2, size_t length);
extern void Print(int vb, char *fmt, ...);
extern void displayBit(uint64 a, int x);
extern char *bit2Str(uint64 n);
extern char *move2Str(int m);
extern char *sq2Str(int sq);
extern void displayBoard(const position_t *pos, int x);
extern int getPiece(const position_t *pos, uint32 sq);
extern int getColor(const position_t *pos, uint32 sq);
extern uint64 getTime(void);
extern uint32 parseMove(sort_t *sort, char *s);
extern int biosKey(void);
extern uint32 reps(const position_t *pos, int n);
extern int getDirIndex(int d);

/* bitutils.c */
extern uint64 rand64(void);
extern __inline uint32 bitCnt(uint64 x);
extern __inline uint32 getFirstBit(uint64 b);
extern __inline uint32 popFirstBit(uint64 *b);
extern uint64 fillUp(uint64 b);
extern uint64 fillDown(uint64 b);
extern uint64 fillUp2(uint64 b);
extern uint64 fillDown2(uint64 b);
extern uint64 shiftLeft(uint64 b, uint32 i);
extern uint64 shiftRight(uint64 b, uint32 i);

/* attacks.c */
extern int key000(uint64 b, int f);
extern int key090(uint64 b, int f);
extern int keyDiag(uint64 _b);
extern int key045(uint64 b, int f);
extern int key135(uint64 b, int f);
extern uint64 getLowestBit(uint64 bb);
extern uint64 _occ_free_board(int bc, int del, uint64 free);
extern void _init_rays(uint64* rays, uint64(*rayFunc) (int, uint64, int), int(*key)(uint64, int));
extern void setBit(int f, uint64 *b);
extern uint64 _rook0(int f, uint64 board, int t);
extern uint64 _rook90(int f, uint64 board, int t);
extern uint64 _bishop45(int f, uint64 board, int t);
extern uint64 _bishop135(int f, uint64 board, int t);
extern uint64 rankAttacks(uint64 occ, int sq);
extern uint64 fileAttacks(uint64 occ, int sq);
extern uint64 diagonalAttacks(uint64 occ, int sq);
extern uint64 antiDiagAttacks(uint64 occ, int sq);
extern uint64 rankAttacksX(uint64 occ, int sq);
extern uint64 fileAttacksX(uint64 occ, int sq);
extern uint64 diagonalAttacksX(uint64 occ, int sq);
extern uint64 antiDiagAttacksX(uint64 occ, int sq);
extern uint64 bishopAttacksBBX(uint32 from, uint64 occ);
extern uint64 rookAttacksBBX(uint32 from, uint64 occ);
extern uint64 bishopAttacksBB(uint32 from, uint64 occ);
extern uint64 rookAttacksBB(uint32 from, uint64 occ);
extern uint64 queenAttacksBB(uint32 from, uint64 occ);
extern uint32 isAtt(const position_t *pos, uint32 color, uint64 target);
extern uint32 kingIsInCheck(const position_t *pos);
extern uint64 pinnedPieces(const position_t *pos, uint32 c);
extern uint64 discoveredCheckCandidates(const position_t *pos, uint32 c);
extern uint32 moveIsLegal(const position_t *pos, uint32 move, uint64 pinned, uint32 incheck);
extern uint32 moveIsCheck(const position_t *pos, uint32 m, uint64 dcc);
extern uint64 attackingPiecesAll(const position_t *pos, uint32 sq);
extern uint64 attackingPiecesSide(const position_t *pos, uint32 sq, uint32 side);
extern uint64 behindFigure(uint64 QR, uint64 QB, uint64 occupied, uint32 from, int dir);
extern int moveAttacksSquare(const position_t *pos, uint32 move, uint32 sq);
extern int swap(const position_t *pos, uint32 m);

/* material.h */
extern  void initMaterial(void);

/* movegen.c */
extern uint32 GenOneForward(uint32 f, uint32 t);
extern uint32 GenTwoForward(uint32 f, uint32 t);
extern uint32 GenPromote(uint32 f, uint32 t, uint32 r, uint32 c);
extern uint32 GenPromoteStraight(uint32 f, uint32 t, uint32 r);
extern uint32 GenEnPassant(uint32 f, uint32 t);
extern uint32 GenPawnMove(uint32 f, uint32 t, uint32 c);
extern uint32 GenKnightMove(uint32 f, uint32 t, uint32 c);
extern uint32 GenBishopMove(uint32 f, uint32 t, uint32 c);
extern uint32 GenRookMove(uint32 f, uint32 t, uint32 c);
extern uint32 GenQueenMove(uint32 f, uint32 t, uint32 c);
extern uint32 GenKingMove(uint32 f, uint32 t, uint32 c);
extern uint32 GenWhiteOO(void);
extern uint32 GenWhiteOOO(void);
extern uint32 GenBlackOO(void);
extern uint32 GenBlackOOO(void);
extern int moveFrom(uint32 m);
extern int  moveTo(uint32 m);
extern int  movePiece(uint32 m);
extern int  moveAction(uint32 m);
extern int  moveCapture(uint32 m);
extern int  moveRemoval(uint32 m);
extern int  movePromote(uint32 m);
extern uint32 isCastle(uint32 m);
extern uint32 isPawn2Forward(uint32 m);
extern uint32 isPromote(uint32 m);
extern uint32 isEnPassant(uint32 m);
extern void genLegal(const position_t *pos, sort_t *sort);
extern void genNonCaptures(const position_t *pos, sort_t *sort);
extern void genCaptures(const position_t *pos, sort_t *sort);
extern void genEvasions(const position_t *pos, sort_t *sort);
extern void genQChecks(const position_t *pos, sort_t *sort);
extern uint32 genMoveIfLegal(const position_t *pos, uint32 move, uint64 pinned);

/* position.c */
extern void unmakeNullMove(position_t *pos, undo_t *undo);
extern void makeNullMove(position_t *pos, undo_t *undo);
extern void unmakeMove(position_t *pos, undo_t *undo);
extern void makeMove(position_t *pos, undo_t *undo, uint32 m);
extern void setPosition(position_t *pos, const char *fen);
extern char *positionToFEN(const position_t *pos);

/* eval.c */
extern int scale(int min, int max, int r);
extern int kingPasser(const position_t *pos, int square, int color);
extern int unstoppablePasser(const position_t *pos, int square, int color);
extern int eval(const position_t *pos);

/* trans.c */
extern int transMove(trans_entry_t * te);
extern int transMateThreat(trans_entry_t * te);
extern int transDate(trans_entry_t * te);
extern int transDepth(trans_entry_t * te);
extern int transMovedepth(trans_entry_t * te);
extern int transMindepth(trans_entry_t * te);
extern int transMaxdepth(trans_entry_t * te);
extern uint32 transHashlock(trans_entry_t * te);
extern int transMinvalue(trans_entry_t * te);
extern int transMaxvalue(trans_entry_t * te);
extern void transSetMove(trans_entry_t * te, uint32 move);
extern void transSetMateThreat(trans_entry_t * te, uint32 mthreat);
extern void transSetDate(trans_entry_t * te, uint32 date);
extern void transSetDepth(trans_entry_t * te, uint32 depth);
extern void transSetMovedepth(trans_entry_t * te, uint32 movedepth);
extern void transSetMindepth(trans_entry_t * te, uint32 mindepth);
extern void transSetMaxdepth(trans_entry_t * te, uint32 maxdepth);
extern void transSetHashlock(trans_entry_t * te, uint32 hashlock);
extern void transSetMinvalue(trans_entry_t * te, int minvalue);
extern void transSetMaxvalue(trans_entry_t * te, int maxvalue);
extern void initTrans(position_t *pos, uint32 target);
extern trans_entry_t *transProbe(position_t *pos);
extern void transStore(position_t *pos, uint32 bm, int d, int min, int max, int mt);
extern void transNewDate(position_t *pos, int date);
extern void transClear(position_t *pos);
extern void initPawnTab(position_t *pos, int size);
extern void pawnTabClear(position_t *pos);

/* movepicker.c */
extern void initSortPhases();
extern void sortInit(const position_t *pos, sort_t *sort, search_info_t *si, uint64 pinned, uint64 target, int depth, int type);
extern move_t *getMove(sort_t *sort);
extern move_t *sortNext(position_t *pos, sort_t *sort, search_info_t *si);
extern uint32 captureIsGood(const position_t *pos, uint32 m);
extern void scoreCaptures(sort_t *sort);
extern void scoreCapturesPure(sort_t *sort);
extern void scoreNonCaptures(sort_t *sort, search_info_t *si);
extern void scoreAll(const position_t *pos, sort_t *sort, search_info_t *si);
extern void scoreAllQ(sort_t *sort, search_info_t *si);
extern void scoreRoot(sort_t *sort);

/* search.c */
extern void ponderHit(search_info_t *si);
extern void check4Input(position_t *pos, search_info_t *si);
extern void initNode(position_t *pos, search_info_t *si);
extern int moveIsTactical(uint32 m);
extern int simpleStalemate(const position_t *pos);
extern void pvFill(position_t *pos, uint32 pv[]);
extern void updatePV(uint32 m, uint32 ply, uint32 *pv, uint32 *newpv);
extern void displayPV(const position_t *pos, search_info_t *si, uint32 *pv, int depth, int alpha, int beta, int score);
extern int historyIndex(uint32 side, uint32 move);
extern int extRoot(const position_t * pos, search_info_t * si, move_t * move, int depth, int incheck, int single_reply);
extern int extPV(const position_t *pos, search_info_t *si, move_t *move, uint64 dcc, int depth, int single_reply);
extern int extSimple(const position_t *pos, search_info_t *si, move_t *move, int depth, int incheck, int single_reply);
extern int qSearchCheck(position_t *pos, search_info_t *si, int alpha, int beta, int depth, uint32 oldPV[]);
extern int qSearch(position_t *pos, search_info_t *si, int alpha, int beta, int depth, uint32 oldPV[]);
extern int searchEvasion(position_t *pos, search_info_t *si, int beta, int depth, uint32 oldPV[]);
extern int searchZero(position_t *pos, search_info_t *si, int beta, int depth, uint32 oldPV[], BOOL donull);
extern int searchPV(position_t *pos, search_info_t *si, int alpha, int beta, int depth, int inCheck, uint32 oldPV[]);
extern void initGameOptions(uci_option_t *opt);
extern void getBestMove(position_t *pos, search_info_t *si);

/* debug.c */
extern int squareIsOk(int s);
extern int colorIsOk(int c);
extern int moveIsOk(int m);
extern int valueIsOk(int v);
extern int rankIsOk(int r);
extern void flipPosition(const position_t *pos, position_t *clone);
extern int evalSymmetryIsOk(const position_t *pos);
extern uint64 pawnHashRecalc(const position_t *pos);
extern void positionIsOk(const position_t *pos);

/* tests.c */
extern void perft(position_t *pos, int maxply, uint64 nodesx[]);
extern int perftDivide(position_t *pos, uint32 depth, uint32 maxply);
extern void runPerft(position_t *pos, int maxply);
extern void runPerftDivide(position_t *pos, uint32 maxply);
extern void nonUCI(position_t *pos);

/* uci.c */
extern option_t *optionFind(const char var[]);
extern void *myMalloc(int size);
extern void myFree(void *address);
extern char *myStrdup(const char string[]);
extern void myStringSet(const char **variable, const char string[]);
extern int optionSet(const char var[], const char val[]);
extern const char *optionGet(const char var[]);
extern int optionGetBool(const char var[]);
extern int optionGetInt(const char var[]);
extern const char *optionGetString(const char var[]);
extern void printUciOptions(void);
extern void uciSetOption(position_t *pos, char string[]);
extern void initOption(void);
extern void uciParseSearchmoves(sort_t *ml, char *str, uint32 moves[]);
extern void uciGo(position_t *pos, char *options);
extern void uciStart(void);
extern void uciSetPosition(position_t *pos, char *str);

/* main.c */
extern void quit(position_t *pos);
extern int main(void);
