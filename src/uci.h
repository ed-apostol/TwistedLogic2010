/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

option_t *optionFind(const char var[]) {
    option_t *opt;

    ASSERT(var!=NULL);

    for (opt = &Option[0]; opt->var != NULL; opt++) {
        if (strcmp(opt->var,var) == 0) return opt;
    }
    return NULL;
}

void *myMalloc(int size) {
    void *address;

    ASSERT(size>0);

    address = malloc(size);
    if (address == NULL) Print(3, "info string myMalloc() error!\n");
    return address;
}

void myFree(void *address) {

    ASSERT(address!=NULL);

    free(address);
}

char *myStrdup(const char string[]) {
    char *address;

    ASSERT(string!=NULL);

    address = (char *) myMalloc(strlen(string)+1);
    strcpy(address,string);
    return address;
}

void myStringSet(const char **variable, const char string[]) {

    ASSERT(variable!=NULL);
    ASSERT(string!=NULL);

    if (*variable != NULL) myFree((void*)(*variable));
    *variable = myStrdup(string);
}

int optionSet(const char var[], const char val[]) {
    option_t *opt;
    char *c;

    ASSERT(var!=NULL);
    ASSERT(val!=NULL);

    opt = optionFind(var);
    if (opt == NULL) return FALSE;

    myStringSet(&opt->val,val);
    c = strchr(opt->val, '\n');
    if (c != NULL) *c = '\0';
    Print(2, "info string %s is set to value %s\n", opt->var, opt->val);
    return TRUE;
}

const char *optionGet(const char var[]) {
    option_t *opt;

    ASSERT(var!=NULL);

    opt = optionFind(var);
    if (opt == NULL) Print(3, "info string optionGet(): unknown option \"%s\"\n",var);
    return opt->val;
}

int optionGetBool(const char var[]) {
    const char *val;

    val = optionGet(var);
    return (val[0] == 't') ? TRUE : FALSE;
}

int optionGetInt(const char var[]) {
    const char *val;

    val = optionGet(var);
    return atoi(val);
}

const char *optionGetString(const char var[]) {
    const char *val;

    val = optionGet(var);
    return val;
}

void printUciOptions(void) {
    option_t *opt;

    for (opt = &Option[0]; opt->var != NULL; opt++) {
        if (opt->show) {
            if (opt->extra != NULL && *opt->extra != '\0') {
                Print(3, "option name %s type %s default %s %s\n",opt->var,opt->type,opt->val,opt->extra);
            } else {
                Print(3, "option name %s type %s default %s\n",opt->var,opt->type,opt->val);
            }
        }
    }
}

void uciSetOption(position_t *pos, char string[]) {
    const char *name;
    char *value;
    int x;

    name = strstr(string,"name ");
    value = strstr(string,"value ");

    if (name == NULL || value == NULL || name >= value) return;

    value[-1] = '\0';
    name += 5;
    value += 6;

    optionSet(name, value);

    if (strcmp(name, "Hash") == 0) {
        if ((x = optionGetInt("Hash")) >= 4) {
            initTrans(pos, x);
        }
    }
    if (strcmp(name, "Pawn Hash") == 0) {
        initPawnTab(pos, optionGetInt("Pawn Hash"));
    }
    if (strcmp(name, "Book File") == 0) {
        initBook(pos);
    }
}

void initOption() {
    option_t *opt;

    for (opt = &Option[0]; opt->var != NULL; opt++) {
        optionSet(opt->var,opt->init);
    }
}

void uciParseSearchmoves(sort_t *ml, char *str, uint32 moves[]) {
    char *c, movestr[10];
    int i;
    int m;
    uint32 *move = moves;

    ASSERT(ml != NULL);
    ASSERT(moves != NULL);

    c = str;
    while (isspace(*c)) c++;
    while (*c != '\0') {
        i = 0;
        while (*c != '\0' && !isspace(*c) && i < 9) movestr[i++] = *c++;
        if (i >= 4 && 'a' <= movestr[0] && movestr[0] <= 'h' &&
                '1' <= movestr[1] && movestr[1] <= '8' &&
                'a' <= movestr[2] && movestr[2] <= 'h' &&
                '1' <= movestr[3] && movestr[3] <= '8') {
            m = parseMove(ml, movestr);
            if (m) *move++ = m;
        } else break;
        while (isspace(*c)) c++;
    }
    *move = 0;
}

