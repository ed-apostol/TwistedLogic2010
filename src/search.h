/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

void ponderHit(search_info_t *si)
{
    int64 time = getTime() - si->start_time;

    ASSERT(si != NULL);

    if ((si->iteration >= 8 && (si->legalmoves == 1 || si->mate_found >= 3)) ||
        (time > si->time_limit_abs))
    {
        si->thinking_status = ABORTED;
        Print(2, "info string Has searched enough the ponder move: aborting\n");
    }
    else
    {
        si->thinking_status = THINKING;
        Print(2, "info string Switch from pondering to thinking\n");
    }
}

void check4Input(position_t *pos, search_info_t *si)
{
    static char input[256];

    ASSERT(si != NULL);

    if (biosKey())
    {
        if (fgets(input, 256, stdin) == NULL)
            strcpy(input, "quit\n");
        Print(2, "%s\n", input);
        if (strncasecmp(input, "quit", 4) == 0)
        {
            quit(pos);
        }
        else if (strncasecmp(input, "stop", 4) == 0)
        {
            si->thinking_status = ABORTED;
            Print(2, "info string Aborting search\n");
            return;
        }
        else if (strncasecmp(input, "ponderhit", 9) == 0)
        {
            ponderHit(si);
        }
        else if (strncasecmp(input, "isready", 7) == 0)
            Print(2, "readyok\n");
        else if (strncasecmp(input, "debug", 5) == 0)
        {
            /* dummy for now */
        }
    }
}

void initNode(position_t *pos, search_info_t *si)
{
    int64 time, time2;
    int i;

    ASSERT(si != NULL);

    si->nodes++;
    si->nodes_since_poll++;
    si->currline[pos->ply] = pos->lastmove;
    if (si->nodes_since_poll >= si->nodes_between_polls)
    {
        si->nodes_since_poll = 0;
        check4Input(pos, si);
        if (si->thinking_status == ABORTED) return;
        time2 = time = getTime();
        if (time - si->last_time > 1000)
        {
            si->last_time = time;
            time = time - si->start_time;
            Print(1, "info ");
            if (pos->uci_options->showcurrline)
            {
                Print(1, "currline ");
                for (i = 1; i <= pos->ply; i++) Print(1, "%s ", move2Str(si->currline[i]));
            }
            Print(1, "time %llu ", time);
            Print(1, "nodes %llu ", si->nodes);
            Print(1, "hashfull %d ", (pos->trans_table.used * 1000) / pos->trans_table.size);
            Print(1, "nps %llu ", (si->nodes * 1000ULL) / (time));
            Print(1, "\n");
        }
        if (si->thinking_status == THINKING && si->time_is_limited && time2 > si->time_limit_max)
        {
            if (time2 < si->time_limit_abs)
            {
                if (si->best_value != -INF && si->best_value + 30 <= si->last_value)
                {
                    si->time_limit_max += ((si->last_value - si->best_value - 20) / 10) * si->alloc_time / 2;
                    if (si->time_limit_max > si->time_limit_abs) si->time_limit_max = si->time_limit_abs;
                    Print(2, "info string Time is extended in search: time = %d\n", time2 - si->start_time);
                }
                else if (si->best_value == -INF && si->last_value + 30 <= si->last_last_value)
                {
                    si->time_limit_max += ((si->last_last_value - si->last_value - 20) / 10) * si->alloc_time / 2;
                    if (si->time_limit_max > si->time_limit_abs) si->time_limit_max = si->time_limit_abs;
                    Print(2, "info string Time is extended in search: time = %d\n", time2 - si->start_time);
                }
                else if (si->research || si->change)
                {
                    si->time_limit_max += si->alloc_time / 2;
                    if (si->time_limit_max > si->time_limit_abs) si->time_limit_max = si->time_limit_abs;
                    Print(2, "info string Time is extended in search: time = %d\n", time2 - si->start_time);
                }
                else
                {
                    si->thinking_status = ABORTED;
                    Print(2, "info string Aborting search: time spent = %d\n", time2 - si->start_time);
                }
            }
            else
            {
                si->thinking_status = ABORTED;
                Print(2, "info string Aborting search: time spent = %d\n", time2 - si->start_time);
            }
        }
    }
    if (si->thinking_status == THINKING && si->node_is_limited && si->nodes >= si->node_limit)
    {
        si->thinking_status = ABORTED;
        Print(2, "info string Aborting search: nodes spent = %x\n", si->nodes);
    }
}

int simpleStalemate(const position_t *pos)
{
    uint32 kpos, to;
    uint64 mv_bits;
    if (bitCnt(pos->color[pos->side] & ~pos->pawns) > 1) return FALSE;
    kpos = pos->kpos[pos->side];
    if (kpos != a1 && kpos != a8 && kpos != h1 && kpos != h8) return FALSE;
    mv_bits = KingMoves[kpos];
    while (mv_bits)
    {
        to = popFirstBit(&mv_bits);
        if (!isAtt(pos, pos->side ^ 1, BitMask[to])) return FALSE;
    }
    return TRUE;
}

void updatePV(uint32 m, uint32 ply, uint32 *pv, uint32 *newpv)
{
    int j;

    ASSERT(pv != NULL);
    ASSERT(newpv != NULL);
    ASSERT(moveIsOk(m));

    pv[ply] = m;
    for (j = ply + 1; newpv[j]; j++) pv[j] = newpv[j];
    pv[j] = 0;
}

