/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

int key000(uint64 b, int f) {
    return (int)((b >> (f & 56)) & 0x7E);
}

#ifdef VERSION64BIT
int key090(uint64 b, int f) {
    uint64 _b = (b >> (f & 7)) & 0x0101010101010101LL;
    _b = _b * 0x0080402010080400LL;
    return (int)(_b >> 57);
}

int keyDiag(uint64 _b) {
    _b = _b * 0x0202020202020202LL;
    return (int)(_b >> 57);
}
#else
int key090(uint64 b, int f) {
    int h;
    b = b >> (f & 7);
    h = (int)((b & 0x1010101) | ((b >> 31) & 0x2020202));
    h = (h & 0x303) | ((h >> 14) & 0xC0C);
    return (h & 0xE) | ((h >> 4) & 0x70);
}

int keyDiag(uint64 _b) {
    int h = (int)(_b | _b >> 32);
    h |= h >> 16;
    h |= h >> 8;
    return h & 0x7E;
}
#endif

int key045(uint64 b, int f) {
    return keyDiag(b & DiagonalMask[f]);
}

int key135(uint64 b, int f) {
    return keyDiag(b & AntiDiagMask[f]);
}

uint64 getLowestBit(uint64 bb) {
    return bb & (-(int64)bb);
}

uint64 _occ_free_board(int bc, int del, uint64 free) {
    uint64 low, perm = free;
    int i;
    for (i = 0; i < bc; i++) {
        low = getLowestBit(free);
        free &= (~low);
        if (!TEST(i, del)) perm &= (~low);
    }
    return perm;
}

void _init_rays(uint64* rays, uint64(*rayFunc) (int, uint64, int), int(*key)(uint64, int)) {
    int i, f, iperm, bc, index;
    uint64 board, mmask, occ, move, xray;
    for (f = 0; f < 64; f++) {
        mmask = (*rayFunc)(f, 0LL, 0) | BitMask[f];
        iperm = 1 << (bc = bitCnt(mmask));
        for (i = 0; i < iperm; i++) {
            board = _occ_free_board(bc, i, mmask);
            move = (*rayFunc)(f, board, 1);
            occ = (*rayFunc)(f, board, 2);
            xray = (*rayFunc)(f, board, 3);
            index = (*key)(board, f);
            rays[(f << 7) + index] = occ | move;
            rays[(f << 7) + index + 0x8000] = xray;
        }
    }
}

void setBit(int f, uint64 *b) {
    *b |= BitMask[f];
}

uint64 _rook0(int f, uint64 board, int t) {
    uint64 free = 0LL, occ = 0LL, xray = 0LL;
    int i, b;
    for (b = 0, i = f + 1; i < 64 && i % 8 != 0; i++) RAYMACRO
        for (b = 0, i = f - 1; i >= 0 && i % 8 != 7; i--) RAYMACRO
            return (t < 2) ? free : (t == 2 ? occ : xray);
}

uint64 _rook90(int f, uint64 board, int t) {
    uint64 free = 0LL, occ = 0LL, xray = 0LL;
    int i, b;
    for (b = 0, i = f - 8; i >= 0; i -= 8) RAYMACRO
        for (b = 0, i = f + 8; i < 64; i += 8) RAYMACRO
            return (t < 2) ? free : (t == 2 ? occ : xray);
}

uint64 _bishop45(int f, uint64 board, int t) {
    uint64 free = 0LL, occ = 0LL, xray = 0LL;
    int i, b;
    for (b = 0, i = f + 9; i < 64 && (i % 8 != 0); i += 9) RAYMACRO
        for (b = 0, i = f - 9; i >= 0 && (i % 8 != 7); i -= 9) RAYMACRO
            return (t < 2) ? free : (t == 2 ? occ : xray);
}

