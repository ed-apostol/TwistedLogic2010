/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2007                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

int transMove(trans_entry_t * te) {
    return (te->data & 0x1ffffff);
}
int transMateThreat(trans_entry_t * te) {
    return (1 & (te->data >> 25));
}
int transDate(trans_entry_t * te) {
    return (0x3f & (te->data >> 26));
}
int transDepth(trans_entry_t * te) {
    return te->depth;
}
int transMovedepth(trans_entry_t * te) {
    return te->movedepth;
}
int transMindepth(trans_entry_t * te) {
    return te->mindepth;
}
int transMaxdepth(trans_entry_t * te) {
    return te->maxdepth;
}
uint32 transHashlock(trans_entry_t * te) {
    return te->hashlock;
}
int transMinvalue(trans_entry_t * te) {
    return te->minvalue;
}
int transMaxvalue(trans_entry_t * te) {
    return te->maxvalue;
}

void transSetMove(trans_entry_t * te, uint32 move) {
    te->data &= 0xfe000000;
    te->data |= (move & 0x1ffffff);
}
void transSetMateThreat(trans_entry_t * te, uint32 mthreat) {
    te->data &= 0xfdffffff;
    te->data |= ((mthreat & 1) << 25);
}
void transSetDate(trans_entry_t * te, uint32 date) {
    te->data &= 0x3ffffff;
    te->data |= ((date & 0x3f) << 26);
}
void transSetDepth(trans_entry_t * te, uint32 depth) {
    te->depth = depth;
}
void transSetMovedepth(trans_entry_t * te, uint32 movedepth) {
    te->movedepth = movedepth;
}
void transSetMindepth(trans_entry_t * te, uint32 mindepth) {
    te->mindepth = mindepth;
}
void transSetMaxdepth(trans_entry_t * te, uint32 maxdepth) {
    te->maxdepth = maxdepth;
}
void transSetHashlock(trans_entry_t * te, uint32 hashlock) {
    te->hashlock = hashlock;
}
void transSetMinvalue(trans_entry_t * te, int minvalue) {
    te->minvalue = minvalue;
}
void transSetMaxvalue(trans_entry_t * te, int maxvalue) {
    te->maxvalue = maxvalue;
}

void initTrans(position_t *pos, uint32 target) {
    uint32 size;

    if (target < 4) target = 16;
    target *= 1024 * 1024;

    if (pos->trans_table.table != NULL) free(pos->trans_table.table);

    for (size = 1; size <= target; size *= 2);
    size /= 2;
    size /= sizeof(trans_entry_t);

    pos->trans_table.size = size + 3;
    pos->trans_table.mask = size - 1;
    pos->trans_table.table = (trans_entry_t*)malloc(pos->trans_table.size * sizeof(trans_entry_t));

    if (pos->trans_table.table == NULL) {
        Print(3, "info string Not enough memory to allocate transposition table.\n");
        quit(pos);
    }
    transClear(pos);
}

trans_entry_t *transProbe(position_t *pos) {
    int t;

    trans_entry_t *entry = pos->trans_table.table + (KEY(pos->hash) & pos->trans_table.mask);

    for (t = 0; t < 4; t++, entry++) {
        if (entry->hashlock == LOCK(pos->hash)) {
            transSetDate(entry, pos->trans_table.date);
            return entry;
        }
    }
    return NULL;
}