void displayPV(const position_t *pos, search_info_t *si, uint32 *pv, int depth, int alpha, int beta, int score)
{
    uint64 time;
    int i;

    ASSERT(si != NULL);
    ASSERT(pv != NULL);
    ASSERT(valueIsOk(score));

    time = getTime();
    si->last_time = time;
    time = si->last_time - si->start_time;

    Print(1, "info depth %d seldepth %d ", depth, si->maxplysearched);

    if (abs(score) < (INF - MAXPLY))
    {
        if (score <= alpha) Print(1, "score cp %d upperbound ", score);
        else if (score >= beta) Print(1, "score cp %d lowerbound ", score);
        else Print(1, "score cp %d ", score);
    }
    else
    {
        Print(1, "score mate %d ", (score > 0) ? (INF - score + 1) / 2 : -(INF + score) / 2);
    }

    Print(1, "time %llu ", time);
    Print(1, "nodes %llu ", si->nodes);
    Print(1, "hashfull %d ", (pos->trans_table.used * 1000) / pos->trans_table.size);
    if (time > 1000) Print(1, "nps %llu ", (si->nodes * 1000) / (time));
    Print(1, "pv ");
    for (i = 0; pv[i]; i++) Print(1, "%s ", move2Str(pv[i]));
    Print(1, "\n");
}

int moveIsTactical(uint32 m)
{
    ASSERT(moveIsOk(m));
    return (m & 0x01fe0000UL);
}

int historyIndex(uint32 side, uint32 move)
{
    return ((((side) << 9) + ((movePiece(move)) << 6) + (moveTo(move))) & 0x3ff);
}

int extDepth(const position_t * pos, search_info_t * si, move_t * move, int depth, BOOL *beSelective, BOOL incheck, BOOL single_reply, BOOL onPV)
{
    static const int checkply[2] = { 1,2 };
    static const int singularply[2] = { 2,2 };
    static const int pawnpushto7thply[2] = { 1,1 };
    static const int passedpawnpushply[2] = { 0,1 };
    static const int recaptureply[2] = { 1,1 };
    static const int pawnendgameply[2] = { 2,2 };
    int extend = 0;
    *beSelective = TRUE;
    if (incheck) extend += checkply[onPV];
    if (single_reply) extend += singularply[onPV];
    if (movePiece(move->m) == PAWN)
    {
        if (PAWN_RANK(moveTo(move->m), pos->side) == Rank7) extend += pawnpushto7thply[onPV];
        if (moveIsPassedPawn(pos, move->m)) extend += passedpawnpushply[onPV];
    }
    if (moveCapture(pos->lastmove) >= KNIGHT)
    {
        if (onPV && swap(pos, move->m) >= 0) extend += recaptureply[onPV];
        if (bitCnt(pos->occupied & ~pos->pawns) <= 7) extend += pawnendgameply[onPV];
    }
    if (extend || isCastle(move->m) || moveIsTactical(move->m) || move->m == si->transmove[pos->ply]) *beSelective = FALSE;
    return depth - 1 + MIN(1, (extend / 2));
}

int qSearchEvasion(position_t *pos, search_info_t *si, int alpha, int beta, int depth, uint32 oldPV[])
{
    int bestvalue = -INF;
    int score;
    int moveGivesCheck;
    uint32 newPV[MAXPLY];
    uint64 pinned;
    uint64 dcc;
    uint64 target = pos->color[pos->side ^ 1] & ~pos->kings;
    move_t *move;
    sort_t sort;
    undo_t undo;

    ASSERT(pos != NULL);
    ASSERT(si != NULL);
    ASSERT(oldPV != NULL);
    ASSERT(valueIsOk(alpha));
    ASSERT(valueIsOk(beta));
    ASSERT(alpha < beta);
    ASSERT(kingIsInCheck(pos));

    oldPV[pos->ply] = EMPTY;
    initNode(pos, si);
    if (si->thinking_status == ABORTED) return 0;
    if (pos->ply > si->maxplysearched) si->maxplysearched = pos->ply;
    if (reps(pos, 1)) return 0;
    if ((pos->ply >= MAXPLY - 1) || (pos->sp >= MAXDATA - 1)) return eval(pos);
    alpha = MAX(-INF + pos->ply, alpha);
    beta = MIN(INF - pos->ply - 1, beta);
    if (alpha >= beta) return alpha;
    pinned = pinnedPieces(pos, pos->side);
    dcc = discoveredCheckCandidates(pos, pos->side);
    sortInit(pos, &sort, si, pinned, target, depth, MoveGenPhaseEvasion);
    while ((move = sortNext(pos, &sort, si)) != NULL)
    {
        moveGivesCheck = moveIsCheck(pos, move->m, dcc);
        if (bestvalue > -INF + MAXPLY && !moveGivesCheck && !moveIsTactical(move->m) && movePiece(move->m) != KING
            && !(pos->castle | (pos->side == WHITE ? (WCKS | WCQS) : (BCKS | BCKS))) && swap(pos, move->m) < 0) continue;
        makeMove(pos, &undo, move->m);
        if (moveGivesCheck) score = -qSearchEvasion(pos, si, -beta, -alpha, depth, newPV);
        else score = -qSearch(pos, si, -beta, -alpha, depth - 1, newPV);
        unmakeMove(pos, &undo);
        if (si->thinking_status == ABORTED) return 0;
        if (score > bestvalue)
        {
            bestvalue = score;
            updatePV(move->m, pos->ply, oldPV, newPV);
            if (score > alpha)
            {
                alpha = score;
                if (score >= beta) return score;
            }
        }
    }
    if (bestvalue == -INF) return -INF + pos->ply;
    ASSERT(valueIsOk(bestvalue));
    return bestvalue;
}

