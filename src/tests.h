/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

/* the recursive perft routine */
void perft(position_t *pos, int maxply, uint64 nodesx[]) {
    sort_t movelist;
    undo_t undo;

    ASSERT(pos != NULL);

    if (pos->ply  >= maxply) return;

    genLegal(pos, &movelist);
    /* if(pos->ply + 1 >= maxply){
        nodesx[pos->ply+1] += movelist.size;
        return;
    } */
    for (movelist.pos = 0; movelist.pos < movelist.size; movelist.pos++) {
        makeMove(pos, &undo, movelist.list[movelist.pos].m);
        perft(pos, maxply, nodesx);
        unmakeMove(pos, &undo);
    }
    nodesx[pos->ply+1] += movelist.size;
}

/* the recursive perft divide routine */
int perftDivide(position_t *pos, uint32 depth, uint32 maxply) {
    int x = 0;
    sort_t movelist;
    undo_t undo;

    ASSERT(pos != NULL);
    if (depth > maxply) return 0;

    genLegal(pos, &movelist);
    for (movelist.pos = 0; movelist.pos < movelist.size; movelist.pos++) {
        makeMove(pos, &undo, movelist.list[movelist.pos].m);
        x += perftDivide(pos, depth + 1, maxply);
        unmakeMove(pos, &undo);
    }
    return x;
}

/* this is the perft controller */
void runPerft(position_t *pos, int maxply) {
    int depth, i, x;
    uint64 nodes, time_start, nodesx[MAXPLY], duration;

    Print(1, "See the logfile.txt on the same directory to view\n");
    Print(1, "the detailed output after running.\n");
    for (x = 0; FenString[x] != NULL; x++) {
        setPosition(pos, FenString[x]);
        Print(3, "FEN %d = %s\n", x+1, FenString[x]);
        displayBoard(pos, 3);
        for (depth = 1; depth <= maxply; depth++) {
            Print(3, "perft %d ", depth);
            memset(nodesx, 0, sizeof(nodesx));
            time_start = getTime();
            perft(pos, depth, nodesx);
            duration = getTime() - time_start;
            nodes = 0;
            if (duration == 0) duration = 1;
            for (i = 1; i <= depth; i++) nodes += nodesx[i];
            Print(3, "%llu\t\t", nodesx[depth]);
            Print(3, "[%llu ms - ", duration);
            Print(3, "%llu KNPS]\n", nodes/duration);
        }
        Print(3, "\nDONE DOING PERFT ON FEN %d\n", x+1);
        Print(3, "\n\n\n");
    }
}

/* this is the perft divide controller */
void runPerftDivide(position_t *pos, uint32 maxply) {
    int x = 0, legal = 1, y = 0;
    sort_t movelist;
    undo_t undo;

    ASSERT(pos != NULL);

    Print(1, "See the logfile.txt on the same directory to view\n");
    Print(1, "the detailed output after running.\n");
    displayBoard(pos, 3);
    genLegal(pos, &movelist);
    if (maxply < 1) {
        Print(3, "Perft divide %d = %d\n", maxply, 0);
        return;
    } else if (maxply == 1) {
        Print(3, "Perft divide %d = %d\n", maxply, movelist.size);
        return;
    }
    for (movelist.pos = 0; movelist.pos < movelist.size; movelist.pos++) {
        makeMove(pos, &undo, movelist.list[movelist.pos].m);
        y += x = perftDivide(pos, 2, maxply);
        unmakeMove(pos, &undo);
        Print(3, "%d: %s %d\n", legal++, move2Str(movelist.list[movelist.pos].m), x);
    }
    Print(3, "Perft divide %d = %d\n", maxply, y);
}

