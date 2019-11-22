/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

/*
// 00000000 00000000 00000000 00111111 = from square     = bits 1-6
// 00000000 00000000 00001111 11000000 = to square       = bits 7-12
// 00000000 00000000 01110000 00000000 = piece           = bits 13-15
// 00000000 00000000 10000000 00000000 = castle	         = bit  16
// 00000000 00000001 00000000 00000000 = pawn 2 forward  = bit  17
// 00000000 00000010 00000000 00000000 = promotion       = bit  18
// 00000000 00011100 00000000 00000000 = capture piece   = bits 19-21
// 00000000 00100000 00000000 00000000 = en passant      = bit  22
// 00000001 11000000 00000000 00000000 = promotion piece = bits 23-25
// 11111110 00000000 00000000 00000000 = reserve bits    = bits 26-32
*/
/* utilities for move generation */
uint32 GenOneForward(uint32 f, uint32 t) {
    return (f | (t << 6) | (PAWN << 12));
}
uint32 GenTwoForward(uint32 f, uint32 t) {
    return (f | (t << 6) | (PAWN << 12) | (1 << 16));
}
uint32 GenPromote(uint32 f, uint32 t, uint32 r, uint32 c) {
    return (f | (t << 6) | (PAWN << 12) | (c << 18) | (r << 22) | (1 << 17));
}
uint32 GenPromoteStraight(uint32 f, uint32 t, uint32 r) {
    return (f | (t << 6) | (PAWN << 12) | (r << 22) | (1 << 17));
}
uint32 GenEnPassant(uint32 f, uint32 t) {
    return (f | (t << 6) | (PAWN << 12) | (PAWN << 18) | (1 << 21));
}
uint32 GenPawnMove(uint32 f, uint32 t, uint32 c) {
    return (f | (t << 6) | (PAWN << 12) | (c << 18));
}
uint32 GenKnightMove(uint32 f, uint32 t, uint32 c) {
    return (f | (t << 6) | (KNIGHT << 12) | (c << 18));
}
uint32 GenBishopMove(uint32 f, uint32 t, uint32 c) {
    return (f | (t << 6) | (BISHOP << 12) | (c << 18));
}
uint32 GenRookMove(uint32 f, uint32 t, uint32 c) {
    return (f | (t << 6) | (ROOK << 12) | (c << 18));
}
uint32 GenQueenMove(uint32 f, uint32 t, uint32 c) {
    return (f | (t << 6) | (QUEEN << 12) | (c << 18));
}
uint32 GenKingMove(uint32 f, uint32 t, uint32 c) {
    return (f | (t << 6) | (KING << 12) | (c << 18));
}
uint32 GenWhiteOO() {
    return (e1 | (g1 << 6) | (KING << 12) | (1 << 15));
}
uint32 GenWhiteOOO() {
    return (e1 | (c1 << 6) | (KING << 12) | (1 << 15));
}
uint32 GenBlackOO() {
    return (e8 | (g8 << 6) | (KING << 12) | (1 << 15));
}
uint32 GenBlackOOO() {
    return (e8 | (c8 << 6) | (KING << 12) | (1 << 15));
}
/* Get from square */
int moveFrom(uint32 m) {
    return (63 & (m));
}
/* Get to square */
int moveTo(uint32 m) {
    return (63 & (m >> 6));
}
/* Get the piece moving */
int movePiece(uint32 m) {
    return  (7 & (m >> 12));
}
/* Get action to do */
int moveAction(uint32 m) {
    return (63 & (m >> 12));
}
/* Get the capture piece */
int moveCapture(uint32 m) {
    return  (7 & (m >> 18));
}
/* Get removal to be done */
int moveRemoval(uint32 m) {
    return (15 & (m >> 18));
}
/* Get promote value */
int movePromote(uint32 m) {
    return  (7 & (m >> 22));
}

uint32 isCastle(uint32 m) {
    return ((m >> 15) & 1);
}
uint32 isPawn2Forward(uint32 m) {
    return ((m >> 16) & 1);
}
uint32 isPromote(uint32 m) {
    return ((m >> 17) & 1);
}
uint32 isEnPassant(uint32 m) {
    return ((m >> 21) & 1);
}