int qSearch(position_t *pos, search_info_t *si, int alpha, int beta, int depth, uint32 oldPV[])
{
    int evalvalue = -INF;
    int bestvalue = -INF;
    int score;
    int moveGivesCheck;
    uint32 newPV[MAXPLY];
    uint64 pinned;
    uint64 dcc;
    uint64 target = pos->color[pos->side ^ 1] & ~pos->kings;
    move_t *move;
    sort_t sort;
    undo_t undo;
    uci_option_t *opt = pos->uci_options;

    ASSERT(pos != NULL);
    ASSERT(si != NULL);
    ASSERT(oldPV != NULL);
    ASSERT(valueIsOk(alpha));
    ASSERT(valueIsOk(beta));
    ASSERT(alpha < beta);
    ASSERT(!kingIsInCheck(pos));

    oldPV[pos->ply] = EMPTY;
    initNode(pos, si);
    if (si->thinking_status == ABORTED) return 0;
    if (pos->ply > si->maxplysearched) si->maxplysearched = pos->ply;
    if (reps(pos, 1)) return 0;
    if ((pos->ply >= MAXPLY - 1) || (pos->sp >= MAXDATA - 1)) return eval(pos);
    alpha = MAX(-INF + pos->ply, alpha);
    beta = MIN(INF - pos->ply - 1, beta);
    if (alpha >= beta) return alpha;
    if (simpleStalemate(pos)) return 0;
    evalvalue = bestvalue = eval(pos);
    if (evalvalue > alpha)
    {
        alpha = evalvalue;
        if (evalvalue >= beta) return evalvalue;
    }
    else if (opt->try_delta && (beta == alpha + 1) && evalvalue < (alpha - (PawnValue + opt->delta)))
    {
        target &= ~(pos->pawns&~Rank7ByColorBB[pos->side ^ 1]);
        if (evalvalue < (alpha - (KnightValue + opt->delta)) && (target & (target - 1)))
        {
            static const int Shift[] = { 9, 7 };
            dcc = (((*ShiftPtr[pos->side])(pos->pawns&pos->color[pos->side] & Rank7ByColorBB[pos->side], Shift[pos->side]) & ~FileABB)
                | ((*ShiftPtr[pos->side])(pos->pawns&pos->color[pos->side] & Rank7ByColorBB[pos->side], Shift[pos->side ^ 1]) & ~FileHBB));
            target &= ~((pos->knights | pos->bishops)&~dcc);
            if (evalvalue < (alpha - (RookValue + opt->delta)))
            {
                target &= ~(pos->rooks&~dcc);
            }
        }
    }
    pinned = pinnedPieces(pos, pos->side);
    dcc = discoveredCheckCandidates(pos, pos->side);
    sortInit(pos, &sort, si, pinned, target, depth, (depth >= 0) ? MoveGenPhaseQuiescenceAndChecks : MoveGenPhaseQuiescence);
    while ((move = sortNext(pos, &sort, si)) != NULL)
    {
        moveGivesCheck = moveIsCheck(pos, move->m, dcc);
        makeMove(pos, &undo, move->m);
        if (moveGivesCheck) score = -qSearchEvasion(pos, si, -beta, -alpha, depth, newPV);
        else score = -qSearch(pos, si, -beta, -alpha, depth - 1, newPV);
        unmakeMove(pos, &undo);
        if (si->thinking_status == ABORTED) return 0;
        if (score > bestvalue)
        {
            bestvalue = score;
            updatePV(move->m, pos->ply, oldPV, newPV);
            if (score > alpha)
            {
                alpha = score;
                if (score >= beta) return score;
            }
        }
    }
    ASSERT(valueIsOk(bestvalue));
    return bestvalue;
}

int searchEvasion(position_t *pos, search_info_t *si, int beta, int depth, uint32 oldPV[])
{
    int bestvalue = -INF;
    int score;
    int newdepth;
    int moveGivesCheck;
    int played = 0;
    BOOL beSelective;
    uint32 bestmove = EMPTY;
    uint32 newPV[MAXPLY];
    uint64 pinned;
    uint64 dcc;
    uint64 target = pos->color[pos->side ^ 1] & ~pos->kings;
    move_t *move;
    sort_t sort;
    undo_t undo;
    trans_entry_t * entry = NULL;
    uci_option_t *opt = pos->uci_options;

    ASSERT(pos != NULL);
    ASSERT(si != NULL);
    ASSERT(oldPV != NULL);
    ASSERT(valueIsOk(beta));
    ASSERT(kingIsInCheck(pos));

    oldPV[pos->ply] = EMPTY;
    si->transmove[pos->ply] = EMPTY;
    initNode(pos, si);
    if (si->thinking_status == ABORTED) return 0;
    if (pos->ply > si->maxplysearched) si->maxplysearched = pos->ply;
    if (reps(pos, 1)) return 0;
    if (-INF + pos->ply >= beta) return beta;
    if (INF - pos->ply - 1 < beta) return beta - 1;
    if (opt->try_hash) entry = transProbe(pos);
    if (entry != NULL)
    {
        si->transmove[pos->ply] = transMove(entry);
        if (depth <= transMindepth(entry) && transMinvalue(entry) >= beta) return transMinvalue(entry);
        if (depth <= transMaxdepth(entry) && transMaxvalue(entry) < beta) return transMaxvalue(entry);
    }
    if ((pos->ply >= MAXPLY - 1) || (pos->sp >= MAXDATA - 1)) return eval(pos);
    pinned = pinnedPieces(pos, pos->side);
    dcc = discoveredCheckCandidates(pos, pos->side);
    sortInit(pos, &sort, si, pinned, target, depth, MoveGenPhaseEvasion);
    while ((move = sortNext(pos, &sort, si)) != NULL)
    {
        moveGivesCheck = moveIsCheck(pos, move->m, dcc);
        newdepth = extDepth(pos, si, move, depth, &beSelective, moveGivesCheck, sort.size == 1, FALSE);
        makeMove(pos, &undo, move->m);
        played++;
        if (moveGivesCheck)
        {
            if (newdepth <= 0) score = -qSearchEvasion(pos, si, -beta, 1 - beta, 0, newPV);
            else score = -searchEvasion(pos, si, 1 - beta, newdepth, newPV);
        }
        else if (newdepth <= 0)
        {
            score = -qSearch(pos, si, -beta, 1 - beta, 0, newPV);
        }
        else
        {
            int newdepthclone = newdepth;
            if (beSelective)
            {
                if (opt->try_lmr1 && depth >= opt->lmr1_depth && played >= opt->lmr1_num)
                {
                    newdepthclone--;
                    if (opt->try_lmr2 && depth >= opt->lmr2_depth && played >= opt->lmr2_num)
                        newdepthclone--;
                }
            }
            if (newdepthclone <= 0) score = -qSearch(pos, si, -beta, 1 - beta, 0, newPV);
            else score = -searchZero(pos, si, 1 - beta, newdepthclone, newPV, TRUE);
            if (si->thinking_status != ABORTED && newdepthclone < newdepth && score >= beta)
            {
                score = -searchZero(pos, si, 1 - beta, newdepth, newPV, FALSE);
            }
        }
        unmakeMove(pos, &undo);
        if (si->thinking_status == ABORTED) return 0;
        if (score > bestvalue)
        {
            bestvalue = score;
            updatePV(move->m, pos->ply, oldPV, newPV);
            if (score >= beta)
            {
                bestmove = move->m;
                goto cut;
            }
        }
    }
    if (bestvalue == -INF) bestvalue = -INF + pos->ply;
cut:
    if (opt->try_hash) transStore(pos, bestmove, depth, ((bestvalue >= beta) ? bestvalue : -INF), ((bestvalue < beta) ? bestvalue : INF), 0);
    ASSERT(valueIsOk(bestvalue));
    return bestvalue;
}