/* this are the non-uci commands */
void nonUCI(position_t *pos) {
    sort_t ml;
    undo_t undo;
    char command[256];
    char temp[256];
    int move;

    Print(3, "This is the main loop of the non-UCI commands\n");
    Print(3, "Used for testing and debugging purposes\n");
    Print(3, "Type help for commands\n");

    while (TRUE) {
        Print(1, "Logic >>");
        if (!fgets(command, 256, stdin)) break;
        if (command[0]=='\n') continue;
        sscanf(command, "%s", temp);
        if (!strcmp(temp, "new")) {
            setPosition(pos, FenString[0]);
        } else if (!strcmp(temp, "undo")) {
            if (pos->sp > 0) unmakeMove(pos, &undo);
        } else if (!strcmp(temp, "moves")) {
            genLegal(pos, &ml);
            Print(3, "legal moves = %d:", ml.size);
            for (ml.pos = 0; ml.pos < ml.size; ml.pos++) {
                if (!(ml.pos%12)) Print(3, "\n");
                Print(3, "%s ", move2Str(ml.list[ml.pos].m));
            }
            Print(3, "\n\n");
        } else if (!strcmp(temp, "qmoves")) {
            ml.target = (pos->color[pos->side^1] & ~pos->kings);
            genCaptures(pos, &ml);
            Print(3, "quiescent moves = %d:", ml.size);
            for (ml.pos = 0; ml.pos < ml.size; ml.pos++) {
                if (!(ml.pos%12)) Print(3, "\n");
                Print(3, "%s ", move2Str(ml.list[ml.pos].m));
            }
            Print(3, "\n\n");
        } else if (!strcmp(temp, "evasions")) {
            genEvasions(pos, &ml);
            Print(3, "evasion moves = %d:", ml.size);
            for (ml.pos = 0; ml.pos < ml.size; ml.pos++) {
                if (!(ml.pos%12)) Print(3, "\n");
                Print(3, "%s ", move2Str(ml.list[ml.pos].m));
            }
            Print(3, "\n\n");
        } else if (!strcmp(temp, "qchecks")) {
            genQChecks(pos, &ml);
            Print(3, "quiescent checks = %d:", ml.size);
            for (ml.pos = 0; ml.pos < ml.size; ml.pos++) {
                if (!(ml.pos%12)) Print(3, "\n");
                Print(3, "%s ", move2Str(ml.list[ml.pos].m));
            }
            Print(3, "\n\n");
        } else if (!strcmp(temp, "d")) {
            displayBoard(pos, 3);
        } else if (!strcmp(temp, "perft")) {
            sscanf(command, "perft %d", &move);
            runPerft(pos, move);
        } else if (!strcmp(temp, "divide")) {
            sscanf(command, "divide %d", &move);
            runPerftDivide(pos, move);
        } else if (!strcmp(temp, "go")) {
            uciGo(pos, temp);
        } else if (!strcmp(temp, "postofen")) {
            Print(1, "FEN %s\n", positionToFEN(pos));
        } else if(!strcmp(temp, "passed")){
            setPosition(pos, "8/3pk3/5p1p/ppP3PP/PP6/8/2K5/8 w - - 0 1");
            displayBoard(pos, 9);
            genPassedPawnMoves(pos, &ml);
            Print(9, "passed pawns = %d:", ml.size);
            for (ml.pos = 0; ml.pos < ml.size; ml.pos++) {
                if (!(ml.pos%12)) Print(9, "\n");
                Print(9, "%s ", move2Str(ml.list[ml.pos].m));
            }
            Print(9, "\n\n");
            setPosition(pos, "8/2pk4/6n1/8/1pB5/2P1P3/PP1K2PP/8 w - - 0 1");
            displayBoard(pos, 9);
            genPassedPawnMoves(pos, &ml);
            Print(9, "passed pawns = %d:", ml.size);
            for (ml.pos = 0; ml.pos < ml.size; ml.pos++) {
                if (!(ml.pos%12)) Print(9, "\n");
                Print(9, "%s ", move2Str(ml.list[ml.pos].m));
            }
            Print(9, "\n\n");
//        } else if(!strcmp(temp, "pawns")){
//            setPosition(pos, "8/3k2p1/1p2pp1p/pP5P/3P3P/4PP2/6K1/8 w - - 0 1");
//            displayBoard(pos, 8);
//            evalPawnStructureDebug(pos);
//            setPosition(pos, "8/7p/2k1Pp2/pp1p2p1/3P2P1/4P3/P3K2P/8 w - - 0 1");
//            displayBoard(pos, 8);
//            evalPawnStructureDebug(pos);
//            setPosition(pos, "8/1p3p2/p4kpp/1PPp4/P7/7P/5K1P/8 b - - 0 1");
//            displayBoard(pos, 8);
//            evalPawnStructureDebug(pos);
//            setPosition(pos, "8/8/1p3p2/p3k1pp/4P2P/P3K1P1/1P6/8 w - - 0 1");
//            displayBoard(pos, 8);
//            evalPawnStructureDebug(pos);
//        }else if(!strcmp(temp, "passedpawns")){
//            setPosition(pos, "r2R4/1P3ppk/7p/8/8/P3P3/2p2PPP/1rR3K1 w - -");
//            displayBoard(pos, 8);
//            evalPassedPawnDebug(pos);
//            flipPosition(pos, &clone);
//            displayBoard(&clone, 8);
//            evalPassedPawnDebug(&clone);
//            setPosition(pos, "5bk1/4qp2/2R3p1/2r1p1Np/2p1N3/1r5P/5PP1/2Q3K1 b - -");
//            displayBoard(pos, 8);
//            evalPassedPawnDebug(pos);
//            flipPosition(pos, &clone);
//            displayBoard(&clone, 8);
//            evalPassedPawnDebug(&clone);
//        }else if(!strcmp(temp, "atakpcs")){
//            setPosition(pos, "r2R4/1P3ppk/7p/8/8/P3P3/2p2PPP/1rR3K1 w - -");
//            displayBoard(pos, 8);
//            for(i = a1; i <= h8; i++){
//                Print(8, "sq = %d\n", i);
//                displayBit(attackingPiecesAll(pos, i), 8);
//            }
//        }
//        else if(!strcmp(temp, "swapdebug")){
//            setPosition(pos, "5bk1/4qp2/2R3p1/2r1p1Np/2p1N3/1r5P/5PP1/2Q3K1 b - -");
//            displayBoard(pos, 8);
//            genCaptures(pos, &ml);
//            Print(8, "quiescent moves = %d\n", ml.size);
//            for(ml.pos = 0; ml.pos < ml.size; ml.pos++){
//                Print(8, "move = %s\n", move2Str(ml.list[ml.pos].m));
//                swapdebug(pos, ml.list[ml.pos].m);
//            }
//            Print(8, "\n\n");
//            flipPosition(pos, &clone);
//            positionIsOk(&clone);
//            displayBoard(&clone, 8);
//            genCaptures(&clone, &ml);
//            Print(8, "quiescent moves = %d\n", ml.size);
//            for(ml.pos = 0; ml.pos < ml.size; ml.pos++){
//                Print(8, "move = %s\n", move2Str(ml.list[ml.pos].m));
//                swapdebug(&clone, ml.list[ml.pos].m);
//            }
//            Print(8, "\n\n");
//        }else if(!strcmp(temp, "swapdebug2")){
//            setPosition(pos, "r2R4/1P3ppk/7p/8/8/P3P3/2p2PPP/1rR3K1 w - -");
//            displayBoard(pos, 8);
//            swapdebug(pos, GenOneForward(b7, b8));
//            flipPosition(pos, &clone);
//            positionIsOk(&clone);
//            displayBoard(&clone, 8);
//            swapdebug(&clone, GenOneForward(b2, b1));
//
//            setPosition(pos, "2r4k/5Rp1/nqPQ3p/2r5/8/P7/1PR5/K7 w - - 0 1");
//            displayBoard(pos, 8);
//            swapdebug(pos, GenOneForward(c6, c7));
//            flipPosition(pos, &clone);
//            positionIsOk(&clone);
//            displayBoard(&clone, 8);
//            swapdebug(&clone, GenOneForward(c3, c2));
//
//            setPosition(pos, "5bk1/4qp2/2R3p1/2r1p1Np/2p1N3/1r5P/5PP1/2Q3K1 b - -");
//            displayBoard(pos, 8);
//            swapdebug(pos, GenOneForward(c4, c3));
//            flipPosition(pos, &clone);
//            positionIsOk(&clone);
//            displayBoard(&clone, 8);
//            swapdebug(&clone, GenOneForward(c5, c6));
//        }else if(!strcmp(temp, "swapdebug3")){
//            setPosition(pos, "r2R4/1P3ppk/7p/8/8/P3P3/2p2PPP/1rR3K1 w - -");
//            displayBoard(pos, 8);
//            see(pos, GenOneForward(b7, b8));
//            flipPosition(pos, &clone);
//            positionIsOk(&clone);
//            displayBoard(&clone, 8);
//            see(&clone, GenOneForward(b2, b1));
//
//            setPosition(pos, "2r4k/5Rp1/nqPQ3p/2r5/8/P7/1PR5/K7 w - - 0 1");
//            displayBoard(pos, 8);
//            see(pos, GenOneForward(c6, c7));
//            flipPosition(pos, &clone);
//            positionIsOk(&clone);
//            displayBoard(&clone, 8);
//            see(&clone, GenOneForward(c3, c2));
//
//            setPosition(pos, "5bk1/4qp2/2R3p1/2r1p1Np/2p1N3/1r5P/5PP1/2Q3K1 b - -");
//            displayBoard(pos, 8);
//            see(pos, GenOneForward(c4, c3));
//            flipPosition(pos, &clone);
//            positionIsOk(&clone);
//            displayBoard(&clone, 8);
//            see(&clone, GenOneForward(c5, c6));
        } else if (!strcmp(temp, "quit")) {
            break;
        } else if (!strcmp(temp, "help")) {
            Print(3, "\nhelp - displays this texts\n");
            Print(3, "new - initialize to the starting position\n");
            Print(3, "d - displays the board\n");
            Print(3, "moves - displays all the pseudo-legal moves\n");
            Print(3, "qmoves - displays all the quiescent moves\n");
            Print(3, "evasions - displays all the evasion moves\n");
            Print(3, "qchecks - displays all the quiescent checks\n");
            Print(3, "search X - search the position for X plies\n");
            Print(3, "undo - undo the last move done\n");
            Print(3, "perft X - do a perft test from a set of test positions for depth X\n");
            Print(3, "divide X - displays perft results for every move for depth X\n");
            Print(3, "quit - quits the program\n");
            Print(3, "enter moves in algebraic format: e2e4, e1g1, e7e8q\n");
            Print(3, "this are just the commands as of now\n");
            Print(3, "press any key to continue...\n");
        } else {
            genLegal(pos, &ml);
            move = parseMove(&ml, command);
            if (move) makeMove(pos, &undo, move);
            else Print(3, "Unknown command: %s\n", command);
        }
    }
    quit(pos);
}