/* the move generator, this generates all legal moves*/
void genLegal(const position_t *pos, sort_t *sort) {
    sort_t mlt;
    uint64 pinned;

    ASSERT(pos != NULL);
    ASSERT(sort != NULL);

    if (kingIsInCheck(pos)) genEvasions(pos, sort);
    else {
        pinned = pinnedPieces(pos, pos->side);
        sort->size = 0;
        mlt.target = pos->color[pos->side ^ 1] & ~pos->kings;
        genCaptures(pos, &mlt);
        for (mlt.pos = 0; mlt.pos < mlt.size; mlt.pos++) {
            if (!moveIsLegal(pos, mlt.list[mlt.pos].m, pinned, FALSE)) continue;
            sort->list[sort->size++] = mlt.list[mlt.pos];
        }
        genNonCaptures(pos, &mlt);
        for (mlt.pos = 0; mlt.pos < mlt.size; mlt.pos++) {
            if (!moveIsLegal(pos, mlt.list[mlt.pos].m, pinned, FALSE)) continue;
            sort->list[sort->size++] = mlt.list[mlt.pos];
        }
    }
}

/* this generates non tactical non checking passed pawn moves only */
void genPassedPawnMoves(const position_t *pos, sort_t *sort) {
    static const int Shift[] = { 9, 7 };
    uint64 pc_bits, mv_bits, pawns, xpawns, mask, dcc;
    int from, to;

    ASSERT(pos != NULL);
    ASSERT(sort != NULL);

    sort->size = 0;
    pawns = pos->pawns & pos->color[pos->side];
    xpawns = pos->pawns & pos->color[pos->side ^ 1];
    dcc = discoveredCheckCandidates(pos, pos->side);
    mask = ~Rank8ByColorBB[pos->side] & ~((*FillPtr2[pos->side ^ 1])((*ShiftPtr[pos->side ^ 1])(pos->pawns, 8)))
        & ~(*FillPtr2[pos->side ^ 1])(((*ShiftPtr[pos->side ^ 1])(xpawns, Shift[pos->side ^ 1]) & ~FileABB)
            | ((*ShiftPtr[pos->side ^ 1])(xpawns, Shift[pos->side]) & ~FileHBB))
        & ~PawnCaps[pos->kpos[pos->side ^ 1]][pos->side ^ 1];

    mv_bits = mask & (*ShiftPtr[pos->side])(pawns, 8) & ~pos->occupied;
    while (mv_bits) {
        to = popFirstBit(&mv_bits);
        pc_bits = (PawnMoves[to][pos->side ^ 1] & pawns) & ~dcc;
        ASSERT(bitCnt(pc_bits) <= 1);
        while (pc_bits) {
            from = popFirstBit(&pc_bits);
            sort->list[sort->size++].m = GenOneForward(from, to);
        }
    }
    mv_bits = mask & Rank4ByColorBB[pos->side] & (*ShiftPtr[pos->side])(pawns, 16)
        & (*ShiftPtr[pos->side])(~pos->occupied, 8) & ~pos->occupied;
    while (mv_bits) {
        to = popFirstBit(&mv_bits);
        pc_bits = pawns & Rank2ByColorBB[pos->side] & FileMask[to] & ~dcc;
        ASSERT(bitCnt(pc_bits) <= 1);
        while (pc_bits) {
            from = popFirstBit(&pc_bits);
            sort->list[sort->size++].m = GenTwoForward(from, to);
        }
    }
}