int searchSelective(position_t *pos, search_info_t *si, int beta, int depth, uint32 oldPV[], uint32 movebanned)
{
    int bestvalue = -INF;
    int score;
    int newdepth;
    int moveGivesCheck;
    int phase;
    int played = 0;
    BOOL beSelective;
    uint32 newPV[MAXPLY];
    uint64 pinned;
    uint64 dcc;
    uint64 target = pos->color[pos->side ^ 1] & ~pos->kings;
    move_t *move;
    sort_t sort;
    undo_t undo;
    uci_option_t *opt = pos->uci_options;

    ASSERT(pos != NULL);
    ASSERT(si != NULL);
    ASSERT(oldPV != NULL);
    ASSERT(valueIsOk(beta));
    ASSERT(!kingIsInCheck(pos));
    ASSERT(depth >= 1);

    oldPV[pos->ply] = EMPTY;
    initNode(pos, si);
    if (si->thinking_status == ABORTED) return 0;

    pinned = pinnedPieces(pos, pos->side);
    dcc = discoveredCheckCandidates(pos, pos->side);

    phase = MoveGenPhaseStandard;
    sortInit(pos, &sort, si, pinned, target, depth, phase);
    while ((move = sortNext(pos, &sort, si)) != NULL)
    {
        if (movebanned == move->m) continue;
        moveGivesCheck = moveIsCheck(pos, move->m, dcc);
        newdepth = extDepth(pos, si, move, depth, &beSelective, moveGivesCheck, FALSE, FALSE);
        makeMove(pos, &undo, move->m);
        played++;
        if (moveGivesCheck)
        {
            if (newdepth <= 0) score = -qSearchEvasion(pos, si, -beta, 1 - beta, 0, newPV);
            else score = -searchEvasion(pos, si, 1 - beta, newdepth, newPV);
        }
        else if (newdepth <= 0)
        {
            score = -qSearch(pos, si, -beta, 1 - beta, 0, newPV);
        }
        else
        {
            int newdepthclone = newdepth;
            if (beSelective)
            {
                if (opt->try_lmr1 && depth >= opt->lmr1_depth && played >= opt->lmr1_num)
                {
                    newdepthclone--;
                    if (opt->try_lmr2 && depth >= opt->lmr2_depth && played >= opt->lmr2_num)
                        newdepthclone--;
                }
            }
            if (newdepthclone <= 0) score = -qSearch(pos, si, -beta, 1 - beta, 0, newPV);
            else score = -searchZero(pos, si, 1 - beta, newdepthclone, newPV, TRUE);
            if (si->thinking_status != ABORTED && newdepthclone < newdepth && score >= beta)
            {
                score = -searchZero(pos, si, 1 - beta, newdepth, newPV, FALSE);
            }
        }
        unmakeMove(pos, &undo);
        if (si->thinking_status == ABORTED) return 0;
        if (score > bestvalue)
        {
            bestvalue = score;
            if (score >= beta)
            {
                goto cut;
            }
        }
    }
cut:
    ASSERT(valueIsOk(bestvalue));
    return bestvalue;
}

