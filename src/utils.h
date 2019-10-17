/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

int strncasecmp(const char *s1, const char *s2, size_t length) {
    unsigned char   u1,
        u2;

    for (; length != 0; length--, s1++, s2++) {
        u1 = (unsigned char)*s1;
        u2 = (unsigned char)*s2;
        if (cm[u1] != cm[u2]) {
            return cm[u1] - cm[u2];
        }
        if (u1 == '\0') {
            return 0;
        }
    }
    return 0;
}

/* a utility to print output into different files:
   bit 0: stdout
   bit 1: logfile
   bit 2: errfile
   bit 3: dumpfile
*/
void Print(int vb, char *fmt, ...) {
    va_list ap;

    ASSERT(vb >= 1 && vb <= 15);

    va_start(ap, fmt);
    if (vb & 1) {
        vprintf(fmt, ap);
        fflush(stdout);
    }
    va_start(ap, fmt);
    if (logfile && ((vb >> 1) & 1)) {
        vfprintf(logfile, fmt, ap);
        fflush(logfile);
    }
    va_start(ap, fmt);
    if (errfile && ((vb >> 2) & 1)) {
        vfprintf(errfile, fmt, ap);
        fflush(errfile);
    }
    va_start(ap, fmt);
    if (dumpfile && ((vb >> 3) & 1)) {
        vfprintf(dumpfile, fmt, ap);
        fflush(dumpfile);
    }
    va_end(ap);
}

/* a utility to print the bits in a position-like format */
void displayBit(uint64 a, int x) {
    int i, j;
    for (i = 56; i >= 0; i -= 8) {
        Print(x, "\n%d  ", (i / 8) + 1);
        for (j = 0; j < 8; ++j) {
            Print(x, "%c ", ((a & ((uint64)1 << (i + j))) ? '1' : '_'));
        }
    }
    Print(x, "\n\n");
    Print(x, "   a b c d e f g h \n\n");
}

/* a utility to convert large int to hex string*/
char *bit2Str(uint64 n) {
    static char s[19];
    int i;
    s[0] = '0';
    s[1] = 'x';
    for (i = 0; i < 16; i++) {
        if ((n & 15) < 10) s[17 - i] = '0' + (n & 15);
        else s[17 - i] = 'a' + (n & 15) - 10;
        n >>= 4;
    }
    s[18] = 0;
    return s;
}

/* a utility to convert int move to string */
char *move2Str(int m) {
    static char promstr[] = "\0pnbrqk";
    static char str[6];

    /* ASSERT(moveIsOk(m)); */

    if (m == 0) sprintf(str, "%c%c%c%c%c", '0', '0', '0', '0', '\0');
    else sprintf(str, "%c%c%c%c%c",
        SQFILE(moveFrom(m)) + 'a',
        '1' + SQRANK(moveFrom(m)),
        SQFILE(moveTo(m)) + 'a',
        '1' + SQRANK(moveTo(m)),
        promstr[movePromote(m)]
    );
    return str;
}

/* a utility to convert int square to string */
char *sq2Str(int sq) {
    static char str[3];

    /* ASSERT(moveIsOk(m)); */

    sprintf(str, "%c%c%c",
        SQFILE(sq) + 'a',
        '1' + SQRANK(sq),
        '\0'
    );
    return str;
}

/* a utility to print the position */
void displayBoard(const position_t *pos, int x) {
    static char pcstr[] = ".PNBRQK.pnbrqk";
    int i, j, c, p;
    for (i = 56; i >= 0; i -= 8) {
        Print(x, "\n%d  ", (i / 8) + 1);
        for (j = 0; j < 8; j++) {
            c = getColor(pos, i + j);
            p = getPiece(pos, i + j);
            Print(x, "%c ", pcstr[p + (c ? 7 : 0)]);
        }
    }
    Print(x, "\n\n");
    Print(x, "   a b c d e f g h \n\n");
    Print(x, "FEN %s\n", positionToFEN(pos));
    Print(x, "%d.%s%s ", (pos->sp) / 2
        + (pos->side ? 1 : 0), pos->side ? " " : " ..",
        move2Str(pos->lastmove));
    Print(x, "%s, ", pos->side == WHITE ? "WHITE" : "BLACK");
    Print(x, "Castle = %d, ", pos->castle);
    Print(x, "Ep = %d, ", pos->epsq);
    Print(x, "Fifty = %d, ", pos->fifty);
    Print(x, "Ev = %d, ", eval(pos));
    Print(x, "Ch = %s,\n",
        isAtt(pos, pos->side ^ 1, pos->kings&pos->color[pos->side])
        ? "T" : "F");
    Print(x, "H = %s, ", bit2Str(pos->hash));
    Print(x, "PH = %s\n", bit2Str(pos->phash));
}