/* the move generator, this generates all pseudo-legal non tactical moves,
castling is generated legally */
void genNonCaptures(const position_t *pos, sort_t *sort) {
    int from, to;
    uint64 pc_bits, mv_bits;
    uint64 occupied = pos->occupied;
    uint64 allies = pos->color[pos->side];
    uint64 mask = ~occupied;

    ASSERT(pos != NULL);
    ASSERT(sort != NULL);

    sort->size = 0;

    if (pos->side == BLACK) {
        if ((pos->castle&BCKS) && (!(occupied&(F8 | G8)))) {
            if (!isAtt(pos, pos->side ^ 1, E8 | F8 | G8))
                sort->list[sort->size++].m = GenBlackOO();
        }
        if ((pos->castle&BCQS) && (!(occupied&(B8 | C8 | D8)))) {
            if (!isAtt(pos, pos->side ^ 1, E8 | D8 | C8))
                sort->list[sort->size++].m = GenBlackOOO();
        }
    }
    else {
        if ((pos->castle&WCKS) && (!(occupied&(F1 | G1)))) {
            if (!isAtt(pos, pos->side ^ 1, E1 | F1 | G1))
                sort->list[sort->size++].m = GenWhiteOO();
        }
        if ((pos->castle&WCQS) && (!(occupied&(B1 | C1 | D1)))) {
            if (!isAtt(pos, pos->side ^ 1, E1 | D1 | C1))
                sort->list[sort->size++].m = GenWhiteOOO();
        }
    }
    pc_bits = pos->knights & allies;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = KnightMoves[from] & mask;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenKnightMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->bishops & allies;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = bishopAttacksBB(from, occupied) & mask;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenBishopMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->rooks & allies;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = rookAttacksBB(from, occupied) & mask;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenRookMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->queens & allies;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = queenAttacksBB(from, occupied) & mask;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenQueenMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->kings & allies;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = KingMoves[from] & mask;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenKingMove(from, to, getPiece(pos, to));
        }
    }
    /* pawn moves 1 forward, no promotions */
    if (pos->side == WHITE) pc_bits = (pos->pawns & allies & ~Rank7ByColorBB[pos->side]) & ((~occupied) >> 8);
    else pc_bits = (pos->pawns & allies & ~Rank7ByColorBB[pos->side]) & ((~occupied) << 8);
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = PawnMoves[from][pos->side];
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenOneForward(from, to);
        }
    }
    /* pawn moves 2 forward */
    if (pos->side == WHITE) pc_bits = (pos->pawns & allies & Rank7ByColorBB[BLACK]) & ((~occupied) >> 8) & ((~occupied) >> 16);
    else pc_bits = (pos->pawns & allies & Rank7ByColorBB[WHITE]) & ((~occupied) << 8) & ((~occupied) << 16);
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = PawnMoves2[from][pos->side];
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenTwoForward(from, to);
        }
    }
}

/* this generate captures including en-passant captures, and promotions*/
void genCaptures(const position_t *pos, sort_t *sort) {
    int from, to;
    uint64 pc_bits, mv_bits;
    uint64 occupied = pos->occupied;
    uint64 allies = pos->color[pos->side];
    uint64 mask = sort->target;

    ASSERT(pos != NULL);
    ASSERT(sort != NULL);

    sort->size = 0;
    /* promotions only */
    pc_bits = pos->pawns & allies & Rank7ByColorBB[pos->side];
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = PawnMoves[from][pos->side] & ~occupied;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenPromoteStraight(from, to, QUEEN);
            sort->list[sort->size++].m = GenPromoteStraight(from, to, KNIGHT);
            sort->list[sort->size++].m = GenPromoteStraight(from, to, ROOK);
            sort->list[sort->size++].m = GenPromoteStraight(from, to, BISHOP);
        }
        mv_bits = PawnCaps[from][pos->side] & mask;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenPromote(from, to, QUEEN, getPiece(pos, to));
            sort->list[sort->size++].m = GenPromote(from, to, KNIGHT, getPiece(pos, to));
            sort->list[sort->size++].m = GenPromote(from, to, ROOK, getPiece(pos, to));
            sort->list[sort->size++].m = GenPromote(from, to, BISHOP, getPiece(pos, to));
        }
    }
    /* pawn captures only */
    pc_bits = pos->pawns & allies & ~Rank7ByColorBB[pos->side];
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = PawnCaps[from][pos->side] & mask;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenPawnMove(from, to, getPiece(pos, to));
        }
    }
    if ((pos->epsq != -1)) {
        mv_bits = pos->pawns & pos->color[pos->side]
            & PawnCaps[pos->epsq][pos->side ^ 1];
        while (mv_bits) {
            from = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenEnPassant(from, pos->epsq);
        }
    }
    pc_bits = pos->knights & allies;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = KnightMoves[from] & mask;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenKnightMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->bishops & allies;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = bishopAttacksBB(from, occupied) & mask;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenBishopMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->rooks & allies;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = rookAttacksBB(from, occupied) & mask;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenRookMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->queens & allies;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = queenAttacksBB(from, occupied) & mask;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenQueenMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->kings & allies;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = KingMoves[from] & mask;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenKingMove(from, to, getPiece(pos, to));
        }
    }
}

