/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

int materialWeigths(int our_pawn_count, int our_knight_count, int our_bishop_count, int our_rook_count, int our_queen_count,
    int their_pawns_count, int their_knight_count, int their_bishop_count, int their_rook_count, int their_queen_count) {
    int our_minor_count, their_minor_count, our_phase, their_phase, mat_weight, our_valuation, their_valuation;
    our_minor_count = our_bishop_count + our_knight_count;
    their_minor_count = their_bishop_count + their_knight_count;
    our_phase = our_minor_count + 2 * our_rook_count + 4 * our_queen_count;
    their_phase = their_minor_count + 2 * their_rook_count + 4 * their_queen_count;
    our_valuation = 3 * (our_bishop_count + our_knight_count) + 5 * our_rook_count + 9 * our_queen_count;
    their_valuation = 3 * (their_bishop_count + their_knight_count) + 5 * their_rook_count + 9 * their_queen_count;
    mat_weight = 10;
    if (!our_pawn_count) {
        if (our_phase == 1)
            mat_weight = 0;
        if (our_phase == 2) {
            if (their_phase == 0) {
                if (our_knight_count == 2) {
                    if (their_pawns_count >= 1)
                        mat_weight = 3;
                    else
                        mat_weight = 0;
                }
            }
            if (their_phase == 1) {
                mat_weight = 1;
                if (our_bishop_count == 2 && their_knight_count == 1)
                    mat_weight = 8;
                if (our_rook_count == 1 && their_knight_count == 1)
                    mat_weight = 2;
            }
            if (their_phase == 2)
                mat_weight = 1;
        }
        if (our_phase == 3 && our_rook_count == 1) {
            if (their_phase == 2 && their_rook_count == 1) {
                if (our_knight_count == 1)
                    mat_weight = 1;
                if (our_bishop_count == 1)
                    mat_weight = 1;
            }
            if (their_phase == 2 && their_rook_count == 0) {
                mat_weight = 2;
                if (our_bishop_count == 1 && their_knight_count == 2)
                    mat_weight = 6;
            }
            if (their_phase == 3)
                mat_weight = 2;
        }
        if (our_phase == 3 && our_rook_count == 0) {
            if (their_phase == 2 && their_rook_count == 1) {
                if (our_knight_count == 2)
                    mat_weight = 2;
                if (our_bishop_count == 2)
                    mat_weight = 7;
            }
            if (their_phase == 2 && their_rook_count == 0) {
                mat_weight = 2;
                if (our_bishop_count == 2 && their_knight_count == 2)
                    mat_weight = 4;
            }
            if (their_phase == 3)
                mat_weight = 2;
        }
        if (our_phase == 4 && our_queen_count) {
            if (their_phase == 2 && their_knight_count == 2)
                mat_weight = 2;
            if (their_phase == 2 && their_knight_count == 1)
                mat_weight = 8;
            if (their_phase == 2 && their_knight_count == 0)
                mat_weight = 7;
            if (their_phase == 3)
                mat_weight = 1;
            if (their_phase == 4)
                mat_weight = 1;
        }
        if (our_phase == 4 && our_rook_count == 2) {
            if (their_phase == 2 && their_rook_count == 0)
                mat_weight = 7;
            if (their_phase == 3)
                mat_weight = 2;
            if (their_phase == 4)
                mat_weight = 1;
        }
        if (our_phase == 4 && our_rook_count == 1) {
            if (their_phase == 3 && their_rook_count == 1)
                mat_weight = 3;
            if (their_phase == 3 && their_rook_count == 0)
                mat_weight = 2;
            if (their_phase == 4)
                mat_weight = 2;
        }
        if (our_phase == 4 && our_rook_count == 0 && our_queen_count == 0) {
            if (their_phase == 3 && their_rook_count == 1)
                mat_weight = 4;
            if (their_phase == 3 && their_rook_count == 0)
                mat_weight = 2;
            if (their_phase == 4 && their_queen_count)
                mat_weight = 8;
            if (their_phase == 4 && their_queen_count == 0)
                mat_weight = 1;
        }
        if (our_phase == 5 && our_queen_count) {
            if (their_phase == 4)
                mat_weight = 2;
            if (their_phase == 5)
                mat_weight = 1;
            if (their_phase == 4 && their_rook_count == 2) {
                if (our_knight_count)
                    mat_weight = 3;
                if (our_bishop_count)
                    mat_weight = 7;
            }
            if (their_phase == 5)
                mat_weight = 1;
        }
        if (our_phase == 5 && our_rook_count == 1) {
            if (their_phase == 4 && their_queen_count)
                mat_weight = 9;
            if (their_phase == 4 && their_rook_count == 2)
                mat_weight = 7;
            if (their_phase == 4 && their_rook_count == 1)
                mat_weight = 3;
            if (their_phase == 4 && their_queen_count == 0 && their_rook_count == 0)
                mat_weight = 1;
            if (their_phase == 5)
                mat_weight = 2;
        }
        if (our_phase == 5 && our_rook_count == 2) {
            if (their_phase == 4 && their_queen_count && our_bishop_count == 1)
                mat_weight = 8;
            if (their_phase == 4 && their_queen_count && our_knight_count == 1)
                mat_weight = 7;
            if (their_phase == 4 && their_rook_count == 2)
                mat_weight = 3;
            if (their_phase == 4 && their_rook_count == 1)
                mat_weight = 2;
            if (their_phase == 4 && their_queen_count == 0 && their_rook_count == 0)
                mat_weight = 1;
            if (their_phase == 5)
                mat_weight = 1;
        }
        if (our_phase == 6 && our_queen_count && our_rook_count) {
            if (their_phase == 4 && their_queen_count == 0 && their_rook_count == 0)
                mat_weight = 2;
            if (their_phase == 5 && their_queen_count)
                mat_weight = 1;
            if (their_phase == 4 && their_rook_count == 1)
                mat_weight = 6;
            if (their_phase == 4 && their_rook_count == 2)
                mat_weight = 3;
            if (their_phase == 5 && their_rook_count)
                mat_weight = 1;
            if (their_phase == 6)
                mat_weight = 1;
        }
        if (our_phase == 6 && our_queen_count && our_rook_count == 0) {
            if (their_phase == 4 && their_queen_count == 0 && their_rook_count == 0)
                mat_weight = 5;
            if (their_phase == 5 && their_queen_count)
                mat_weight = 2;
            if (their_phase == 5 && their_rook_count == 2)
                mat_weight = 2;
            if (their_phase == 5 && their_rook_count == 1)
                mat_weight = 1;
            if (their_phase == 6)
                mat_weight = 1;
        }
        if (our_phase == 6 && our_queen_count == 0 && our_rook_count == 2) {
            if (their_phase == 5 && their_queen_count)
                mat_weight = 7;
            if (their_phase == 5 && their_rook_count == 1)
                mat_weight = 1;
            if (their_phase == 5 && their_rook_count == 2)
                mat_weight = 2;
            if (their_phase == 6)
                mat_weight = 1;
        }
        if (our_phase == 6 && our_queen_count == 0 && our_rook_count == 1) {
            if (their_phase == 5 && their_queen_count)
                mat_weight = 9;
            if (their_phase == 5 && their_rook_count == 2)
                mat_weight = 3;
            if (their_phase == 5 && their_rook_count == 1)
                mat_weight = 2;
            if (their_phase == 6)
                mat_weight = 1;
            if (their_phase == 6 && their_queen_count)
                mat_weight = 2;
            if (their_phase == 6 && their_queen_count && their_rook_count)
                mat_weight = 4;
        }
        if (our_phase >= 7) {
            if (our_valuation > their_valuation + 4)
                mat_weight = 9;
            if (our_valuation == their_valuation + 4)
                mat_weight = 7;
            if (our_valuation == their_valuation + 3)
                mat_weight = 4;
            if (our_valuation == their_valuation + 2)
                mat_weight = 2;
            if (our_valuation < their_valuation + 2)
                mat_weight = 1;
        }
    }
    if (our_pawn_count == 1) {
        if (their_phase == 1) {
            if (our_phase == 1)
                mat_weight = 3;
            if (our_phase == 2 && our_knight_count == 2) {
                if (their_pawns_count == 0)
                    mat_weight = 3;
                else
                    mat_weight = 5;
            }
            if (our_phase == 2 && our_rook_count == 1)
                mat_weight = 7;
        }
        if (their_phase == 2 && their_rook_count == 1 && our_phase == 2 && our_rook_count == 1)
            mat_weight = 8;
        if (their_phase == 2 && their_rook_count == 0 && our_phase == 2)
            mat_weight = 4;
        if (their_phase >= 3 && their_minor_count > 0 && our_phase == their_phase)
            mat_weight = 3;
        if (their_phase >= 3 && their_minor_count == 0 && our_phase == their_phase)
            mat_weight = 5;
        if (their_phase == 4 && their_queen_count == 1 && our_phase == their_phase)
            mat_weight = 7;
    }
    ASSERT(mat_weight >= 0 && mat_weight <= 10);
    return mat_weight;
}