/* a utility to get a certain piece from a position given a square */
int getPiece(const position_t *pos, uint32 sq) {
    ASSERT(pos != NULL);
    ASSERT(squareIsOk(sq));

    return pos->pieces[sq];
    /* if(BitMask[sq] & pos->pawns) return PAWN;
    else if(BitMask[sq] & pos->knights) return KNIGHT;
    else if(BitMask[sq] & pos->bishops) return BISHOP;
    else if(BitMask[sq] & pos->rooks) return ROOK;
    else if(BitMask[sq] & pos->queens) return QUEEN;
    else if(BitMask[sq] & pos->kings) return KING;
    else return EMPTY; */
}

/* a utility to get a certain color from a position given a square */
int getColor(const position_t *pos, uint32 sq) {
    uint64 mask = BitMask[sq];

    ASSERT(pos != NULL);
    ASSERT(squareIsOk(sq));

    if (mask & pos->color[WHITE]) return WHITE;
    else if (mask & pos->color[BLACK]) return BLACK;
    else {
        ASSERT(mask & ~pos->occupied);
        return WHITE;
    }
}

/* returns time in milli-seconds */
uint64 getTime(void) {
#if defined(_WIN32) || defined(_WIN64)
    static struct _timeb tv;
    _ftime(&tv);
    return(tv.time * 1000 + tv.millitm);
#else
    static struct timeval tv;
    static struct timezone tz;
    gettimeofday(&tv, &tz);
    return(tv.tv_sec * 1000 + (tv.tv_usec / 1000));
#endif
}

/* parse the move from string and returns a move from the
move list of generated moves if the move string matched
one of them */
uint32 parseMove(sort_t *sort, char *s) {
    uint32 m;
    int from, to, p;

    ASSERT(sort != NULL);
    ASSERT(s != NULL);

    from = (s[0] - 'a') + (8 * (s[1] - '1'));
    to = (s[2] - 'a') + (8 * (s[3] - '1'));
    m = (from) | (to << 6);
    for (sort->pos = 0; sort->pos < sort->size; sort->pos++) {
        if (m == (sort->list[sort->pos].m & 0xfff)) {
            p = EMPTY;
            if (movePromote(sort->list[sort->pos].m)) {
                switch (s[4]) {
                case 'n':
                case 'N':
                    p = KNIGHT;
                    break;
                case 'b':
                case 'B':
                    p = BISHOP;
                    break;
                case 'r':
                case 'R':
                    p = ROOK;
                    break;
                default:
                    p = QUEEN;
                    break;
                }
            }
            if (p == movePromote(sort->list[sort->pos].m)) return sort->list[sort->pos].m;
        }
    }
    return 0;
}

int biosKey(void) {
#if defined(_WIN32) || defined(_WIN64)
    /* Windows-version */
    static int init = 0, pipe;
    static HANDLE inh;
    DWORD dw;
    //if (stdin->_cnt > 0) return stdin->_cnt;
    if (!init) {
        init = 1;
        inh = GetStdHandle(STD_INPUT_HANDLE);
        pipe = !GetConsoleMode(inh, &dw);
        if (!pipe) {
            SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(inh);
        }
    }
    if (pipe) {
        if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
        return dw;
    }
    else {
        GetNumberOfConsoleInputEvents(inh, &dw);
        return dw <= 1 ? 0 : dw;
    }
#else
    /* Non-windows version */
    fd_set readfds;
    struct timeval timeout;
    FD_ZERO(&readfds);
    FD_SET(fileno(stdin), &readfds);
    /* Set to timeout immediately */
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    select(16, &readfds, 0, 0, &timeout);
    return (FD_ISSET(fileno(stdin), &readfds));
#endif
}

/* this determines the repetition in the game tree */
uint32 reps(const position_t *pos, int n) {
    int i, r = 0;
    for (i = 4; i <= pos->fifty; i += 2) {
        if (pos->sp - i < 0) break;
        if (pos->stack[pos->sp - i] == pos->hash) r++;
        if (r >= n) return TRUE;
    }
    return FALSE;
}

/* this must not be called with "d" other than the possible move deltas */
int getDirIndex(int d) {
    switch (d) {
    case -9:
        return 0;
    case -1:
        return 1;
    case 7:
        return 2;
    case 8:
        return 3;
    case 9:
        return 4;
    case 1:
        return 5;
    case -7:
        return 6;
    case -8:
        return 7;
    default:
        ASSERT(FALSE);
        return 0;
    }
}