/* this generate legal moves when in check */
void genEvasions(const position_t *pos, sort_t *sort) {
    int sqchecker, from, to, ksq, side, xside;
    uint64  pc_bits, mv_bits, enemies, friends, temp, checkers, pinned;

    ASSERT(pos != NULL);
    ASSERT(sort != NULL);

    sort->size = 0;

    side = pos->side;
    xside = side ^ 1;
    friends = pos->color[side];
    enemies = pos->color[xside];
    ksq = pos->kpos[side];
    checkers = attackingPiecesSide(pos, ksq, xside);
    mv_bits = KingMoves[ksq] & ~pos->color[side];

    while (mv_bits) {
        to = popFirstBit(&mv_bits);
        temp = pos->occupied ^ BitMask[ksq] ^ BitMask[to];
        if (((PawnCaps[to][side] & pos->pawns & enemies) == EmptyBoardBB) &&
            ((KnightMoves[to] & pos->knights & enemies) == EmptyBoardBB) &&
            ((KingMoves[to] & pos->kings & enemies) == EmptyBoardBB) &&
            ((bishopAttacksBB(to, temp) & pos->bishops & enemies) == EmptyBoardBB) &&
            ((rookAttacksBB(to, temp) & pos->rooks & enemies) == EmptyBoardBB) &&
            ((queenAttacksBB(to, temp) & pos->queens & enemies) == EmptyBoardBB))
            sort->list[sort->size++].m = GenKingMove(ksq, to, getPiece(pos, to));
    }

    if (checkers & (checkers - 1)) return;

    pinned = pinnedPieces(pos, side);
    sqchecker = getFirstBit(checkers);

    mv_bits = PawnCaps[sqchecker][xside] & pos->pawns & friends & ~pinned;
    while (mv_bits) {
        from = popFirstBit(&mv_bits);
        if ((Rank7ByColorBB[side] & BitMask[from])) {
            sort->list[sort->size++].m = GenPromote(from, sqchecker, QUEEN, getPiece(pos, sqchecker));
            sort->list[sort->size++].m = GenPromote(from, sqchecker, KNIGHT, getPiece(pos, sqchecker));
            sort->list[sort->size++].m = GenPromote(from, sqchecker, ROOK, getPiece(pos, sqchecker));
            sort->list[sort->size++].m = GenPromote(from, sqchecker, BISHOP, getPiece(pos, sqchecker));
        }
        else
            sort->list[sort->size++].m = GenPawnMove(from, sqchecker, getPiece(pos, sqchecker));
    }
    if ((BitMask[sqchecker] & pos->pawns & enemies) &&
        ((sqchecker + ((side == WHITE) ? (8) : (-8))) == pos->epsq)) {
        mv_bits = PawnCaps[pos->epsq][xside] & pos->pawns & friends & ~pinned;
        while (mv_bits) {
            from = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenEnPassant(from, pos->epsq);
        }
    }
    mv_bits = KnightMoves[sqchecker] & pos->knights & friends & ~pinned;
    while (mv_bits) {
        from = popFirstBit(&mv_bits);
        sort->list[sort->size++].m = GenKnightMove(from, sqchecker, getPiece(pos, sqchecker));
    }
    mv_bits = bishopAttacksBB(sqchecker, pos->occupied) & pos->bishops & friends & ~pinned;
    while (mv_bits) {
        from = popFirstBit(&mv_bits);
        sort->list[sort->size++].m = GenBishopMove(from, sqchecker, getPiece(pos, sqchecker));
    }
    mv_bits = rookAttacksBB(sqchecker, pos->occupied) & pos->rooks & friends & ~pinned;
    while (mv_bits) {
        from = popFirstBit(&mv_bits);
        sort->list[sort->size++].m = GenRookMove(from, sqchecker, getPiece(pos, sqchecker));
    }
    mv_bits = queenAttacksBB(sqchecker, pos->occupied) & pos->queens & friends & ~pinned;
    while (mv_bits) {
        from = popFirstBit(&mv_bits);
        sort->list[sort->size++].m = GenQueenMove(from, sqchecker, getPiece(pos, sqchecker));
    }

    if (!(checkers & (pos->queens | pos->rooks | pos->bishops) & pos->color[xside])) return;

    temp = InBetween[sqchecker][ksq];

    pc_bits = pos->pawns & pos->color[side] & ~pinned;
    if (side == WHITE) mv_bits = (pc_bits << 8) & temp;
    else mv_bits = (pc_bits >> 8) & temp;
    while (mv_bits) {
        to = popFirstBit(&mv_bits);
        if (side == WHITE) from = to - 8;
        else from = to + 8;
        if ((Rank7ByColorBB[side] & BitMask[from])) {
            sort->list[sort->size++].m = GenPromoteStraight(from, to, QUEEN);
            sort->list[sort->size++].m = GenPromoteStraight(from, to, KNIGHT);
            sort->list[sort->size++].m = GenPromoteStraight(from, to, ROOK);
            sort->list[sort->size++].m = GenPromoteStraight(from, to, BISHOP);
        }
        else
            sort->list[sort->size++].m = GenOneForward(from, to);
    }
    if (side == WHITE) mv_bits = (((pc_bits << 8) & ~pos->occupied & Rank3BB) << 8) & temp;
    else mv_bits = (((pc_bits >> 8) & ~pos->occupied & Rank6BB) >> 8) & temp;
    while (mv_bits) {
        to = popFirstBit(&mv_bits);
        if (side == WHITE) from = to - 16;
        else from = to + 16;
        sort->list[sort->size++].m = GenTwoForward(from, to);
    }
    pc_bits = pos->knights & friends & ~pinned;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = KnightMoves[from] & temp;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenKnightMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->bishops & friends & ~pinned;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = bishopAttacksBB(from, pos->occupied) & temp;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenBishopMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->rooks & friends & ~pinned;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = rookAttacksBB(from, pos->occupied) & temp;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenRookMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->queens & friends & ~pinned;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        mv_bits = queenAttacksBB(from, pos->occupied) & temp;
        while (mv_bits) {
            to = popFirstBit(&mv_bits);
            sort->list[sort->size++].m = GenQueenMove(from, to, getPiece(pos, to));
        }
    }
    if ((pos->epsq != -1) && (checkers & pos->pawns & pos->color[xside])) {
        to = pos->epsq;
        mv_bits = pos->pawns & pos->color[side] & PawnCaps[to][xside] & ~pinned;
        while (mv_bits) {
            from = popFirstBit(&mv_bits);
            temp = pos->occupied ^ BitMask[sqchecker] ^ BitMask[from];
            if (((bishopAttacksBB(ksq, temp) & (pos->queens | pos->bishops)
                & pos->color[xside]) == EmptyBoardBB) &&
                ((rookAttacksBB(ksq, temp) & (pos->queens | pos->rooks)
                    & pos->color[xside]) == EmptyBoardBB))
                sort->list[sort->size++].m = GenEnPassant(from, to);
        }
    }
}