void uciGo(position_t *pos, char *options) {
    char *ponder, *infinite;
    char *c;
    int64 mytime = 0, t_inc = 0;
    int wtime=0, btime=0, winc=0, binc=0, movestogo=0, maxdepth=0, nodes=0, mate=0, movetime=0;
    search_info_t si;
    sort_t ml;

    ASSERT(pos != NULL);
    ASSERT(options != NULL);

    /* initialization */
    si.thinking_status = ABORTED;
    si.depth_is_limited = FALSE;
    si.depth_limit = MAXPLY;
    si.moves_is_limited = FALSE;
    si.time_is_limited = FALSE;
    si.time_limit_max = 0;
    si.time_limit_abs = 0;
    si.node_is_limited = FALSE;
    si.node_limit = 0;
    si.nodes_since_poll = 0;
    si.nodes_between_polls = 8192;
    si.nodes = 0;
    si.start_time = si.last_time = getTime();
    si.alloc_time = 0;
    si.best_value = -INF;
    si.last_value = -INF;
    si.last_last_value = -INF;
    si.change = 0;
    si.research = 0;
    si.bestmove = 0;
    si.pondermove = 0;
    si.mate_found = 0;
    si.singularExtension = 0;

    memset(si.history, 0, sizeof(si.history));
    memset(si.transmove, 0, sizeof(si.transmove));
    memset(si.moves, 0, sizeof(si.moves));
    memset(si.killer1, 0, sizeof(si.killer1));
    memset(si.killer2, 0, sizeof(si.killer2));

    infinite = strstr(options, "infinite");
    ponder = strstr(options, "ponder");
    c = strstr(options, "wtime");
    if (c != NULL) sscanf(c+6, "%d", &wtime);
    c = strstr(options, "btime");
    if (c != NULL) sscanf(c+6, "%d", &btime);
    c = strstr(options, "winc");
    if (c != NULL) sscanf(c + 5, "%d", &winc);
    c = strstr(options, "binc");
    if (c != NULL) sscanf(c + 5, "%d", &binc);
    c = strstr(options, "movestogo");
    if (c != NULL) sscanf(c + 10, "%d", &movestogo);
    c = strstr(options, "depth");
    if (c != NULL) sscanf(c + 6, "%d", &maxdepth);
    c = strstr(options, "nodes");
    if (c != NULL) sscanf(c + 6, "%d", &nodes);
    c = strstr(options, "mate");
    if (c != NULL) sscanf(c + 5, "%d", &mate);
    c = strstr(options, "movetime");
    if (c != NULL) sscanf(c + 9, "%d", &movetime);
    c = strstr(options, "searchmoves");
    if (c != NULL) {
        genLegal(pos, &ml);
        uciParseSearchmoves(&ml, c + 12, &(si.moves[0]));
    }

    if (infinite) {
        si.depth_is_limited = TRUE;
        si.depth_limit = MAXPLY;
        Print(2, "info string Infinite\n");
    }
    if (maxdepth > 0) {
        si.depth_is_limited = TRUE;
        si.depth_limit = maxdepth;
        Print(2, "info string Depth is limited to %d half moves\n", si.depth_limit);
    }
    if (mate > 0) {
        si.depth_is_limited = TRUE;
        si.depth_limit = mate * 2 - 1;
        Print(2, "info string Mate in %d half moves\n", si.depth_limit);
    }
    if (nodes > 0) {
        si.node_is_limited = TRUE;
        si.node_limit = nodes;
        Print(2, "info string Nodes is limited to %d positions\n", si.node_limit);
    }
    if (si.moves[0]) {
        si.moves_is_limited = TRUE;
        Print(2, "info string Moves is limited\n");
    }

    if (movetime > 0) {
        si.time_is_limited = TRUE;
        si.alloc_time = movetime;
        si.time_limit_max = si.start_time + movetime;
        si.time_limit_abs = si.start_time + movetime;
        Print(2, "info string Fixed time per move: %d ms\n", movetime);
    }
    if (pos->side == WHITE) {
        mytime = wtime;
        t_inc = winc;
    } else {
        mytime = btime;
        t_inc = binc;
    }
    if (mytime > 0) {
        si.time_is_limited = TRUE;
        mytime = ((mytime * 95) / 100) - pos->uci_options->time_buffer;
        if (mytime  < 0) mytime = 0;
        if (movestogo <= 0 || movestogo > 30) movestogo = 30;

        si.time_limit_max = (mytime / movestogo) + ((t_inc * 4) / 5);
        if (ponder) si.time_limit_max += si.time_limit_max / 4;
        if (si.time_limit_max > mytime) si.time_limit_max = mytime;

        si.time_limit_abs = ((mytime * 2) / 5) + ((t_inc * 4) / 5);
        if (si.time_limit_abs < si.time_limit_max) si.time_limit_abs = si.time_limit_max;
        if (si.time_limit_abs > mytime) si.time_limit_abs = mytime;

        Print(2, "info string Time is limited: ");
        Print(2, "max = %d, ", si.time_limit_max);
        Print(2, "abs = %d\n", si.time_limit_abs);
        si.alloc_time = si.time_limit_max;
        si.time_limit_max += si.start_time;
        si.time_limit_abs += si.start_time;
    }
    if (infinite) {
        si.thinking_status = ANALYSING;
        Print(2, "info string Search status is ANALYSING\n");
        transClear(pos);
        pawnTabClear(pos);
    } else if (ponder) {
        si.thinking_status = PONDERING;
        Print(2, "info string Search status is PONDERING\n");
    } else {
        si.thinking_status = THINKING;
        Print(2, "info string Search status is THINKING\n");
    }

    /* initialize UCI parameters to be used in search */
    initGameOptions(pos->uci_options);

    getBestMove(pos, &si);

    if (!si.bestmove) {
        Print(3, "info string No legal move found. Start a new game.\n\n");
        return;
    } else {
        Print(3, "bestmove %s", move2Str(si.bestmove));
        if (si.pondermove) Print(3, " ponder %s", move2Str(si.pondermove));
        Print(3, "\n\n");
    }
}