/* the following routines are for polyglot book handling */
void bookMove2String(char move_s[6], uint16 move) {
    int f, fr, ff, t, tr, tf, p;
    static const char *promote_pieces = " nbrq";
    f = (move >> 6) & 077;
    fr = (f >> 3) & 0x7;
    ff = f & 0x7;
    t = move & 077;
    tr = (t >> 3) & 0x7;
    tf = t & 0x7;
    p = (move >> 12) & 0x7;
    move_s[0] = ff + 'a';
    move_s[1] = fr + '1';
    move_s[2] = tf + 'a';
    move_s[3] = tr + '1';
    if (p) {
        move_s[4] = promote_pieces[p];
        move_s[5] = '\0';
    }
    else {
        move_s[4] = '\0';
    }
    if (!strcmp(move_s, "e1h1")) {
        strcpy(move_s, "e1g1");
    }
    else  if (!strcmp(move_s, "e1a1")) {
        strcpy(move_s, "e1c1");
    }
    else  if (!strcmp(move_s, "e8h8")) {
        strcpy(move_s, "e8g8");
    }
    else  if (!strcmp(move_s, "e8a8")) {
        strcpy(move_s, "e8c8");
    }
}

int intFromFile(FILE *f, int l, uint64 *r) {
    int i, c;
    for (i = 0; i < l; i++) {
        c = fgetc(f);
        if (c == EOF) {
            return 1;
        }
        (*r) = ((*r) << 8) + c;
    }
    return 0;
}

int entryFromFile(FILE *f, book_entry_t *entry) {
    uint64 r;
    if (intFromFile(f, 8, &r)) return 1;
    entry->key = r;
    if (intFromFile(f, 2, &r)) return 1;
    entry->move = r;
    if (intFromFile(f, 2, &r)) return 1;
    entry->weight = r;
    if (intFromFile(f, 4, &r)) return 1;
    entry->learn = r;
    return 0;
}

int findKey(FILE *f, uint64 key, book_entry_t *entry) {
    int first, last, middle;
    book_entry_t first_entry = BookEntryNone, last_entry, middle_entry;
    first = -1;
    if (fseek(f, -16, SEEK_END)) {
        *entry = BookEntryNone;
        entry->key = key + 1; //hack
        return -1;
    }
    last = ftell(f) / 16;
    entryFromFile(f, &last_entry);
    while (TRUE) {
        if (last - first == 1) {
            *entry = last_entry;
            return last;
        }
        middle = (first + last) / 2;
        fseek(f, 16 * middle, SEEK_SET);
        entryFromFile(f, &middle_entry);
        if (key <= middle_entry.key) {
            last = middle;
            last_entry = middle_entry;
        }
        else {
            first = middle;
            first_entry = middle_entry;
        }
    }
}

uint32 getBookMove(const position_t *pos, sort_t *sort) {
    book_entry_t entry;
    book_entry_t entries[MAXMOVES];
    uint64 key = pos->hash;
    int book_index;
    int book_random;
    int count = 0;
    int i;
    int total_weight;
    int offset;
    char move_s[6];
    FILE *f = pos->book.bookfile;

    if (f == NULL || pos->book.size == 0) return 0;

    offset = findKey(f, key, &entry);
    if (entry.key != key) return 0;

    entries[0] = entry;
    count = 1;
    fseek(f, 16 * (offset + 1), SEEK_SET);
    while (TRUE) {
        if (count >= MAXMOVES - 1) break;
        if (entryFromFile(f, &entry)) break;
        if (entry.key != key) break;
        entries[count++] = entry;
    }
    total_weight = 0;
    for (i = 0; i < count; i++) {
        total_weight += entries[i].weight;
    }
    // chose here the move from the array and verify if it exists in the movelist
    book_random = ((getTime() % count) * total_weight) / count;
    book_index = 0;
    offset = 0;
    for (i = 0; i < count; i++) {
        offset += entries[i].weight;
        if (offset > book_random) {
            book_index = i;
            break;
        }
    }
    if (optionGetBool("Display Book Moves") == TRUE) {
        Print(3, "info string book moves: ");
        for (i = 0; i < count; i++) {
            bookMove2String(move_s, entries[i].move);
            Print(3, "%s(%5.2f%%) ", move_s, 100 * ((double)entries[i].weight / (double)total_weight));
        }
        Print(3, "\n");
    }
    bookMove2String(move_s, entries[book_index].move);
    genLegal(pos, sort);
    return parseMove(sort, move_s);
}

void initBook(position_t *pos) {
    if (pos->book.bookfile != NULL) fclose(pos->book.bookfile);
    pos->book.bookfile = fopen(optionGetString("Book File"), "rb");
    if (pos->book.bookfile != NULL) {
        fseek(pos->book.bookfile, 0, SEEK_END);
        pos->book.size = ftell(pos->book.bookfile) / 16;
    }
}
