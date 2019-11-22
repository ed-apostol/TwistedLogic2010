/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

void initPST(void) {
    const int PawnFileMul[8] = {
        -3, -1, +0, +1, +1, +0, -1, -3,
    };
    const int KnightLineMul[8] = {
        -4, -2, +0, +1, +1, +0, -2, -4,
    };
    const int KnightRankMul[8] = {
        -2, -1, +0, +1, +2, +3, +2, +1,
    };
    const int BishopLineMul[8] = {
        -3, -1, +0, +1, +1, +0, -1, -3,
    };
    const int RookFileMul[8] = {
        -2, -1, +0, +1, +1, +0, -1, -2,
    };
    const int QueenLineMul[8] = {
        -3, -1, +0, +1, +1, +0, -1, -3,
    };
    const int KingLineMul[8] = {
        -3, -1, +0, +1, +1, +0, -1, -3,
    };
    const int KingFileMul[8] = {
        +3, +4, +2, +0, +0, +2, +4, +3,
    };
    const int KingRankMul[8] = {
        +1, +0, -2, -3, -4, -5, -6, -7,
    };

    const int MidgamePawnFile = 6;
    const int EndgamePawnFile = -3;
    const int MidgameKnightCentre = 11;
    const int EndgameKnightCentre = 2;
    const int MidgameKnightRank = 11;
    const int KnightTrapped = 100;
    const int MidgameBishopCentre = 5;
    const int EndgameBishopCentre = 2;
    const int MidgameBishopBackRank = 8;
    const int MidgameBishopDiagonal = 12;
    const int MidgameRookFile = 3;
    const int MidgameQueenCentre = 3;
    const int EndgameQueenCentre = 3;
    const int MidgameQueenBackRank = 6;
    const int EndgameKingCentre = 13;
    const int MidgameKingFile = 15;
    const int MidgameKingRank = 0;

    int i, j, k;

    memset(PcSqTb, 0, sizeof(PcSqTb));

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 64; j++) {
            switch (i) {
            case PAWN:
                PST(WHITE, i, j, MIDGAME) += PawnFileMul[SQFILE(j)] * MidgamePawnFile;
                PST(WHITE, i, j, ENDGAME) += PawnFileMul[SQFILE(j)] * EndgamePawnFile;
                break;
            case KNIGHT:
                PST(WHITE, i, j, MIDGAME) += KnightLineMul[SQFILE(j)] * MidgameKnightCentre;
                PST(WHITE, i, j, MIDGAME) += KnightLineMul[SQRANK(j)] * MidgameKnightCentre;
                PST(WHITE, i, j, MIDGAME) += KnightRankMul[SQRANK(j)] * MidgameKnightRank;
                PST(WHITE, i, j, ENDGAME) += KnightLineMul[SQFILE(j)] * EndgameKnightCentre;
                PST(WHITE, i, j, ENDGAME) += KnightLineMul[SQRANK(j)] * EndgameKnightCentre;
                break;
            case BISHOP:
                PST(WHITE, i, j, MIDGAME) += BishopLineMul[SQFILE(j)] * MidgameBishopCentre;
                PST(WHITE, i, j, ENDGAME) += BishopLineMul[SQFILE(j)] * EndgameBishopCentre;
                PST(WHITE, i, j, MIDGAME) += BishopLineMul[SQRANK(j)] * MidgameBishopCentre;
                PST(WHITE, i, j, ENDGAME) += BishopLineMul[SQRANK(j)] * EndgameBishopCentre;
                break;
            case ROOK:
                PST(WHITE, i, j, MIDGAME) += RookFileMul[SQFILE(j)] * MidgameRookFile;
                break;
            case QUEEN:
                PST(WHITE, i, j, MIDGAME) += QueenLineMul[SQFILE(j)] * MidgameQueenCentre;
                PST(WHITE, i, j, ENDGAME) += QueenLineMul[SQFILE(j)] * EndgameQueenCentre;
                PST(WHITE, i, j, MIDGAME) += QueenLineMul[SQRANK(j)] * MidgameQueenCentre;
                PST(WHITE, i, j, ENDGAME) += QueenLineMul[SQRANK(j)] * EndgameQueenCentre;
                break;
            case KING:
                PST(WHITE, i, j, MIDGAME) += KingFileMul[SQFILE(j)] * MidgameKingFile;
                PST(WHITE, i, j, ENDGAME) += KingLineMul[SQFILE(j)] * EndgameKingCentre;
                PST(WHITE, i, j, MIDGAME) += KingRankMul[SQRANK(j)] * MidgameKingRank;
                PST(WHITE, i, j, ENDGAME) += KingLineMul[SQRANK(j)] * EndgameKingCentre;
                break;
            }
        }
    }

    PST(WHITE, KNIGHT, a8, MIDGAME) -= KnightTrapped;
    PST(WHITE, KNIGHT, h8, MIDGAME) -= KnightTrapped;

    for (i = a1; i <= h1; i++) PST(WHITE, BISHOP, i, MIDGAME) -= MidgameBishopBackRank;
    for (i = a1; i <= h1; i++) PST(WHITE, QUEEN, i, MIDGAME) -= MidgameQueenBackRank;

    for (i = 0; i < 8; i++) {
        j = (i * 8) + i;
        PST(WHITE, BISHOP, j, MIDGAME) += MidgameBishopDiagonal;
        j = ((7 - i) * 8) + i;
        PST(WHITE, BISHOP, j, MIDGAME) += MidgameBishopDiagonal;
    }

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 64; j++) {
            k = ((7 - SQRANK(j)) * 8) + SQFILE(j);
            PST(BLACK, i, k, MIDGAME) = PST(WHITE, i, j, MIDGAME);
            PST(BLACK, i, k, ENDGAME) = PST(WHITE, i, j, ENDGAME);
        }
    }

    for (j = 0; j < 64; j++) {
        k = ((7 - SQRANK(j)) * 8) + SQFILE(j);
        KingPosPenalty[WHITE][j] = (4 - KingFileMul[SQFILE(j)]) + (1 - KingRankMul[SQRANK(j)]);
        KingPosPenalty[BLACK][k] = KingPosPenalty[WHITE][j];
    }
}