int searchZero(position_t *pos, search_info_t *si, int beta, int depth, uint32 oldPV[], BOOL donull)
{
    int bestvalue = -INF;
    int evalvalue;
    int score;
    int newdepth;
    int moveGivesCheck;
    int phase;
    int played = 0;
    int firstExtend = FALSE;
    int transMinDepth = 0;
    int transMinScore = -INF;
    BOOL beSelective;
    uint32 bestmove = EMPTY;
    uint32 newPV[MAXPLY];
    uint64 pinned;
    uint64 dcc;
    uint64 target = pos->color[pos->side ^ 1] & ~pos->kings;
    move_t *move;
    sort_t sort;
    undo_t undo;
    trans_entry_t * entry = NULL;
    uci_option_t *opt = pos->uci_options;

    ASSERT(pos != NULL);
    ASSERT(si != NULL);
    ASSERT(oldPV != NULL);
    ASSERT(valueIsOk(beta));
    ASSERT(!kingIsInCheck(pos));
    ASSERT(depth >= 1);

    oldPV[pos->ply] = EMPTY;
    si->transmove[pos->ply] = EMPTY;
    initNode(pos, si);
    if (si->thinking_status == ABORTED) return 0;
    if (pos->ply > si->maxplysearched) si->maxplysearched = pos->ply;
    if (reps(pos, 1)) return 0;
    if (-INF + pos->ply >= beta) return beta;
    if (INF - pos->ply - 1 < beta) return beta - 1;
    if (opt->try_hash) entry = transProbe(pos);
    if (entry != NULL)
    {
        si->transmove[pos->ply] = transMove(entry);
        transMinScore = transMinvalue(entry);
        transMinDepth = transMindepth(entry);
        if (depth <= transMinDepth && transMinScore >= beta) return transMinScore;
        if (depth <= transMaxdepth(entry) && transMaxvalue(entry) < beta) return transMaxvalue(entry);
    }
    if ((pos->ply >= MAXPLY - 1) || (pos->sp >= MAXDATA - 1)) return eval(pos);
    evalvalue = eval(pos);

    if (donull && (pos->color[pos->side] & ~(pos->pawns | pos->kings))
        && beta > -INF + MAXPLY && beta < INF - MAXPLY
        && (pos->color[WHITE] & pos->pawns & Rank7BB) == 0
        && (pos->color[BLACK] & pos->pawns & Rank2BB) == 0)
    {
        int guess = depth * depth*opt->raz_margin;
        if (evalvalue > beta + guess) return evalvalue - guess;
    }

    if (opt->try_null && donull && depth >= 2
        && (bitCnt(pos->color[pos->side] & ~pos->pawns) >= 2)
        && beta > -INF + MAXPLY && beta < INF - MAXPLY && evalvalue >= beta)
    {
        int R = 3 + (depth / 5) + (evalvalue - beta > 75);
        makeNullMove(pos, &undo);
        if (depth <= R)  score = -qSearch(pos, si, -beta, 1 - beta, 0, newPV);
        else score = -searchZero(pos, si, 1 - beta, depth - R, newPV, FALSE);
        unmakeNullMove(pos, &undo);
        if (si->thinking_status == ABORTED) return 0;
        if (score >= beta)
        {
            if (depth <= 6 || bitCnt(pos->color[pos->side] & ~pos->pawns) >= 3) return score;
            score = searchZero(pos, si, beta, depth - 6, newPV, FALSE);
            if (si->thinking_status == ABORTED) return 0;
            if (score >= beta) return score;
        }
    }

    pinned = pinnedPieces(pos, pos->side);
    dcc = discoveredCheckCandidates(pos, pos->side);

    if (depth >= 8)
    {
        if (si->transmove[pos->ply] == EMPTY)
        {
            transMinDepth = depth / 2;
            transMinScore = searchZero(pos, si, beta, transMinDepth, newPV, FALSE);
            if (si->thinking_status == ABORTED) return 0;
            si->transmove[pos->ply] = newPV[pos->ply];
        }
        if (si->transmove[pos->ply] != EMPTY && transMinDepth >= depth - 3 && transMinScore != -INF
            && !moveIsCheck(pos, si->transmove[pos->ply], dcc))
        {
            int targetScore = transMinScore - 150;
            int value = searchSelective(pos, si, targetScore, depth / 2, newPV, si->transmove[pos->ply]);
            if (si->thinking_status == ABORTED) return 0;
            if (value < targetScore) firstExtend = TRUE;
        }
    }

    phase = MoveGenPhaseStandard;
    if (opt->try_raz && (pos->color[pos->side ^ 1] & ~(pos->pawns | pos->kings)))
    {
        score = evalvalue + ((depth - 1) * (depth - 1) + 1) * opt->raz_mul;
        if (score < beta)
        {
            bestvalue = score;
            if (si->transmove[pos->ply] == EMPTY) phase = MoveGenPhaseSelectiveTactical;
            else phase = MoveGenPhaseSelective;
        }
    }
    sortInit(pos, &sort, si, pinned, target, depth, phase);
    while ((move = sortNext(pos, &sort, si)) != NULL)
    {
        moveGivesCheck = moveIsCheck(pos, move->m, dcc);
        if (opt->try_prunerazor && phase != MoveGenPhaseStandard && !moveGivesCheck)
        {
            if ((MoveGenPhase[sort.phase] == PH_BAD_CAPTURES)
                && DISTANCE(moveTo(move->m), pos->kpos[pos->side]) >= 3
                && DISTANCE(moveTo(move->m), pos->kpos[pos->side ^ 1]) >= 3
                && evalvalue + (depth - 1) * (depth - 1) * opt->raz_margin + PcValSEE[moveCapture(move->m)] < beta)
                continue;
            if ((MoveGenPhase[sort.phase] == PH_QUIET_PASSEDPAWNS_PURE ||
                MoveGenPhase[sort.phase] == PH_QUIET_PASSEDPAWNS) &&
                (evalvalue + BishopValue + depth * depth * opt->raz_margin <= beta
                    || DISTANCE(moveTo(move->m), PAWN_PROMOTE(moveTo(move->m), pos->side)) > depth / 2 + 2))
                continue;
        }
        newdepth = extDepth(pos, si, move, depth, &beSelective, moveGivesCheck, FALSE, FALSE);
        if (newdepth < depth && move->m == si->transmove[pos->ply] && firstExtend) newdepth++;
        makeMove(pos, &undo, move->m);
        played++;
        if (moveGivesCheck)
        {
            if (newdepth <= 0) score = -qSearchEvasion(pos, si, -beta, 1 - beta, 0, newPV);
            else score = -searchEvasion(pos, si, 1 - beta, newdepth, newPV);
        }
        else if (newdepth <= 0)
        {
            score = -qSearch(pos, si, -beta, 1 - beta, 0, newPV);
        }
        else
        {
            int newdepthclone = newdepth;
            if (beSelective)
            {
                if (opt->try_lmr1 && depth >= opt->lmr1_depth && played >= opt->lmr1_num)
                {
                    newdepthclone--;
                    if (opt->try_lmr2 && depth >= opt->lmr2_depth && played >= opt->lmr2_num)
                        newdepthclone--;
                }
            }
            if (newdepthclone <= 0) score = -qSearch(pos, si, -beta, 1 - beta, 0, newPV);
            else score = -searchZero(pos, si, 1 - beta, newdepthclone, newPV, TRUE);
            if (si->thinking_status != ABORTED && newdepthclone < newdepth && score >= beta)
            {
                score = -searchZero(pos, si, 1 - beta, newdepth, newPV, FALSE);
            }
        }
        unmakeMove(pos, &undo);
        if (si->thinking_status == ABORTED) return 0;
        if (score > bestvalue)
        {
            bestvalue = score;
            updatePV(move->m, pos->ply, oldPV, newPV);
            if (score >= beta)
            {
                bestmove = move->m;
                goto cut;
            }
        }
    }
    if (bestvalue == -INF)  bestvalue = 0;
cut:
    if (bestmove != EMPTY && !moveIsTactical(bestmove))
    {
        int index = historyIndex(pos->side, bestmove);
        si->history[index] += (depth * depth);
        if (si->history[index] >= MAXHIST)
        {
            int i;
            for (i = 0; i < 1024; i++)
                si->history[i] = (si->history[i] + 1) / 2;
        }
        if (si->killer1[pos->ply] != bestmove)
        {
            si->killer2[pos->ply] = si->killer1[pos->ply];
            si->killer1[pos->ply] = bestmove;
        }
    }
    if (opt->try_hash) transStore(pos, bestmove, depth, ((bestvalue >= beta) ? bestvalue : -INF), ((bestvalue < beta) ? bestvalue : INF), 0);
    ASSERT(valueIsOk(bestvalue));
    return bestvalue;
}

