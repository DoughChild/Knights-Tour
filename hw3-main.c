/* hw3-main.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int next_thread_id; /* initialize to 1 */

int max_squares; /* initialize to 0 */

int total_tours; /* initialize to 0 */

/* write the simulate() function and place all of your code in hw3.c */
int simulate(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    next_thread_id = 1;
    max_squares = 0;
    total_tours = 0;
    int rc = simulate(argc, argv);

    /* on Submitty, there will be more code here that validates the
        global variables at this point... */

    return rc;
}