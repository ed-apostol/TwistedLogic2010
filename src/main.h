/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

void quit(position_t *pos) {
    Print(2, "info string Twisted Logic is quitting.\n");
    fclose(logfile);
    fclose(errfile);
    fclose(dumpfile);
    if (pos->book.bookfile != NULL) fclose(pos->book.bookfile);
    free(pos->trans_table.table);
    free(pos->pawn_table.table);
    exit(EXIT_SUCCESS);
}

int main(void) {
    char command[4096];
    position_t pos;
    uci_option_t uci_option;

    setbuf(stdout, NULL);
    setbuf(stdin, NULL);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    logfile = fopen("logfile.txt", "w");
    errfile = fopen("errfile.txt", "a+");
    dumpfile = fopen("dumpfile.txt", "a+");

    Print(3, "Twisted Logic %s by Edsel Apostol, Philippines\n", VERSION);
    Print(3, "Use Universal Chess Interface commands\n");
    Print(3, "Active Beta Testers: Audy Arandela, Tobias Lagemann, Ray Bongalon\n\n");

    pos.uci_options = &uci_option;
    pos.pawn_table.table = NULL;
    pos.trans_table.table = NULL;
    pos.book.bookfile = NULL;

    initOption();
    initArr();
    initPST();
    initSortPhases();
    initMaterial();
    initBook(&pos);
    initPawnTab(&pos, optionGetInt("Pawn Hash"));
    initTrans(&pos, optionGetInt("Hash"));
    initGameOptions(pos.uci_options);

    setPosition(&pos, FenString[0]);

    while (TRUE) {
        if (fgets(command, 4096, stdin) == NULL)
            strcpy(command, "quit\n");
        Print(2, "%s\n", command);
        if (strncasecmp(command, "ucinewgame", 10) == 0) {
            transClear(&pos);
            pawnTabClear(&pos);
        }
        else if (strncasecmp(command, "uci", 3) == 0) {
            uciStart();
        }
        else if (strncasecmp(command, "debug", 5) == 0) {
            /* dummy */
        }
        else if (strncasecmp(command, "isready", 7) == 0) {
            Print(3, "readyok\n");
        }
        else if (strncasecmp(command, "position", 8) == 0) {
            uciSetPosition(&pos, command + 9);
        }
        else if (strncasecmp(command, "go", 2) == 0) {
            uciGo(&pos, command + 3);
        }
        else if (strncasecmp(command, "setoption", 9) == 0) {
            uciSetOption(&pos, command);
        }
        else if (strncasecmp(command, "testloop", 8) == 0) {
#ifdef DEBUG
            nonUCI(&pos);
#endif
        }
        else if (strncasecmp(command, "stop", 4) == 0) {
            /* no op */
        }
        else if (strncasecmp(command, "quit", 4) == 0) {
            break;
        }
        else Print(3, "info string Unknown UCI command.\n");
    }
    quit(&pos);
}