int searchPV(position_t *pos, search_info_t *si, int alpha, int beta, int depth, int inCheck, uint32 oldPV[])
{
    int bestvalue = -INF;
    int oldalpha = alpha;
    int score;
    int newdepth;
    int moveGivesCheck;
    int played = 0;
    int firstExtend = FALSE;
    int transMinDepth = 0;
    int transMinScore = -INF;
    BOOL beSelective;
    uint32 bestmove = EMPTY;
    uint32 newPV[MAXPLY];
    uint64 pinned;
    uint64 dcc;
    uint64 target = pos->color[pos->side ^ 1] & ~pos->kings;
    move_t *move;
    sort_t sort;
    undo_t undo;
    trans_entry_t * entry = NULL;
    uci_option_t *opt = pos->uci_options;

    ASSERT(pos != NULL);
    ASSERT(si != NULL);
    ASSERT(oldPV != NULL);
    ASSERT(valueIsOk(alpha));
    ASSERT(valueIsOk(beta));
    ASSERT(alpha < beta);
    ASSERT(depth >= 1);

    oldPV[pos->ply] = EMPTY;
    si->transmove[pos->ply] = EMPTY;
    initNode(pos, si);
    if (si->thinking_status == ABORTED) return 0;
    if (pos->ply > si->maxplysearched) si->maxplysearched = pos->ply;
    if (reps(pos, 1) || pos->fifty >= 100) return 0;
    alpha = MAX(-INF + pos->ply, alpha);
    beta = MIN(INF - pos->ply - 1, beta);
    if (alpha >= beta) return alpha;
    if (opt->try_hash) entry = transProbe(pos);
    if (entry != NULL)
    {
        si->transmove[pos->ply] = transMove(entry);
        transMinScore = transMinvalue(entry);
        transMinDepth = transMindepth(entry);
    }
    if ((pos->ply >= MAXPLY - 1) || (pos->sp >= MAXDATA - 1)) return eval(pos);
    if (opt->try_iid && si->transmove[pos->ply] == EMPTY && depth >= opt->iid_depth)
    {
        transMinDepth = depth / 2;
        transMinScore = searchPV(pos, si, alpha, beta, depth - 2, inCheck, newPV);
        if (si->thinking_status == ABORTED) return 0;
        /*if (transMinScore <= alpha) {
            transMinScore = searchPV(pos, si, -INF, beta, depth-2, inCheck, newPV);
            if (si->thinking_status == ABORTED) return 0;
        }*/
        si->transmove[pos->ply] = newPV[pos->ply];
    }

    pinned = pinnedPieces(pos, pos->side);
    dcc = discoveredCheckCandidates(pos, pos->side);

    if (depth >= 6 && si->transmove[pos->ply] != EMPTY && transMinDepth >= depth - 3
        && transMinScore != -INF && !moveIsCheck(pos, si->transmove[pos->ply], dcc))
    {
        int targetDepth = depth - 2;
        int targetScore = transMinScore - 150;
        int value = searchSelective(pos, si, targetScore, targetDepth, newPV, si->transmove[pos->ply]);
        if (si->thinking_status == ABORTED) return 0;
        if (value < targetScore) firstExtend = TRUE;
    }

    sortInit(pos, &sort, si, pinned, target, depth, (inCheck ? MoveGenPhaseEvasion : MoveGenPhaseStandard));
    while ((move = sortNext(pos, &sort, si)) != NULL)
    {
        moveGivesCheck = moveIsCheck(pos, move->m, dcc);
        newdepth = extDepth(pos, si, move, depth, &beSelective, moveGivesCheck, (inCheck && sort.size == 1), TRUE);
        if (newdepth < depth && move->m == si->transmove[pos->ply] && firstExtend) newdepth++;
        makeMove(pos, &undo, move->m);
        played++;
        if (bestvalue == -INF)
        {
            if (newdepth <= 0)
            {
                ASSERT(!moveGivesCheck);
                score = -qSearch(pos, si, -beta, -alpha, 0, newPV);
            }
            else score = -searchPV(pos, si, -beta, -alpha, newdepth, moveGivesCheck, newPV);
        }
        else
        {
            if (moveGivesCheck)
            {
                if (newdepth <= 0) score = -qSearchEvasion(pos, si, -alpha - 1, -alpha, 0, newPV);
                else score = -searchEvasion(pos, si, -alpha, newdepth, newPV);
            }
            else if (newdepth <= 0)
            {
                score = -qSearch(pos, si, -alpha - 1, -alpha, 0, newPV);
            }
            else
            {
                int newdepthclone = newdepth;
                if (beSelective && MoveGenPhase[sort.phase] == PH_QUIET_MOVES)
                {
                    if (opt->try_pvlmr1 && depth >= opt->lmr1_depth && played >= opt->pvlmr_num) newdepthclone--;
                }
                if (newdepthclone <= 0) score = -qSearch(pos, si, -alpha - 1, -alpha, 0, newPV);
                else score = -searchZero(pos, si, -alpha, newdepthclone, newPV, TRUE);
                if (si->thinking_status != ABORTED && newdepthclone < newdepth && score > alpha)
                {
                    score = -searchZero(pos, si, -alpha, newdepth, newPV, FALSE);
                }
            }
            if (si->thinking_status != ABORTED && score > alpha)
            {
                if (newdepth <= 0) score = -qSearch(pos, si, -beta, -alpha, 0, newPV);
                else score = -searchPV(pos, si, -beta, -alpha, newdepth, moveGivesCheck, newPV);
            }
        }
        unmakeMove(pos, &undo);
        if (si->thinking_status == ABORTED) return 0;
        if (score > bestvalue)
        {
            bestvalue = score;
            updatePV(move->m, pos->ply, oldPV, newPV);
            if (score > alpha)
            {
                bestmove = move->m;
                alpha = score;
                if (score >= beta) goto cut;
            }
        }
    }
    if (bestvalue == -INF)
    {
        if (inCheck) bestvalue = -INF + pos->ply;
        else bestvalue = 0;
    }
cut:
    if (bestmove != EMPTY && !moveIsTactical(bestmove) && bestvalue >= beta)
    {
        int index = historyIndex(pos->side, bestmove);
        si->history[index] += (depth * depth);
        if (si->history[index] >= MAXHIST)
        {
            int i;
            for (i = 0; i < 1024; i++)
                si->history[i] = (si->history[i] + 1) / 2;
        }
        if (si->killer1[pos->ply] != bestmove)
        {
            si->killer2[pos->ply] = si->killer1[pos->ply];
            si->killer1[pos->ply] = bestmove;
        }
    }

    if (opt->try_hash) transStore(pos, bestmove, depth, ((bestvalue > oldalpha) ? bestvalue : -INF), ((bestvalue < beta) ? bestvalue : INF), 0);
    ASSERT(valueIsOk(bestvalue));
    return bestvalue;
}