uint64 _bishop135(int f, uint64 board, int t) {
    uint64 free = 0LL, occ = 0LL, xray = 0LL;
    int i, b;
    for (b = 0, i = f - 7; i >= 0 && (i % 8 != 0); i -= 7) RAYMACRO
        for (b = 0, i = f + 7; i < 64 && (i % 8 != 7); i += 7) RAYMACRO
            return (t < 2) ? free : (t == 2 ? occ : xray);
}

uint64 rankAttacks(uint64 occ, int sq) {
    return RayMagic[((sq) << 7) | key000(occ, sq)];
}

uint64 fileAttacks(uint64 occ, int sq) {
    return RayMagic[((sq) << 7) | key090(occ, sq) | 0x2000];
}

uint64 diagonalAttacks(uint64 occ, int sq) {
    return RayMagic[((sq) << 7) | key045(occ, sq) | 0x4000];
}

uint64 antiDiagAttacks(uint64 occ, int sq) {
    return RayMagic[((sq) << 7) | key135(occ, sq) | 0x6000];
}

uint64 rankAttacksX(uint64 occ, int sq) {
    return RayMagic[((sq) << 7) | key000(occ, sq) | 0x8000];
}

uint64 fileAttacksX(uint64 occ, int sq) {
    return RayMagic[((sq) << 7) | key090(occ, sq) | 0xA000];
}

uint64 diagonalAttacksX(uint64 occ, int sq) {
    return RayMagic[((sq) << 7) | key045(occ, sq) | 0xC000];
}

uint64 antiDiagAttacksX(uint64 occ, int sq) {
    return RayMagic[((sq) << 7) | key135(occ, sq) | 0xE000];
}

/* the following routines returns a 64 bit xray attacks of pieces
on the From square with the limiting Occupied bits */
uint64 bishopAttacksBBX(uint32 from, uint64 occ) {
    return (diagonalAttacksX(occ, from) | antiDiagAttacksX(occ, from));
}

uint64 rookAttacksBBX(uint32 from, uint64 occ) {
    return (fileAttacksX(occ, from) | rankAttacksX(occ, from));
}

/* the following routines returns a 64 bit attacks of pieces
on the From square with the limiting Occupied bits */
uint64 bishopAttacksBB(uint32 from, uint64 occ) {
    return (diagonalAttacks(occ, from) | antiDiagAttacks(occ, from));
}

uint64 rookAttacksBB(uint32 from, uint64 occ) {
    return (fileAttacks(occ, from) | rankAttacks(occ, from));
}

uint64 queenAttacksBB(uint32 from, uint64 occ) {
    return (diagonalAttacks(occ, from) | antiDiagAttacks(occ, from)
        | fileAttacks(occ, from) | rankAttacks(occ, from));
}

/* this is the attack routine, capable of multiple targets,
can be used to determine if the bits on the 64 bit parameter
is being attacked by the side color */
uint32 isAtt(const position_t *pos, uint32 color, uint64 target) {
    int from;

    ASSERT(pos != NULL);
    ASSERT(target);
    ASSERT(colorIsOk(color));

    while (target) {
        from = popFirstBit(&target);
        if ((pos->rooks | pos->queens) & pos->color[color]
            & rookAttacksBB(from, pos->occupied)) return TRUE;
        if ((pos->bishops | pos->queens) & pos->color[color]
            & bishopAttacksBB(from, pos->occupied)) return TRUE;
        if (pos->knights & pos->color[color] & KnightMoves[from]) return TRUE;
        if (pos->kings & pos->color[color] & KingMoves[from]) return TRUE;
        if (pos->pawns & pos->color[color] & PawnCaps[from][color ^ 1]) return TRUE;
    }
    return FALSE;
}