void initMaterial(void) {
    int score, openscore, midscore1, midscore2, endscore, summ1, summ2, phase, flags;
    int bp, wp, bn, wn, bb, wb, br, wr, bq, wq;

    memset(MaterialTable, 0, sizeof(MaterialTable));

    for (wq = 0; wq <= 1; wq++)
        for (bq = 0; bq <= 1; bq++)
            for (wr = 0; wr <= 2; wr++)
                for (br = 0; br <= 2; br++)
                    for (wb = 0; wb <= 2; wb++)
                        for (bb = 0; bb <= 2; bb++)
                            for (wn = 0; wn <= 2; wn++)
                                for (bn = 0; bn <= 2; bn++)
                                    for (wp = 0; wp <= 8; wp++)
                                        for (bp = 0; bp <= 8; bp++) {
                                            summ1 = wp * MatSummValue[PAWN] + wn * MatSummValue[KNIGHT] + wb * MatSummValue[BISHOP] +
                                                wr * MatSummValue[ROOK] + wq * MatSummValue[QUEEN];
                                            summ2 = bp * MatSummValue[PAWN] + bn * MatSummValue[KNIGHT] + bb * MatSummValue[BISHOP] +
                                                br * MatSummValue[ROOK] + bq * MatSummValue[QUEEN];

                                            phase = wb + bb + wn + bn + wq * 6 + bq * 6 + wr * 3 + br * 3;
                                            if (phase > 32) phase = 32;

                                            openscore =
                                                ((wq - bq) * QueenValueOpen)
                                                + ((wr - br) * RookValueOpen)
                                                + ((wb - bb) * BishopValueOpen)
                                                + ((wn - bn) * KnightValueOpen)
                                                + ((wp - bp) * PawnValueOpen)
                                                + ((wb >= 2) * BishopPairBonusOpen)
                                                - ((bb >= 2) * BishopPairBonusOpen)
                                                + ((wp - 5) * wn * 0) - ((bp - 5) * bn * 0)
                                                - ((wp - 5) * wr * 5) + ((bp - 5) * br * 5)
                                                - ((wr == 2) * 16) + ((br == 2) * 16)
                                                - ((wq + wr > 2) * 8) + ((bq + br > 2) * 8);

                                            midscore1 =
                                                ((wq - bq) * QueenValueMid1)
                                                + ((wr - br) * RookValueMid1)
                                                + ((wb - bb) * BishopValueMid1)
                                                + ((wn - bn) * KnightValueMid1)
                                                + ((wp - bp) * PawnValueMid1)
                                                + ((wb >= 2) * BishopPairBonusMid1)
                                                - ((bb >= 2) * BishopPairBonusMid1)
                                                + ((wp - 5) * wn * 2) - ((bp - 5) * bn * 2)
                                                - ((wp - 5) * wr * 4) + ((bp - 5) * br * 4)
                                                - ((wr == 2) * 20) + ((br == 2) * 20)
                                                - ((wq + wr > 2) * 10) + ((bq + br > 2) * 10);

                                            midscore2 =
                                                ((wq - bq) * QueenValueMid2)
                                                + ((wr - br) * RookValueMid2)
                                                + ((wb - bb) * BishopValueMid2)
                                                + ((wn - bn) * KnightValueMid2)
                                                + ((wp - bp) * PawnValueMid2)
                                                + ((wb >= 2) * BishopPairBonusMid2)
                                                - ((bb >= 2) * BishopPairBonusMid2)
                                                + ((wp - 5) * wn * 4) - ((bp - 5) * bn * 4)
                                                - ((wp - 5) * wr * 2) + ((bp - 5) * br * 2)
                                                - ((wr == 2) * 32) + ((br == 2) * 32)
                                                - ((wq + wr > 2) * 32) + ((bq + br > 2) * 32);

                                            endscore =
                                                ((wq - bq) * QueenValueEnd)
                                                + ((wr - br) * RookValueEnd)
                                                + ((wb - bb) * BishopValueEnd)
                                                + ((wn - bn) * KnightValueEnd)
                                                + ((wp - bp) * PawnValueEnd)
                                                + ((wb >= 2) * BishopPairBonusEnd)
                                                - ((bb >= 2) * BishopPairBonusEnd)
                                                + ((wp - 5) * wn * 5) - ((bp - 5) * bn * 5)
                                                - ((wp - 5) * wr * 0) + ((bp - 5) * br * 0)
                                                - ((wr == 2) * 48) + ((br == 2) * 48)
                                                - ((wq + wr > 2) * 48) + ((bq + br > 2) * 48);

                                            if (phase < 8) {
                                                endscore *= 8 - phase;
                                                midscore2 *= phase;
                                                score = (midscore2 + endscore) / 8;
                                            }
                                            else if (phase < 24) {
                                                midscore2 *= 24 - phase;
                                                midscore1 *= phase - 8;
                                                score = (midscore1 + midscore2) / 16;
                                            }
                                            else {
                                                midscore1 *= 32 - phase;
                                                openscore *= phase - 24;
                                                score = (openscore + midscore1) / 8;
                                            }

                                            if (score > 0) score = score * materialWeigths(wp, wn, wb, wr, wq, bp, bn, bb, br, bq) / 10;
                                            else if (score < 0) score = score * materialWeigths(bp, bn, bb, br, bq, wp, wn, wb, wr, wq) / 10;

                                            MaterialTable[summ1][summ2].value = score;

                                            flags = phase << 3;
                                            if (wq + bq + wr + br == 0 && wb == 1 && bb == 1 && (wn + bn) == 0 && ((wp - bp) < 3) && ((wp - bp) > -3)) flags |= 1;
                                            if (bq != 0 && (br + bb + bn) > 0) flags |= KingAtkPhaseMaskByColor[WHITE];
                                            if (wq != 0 && (wr + wb + wn) > 0) flags |= KingAtkPhaseMaskByColor[BLACK];

                                            MaterialTable[summ1][summ2].flags = flags;
                                        }
}