void uciStart(void) {

    Print(3, "id name Twisted Logic %s\n", VERSION);
    Print(3, "id author Edsel Apostol, Philippines\n");
    printUciOptions();
    Print(3, "uciok\n");
}

void uciSetPosition(position_t *pos, char *str) {
    char *c = str, *m;
    char movestr[10];
    int move;
    sort_t ml;
    undo_t undo;

    ASSERT(pos != NULL);
    ASSERT(str != NULL);

    while (isspace(*c)) c++;
    if (strncasecmp(c, "startpos", 8) == 0) {
        c += 8;
        while (isspace(*c)) c++;
        setPosition(pos, FenString[0]);
    } else if (strncasecmp(c, "fen", 3) == 0) {
        c += 3;
        while (isspace(*c)) c++;
        setPosition(pos, c);
        while (*c != '\0' && strncasecmp(c, "moves", 5) != 0) c++;
    }
    while (isspace(*c)) c++;
    if (strncasecmp(c, "moves", 5) == 0) {
        c += 5;
        while (isspace(*c)) c++;
        while (*c != '\0') {
            m = movestr;
            while (*c != '\0' && !isspace(*c)) *m++ = *c++;
            *m = '\0';
            genLegal(pos, &ml);
            move = parseMove(&ml, movestr);
            if (!move) {
                Print(3, "info string Illegal move: %s\n", movestr);
                return;
            } else makeMove(pos, &undo, move);
            while (isspace(*c)) c++;
        }
    }
}