/* this determines if the side to move is in check */
uint32 kingIsInCheck(const position_t *pos) {
    return isAtt(pos, pos->side ^ 1, pos->kings & pos->color[pos->side]);
}
/* this returns the pinned pieces to the King of the side Color */
uint64 pinnedPieces(const position_t *pos, uint32 c) {
    uint64 b, pin, pinners;
    int t, ksq;
    pin = 0ULL;
    ksq = pos->kpos[c];
    pinners = pos->color[c ^ 1] & (pos->queens | pos->rooks);
    if (pinners) {
        b = rookAttacksBBX(ksq, pos->occupied) & pinners;
        while (b) {
            t = popFirstBit(&b);
            pin |= InBetween[t][ksq] & pos->color[c];
        }
    }
    pinners = pos->color[c ^ 1] & (pos->queens | pos->bishops);
    if (pinners) {
        b = bishopAttacksBBX(ksq, pos->occupied) & pinners;
        while (b) {
            t = popFirstBit(&b);
            pin |= InBetween[t][ksq] & pos->color[c];
        }
    }
    return pin;
}
/* this returns the discovered check candidates pieces to the King of
the opposite side of Color */
uint64 discoveredCheckCandidates(const position_t *pos, uint32 c) {
    uint64 b, pin, pinners;
    int t, ksq;
    pin = 0ULL;
    ksq = pos->kpos[c ^ 1];
    pinners = pos->color[c] & (pos->queens | pos->rooks);
    if (pinners) {
        b = rookAttacksBBX(ksq, pos->occupied) & pinners;
        while (b) {
            t = popFirstBit(&b);
            pin |= InBetween[t][ksq] & pos->color[c];
        }
    }
    pinners = pos->color[c] & (pos->queens | pos->bishops);
    if (pinners) {
        b = bishopAttacksBBX(ksq, pos->occupied) & pinners;
        while (b) {
            t = popFirstBit(&b);
            pin |= InBetween[t][ksq] & pos->color[c];
        }
    }
    return pin;
}

/* this determines if a pseudo-legal move is legal without executing
makeMove */
uint32 moveIsLegal(const position_t *pos, uint32 move, uint64 pinned, uint32 incheck) {
    int us, them, ksq, from, to, capsq;
    uint64 b;

    ASSERT(pos != NULL);
    ASSERT(moveIsOk(move));

    if (incheck) return TRUE;
    if (isCastle(move)) return TRUE;

    us = pos->side;
    them = us ^ 1;
    from = moveFrom(move);
    to = moveTo(move);
    ksq = pos->kpos[us];

    if (isEnPassant(move)) {
        capsq = (SQRANK(from) << 3) + SQFILE(to);
        b = pos->occupied ^ BitMask[from] ^ BitMask[capsq] ^ BitMask[to];
        return
            (!(rookAttacksBB(ksq, b) & (pos->queens | pos->rooks) & pos->color[them]) &&
                !(bishopAttacksBB(ksq, b) & (pos->queens | pos->bishops) & pos->color[them]));
    }
    if (from == ksq) return !(isAtt(pos, them, BitMask[to]));
    if (!(pinned & BitMask[from])) return TRUE;
    if (Direction[from][ksq] == Direction[to][ksq]) return TRUE;
    return FALSE;
}