void initGameOptions(uci_option_t *opt)
{
    opt->time_buffer = optionGetInt("Time Buffer");
    opt->try_hash = optionGetBool("Transposition Table");
    opt->try_book = optionGetBool("OwnBook");
    opt->book_limit = optionGetInt("Book Move Limit");
    opt->try_null = optionGetBool("Null Move Pruning");
    opt->try_iid = optionGetBool("Internal Iterative Deepening");
    opt->iid_depth = optionGetInt("Internal Iterative Deepening Depth");
    opt->try_delta = optionGetBool("Delta Pruning");
    opt->delta = (optionGetInt("Delta Pruning Threshold") * PawnValue) / 100;
    opt->qdepth = optionGetInt("Quiescence Check Depth");
    opt->try_raz = optionGetBool("Zero Pruning");
    opt->raz_margin = (optionGetInt("Zero Pruning Threshold") * PawnValue) / 100;
    opt->raz_mul = (optionGetInt("Zero Pruning Multiplier") * PawnValue) / 100;
    opt->try_prunerazor = optionGetBool("Razor Pruning");
    opt->try_lmr1 = optionGetBool("Speculation");
    opt->lmr1_depth = optionGetInt("Speculation Base");
    opt->lmr1_num = optionGetInt("Speculation Level");
    opt->try_lmr2 = optionGetBool("Extended Speculation");
    opt->lmr2_depth = optionGetInt("Extended Speculation Base");
    opt->lmr2_num = optionGetInt("Extended Speculation Level");
    opt->try_pvlmr1 = optionGetBool("Speculation PV");
    opt->pvlmr_num = optionGetInt("Speculation PV Level");
    opt->king_atks_mul = optionGetInt("King Attacks Aggression Level");
    opt->pc_atks_mid_mul = optionGetInt("Piece Attacks Midgame");
    opt->pc_atks_end_mul = optionGetInt("Piece Attacks Endgame");
    opt->showcurrline = optionGetBool("UCI_ShowCurrLine");
}