/* this generates all pseudo-legal non-capturing, non-promoting checks */
void genQChecks(const position_t *pos, sort_t *sort) {
    int us, them, ksq, from, to;
    uint64 dc, empty, checkSqs, bit1, bit2, bit3;

    sort->size = 0;
    us = pos->side;
    them = us ^ 1;

    ksq = pos->kpos[them];
    dc = discoveredCheckCandidates(pos, us);
    empty = ~pos->occupied;
    if (us == WHITE) {
        bit1 = pos->pawns & pos->color[us] & ~FileBB[SQFILE(ksq)];
        bit2 = bit3 = ((bit1 & dc) << 8) & ~Rank8BB & empty;
        while (bit3) {
            to = popFirstBit(&bit3);
            sort->list[sort->size++].m = GenOneForward(to - 8, to);
        }
        bit3 = ((bit2 & Rank3BB) << 8) & empty;
        while (bit3) {
            to = popFirstBit(&bit3);
            sort->list[sort->size++].m = GenTwoForward(to - 16, to);
        }
        bit1 &= ~dc;
        if (SQFILE(ksq) > FileA) bit1 &= FileBB[SQFILE(ksq) - 1];
        if (SQFILE(ksq) < FileH) bit1 &= FileBB[SQFILE(ksq) + 1];
        bit2 = (bit1 << 8) & empty;
        bit3 = bit2 & PawnCaps[ksq][BLACK];
        while (bit3) {
            to = popFirstBit(&bit3);
            sort->list[sort->size++].m = GenOneForward(to - 8, to);
        }
        bit3 = ((bit2 & Rank3BB) << 8) & empty & PawnCaps[ksq][BLACK];
        while (bit3) {
            to = popFirstBit(&bit3);
            sort->list[sort->size++].m = GenTwoForward(to - 16, to);
        }
    }
    else {
        bit1 = pos->pawns & pos->color[us] & ~FileBB[SQFILE(ksq)];
        bit2 = bit3 = ((bit1 & dc) >> 8) & ~Rank1BB & empty;
        while (bit3) {
            to = popFirstBit(&bit3);
            sort->list[sort->size++].m = GenOneForward(to + 8, to);
        }
        bit3 = ((bit2 & Rank6BB) >> 8) & empty;
        while (bit3) {
            to = popFirstBit(&bit3);
            sort->list[sort->size++].m = GenTwoForward(to + 16, to);
        }
        bit1 &= ~dc;
        if (SQFILE(ksq) > FileA) bit1 &= FileBB[SQFILE(ksq) - 1];
        if (SQFILE(ksq) < FileH) bit1 &= FileBB[SQFILE(ksq) + 1];
        bit2 = (bit1 >> 8) & empty;
        bit3 = bit2 & PawnCaps[ksq][WHITE];
        while (bit3) {
            to = popFirstBit(&bit3);
            sort->list[sort->size++].m = GenOneForward(to + 8, to);
        }
        bit3 = ((bit2 & Rank6BB) >> 8) & empty & PawnCaps[ksq][WHITE];
        while (bit3) {
            to = popFirstBit(&bit3);
            sort->list[sort->size++].m = GenTwoForward(to + 16, to);
        }
    }
    bit1 = pos->knights & pos->color[us];
    if (bit1) {
        bit2 = bit1 & dc;
        while (bit2) {
            from = popFirstBit(&bit2);
            bit3 = KnightMoves[from] & empty;
            while (bit3) {
                to = popFirstBit(&bit3);
                sort->list[sort->size++].m = GenKnightMove(from, to, getPiece(pos, to));
            }
        }
        bit2 = bit1 & ~dc;
        checkSqs = KnightMoves[ksq] & empty;
        while (bit2) {
            from = popFirstBit(&bit2);
            bit3 = KnightMoves[from] & checkSqs;
            while (bit3) {
                to = popFirstBit(&bit3);
                sort->list[sort->size++].m = GenKnightMove(from, to, getPiece(pos, to));
            }
        }
    }
    bit1 = pos->bishops & pos->color[us];
    if (bit1) {
        bit2 = bit1 & dc;
        while (bit2) {
            from = popFirstBit(&bit2);
            bit3 = bishopAttacksBB(from, pos->occupied) & empty;
            while (bit3) {
                to = popFirstBit(&bit3);
                sort->list[sort->size++].m = GenBishopMove(from, to, getPiece(pos, to));
            }
        }
        bit2 = bit1 & ~dc;
        checkSqs = bishopAttacksBB(ksq, pos->occupied) & empty;
        while (bit2) {
            from = popFirstBit(&bit2);
            bit3 = bishopAttacksBB(from, pos->occupied) & checkSqs;
            while (bit3) {
                to = popFirstBit(&bit3);
                sort->list[sort->size++].m = GenBishopMove(from, to, getPiece(pos, to));
            }
        }
    }
    bit1 = pos->rooks & pos->color[us];
    if (bit1) {
        bit2 = bit1 & dc;
        while (bit2) {
            from = popFirstBit(&bit2);
            bit3 = rookAttacksBB(from, pos->occupied) & empty;
            while (bit3) {
                to = popFirstBit(&bit3);
                sort->list[sort->size++].m = GenRookMove(from, to, getPiece(pos, to));
            }
        }

        bit2 = bit1 & ~dc;
        checkSqs = rookAttacksBB(ksq, pos->occupied) & empty;
        while (bit2) {
            from = popFirstBit(&bit2);
            bit3 = rookAttacksBB(from, pos->occupied) & checkSqs;
            while (bit3) {
                to = popFirstBit(&bit3);
                sort->list[sort->size++].m = GenRookMove(from, to, getPiece(pos, to));
            }
        }
    }
    bit1 = pos->queens & pos->color[us];
    if (bit1) {
        checkSqs = queenAttacksBB(ksq, pos->occupied) & empty;
        while (bit1) {
            from = popFirstBit(&bit1);
            bit2 = queenAttacksBB(from, pos->occupied) & checkSqs;
            while (bit2) {
                to = popFirstBit(&bit2);
                sort->list[sort->size++].m = GenQueenMove(from, to, getPiece(pos, to));
            }
        }
    }
    if (BitMask[pos->kpos[us]] & dc) {
        bit2 = KingMoves[pos->kpos[us]] & empty;
        bit2 &= ~DirA[getDirIndex(Direction[ksq][pos->kpos[us]])][ksq];
        while (bit2) {
            to = popFirstBit(&bit2);
            sort->list[sort->size++].m = GenKingMove(pos->kpos[us], to, getPiece(pos, to));
        }
    }
    if (((us == WHITE) ? (pos->castle & WCKS) : (pos->castle & BCKS))
        && (!(pos->occupied & ((us == WHITE) ? (F1 | G1) : (F8 | G8))))
        && !isAtt(pos, pos->side ^ 1, (us == WHITE) ? (E1 | F1 | G1) : (E8 | F8 | G8))) {
        bit1 = pos->occupied ^ BitMask[(us == WHITE) ? e1 : e8] ^
            BitMask[(us == WHITE) ? g1 : g8];
        if (rookAttacksBB(ksq, bit1) & BitMask[(us == WHITE) ? f1 : f8])
            sort->list[sort->size++].m = (us == WHITE) ? GenWhiteOO() : GenBlackOO();
    }
    if (((us == WHITE) ? (pos->castle & WCQS) : (pos->castle & BCQS))
        && (!(pos->occupied & ((us == WHITE) ? (B1 | C1 | D1) : (B8 | C8 | D8))))
        && !isAtt(pos, pos->side ^ 1, (us == WHITE) ? (E1 | D1 | C1) : (E8 | D8 | C8))) {
        bit1 = pos->occupied ^ BitMask[(us == WHITE) ? e1 : e8] ^
            BitMask[(us == WHITE) ? c1 : c8];
        if (rookAttacksBB(ksq, bit1) & BitMask[(us == WHITE) ? d1 : d8])
            sort->list[sort->size++].m = (us == WHITE) ? GenWhiteOOO() : GenBlackOOO();
    }
}

