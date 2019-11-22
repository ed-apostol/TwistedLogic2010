/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

int squareIsOk(int s) {
    if (s < a1 || s > h8) return FALSE;
    return TRUE;
}

int colorIsOk(int c) {
    if (c == WHITE || c == BLACK) return TRUE;
    return FALSE;
}

int moveIsOk(int m) {
    int from = moveFrom(m);
    int to = moveTo(m);
    int pc = movePiece(m);
    int capt = moveCapture(m);
    int prom = movePromote(m);
    int x = TRUE;

    if (m == EMPTY) return TRUE;
    if (from < a1 || from > h8) x = FALSE;
    if (to < a1 || to > h8) x = FALSE;
    if (pc < PAWN || pc > KING) x = FALSE;
    if (capt < EMPTY || capt > QUEEN) x = FALSE;
    if (prom < EMPTY || prom > QUEEN) x = FALSE;

    if (x == FALSE) Print(8, "from = %d, to = %d, pc = %d, capt = %d, prom = %d\n",
        from, to, pc, capt, prom);
    return x;
}

int valueIsOk(int v) {
    if (abs(v) > INF) return FALSE;
    return TRUE;
}

int rankIsOk(int r) {
    if (r < 0 || r > 7) return FALSE;
    return TRUE;
}

/* this flips the entire position and save it into another position
data structure */
void flipPosition(const position_t *pos, position_t *clone) {
    int sq, temp, c;
    uint64 pc_bits;

    ASSERT(pos != NULL);
    ASSERT(clone != NULL);

    clone->uci_options = pos->uci_options;

    clone->book.bookfile = pos->book.bookfile;
    clone->book.size = pos->book.size;

    clone->pawn_table.table = pos->pawn_table.table;
    clone->pawn_table.size = pos->pawn_table.size;
    clone->pawn_table.mask = pos->pawn_table.mask;

    clone->trans_table.table = pos->trans_table.table;
    clone->trans_table.size = pos->trans_table.size;
    clone->trans_table.mask = pos->trans_table.mask;
    clone->trans_table.date = pos->trans_table.date;
    for (c = 0; c < DATESIZE; c++)
        clone->trans_table.age[c] = pos->trans_table.age[c];

    clone->pawns = EmptyBoardBB;
    clone->knights = EmptyBoardBB;
    clone->bishops = EmptyBoardBB;
    clone->rooks = EmptyBoardBB;
    clone->queens = EmptyBoardBB;
    clone->kings = EmptyBoardBB;
    clone->color[WHITE] = EmptyBoardBB;
    clone->color[BLACK] = EmptyBoardBB;
    clone->occupied = EmptyBoardBB;

    clone->side = pos->side ^ 1;
    clone->ply = pos->ply;
    clone->fifty = pos->fifty;

    for (sq = a1; sq <= h8; sq++) {
        clone->pieces[sq] = EMPTY;
    }

    clone->open[WHITE] = 0;
    clone->open[BLACK] = 0;
    clone->end[WHITE] = 0;
    clone->end[BLACK] = 0;

    clone->hash = 0;
    clone->phash = 0;

    clone->castle = 0;
    if (pos->castle&WCKS) clone->castle |= BCKS;
    if (pos->castle&WCQS) clone->castle |= BCQS;
    if (pos->castle&BCKS) clone->castle |= WCKS;
    if (pos->castle&BCQS) clone->castle |= WCQS;

    clone->epsq = -1;
    if (pos->epsq != -1) {
        sq = pos->epsq;
        temp = ((7 - SQRANK(sq)) << 3) + SQFILE(sq);
        clone->epsq = temp;
    }

    pc_bits = pos->color[WHITE];
    while (pc_bits) {
        sq = popFirstBit(&pc_bits);
        temp = ((7 - SQRANK(sq)) << 3) + SQFILE(sq);
        clone->color[BLACK] |= BitMask[temp];
    }

    pc_bits = pos->color[BLACK];
    while (pc_bits) {
        sq = popFirstBit(&pc_bits);
        temp = ((7 - SQRANK(sq)) << 3) + SQFILE(sq);
        clone->color[WHITE] |= BitMask[temp];
    }

    pc_bits = pos->pawns;
    while (pc_bits) {
        sq = popFirstBit(&pc_bits);
        temp = ((7 - SQRANK(sq)) << 3) + SQFILE(sq);
        clone->pawns |= BitMask[temp];
        clone->pieces[temp] = PAWN;
        c = getColor(clone, temp);
        clone->open[c] += PST(c, PAWN, temp, MIDGAME);
        clone->end[c] += PST(c, PAWN, temp, ENDGAME);
        clone->hash ^= ZobPiece[c][PAWN][temp];
        clone->phash ^= ZobPiece[c][PAWN][temp];
    }

    pc_bits = pos->knights;
    while (pc_bits) {
        sq = popFirstBit(&pc_bits);
        temp = ((7 - SQRANK(sq)) << 3) + SQFILE(sq);
        clone->knights |= BitMask[temp];
        clone->pieces[temp] = KNIGHT;
        c = getColor(clone, temp);
        clone->open[c] += PST(c, KNIGHT, temp, MIDGAME);
        clone->end[c] += PST(c, KNIGHT, temp, ENDGAME);
        clone->hash ^= ZobPiece[c][KNIGHT][temp];
    }

    pc_bits = pos->bishops;
    while (pc_bits) {
        sq = popFirstBit(&pc_bits);
        temp = ((7 - SQRANK(sq)) << 3) + SQFILE(sq);
        clone->bishops |= BitMask[temp];
        clone->pieces[temp] = BISHOP;
        c = getColor(clone, temp);
        clone->open[c] += PST(c, BISHOP, temp, MIDGAME);
        clone->end[c] += PST(c, BISHOP, temp, ENDGAME);
        clone->hash ^= ZobPiece[c][BISHOP][temp];
    }

    pc_bits = pos->rooks;
    while (pc_bits) {
        sq = popFirstBit(&pc_bits);
        temp = ((7 - SQRANK(sq)) << 3) + SQFILE(sq);
        clone->rooks |= BitMask[temp];
        clone->pieces[temp] = ROOK;
        c = getColor(clone, temp);
        clone->open[c] += PST(c, ROOK, temp, MIDGAME);
        clone->end[c] += PST(c, ROOK, temp, ENDGAME);
        clone->hash ^= ZobPiece[c][ROOK][temp];
    }

    pc_bits = pos->queens;
    while (pc_bits) {
        sq = popFirstBit(&pc_bits);
        temp = ((7 - SQRANK(sq)) << 3) + SQFILE(sq);
        clone->queens |= BitMask[temp];
        clone->pieces[temp] = QUEEN;
        c = getColor(clone, temp);
        clone->open[c] += PST(c, QUEEN, temp, MIDGAME);
        clone->end[c] += PST(c, QUEEN, temp, ENDGAME);
        clone->hash ^= ZobPiece[c][QUEEN][temp];
    }

    pc_bits = pos->kings;
    while (pc_bits) {
        sq = popFirstBit(&pc_bits);
        temp = ((7 - SQRANK(sq)) << 3) + SQFILE(sq);
        clone->kings |= BitMask[temp];
        clone->pieces[temp] = KING;
        c = getColor(clone, temp);
        clone->open[c] += PST(c, KING, temp, MIDGAME);
        clone->end[c] += PST(c, KING, temp, ENDGAME);
        clone->kpos[c] = temp;
        clone->hash ^= ZobPiece[c][KING][temp];
    }
    clone->occupied = clone->color[WHITE] | clone->color[BLACK];

    clone->hash ^= ZobCastle[clone->castle];
    if (clone->epsq != -1) clone->hash ^= ZobEpsq[SQFILE(clone->epsq)];
    if (clone->side == WHITE) clone->hash ^= ZobColor;

    clone->mat_summ[WHITE] =
        bitCnt(clone->pawns & clone->color[WHITE]) +            // 1
        bitCnt(clone->knights & clone->color[WHITE]) * 9 +        // 9
        bitCnt(clone->bishops & clone->color[WHITE]) * 9 * 3 +      // 27
        bitCnt(clone->rooks & clone->color[WHITE]) * 9 * 3 * 3 +    // 81
        bitCnt(clone->queens & clone->color[WHITE]) * 9 * 3 * 3 * 3;   // 243
    clone->mat_summ[BLACK] =
        bitCnt(clone->pawns & clone->color[BLACK]) +            // 1
        bitCnt(clone->knights & clone->color[BLACK]) * 9 +        // 9
        bitCnt(clone->bishops & clone->color[BLACK]) * 9 * 3 +      // 27
        bitCnt(clone->rooks & clone->color[BLACK]) * 9 * 3 * 3 +    // 81
        bitCnt(clone->queens & clone->color[BLACK]) * 9 * 3 * 3 * 3;   // 243
}

