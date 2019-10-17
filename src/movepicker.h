/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/


void initSortPhases() {
    int i = 0;

    MoveGenPhaseEvasion = i;
    MoveGenPhase[i++] = PH_NONE;
    MoveGenPhase[i++] = PH_EVASION;
    MoveGenPhase[i++] = PH_END;

    MoveGenPhaseStandard = i;
    MoveGenPhase[i++] = PH_NONE;
    MoveGenPhase[i++] = PH_TRANS;
    MoveGenPhase[i++] = PH_GOOD_CAPTURES;
    MoveGenPhase[i++] = PH_KILLER_MOVES;
    MoveGenPhase[i++] = PH_QUIET_MOVES;
    MoveGenPhase[i++] = PH_BAD_CAPTURES;
    MoveGenPhase[i++] = PH_END;

    MoveGenPhaseSelective = i;
    MoveGenPhase[i++] = PH_NONE;
    MoveGenPhase[i++] = PH_TRANS;
    MoveGenPhase[i++] = PH_GOOD_CAPTURES;
    MoveGenPhase[i++] = PH_KILLER_MOVES;
    MoveGenPhase[i++] = PH_NONTACTICAL_CHECKS;
    MoveGenPhase[i++] = PH_QUIET_PASSEDPAWNS;
    MoveGenPhase[i++] = PH_BAD_CAPTURES;
    MoveGenPhase[i++] = PH_END;

    MoveGenPhaseSelectiveTactical = i;
    MoveGenPhase[i++] = PH_NONE;
    MoveGenPhase[i++] = PH_GOOD_CAPTURES;
    MoveGenPhase[i++] = PH_NONTACTICAL_CHECKS_PURE;
    MoveGenPhase[i++] = PH_QUIET_PASSEDPAWNS_PURE;
    MoveGenPhase[i++] = PH_BAD_CAPTURES;
    MoveGenPhase[i++] = PH_END;

    MoveGenPhaseQuiescence = i;
    MoveGenPhase[i++] = PH_NONE;
    MoveGenPhase[i++] = PH_GOOD_CAPTURES_PURE;
    MoveGenPhase[i++] = PH_END;

    MoveGenPhaseQuiescenceAndChecks = i;
    MoveGenPhase[i++] = PH_NONE;
    MoveGenPhase[i++] = PH_GOOD_CAPTURES_PURE;
    MoveGenPhase[i++] = PH_NONTACTICAL_CHECKS_WIN;
    MoveGenPhase[i++] = PH_END;
}

void sortInit(const position_t *pos, sort_t *sort, search_info_t *si, uint64 pinned, uint64 target, int depth, int type) {

    sort->transmove = si->transmove[pos->ply];
    sort->killer1 = si->killer1[pos->ply];
    sort->killer2 = si->killer2[pos->ply];
    sort->pos = 0;
    sort->size = 0;
    sort->pinned = pinned;
    sort->target = target;
    sort->ply = pos->ply;
    sort->side = pos->side;
    sort->depth = depth;
    sort->phase = type;
}

move_t *getMove(sort_t *sort) {
    move_t *best, *temp, *start, *end;

    ASSERT(sort != NULL);

    start = &sort->list[sort->pos++];
    end = &sort->list[sort->size];
    if (start >= end) return NULL;
    best = start;
    for (temp = start+1; temp < end; temp++) {
        if (temp->s > best->s) best = temp;
    }
    if (best == start) return start;
    *temp = *start;
    *start = *best;
    *best = *temp;
    return start;
}

BOOL moveIsPassedPawn(const position_t * pos, uint32 move) {
    if (movePiece(move) == PAWN && !((*FillPtr[pos->side])(BitMask[moveTo(move)]) & pos->pawns)) {
        if (!(pos->pawns & pos->color[pos->side^1] & PassedMask[pos->side][moveTo(move)])) return TRUE;
    }
    return FALSE;
}