void searchRoot(position_t *pos, search_info_t *si, sort_t *sort, uint64 dcc, int alpha, int beta, int id)
{
    int bestmoveindex;
    int ndepth;
    int score = 0;
    int oldalpha = alpha;
    BOOL moveGivesCheck;
    BOOL beSelective;
    uint64 lastnodes;
    uint64 maxnodes;
    uint32 newPV[MAXPLY];
    int64 time;
    move_t *move;
    undo_t undo;
    uint32 rootPV[MAXPLY];

    rootPV[pos->ply] = 0;
    sort->pos = 0;
    si->best_value = -INF;
    si->iteration = id;
    si->maxplysearched = 0;
    si->change = 0;
    si->research = 0;
    maxnodes = 0;
    bestmoveindex = 0;
    while ((move = getMove(sort)) != NULL)
    {
        si->currmovenumber = sort->pos;
        si->currline[pos->ply] = pos->lastmove;
        if (getTime() - si->start_time > 10000) Print(1, "info currmove %s currmovenumber %d\n", move2Str(move->m), sort->pos);
        moveGivesCheck = moveIsCheck(pos, move->m, dcc);
        ndepth = extDepth(pos, si, move, id, &beSelective, moveGivesCheck, (si->legalmoves == 1), TRUE);
        makeMove(pos, &undo, move->m);
        lastnodes = si->nodes;
        if (ndepth <= 0)
        {
            ASSERT(!moveGivesCheck);
            score = -qSearch(pos, si, -beta, -alpha, 0, newPV);
        }
        else if (si->best_value != -INF)
        {
            if (moveGivesCheck) score = -searchEvasion(pos, si, -alpha, ndepth, newPV);
            else score = -searchZero(pos, si, -alpha, ndepth, newPV, TRUE);
            if (si->thinking_status != ABORTED && score > alpha)
            {
                si->research = 1;
                if (getTime() - si->start_time > 10000) Print(1, "info currmove %s currmovenumber %d\n", move2Str(move->m), sort->pos);
                score = -searchPV(pos, si, -beta, -alpha, ndepth, moveGivesCheck, newPV);
                si->research = 0;
            }
        }
        else score = -searchPV(pos, si, -beta, -alpha, ndepth, moveGivesCheck, newPV);
        unmakeMove(pos, &undo);
        if (si->thinking_status == ABORTED) break;
        move->s = (si->nodes - lastnodes) >> 13;
        if (move->s > maxnodes) maxnodes = move->s;
        if (score > si->best_value)
        {
            si->best_value = score;
            updatePV(move->m, pos->ply, rootPV, newPV);
            if (score > alpha)
            {
                alpha = score;
                si->bestmove = rootPV[0];
                si->pondermove = rootPV[1];
                if (sort->pos > 1) si->change = 1;
                bestmoveindex = sort->pos - 1;
            }
        }
    }
    if (id >= 8 && si->best_value != -INF) displayPV(pos, si, rootPV, id, oldalpha, beta, si->best_value);

    sort->list[bestmoveindex].s = maxnodes + 1024;
    if (si->best_value > INF - MAXPLY) si->mate_found++;
    if (si->thinking_status == THINKING)
    {
        if (si->time_is_limited)
        {
            time = getTime();
            if (id >= 8 && (si->legalmoves == 1 || si->mate_found >= 3) && si->thinking_status == THINKING)
            {
                si->thinking_status = ABORTED;
            }
            else if (time >= si->start_time + (si->alloc_time / 2))
            {
                if (si->best_value + 30 <= si->last_value)
                {
                    si->time_limit_max += ((si->last_value - si->best_value - 20) / 10) * si->alloc_time / 2;
                    if (si->time_limit_max > si->time_limit_abs) si->time_limit_max = si->time_limit_abs;
                }
                else if (si->change)
                {
                    si->time_limit_max += si->alloc_time / 2;
                    if (si->time_limit_max > si->time_limit_abs) si->time_limit_max = si->time_limit_abs;
                }
                else if (si->best_value <= oldalpha)
                {
                    si->time_limit_max += si->alloc_time / 2;
                    if (si->time_limit_max > si->time_limit_abs) si->time_limit_max = si->time_limit_abs;
                }
            }
        }
        else if (si->depth_is_limited && id >= si->depth_limit && si->thinking_status == THINKING)
        {
            si->thinking_status = ABORTED;
        }
    }
    si->last_last_value = si->last_value;
    si->last_value = si->best_value;

    if (id >= MAXPLY || id >= si->depth_limit)
    {
        if (si->thinking_status == THINKING) si->thinking_status = ABORTED;
    }
}

void getBestMove(position_t *pos, search_info_t *si)
{
    int alpha;
    int beta;
    int id;
    uint64 dcc;
    sort_t sort;
    trans_entry_t *entry;
    uci_option_t *opt = pos->uci_options;

    ASSERT(pos != NULL);
    ASSERT(rootPV != NULL);

    if (opt->try_book && pos->sp <= opt->book_limit)
    {
        if ((si->bestmove = getBookMove(pos, &sort)) != 0) return;
    }
    pos->ply = 0;
    si->transmove[pos->ply] = 0;
    transNewDate(pos, pos->trans_table.date);
    entry = transProbe(pos);
    if (entry != NULL) si->transmove[pos->ply] = transMove(entry);
    if (si->transmove[pos->ply] == EMPTY)
    {
        si->time_limit_max += si->alloc_time / 2;
        if (si->time_limit_max > si->time_limit_abs)
            si->time_limit_max = si->time_limit_abs;
    }
    sortInit(pos, &sort, si, pinnedPieces(pos, pos->side), pos->color[pos->side ^ 1] & ~pos->kings, 1, MoveGenPhaseStandard);
    if (si->moves_is_limited == TRUE)
    {
        for (sort.size = 0; si->moves[sort.size] != 0; sort.size++)
        {
            sort.list[sort.size].m = si->moves[sort.size];
        }
    }
    else
    {
        genLegal(pos, &sort);
    }
    scoreRoot(&sort);
    if (sort.size == 0)
    {
        Print(10, "info string No legal moves found!\n");
        displayBoard(pos, 8);
        return;
    }
    si->legalmoves = sort.size;
    si->last_last_value = si->last_value = eval(pos);
    dcc = discoveredCheckCandidates(pos, pos->side);
    for (id = 1; id <= MAXPLY; id++)
    {
        alpha = -INF;
        beta = INF;
        Print(1, "info depth %d\n", id);
        if (id >= 7)
        {
            ASSERT(si->best_value != -INF);
            alpha = beta = si->best_value;
            do
            {
                if (si->best_value <= alpha) alpha = MAX(si->best_value - 16, -INF);
                if (si->best_value >= beta) beta = MIN(si->best_value + 16, INF);

                searchRoot(pos, si, &sort, dcc, alpha, beta, id);
                if (si->thinking_status == ABORTED) break;
            } while (si->best_value <= alpha || si->best_value >= beta);
        }
        else
        {
            searchRoot(pos, si, &sort, dcc, alpha, beta, id);
        }

        if (si->thinking_status == ABORTED) break;
    }
    if (si->thinking_status != ABORTED)
    {
        Print(3, "info string Waiting for stop, quit, or ponderhit\n");
    }
    while (si->thinking_status != ABORTED)
    {
        check4Input(pos, si);
    }
}