void transStore(position_t *pos, uint32 bm, int d, int min, int max, int mt) {
    int worst = -INF, t, score;
    trans_entry_t *replace, *entry;

    replace = entry = pos->trans_table.table + (KEY(pos->hash) & pos->trans_table.mask);

    for (t = 0; t < 4; t++, entry++) {
        if (transHashlock(entry) == LOCK(pos->hash)) {
            if (transDate(entry) != pos->trans_table.date) pos->trans_table.used++;
            transSetDate(entry, pos->trans_table.date);
            transSetMateThreat(entry, mt);
            if (d > transDepth(entry)) transSetDepth(entry, d);
            if (bm && d > transMovedepth(entry)) {
                transSetMovedepth(entry, d);
                transSetMove(entry, bm);
            }
            if (max < INF && d >= transMaxdepth(entry)) {
                transSetMaxdepth(entry, d);
                transSetMaxvalue(entry, max);
            }
            if (min > -INF && d >= transMindepth(entry)) {
                transSetMindepth(entry, d);
                transSetMinvalue(entry, min);
            }
            return;
        }
        score = (pos->trans_table.age[transDate(entry)] * 256) - transDepth(entry);
        if (score > worst) {
            worst = score;
            replace = entry;
        }
    }

    if (transDate(replace) != pos->trans_table.date) pos->trans_table.used++;
    transSetHashlock(replace, LOCK(pos->hash));
    transSetMove(replace, bm);
    transSetMateThreat(replace, mt);
    transSetDate(replace, pos->trans_table.date);
    transSetDepth(replace, d);
    transSetMovedepth(replace, (bm ? d : 0));
    transSetMindepth(replace, ((min > -INF) ? d : 0));
    transSetMaxdepth(replace, ((max < INF) ? d : 0));
    transSetMinvalue(replace, min);
    transSetMaxvalue(replace, max);
}

void transNewDate(position_t *pos, int date) {
    pos->trans_table.date = (date + 1) % DATESIZE;
    for (date = 0; date < DATESIZE; date++) {
        pos->trans_table.age[date] = pos->trans_table.date - date + ((pos->trans_table.date < date) ? DATESIZE : 0);
    }
    pos->trans_table.used = 1;
}

void transClear(position_t *pos) {
    trans_entry_t *te;

    transNewDate(pos, -1);

    if (pos->trans_table.table != NULL)
        for (te = &pos->trans_table.table[0]; te < &pos->trans_table.table[pos->trans_table.size]; te++) {
            transSetHashlock(te, 0);
            transSetMove(te, 0);
            transSetMateThreat(te, 0);
            transSetDate(te, 0);
            transSetDepth(te, 0);
            transSetMovedepth(te, 0);
            transSetMindepth(te, 0);
            transSetMaxdepth(te, 0);
            transSetMinvalue(te, -INF);
            transSetMaxvalue(te, INF);
        }
}

void initPawnTab(position_t *pos, int size) {
    if (pos->pawn_table.table != NULL) free(pos->pawn_table.table);

    pos->pawn_table.size = (uint64)1 << size;
    pos->pawn_table.mask = pos->pawn_table.size - 1;
    pos->pawn_table.table = (pawn_entry_t*)malloc(pos->pawn_table.size * sizeof(pawn_entry_t));
    if (pos->pawn_table.table == NULL) {
        Print(3, "info string Not enough memory to allocate pawn table.\n");
        quit(pos);
    }
    pawnTabClear(pos);
}

void pawnTabClear(position_t *pos) {
    int i;
    for (i = 0; i < pos->pawn_table.size; i++) {
        pos->pawn_table.table[i].passedbits = 0;
        pos->pawn_table.table[i].hashlock = 0;
        pos->pawn_table.table[i].opn = 0;
        pos->pawn_table.table[i].end = 0;
        pos->pawn_table.table[i].shield[WHITE][0] = 0;
        pos->pawn_table.table[i].shield[WHITE][1] = 0;
        pos->pawn_table.table[i].shield[WHITE][2] = 0;
        pos->pawn_table.table[i].shield[BLACK][0] = 0;
        pos->pawn_table.table[i].shield[BLACK][1] = 0;
        pos->pawn_table.table[i].shield[BLACK][2] = 0;
        pos->pawn_table.table[i].storm[WHITE][0] = 0;
        pos->pawn_table.table[i].storm[WHITE][1] = 0;
        pos->pawn_table.table[i].storm[WHITE][2] = 0;
        pos->pawn_table.table[i].storm[BLACK][0] = 0;
        pos->pawn_table.table[i].storm[BLACK][1] = 0;
        pos->pawn_table.table[i].storm[BLACK][2] = 0;
    }
}