move_t *sortNext(position_t *pos, sort_t *sort, search_info_t *si) {
    move_t *move;
    trans_entry_t *entry;
    while (TRUE) {
        while (sort->pos < sort->size) {
            move = getMove(sort);

            ASSERT(moveIsOk(move->m));

            switch (MoveGenPhase[sort->phase]) {
                case PH_EVASION:
                    ASSERT(kingIsInCheck(pos));
                    break;
                case PH_TRANS:
                    if (!genMoveIfLegal(pos, move->m, sort->pinned)) {
                        entry = transProbe(pos);
                        Print(8, "trans move failed = %s\n", move2Str(move->m));
                        Print(8, "from = %d, to = %d, pc = %d, capt = %d, prom = %d\n",
                            moveFrom(move->m), moveTo(move->m), movePiece(move->m),
                            moveCapture(move->m), movePromote(move->m));
                        Print(8, "hashlock = %s, transhashlock = %s\n", bit2Str(LOCK(pos->hash)), bit2Str(entry->hashlock));
                        displayBoard(pos, 8);
                        continue;
                    }
                    break;
                case PH_ALL_CAPTURES:
                    if (move->m == sort->transmove) continue;
                case PH_ALL_CAPTURES_PURE:
                    if (!moveIsLegal(pos, move->m, sort->pinned, FALSE)) continue;
                    break;
                case PH_GOOD_CAPTURES:
                    if (move->m == sort->transmove) continue;
                    if (!captureIsGood(pos, move->m)) {
                        sort->bad[sort->sizebad++] = *move;
                        continue;
                    }
                    if (!moveIsLegal(pos, move->m, sort->pinned, FALSE)) continue;
                    break;
                case PH_GOOD_CAPTURES_PURE:
                    if (!captureIsGood(pos, move->m)) continue;
                    if (!moveIsLegal(pos, move->m, sort->pinned, FALSE)) continue;
                    break;
                case PH_BAD_CAPTURES:
                    if (move->m == sort->transmove) continue;
                    if (!moveIsLegal(pos, move->m, sort->pinned, FALSE)) continue;
                    break;
                case PH_KILLER_MOVES:
                    if (move->m == sort->transmove) continue;
                    if (!genMoveIfLegal(pos, move->m, sort->pinned)) continue;
                    break;
                case PH_QUIET_MOVES:
                    if (move->m == sort->transmove) continue;
                    if (move->m == sort->killer1) continue;
                    if (move->m == sort->killer2) continue;
                    if (!moveIsLegal(pos, move->m, sort->pinned, FALSE)) continue;
                    break;
                case PH_QUIET_PASSEDPAWNS:
                    ASSERT(moveIsPassedPawn(pos, move->m));
                    if (move->m == sort->transmove) continue;
                    if (move->m == sort->killer1) continue;
                    if (move->m == sort->killer2) continue;
                case PH_QUIET_PASSEDPAWNS_PURE:
                    if (!moveIsLegal(pos, move->m, sort->pinned, FALSE)) continue;
                    break;
                case PH_NONTACTICAL_CHECKS:
                    if (move->m == sort->transmove) continue;
                    if (move->m == sort->killer1) continue;
                    if (move->m == sort->killer2) continue;
                    if (!moveIsLegal(pos, move->m, sort->pinned, FALSE)) continue;
                    break;
                case PH_NONTACTICAL_CHECKS_WIN:
                    if (swap(pos, move->m) < 0) continue;
                case PH_NONTACTICAL_CHECKS_PURE:
                    if (!moveIsLegal(pos, move->m, sort->pinned, FALSE)) continue;
                    break;
                default:
                    return NULL;
            }
            return move;
        }

        sort->phase++;
        sort->pos = 0;

        switch (MoveGenPhase[sort->phase]) {
            case PH_EVASION:
                genEvasions(pos, sort);
                if (sort->depth <= 0) scoreAllQ(sort, si);
                else scoreAll(pos, sort, si);
                break;
            case PH_TRANS:
                sort->size = 0;
                if (sort->transmove != EMPTY) sort->list[sort->size++].m = sort->transmove;
                break;
            case PH_ALL_CAPTURES:
            case PH_ALL_CAPTURES_PURE:
                genCaptures(pos, sort);
                scoreCapturesPure(sort);
                break;
            case PH_GOOD_CAPTURES:
                sort->sizebad = 0;
            case PH_GOOD_CAPTURES_PURE:
                genCaptures(pos, sort);
                scoreCapturesPure(sort);
                break;
            case PH_BAD_CAPTURES:
                for (sort->size = 0; sort->size < sort->sizebad; sort->size++)
                    sort->list[sort->size] = sort->bad[sort->size];
                break;
            case PH_KILLER_MOVES:
                sort->size = 0;
                if (sort->killer1 != EMPTY) {
                    sort->list[sort->size].m = sort->killer1;
                    sort->list[sort->size].s = MAXHIST+4;
                    sort->size++;
                }
                if (sort->killer2 != EMPTY) {
                    sort->list[sort->size].m = sort->killer2;
                    sort->list[sort->size].s = MAXHIST+2;
                    sort->size++;
                }
                break;
            case PH_QUIET_MOVES:
                genNonCaptures(pos, sort);
                scoreNonCaptures(sort, si);
                break;
            case PH_QUIET_PASSEDPAWNS_PURE:
            case PH_QUIET_PASSEDPAWNS:
                genPassedPawnMoves(pos, sort);
                scoreNonCaptures(sort, si);
                break;
            case PH_NONTACTICAL_CHECKS:
            case PH_NONTACTICAL_CHECKS_WIN:
            case PH_NONTACTICAL_CHECKS_PURE:
                genQChecks(pos, sort);
                scoreNonCaptures(sort, si);
                break;
            default:
                ASSERT(MoveGenPhase[sort->phase] == PH_END);
                return NULL;
        }
    }
}