/* this determines if a not necessarily pseudo move is legal or not,
this must not be called when the position is in check,  */
uint32 genMoveIfLegal(const position_t *pos, uint32 move, uint64 pinned) {
    int from, to, pc, capt, prom, me, opp;
    int inc, delta;
    uint64 occupied;

    from = moveFrom(move);
    to = moveTo(move);
    pc = movePiece(move);
    capt = moveCapture(move);
    prom = movePromote(move);
    me = pos->side;
    opp = me ^ 1;
    occupied = pos->occupied;

    if (pc < PAWN || pc > KING) return FALSE;
    if (capt > QUEEN) return FALSE;
    if (prom > QUEEN) return FALSE;
    if (move == EMPTY) return FALSE;

    if (getPiece(pos, from) != pc) return FALSE;
    if (getColor(pos, from) != me) return FALSE;

    if (isEnPassant(move)) {
        inc = ((me == WHITE) ? -8 : 8);
        if (to != pos->epsq) return FALSE;
        if (getPiece(pos, pos->epsq + inc) != PAWN) return FALSE;
        if (getColor(pos, pos->epsq + inc) != opp) return FALSE;
        if (!moveIsLegal(pos, move, pinned, FALSE)) return FALSE;
        return TRUE;
    }
    else if (isCastle(move)) {
        if (me == WHITE) {
            if (from - to == 2) {
                if (!(pos->castle & WCQS)) return FALSE;
                if ((occupied&(B1 | C1 | D1)) || isAtt(pos, opp, C1 | D1)) return FALSE;
            }
            if (from - to == -2) {
                if (!(pos->castle & WCKS)) return FALSE;
                if ((occupied&(F1 | G1)) || isAtt(pos, opp, F1 | G1)) return FALSE;
            }
        }
        else {
            if (from - to == 2) {
                if (!(pos->castle & BCQS)) return FALSE;
                if ((occupied&(B8 | C8 | D8)) || isAtt(pos, opp, C8 | D8)) return FALSE;
            }
            if (from - to == -2) {
                if (!(pos->castle & BCKS)) return FALSE;
                if ((occupied&(F8 | G8)) || isAtt(pos, opp, F8 | G8)) return FALSE;
            }
        }
        return TRUE;
    }
    else if (pc == PAWN) {
        if (getPiece(pos, to) != capt) return FALSE;
        if (capt && getColor(pos, to) != opp) return FALSE;
        inc = ((me == WHITE) ? 8 : -8);
        delta = to - from;
        if (capt == EMPTY) {
            if (!(delta == inc) && !(delta == (2 * inc)
                && PAWN_RANK(from, me) == Rank2
                && getPiece(pos, from + inc) == EMPTY)) return FALSE;
        }
        else {
            if (!(delta == (inc - 1)) && !(delta == (inc + 1))) return FALSE;
        }
        if (prom) {
            if (PAWN_RANK(to, me) != Rank8) return FALSE;
        }
        else {
            if (PAWN_RANK(to, me) == Rank8) return FALSE;
        }
        if (!moveIsLegal(pos, move, pinned, FALSE)) return FALSE;
        return TRUE;
    }
    else {
        if (getPiece(pos, to) != capt) return FALSE;
        if (capt && getColor(pos, to) != opp) return FALSE;
        if (pc == KNIGHT) {
            if (!(KnightMoves[from] & BitMask[to])) return FALSE;
        }
        else if (pc == BISHOP) {
            if (!(bishopAttacksBB(from, occupied) & BitMask[to])) return FALSE;
        }
        else if (pc == ROOK) {
            if (!(rookAttacksBB(from, occupied) & BitMask[to])) return FALSE;
        }
        else if (pc == QUEEN) {
            if (!(queenAttacksBB(from, occupied) & BitMask[to])) return FALSE;
        }
        else if (pc == KING) {
            if (!(KingMoves[from] & BitMask[to])) return FALSE;
        }
        if (!moveIsLegal(pos, move, pinned, FALSE)) return FALSE;
        return TRUE;
    }
    ASSERT(FALSE);
    return FALSE;
}