/* this determines if a move gives check */
uint32 moveIsCheck(const position_t *pos, uint32 m, uint64 dcc) {
    int us, them, ksq, from, to;
    uint64 temp;
    us = pos->side;
    them = us ^ 1;

    ASSERT(pos != NULL);
    ASSERT(moveIsOk(m));

    from = moveFrom(m);
    to = moveTo(m);
    ksq = pos->kpos[them];

    switch (movePiece(m)) {
    case PAWN:
        if (PawnCaps[ksq][them] & BitMask[to]) return TRUE;
        else if ((dcc & BitMask[from]) && Direction[from][ksq] != Direction[to][ksq]) return TRUE;
        temp = pos->occupied ^ BitMask[from] ^ BitMask[to];
        switch (movePromote(m)) {
        case EMPTY:
            break;
        case KNIGHT:
            if (KnightMoves[ksq] & BitMask[to]) return TRUE;
            return FALSE;
        case BISHOP:
            if (bishopAttacksBB(ksq, temp) & BitMask[to]) return TRUE;
            return FALSE;
        case ROOK:
            if (rookAttacksBB(ksq, temp) & BitMask[to]) return TRUE;
            return FALSE;
        case QUEEN:
            if (queenAttacksBB(ksq, temp) & BitMask[to]) return TRUE;
            return FALSE;
        }
        if (isEnPassant(m)) {
            temp ^= BitMask[(SQRANK(from) << 3) + SQFILE(to)];
            if ((rookAttacksBB(ksq, temp) & (pos->queens | pos->rooks)
                & pos->color[us]) || (bishopAttacksBB(ksq, temp) & (pos->queens | pos->bishops) & pos->color[us]))
                return TRUE;
        }
        return FALSE;

    case KNIGHT:
        if ((dcc & BitMask[from])) return TRUE;
        else if (KnightMoves[ksq] & BitMask[to]) return TRUE;
        return FALSE;

    case BISHOP:
        if ((dcc & BitMask[from])) return TRUE;
        else if (bishopAttacksBB(ksq, pos->occupied) & BitMask[to]) return TRUE;
        return FALSE;

    case ROOK:
        if ((dcc & BitMask[from])) return TRUE;
        else if (rookAttacksBB(ksq, pos->occupied) & BitMask[to]) return TRUE;
        return FALSE;

    case QUEEN:
        ASSERT(!(dcc & BitMask[from]));
        if (queenAttacksBB(ksq, pos->occupied) & BitMask[to]) return TRUE;
        return FALSE;

    case KING:
        if ((dcc & BitMask[from]) &&
            Direction[from][ksq] != Direction[to][ksq]) return TRUE;
        temp = pos->occupied ^ BitMask[from] ^ BitMask[to];
        if (from - to == 2) {
            if (rookAttacksBB(ksq, temp) & BitMask[from - 1]) return TRUE;
        }
        if (from - to == -2) {
            if (rookAttacksBB(ksq, temp) & BitMask[from + 1]) return TRUE;
        }
        return FALSE;

    default:
        ASSERT(FALSE);
        return FALSE;
    }
    ASSERT(FALSE);
    return FALSE;
}

/* this returns the bitboard of all pieces attacking a certain square */
uint64 attackingPiecesAll(const position_t *pos, uint32 sq) {
    uint64 attackers = 0;

    ASSERT(pos != NULL);
    ASSERT(squareIsOk(sq));

    attackers |= PawnCaps[sq][BLACK] & pos->pawns & pos->color[WHITE];
    attackers |= PawnCaps[sq][WHITE] & pos->pawns & pos->color[BLACK];
    attackers |= KnightMoves[sq] & pos->knights;
    attackers |= KingMoves[sq] & pos->kings;
    attackers |= bishopAttacksBB(sq, pos->occupied) & (pos->bishops | pos->queens);
    attackers |= rookAttacksBB(sq, pos->occupied) & (pos->rooks | pos->queens);
    return attackers;
}

/* this returns the bitboard of all pieces of a given side attacking a certain square */
uint64 attackingPiecesSide(const position_t *pos, uint32 sq, uint32 side) {
    uint64 attackers = 0;

    ASSERT(pos != NULL);
    ASSERT(squareIsOk(sq));
    ASSERT(colorIsOk(side));

    attackers |= PawnCaps[sq][side ^ 1] & pos->pawns;
    attackers |= KnightMoves[sq] & pos->knights;
    attackers |= KingMoves[sq] & pos->kings;
    attackers |= bishopAttacksBB(sq, pos->occupied) & (pos->bishops | pos->queens);
    attackers |= rookAttacksBB(sq, pos->occupied) & (pos->rooks | pos->queens);
    return (attackers & pos->color[side]);
}