/* this initializes the pseudo-constant variables used in the program */
void initArr(void) {
    int i, j, m, k, n;
    const int kingd[] = { -9, -1, 7, 8, 9, 1, -7, -8 };
    const int knightd[] = { -17, -10, 6, 15, 17, 10, -6, -15 };
    const int wpawnd[] = { 8 };
    const int bpawnd[] = { -8 };
    const int wpawnc[] = { 7, 9 };
    const int bpawnc[] = { -7, -9 };
    const int wpawn2mov[] = { 16 };
    const int bpawn2mov[] = { -16 };

    memset(KnightMoves, 0, sizeof(KnightMoves));
    memset(KingMoves, 0, sizeof(KingMoves));
    memset(PawnMoves, 0, sizeof(PawnMoves));
    memset(PawnCaps, 0, sizeof(PawnCaps));
    memset(PawnMoves2, 0, sizeof(PawnMoves2));
    memset(RankMask, 0, sizeof(RankMask));
    memset(FileMask, 0, sizeof(FileMask));
    memset(PawnShelterMask1, 0, sizeof(PawnShelterMask1));
    memset(PawnShelterMask2, 0, sizeof(PawnShelterMask2));
    memset(PawnShelterMask3, 0, sizeof(PawnShelterMask3));
    memset(Direction, 0, sizeof(Direction));
    memset(DirA, 0, sizeof(DirA));
    memset(InBetween, 0, sizeof(InBetween));
    memset(PassedMask, 0, sizeof(PassedMask));
    memset(RayMagic, 0, sizeof(RayMagic));

    _init_rays(RayMagic, _rook0, key000);
    _init_rays(RayMagic + 0x2000, _rook90, key090);
    _init_rays(RayMagic + 0x4000, _bishop45, key045);
    _init_rays(RayMagic + 0x6000, _bishop135, key135);

    for (i = 0; i < 0x40; i++) CastleMask[i] = 0xF;

    CastleMask[e1] &= ~WCKS;
    CastleMask[h1] &= ~WCKS;
    CastleMask[e1] &= ~WCQS;
    CastleMask[a1] &= ~WCQS;
    CastleMask[e8] &= ~BCKS;
    CastleMask[h8] &= ~BCKS;
    CastleMask[e8] &= ~BCQS;
    CastleMask[a8] &= ~BCQS;

    for (i = 0; i < 0x40; i++) {
        for (j = 0; j < 0x40; j++) {
            if (SQRANK(i) == SQRANK(j)) RankMask[i] |= BitMask[j];
            if (SQFILE(i) == SQFILE(j)) FileMask[i] |= BitMask[j];
        }

        for (j = 0; j < 8; j++) {
            n = i + knightd[j];
            if (n < 64 && n >= 0 && abs(((n & 7) - (i & 7))) <= 2)
                KnightMoves[i] |= BitMask[n];
        }
        for (j = 0; j < 8; j++) {
            n = i + kingd[j];
            if (n < 64 && n >= 0 && abs(((n & 7) - (i & 7))) <= 2)
                KingMoves[i] |= BitMask[n];
        }
        for (j = 0; j < 1; j++) {
            n = i + wpawnd[j];
            if (n < 64 && n >= 0 && abs(((n & 7) - (i & 7))) <= 2)
                PawnMoves[i][WHITE] |= BitMask[n];
        }
        for (j = 0; j < 1; j++) {
            n = i + bpawnd[j];
            if (n < 64 && n >= 0 && abs(((n & 7) - (i & 7))) <= 2)
                PawnMoves[i][BLACK] |= BitMask[n];
        }
        for (j = 0; j < 2; j++) {
            n = i + wpawnc[j];
            if (n < 64 && n >= 0 && abs(((n & 7) - (i & 7))) <= 2)
                PawnCaps[i][WHITE] |= BitMask[n];
        }
        for (j = 0; j < 2; j++) {
            n = i + bpawnc[j];
            if (n < 64 && n >= 0 && abs(((n & 7) - (i & 7))) <= 2)
                PawnCaps[i][BLACK] |= BitMask[n];
        }
        for (j = 0; j < 1; j++) {
            n = i + wpawn2mov[j];
            if (n < 64 && n >= 0 && abs(((n & 7) - (i & 7))) <= 2)
                PawnMoves2[i][WHITE] |= BitMask[n];
        }
        for (j = 0; j < 1; j++) {
            n = i + bpawn2mov[j];
            if (n < 64 && n >= 0 && abs(((n & 7) - (i & 7))) <= 2)
                PawnMoves2[i][BLACK] |= BitMask[n];
        }
    }

    for (i = 0; i < 64; i++) {
        for (k = 0; k < 8; k++) {
            DirA[k][i] = 0;
            for (m = -1, j = i;;) {
                n = j + kingd[k];
                if (n < 0 || n > 63 || (j % 8 == 0 && n % 8 == 7)
                    || (j % 8 == 7 && n % 8 == 0))
                    break;
                Direction[i][n] = kingd[k];
                DirA[k][i] |= BitMask[n];
                m = j = n;
            }
        }
    }
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            k = Direction[i][j];
            if (k != 0) {
                m = getDirIndex(k);
                n = getDirIndex(-k);
                InBetween[i][j] = DirA[m][i] & DirA[n][j];
            }
        }
    }
    for (i = 8; i < 56; i++) {
        uint64 b = (uint64)1 << (i + 8);
        if (SQFILE(i) > FileA) b |= (uint64)1 << (i + 7);
        if (SQFILE(i) < FileH) b |= (uint64)1 << (i + 9);
        PassedMask[0][i] = fillUp2(b);

        b = (uint64)1 << (i - 8);
        if (SQFILE(i) > FileA) b |= (uint64)1 << (i - 9);
        if (SQFILE(i) < FileH) b |= (uint64)1 << (i - 7);
        PassedMask[1][i] = fillDown2(b);
    }
    for (i = 0; i < 3; i++) {
        int wksq[3] = { b1, e1, g1 };
        int bksq[3] = { b8, e8, g8 };
        uint64 b1, b2, b3;
        b1 = b2 = b3 = 0;
        b1 |= BitMask[wksq[i] + 7];
        b1 |= BitMask[wksq[i] + 8];
        b1 |= BitMask[wksq[i] + 9];
        b2 |= BitMask[wksq[i] + 15];
        b2 |= BitMask[wksq[i] + 16];
        b2 |= BitMask[wksq[i] + 17];
        b3 |= BitMask[wksq[i] + 23];
        b3 |= BitMask[wksq[i] + 24];
        b3 |= BitMask[wksq[i] + 25];
        PawnShelterMask1[WHITE][i] = b1;
        PawnShelterMask2[WHITE][i] = b2;
        PawnShelterMask3[WHITE][i] = b3;

        b1 = b2 = b3 = 0;
        b1 |= BitMask[bksq[i] - 9];
        b1 |= BitMask[bksq[i] - 8];
        b1 |= BitMask[bksq[i] - 7];
        b2 |= BitMask[bksq[i] - 17];
        b2 |= BitMask[bksq[i] - 16];
        b2 |= BitMask[bksq[i] - 15];
        b3 |= BitMask[bksq[i] - 25];
        b3 |= BitMask[bksq[i] - 24];
        b3 |= BitMask[bksq[i] - 23];
        PawnShelterMask1[BLACK][i] = b1;
        PawnShelterMask2[BLACK][i] = b2;
        PawnShelterMask3[BLACK][i] = b3;
    }
}
