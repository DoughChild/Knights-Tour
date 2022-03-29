/* Opsys HW 3
 * Knight's Tour, brute-forcing using multithreading.
 * Compile using:
 * gcc -Wall -Werror hw3-main.c hw3.c -pthread
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>

extern int next_thread_id;
extern int max_squares;
extern int total_tours;

    pthread_mutex_t max_squares_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t total_tours_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct Board
{
    //int thread_num;
    int **board;
    int rows;
    int cols;
    int cur_row;
    int cur_col;
    int num_squares;
} Board;


// acting like a tuple, a move just being a spot on the board
typedef struct Move
{
    int x;
    int y;
} Move;


Move* getMoves(Board *cur_board, int *num_moves)
{
    int cur_row = cur_board->cur_row;
    int cur_col = cur_board->cur_col;
    // array of possible moves
    Move *moves = NULL;
    // check all moves, starting at row - 2, column - 1, then going counter-clockwise
    int row_change = 0, col_change = 0, temp_row, temp_col;
    for (int i = 0; i < 8; i++)
    {
        switch (i)
        {
        case 0:
            row_change = -2;
            col_change = -1;
            break;
        case 1:
            row_change = -1;
            col_change = -2;
            break;
        case 2:
            row_change = 1;
            col_change = -2;
            break;
        case 3:
            row_change = 2;
            col_change = -1;
            break;
        case 4:
            row_change = 2;
            col_change = 1;
            break;
        case 5:
            row_change = 1;
            col_change = 2;
            break;
        case 6:
            row_change = -1;
            col_change = 2;
            break;
        case 7:
            row_change = -2;
            col_change = 1;
            break;
        }

        temp_row = cur_row + row_change;
        temp_col = cur_col + col_change;
        if (0 <= temp_row && temp_row < cur_board->rows)
        {
            if (0 <= temp_col && temp_col < cur_board->cols)
            {
                if (cur_board->board[temp_row][temp_col] == -1) {
                    (*num_moves)++;
                    Move move = {.x = temp_row, .y = temp_col};
                    moves = realloc(moves, sizeof(Move) * (*num_moves));
                    moves[*num_moves - 1] = move;
                }
            }
        }
    }
    return moves;
}


// copies board 2 into board 1
Board* copyBoard(Board *board_1, Board *board_2)
{
    board_1->rows = board_2->rows;
    board_1->cols = board_2->cols;
    board_1->cur_row = board_2->cur_row;
    board_1->cur_col = board_2->cur_col;
    board_1->num_squares = board_2->num_squares;
    for (int i = 0; i < board_2->rows; i++)
    {
        for (int j = 0; j < board_2->cols; j++)
        {
            board_1->board[i][j] = board_2->board[i][j];
        }
    }
    return board_1;
}


// will allocate memory to the rows and columns of a board
int callocBoard(Board* b, int rows, int cols) {
    b->board = calloc(sizeof(int *), rows);
    for (int i = 0; i < rows; i++)
    {
        b->board[i] = calloc(sizeof(int), cols);
    }
    return EXIT_SUCCESS;
}


void* thread_function(void *arg)
{
    // detach from the parent thread so resources are immediately freed
    pthread_t mytid = pthread_self();
    pthread_detach(mytid);

    
    // get the board update values
    Board *cur_board = (Board *)arg;
    cur_board->num_squares++;
    cur_board->board[cur_board->cur_row][cur_board->cur_col] = cur_board->num_squares;
    //printf("T%lu: row/col %d/%d,  current location (%d, %d), num_squares: %d\n", (unsigned long)mytid, cur_board->rows, cur_board->cols, cur_board->cur_row, cur_board->cur_col, cur_board->num_squares);


    for (int i = 0; i < 1; i++) {
        // get new moves
        int num_moves = 0;
        Move* possible_moves = getMoves(cur_board, &num_moves);

        printf("T%lu: Num moves %d, and are: ", (unsigned long)mytid, num_moves);
        for (int k = 0; k < num_moves; k++) {
            printf(" (%d, %d)", possible_moves[k].x, possible_moves[k].y);
        }
        printf(". Num_squares is %d\n", cur_board->num_squares);        
        
        // if there are no possible moves then we should print out dead end
        if (num_moves == 0) {
            int total_squares = cur_board->rows * cur_board->cols;
            if (cur_board->num_squares == total_squares) {
                // locks are to protect critcal sections
                // get lock on updating max squares
                pthread_mutex_lock(&max_squares_mutex);
                {
                    if (cur_board->num_squares > max_squares) {
                        printf("T%lu: Sonny found a full knight's tour; incremented total_tours; updated max_squares\n", (unsigned long)mytid);            
                        max_squares = cur_board->num_squares;
                    } else {
                        printf("T%lu: Sonny found a full knight's tour; incremented total_tours\n", (unsigned long)mytid);            
                    }
                }
                pthread_mutex_unlock(&max_squares_mutex);

                // get lock on updating max tours
                pthread_mutex_lock(&total_tours_mutex);
                {
                    total_tours++;
                }
                pthread_mutex_unlock(&max_squares_mutex);

            } else {
                // get the lock for the max squares, update the value
                pthread_mutex_lock(&max_squares_mutex);
                {
                    if (cur_board->num_squares > max_squares) {
                        printf("T%lu: Dead end at move #%d; updated max_squares\n", (unsigned long)mytid, cur_board->num_squares);
                        max_squares = cur_board->num_squares;
                    } else {
                        printf("T%lu: Dead end at move #%d\n", (unsigned long)mytid, cur_board->num_squares);
                    }
                }
                pthread_mutex_unlock(&max_squares_mutex);
            }
        } else if (num_moves == 1) { 
            // this is the only case where we stay within this loop
            // we update the information about our current board and go again
            cur_board->cur_row = possible_moves[0].x;
            cur_board->cur_col = possible_moves[0].y;
            cur_board->num_squares++;
            cur_board->board[cur_board->cur_row][cur_board->cur_col] = cur_board->num_squares;
            i--;
        } else {
            // the last case is that there are multiple moves
            // in this case we have to create a number of child threads
            printf("T%lu: %d possible moves after move #%d; creating %d child threads...\n", (unsigned long)mytid, num_moves, cur_board->num_squares, num_moves);
            for (int j = 0; j < num_moves; j++)
            {
                // creating a board for each child thread
                Board* b = (struct Board *)calloc(sizeof(Board), 1);
                callocBoard(b, cur_board->rows, cur_board->cols);
                b = copyBoard(b, cur_board);
                b->cur_row = possible_moves[j].x;
                b->cur_col = possible_moves[j].y;

                pthread_t tid;
                int rc = pthread_create(&tid, NULL, thread_function, b);
                if (rc != 0)
                {
                    fprintf(stderr, "ERROR: pthread_create() failed (%d): %s\n", rc, strerror(rc));
                    return NULL;
                }
            }
        }
    }

    sleep(1);
    return NULL;
}



int simulate(int argc, char *argv[])
{
    /* Error checking */
    if (argc != 5)
    {
        fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> <r> <c>\n");
        return EXIT_FAILURE;
    }
    for (int i = 1; i < argc; i++)
    {
        if (!isdigit(argv[i][0]))
        {
            fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: hw2.out <m> <n> <r> <c>\n");
            return EXIT_FAILURE;
        }
    }
    // getting args
    int rows = atoi(argv[1]);
    int cols = atoi(argv[2]);
    int start_row = atoi(argv[3]);
    int start_col = atoi(argv[4]);
    // error checking
    if (rows <= 2 || cols <= 2)
    {
        fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> <r> <c>\n");
        return EXIT_FAILURE;
    }

    // suggested in HW, helpful to debug
    setvbuf( stdout, NULL, _IONBF, 0 );

    // ensure these are what they are meant to be
    next_thread_id = 1;
    // and cheekily update this value, because Sonny will always discover the tile beneath his feet
    max_squares = 1;
    total_tours = 0;




    // create first board:
    Board *start_board = (struct Board *)calloc(sizeof(Board), 1);
    callocBoard(start_board, rows, cols);
    // initialize all undiscovered spots to -1
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            start_board->board[i][j] = -1;
        }
    }
    start_board->num_squares = 1;
    // discover the starting spot
    start_board->board[start_row][start_col] = 1;
    start_board->cur_row = start_row;
    start_board->cur_col = start_col;
    start_board->rows = rows;
    start_board->cols = cols;

    printf("MAIN: Solving Sonny's knight's tour problem for a %dx%d board\n", rows, cols);
    printf("MAIN: Sonny starts at row %d and column %d (move #1)\n", start_row, start_col);

    // get the moves to generate any threads
    int num_moves = 0;
    Move* possible_moves = getMoves(start_board, &num_moves);
    if (num_moves > 0) {
        printf("MAIN: %d possible moves after move #1; creating %d child threads...\n", num_moves, num_moves);
    } else {
        printf("MAIN: Dead end at move #1\n");
        printf("MAIN: Search complete; best solution(s) visited 1 squares out of %d\n", (rows * cols));
    }

    // make array to hold IDs of al child threads, so that we can wait on them
    for (int i = 0; i < num_moves; i++)
    {
        // creating a board for each child thread
        Board* b = (struct Board *)calloc(sizeof(Board), 1);
        callocBoard(b, rows, cols);
        b = copyBoard(b, start_board);
        b->cur_row = possible_moves[i].x;
        b->cur_col = possible_moves[i].y;
        //b->thread_num = i + 1;

        // GENERATE THREADS HERE
        pthread_t tid;
        int rc = pthread_create(&tid, NULL, thread_function, b);
        if (rc != 0)
        {
            fprintf(stderr, "ERROR: pthread_create() failed (%d): %s\n", rc, strerror(rc));
            return EXIT_FAILURE;
        }
    }

    //printf("freeing array\n");
    free(possible_moves);
    for (int i = 0; i < rows; i++)
    {
        free(start_board->board[i]);
    }
    free(start_board->board);
    free(start_board);

    sleep(3);

    printf("most squares found: %d\n", max_squares);
    printf("knight's tours found: %d\n", total_tours);
    printf("ran <3 <3 <3 \n");

    return EXIT_SUCCESS;
}