/* this returns the bitboard of sliding pieces attacking in a direction
behind the piece attacker */
uint64 behindFigure(uint64 QR, uint64 QB, uint64 occupied, uint32 from, int dir) {
    ASSERT(squareIsOk(from));

    switch (dir) {
    case -9: return diagonalAttacks(occupied, from) & QB & DirA[0][from];
    case -1: return rankAttacks(occupied, from) & QR & DirA[1][from];
    case 7: return antiDiagAttacks(occupied, from) & QB & DirA[2][from];
    case 8: return fileAttacks(occupied, from) & QR & DirA[3][from];
    case 9: return diagonalAttacks(occupied, from) & QB & DirA[4][from];
    case 1: return rankAttacks(occupied, from) & QR & DirA[5][from];
    case -7: return antiDiagAttacks(occupied, from) & QB & DirA[6][from];
    case -8: return fileAttacks(occupied, from) & QR & DirA[7][from];
    default: return 0;
    }
}

/* this is the Static Exchange Evaluator */
int swap(const position_t *pos, uint32 m) {
    int to = moveTo(m);
    int from = moveFrom(m);
    int pc = movePiece(m);
    int lastvalue = PcValSEE[pc];
    int n = 1;
    int lsb;
    int c = pos->side ^ 1;
    int dir = Direction[to][from];
    int slist[32];
    uint64 attack;

    ASSERT(pos != NULL);
    ASSERT(moveIsOk(m));

    attack = attackingPiecesAll(pos, to);
    if (!(pc == PAWN && moveCapture(m) == EMPTY)) {
        attack ^= BitMask[from];
    }

    slist[0] = PcValSEE[moveCapture(m)];

    if (pc == PAWN && PAWN_RANK(from, pos->side) == Rank7) {
        lastvalue = PcValSEE[movePromote(m)];
        slist[0] += PcValSEE[QUEEN] - PcValSEE[PAWN];
    }

    if (dir && pc != KING) {
        attack |= behindFigure((pos->queens | pos->rooks), (pos->queens | pos->bishops),
            pos->occupied, from, dir);
    }

    if (isEnPassant(m)) {
        attack |= behindFigure((pos->queens | pos->rooks), (pos->queens | pos->bishops), pos->occupied, to, Direction[to][to + ((pos->side == WHITE) ? -8 : 8)]);
    }

    while (attack) {
        if (attack & pos->pawns & pos->color[c]) {
            lsb = getFirstBit(attack & pos->pawns & pos->color[c]);
            pc = PAWN;
        }
        else if (attack & pos->knights & pos->color[c]) {
            lsb = getFirstBit(attack & pos->knights & pos->color[c]);
            pc = KNIGHT;
        }
        else if (attack & pos->bishops & pos->color[c]) {
            lsb = getFirstBit(attack & pos->bishops & pos->color[c]);
            pc = BISHOP;
        }
        else if (attack & pos->rooks & pos->color[c]) {
            lsb = getFirstBit(attack & pos->rooks & pos->color[c]);
            pc = ROOK;
        }
        else if (attack & pos->queens & pos->color[c]) {
            lsb = getFirstBit(attack & pos->queens & pos->color[c]);
            pc = QUEEN;
        }
        else if (attack & pos->kings & pos->color[c]) {
            lsb = getFirstBit(attack & pos->kings & pos->color[c]);
            pc = KING;
        }
        else break;

        slist[n] = -slist[n - 1] + lastvalue;
        if (lastvalue == PcValSEE[KING]) break;
        if (pc == PAWN && PAWN_RANK(lsb, c) == Rank7) {
            lastvalue = PcValSEE[QUEEN];
            slist[n] += PcValSEE[QUEEN] - PcValSEE[PAWN];
        }
        else lastvalue = PcValSEE[pc];
        dir = Direction[to][lsb];
        if (dir && pc != KING)
            attack |= behindFigure((pos->queens | pos->rooks), (pos->queens | pos->bishops),
                pos->occupied, lsb, dir);
        attack ^= BitMask[lsb];
        n++;
        c ^= 1;
    }
    while (--n) {
        if (slist[n] > -slist[n - 1])
            slist[n - 1] = -slist[n];
    }
    return slist[0];
}