uint32 captureIsGood(const position_t *pos, uint32 m) {
    uint32 prom = movePromote(m);
    uint32 capt = moveCapture(m);
    uint32 pc = movePiece(m);

    ASSERT(pos != NULL);
    ASSERT(moveIsOk(m));
    ASSERT(moveIsTactical(m));

    if (prom != EMPTY && prom != QUEEN) return FALSE;
    if (capt != EMPTY) {
        if (prom != EMPTY) return TRUE;
        if (capt >= pc) return TRUE;
    }
    return (swap(pos, m) >= 0);
}

void scoreCapturesPure(sort_t *sort) {
    move_t *m;

    ASSERT(sort != NULL);

    for (m = &sort->list[0]; m < &sort->list[sort->size]; m++) {
        m->s = (moveCapture(m->m) * 6) + movePromote(m->m) - movePiece(m->m);
    }
}

void scoreCaptures(sort_t *sort) {
    move_t *m;

    ASSERT(sort != NULL);

    for (m = &sort->list[0]; m < &sort->list[sort->size]; m++) {
        m->s = (moveCapture(m->m) * 6) + movePromote(m->m) - movePiece(m->m);
        if (m->m == sort->transmove) m->s += MAXHIST;
    }
}

void scoreNonCaptures(sort_t *sort, search_info_t *si) {
    move_t *m;

    ASSERT(sort != NULL);

    for (m = &sort->list[0]; m < &sort->list[sort->size]; m++) {
        m->s = si->history[historyIndex(sort->side, m->m)];
    }
}

void scoreAll(const position_t *pos, sort_t *sort, search_info_t *si) {
    move_t *m;

    ASSERT(pos != NULL);
    ASSERT(sort != NULL);

    for (m = &sort->list[0]; m < &sort->list[sort->size]; m++) {
        if (m->m == sort->transmove) m->s = MAXHIST * 3;
        else if (moveIsTactical(m->m)) {
            m->s = (moveCapture(m->m) * 6) + movePromote(m->m) - movePiece(m->m);
            if (captureIsGood(pos, m->m)) m->s += MAXHIST * 2;
            else m->s -= MAXHIST;
        }
        else if (m->m == sort->killer1) m->s = MAXHIST + 4;
        else if (m->m == sort->killer2) m->s = MAXHIST + 2;
        else m->s = si->history[historyIndex(sort->side, m->m)];
    }
}

void scoreAllQ(sort_t *sort, search_info_t *si) {
    move_t *m;

    ASSERT(sort != NULL);

    for (m = &sort->list[0]; m < &sort->list[sort->size]; m++) {
        if (moveIsTactical(m->m))  m->s = MAXHIST + (moveCapture(m->m) * 6) + movePromote(m->m) - movePiece(m->m);
        else m->s = si->history[historyIndex(sort->side, m->m)];
    }
}

void scoreRoot(sort_t *sort) {
    move_t *m;

    ASSERT(sort != NULL);

    for (m = &sort->list[0]; m < &sort->list[sort->size]; m++) {
        if (m->m == sort->transmove) m->s = MAXHIST * 3;
        else if (moveIsTactical(m->m))  m->s = MAXHIST + (moveCapture(m->m) * 6) + movePromote(m->m) - movePiece(m->m);
        else m-> s = 0;
    }
}