int evalSymmetryIsOk(const position_t *pos) {
    int score1, score2;
    position_t clone;

    flipPosition(pos, &clone);
    score1 = eval(pos);
    score2 = eval(&clone);
    if (score1 != score2) {
        Print(8, "\n==============================================\n");
        Print(8, "score1 = %d, score2 = %d\n", score1, score2);
        displayBoard(pos, 8);
        displayBoard(&clone, 8);
        Print(8, "\n==============================================\n");
        return FALSE;
    }
    return TRUE;
}

uint64 pawnHashRecalc(const position_t *pos) {
    uint64 pc_bits, pawnhash;
    int from;

    pawnhash = 0;
    pc_bits = pos->pawns & pos->color[WHITE];
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        pawnhash ^= ZobPiece[WHITE][PAWN][from];
    }

    pc_bits = pos->pawns & pos->color[BLACK];
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        pawnhash ^= ZobPiece[BLACK][PAWN][from];
    }
    return pawnhash;
}

void positionIsOk(const position_t *pos) {
    uint64 pc_bits, whitebits, blackbits, hash, pawnhash;
    int open_score[2], end_score[2], mat_summ[2];
    int from;

    ASSERT(pos != NULL);
    ASSERT(pos->pawn_table.table != NULL);
    ASSERT(pos->trans_table.table != NULL);
    ASSERT(pos->kings & pos->color[pos->side]);
    ASSERT(pos->kings & pos->color[pos->side ^ 1]);

    open_score[WHITE] = open_score[BLACK] =
        end_score[WHITE] = end_score[BLACK] = 0;
    hash = pawnhash = 0;
    whitebits = pos->color[WHITE];
    blackbits = pos->color[BLACK];

    pc_bits = pos->pawns & whitebits;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        open_score[WHITE] += PST(WHITE, PAWN, from, MIDGAME);
        end_score[WHITE] += PST(WHITE, PAWN, from, ENDGAME);
        hash ^= ZobPiece[WHITE][PAWN][from];
        pawnhash ^= ZobPiece[WHITE][PAWN][from];
    }

    pc_bits = pos->pawns & blackbits;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        open_score[BLACK] += PST(BLACK, PAWN, from, MIDGAME);
        end_score[BLACK] += PST(BLACK, PAWN, from, ENDGAME);
        hash ^= ZobPiece[BLACK][PAWN][from];
        pawnhash ^= ZobPiece[BLACK][PAWN][from];
    }

    pc_bits = pos->knights & whitebits;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        open_score[WHITE] += PST(WHITE, KNIGHT, from, MIDGAME);
        end_score[WHITE] += PST(WHITE, KNIGHT, from, ENDGAME);
        hash ^= ZobPiece[WHITE][KNIGHT][from];
    }

    pc_bits = pos->knights & blackbits;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        open_score[BLACK] += PST(BLACK, KNIGHT, from, MIDGAME);
        end_score[BLACK] += PST(BLACK, KNIGHT, from, ENDGAME);
        hash ^= ZobPiece[BLACK][KNIGHT][from];
    }

    pc_bits = pos->bishops & whitebits;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        open_score[WHITE] += PST(WHITE, BISHOP, from, MIDGAME);
        end_score[WHITE] += PST(WHITE, BISHOP, from, ENDGAME);
        hash ^= ZobPiece[WHITE][BISHOP][from];
    }

    pc_bits = pos->bishops & blackbits;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        open_score[BLACK] += PST(BLACK, BISHOP, from, MIDGAME);
        end_score[BLACK] += PST(BLACK, BISHOP, from, ENDGAME);
        hash ^= ZobPiece[BLACK][BISHOP][from];
    }

    pc_bits = pos->rooks & whitebits;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        open_score[WHITE] += PST(WHITE, ROOK, from, MIDGAME);
        end_score[WHITE] += PST(WHITE, ROOK, from, ENDGAME);
        hash ^= ZobPiece[WHITE][ROOK][from];
    }

    pc_bits = pos->rooks & blackbits;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        open_score[BLACK] += PST(BLACK, ROOK, from, MIDGAME);
        end_score[BLACK] += PST(BLACK, ROOK, from, ENDGAME);
        hash ^= ZobPiece[BLACK][ROOK][from];
    }

    pc_bits = pos->queens & whitebits;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        open_score[WHITE] += PST(WHITE, QUEEN, from, MIDGAME);
        end_score[WHITE] += PST(WHITE, QUEEN, from, ENDGAME);
        hash ^= ZobPiece[WHITE][QUEEN][from];
    }

    pc_bits = pos->queens & blackbits;
    while (pc_bits) {
        from = popFirstBit(&pc_bits);
        open_score[BLACK] += PST(BLACK, QUEEN, from, MIDGAME);
        end_score[BLACK] += PST(BLACK, QUEEN, from, ENDGAME);
        hash ^= ZobPiece[BLACK][QUEEN][from];
    }

    pc_bits = pos->kings & whitebits;
    from = popFirstBit(&pc_bits);
    ASSERT(from == pos->kpos[WHITE]);
    open_score[WHITE] += PST(WHITE, KING, from, MIDGAME);
    end_score[WHITE] += PST(WHITE, KING, from, ENDGAME);
    hash ^= ZobPiece[WHITE][KING][from];

    pc_bits = pos->kings & blackbits;
    from = popFirstBit(&pc_bits);
    ASSERT(from == pos->kpos[BLACK]);
    open_score[BLACK] += PST(BLACK, KING, from, MIDGAME);
    end_score[BLACK] += PST(BLACK, KING, from, ENDGAME);
    hash ^= ZobPiece[BLACK][KING][from];

    hash ^= ZobCastle[pos->castle];
    if (pos->epsq != -1) hash ^= ZobEpsq[SQFILE(pos->epsq)];
    if (pos->side == WHITE) hash ^= ZobColor;

    mat_summ[WHITE] =
        bitCnt(pos->pawns & pos->color[WHITE]) * MatSummValue[PAWN] +
        bitCnt(pos->knights & pos->color[WHITE]) * MatSummValue[KNIGHT] +
        bitCnt(pos->bishops & pos->color[WHITE]) * MatSummValue[BISHOP] +
        bitCnt(pos->rooks & pos->color[WHITE]) * MatSummValue[ROOK] +
        bitCnt(pos->queens & pos->color[WHITE]) * MatSummValue[QUEEN];
    mat_summ[BLACK] =
        bitCnt(pos->pawns & pos->color[BLACK]) * MatSummValue[PAWN] +
        bitCnt(pos->knights & pos->color[BLACK]) * MatSummValue[KNIGHT] +
        bitCnt(pos->bishops & pos->color[BLACK]) * MatSummValue[BISHOP] +
        bitCnt(pos->rooks & pos->color[BLACK]) * MatSummValue[ROOK] +
        bitCnt(pos->queens & pos->color[BLACK]) * MatSummValue[QUEEN];

    ASSERT(pos->mat_summ[WHITE] == mat_summ[WHITE]);
    ASSERT(pos->mat_summ[BLACK] == mat_summ[BLACK]);
    ASSERT(pos->open[WHITE] == open_score[WHITE]);
    ASSERT(pos->open[BLACK] == open_score[BLACK]);
    ASSERT(pos->end[WHITE] == end_score[WHITE]);
    ASSERT(pos->end[BLACK] == end_score[BLACK]);
    ASSERT(pos->hash == hash);
    ASSERT(pos->phash == pawnhash);
    ASSERT(!(pos->color[BLACK] & pos->color[WHITE]));
    ASSERT((pos->side == WHITE || pos->side == BLACK));
    ASSERT(pos->castle <= 15);
    ASSERT(bitCnt(pos->occupied) >= 2 && bitCnt(pos->occupied) <= 32);
    ASSERT(bitCnt(pos->color[WHITE]) >= 1 && bitCnt(pos->color[WHITE]) <= 16);
    ASSERT(bitCnt(pos->color[BLACK]) >= 1 && bitCnt(pos->color[BLACK]) <= 16);
    ASSERT(bitCnt(pos->pawns) <= 16);
    ASSERT(bitCnt(pos->kings) == 2);
    ASSERT((pos->color[WHITE] & pos->color[BLACK]) == 0);
    ASSERT(pos->kings & pos->color[WHITE] & BitMask[pos->kpos[WHITE]]);
    ASSERT(pos->kings & pos->color[BLACK] & BitMask[pos->kpos[BLACK]]);
    if (pos->epsq != -1)
        ASSERT(pos->epsq >= a3 && pos->epsq <= h6);
}
