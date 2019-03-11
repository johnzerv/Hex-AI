#define XORSWAP(a,b) ((a)^=(b),(b)^=(a),(a)^=(b)) /* Alternative way of swapping two variable's values */

typedef struct move_list Move;

#define MAX_DIRECTIVE 10
#define MAX_WORDS      6
#define MAX_WORD_SIZE 32

#define MAX_DIM 10  /* If the grid's dimension is bigger than this value, cont will initially play randomly */

/* Directive indeces */
#define NEWGAME     0
#define PLAY        1
#define CONT        2
#define UNDO        3
#define SUGGEST     4
#define LEVEL       5
#define SWAP        6
#define SAVE        7
#define LOAD        8
#define SHOWSTATE   9
#define QUIT       10

char **next_directive(void); /* Parses a line into words and stores them in a string vector */
int get_index(char **); /* Returns the index corresponding to a given directive */
void process(char **); /* Processes a directive */

/* The following functions return either NO_ERROR or an error index */
int newgame(char **); /* Starts a new game with default settings, if no parameters are given */
int play(char **, Move *); /* Reads the next user's move and updates the hex grid accordingly */
int cont(char **, Move *); /* Computes and plays the best move for the player-computer */
int undo(char **); /* Deletes the user's last move (and the computer's, if needed) */
int suggest(char **); /* Suggests the optimal move for the player-user */
int level(char **); /* Updates or prints the game's difficulty */
int swap(char **); /* Applies the swap rule (if that's possible) */
int save(char **); /* Saves the current game state in a file */
int load(char **); /* Loads a game state from a file */

#define NO_ERROR 0

/* Error indeces */
#define INVALID_MOVE         1
#define WRONG_PLAYER         2
#define OCCUPIED_POSITION    3
#define INVALID_DIRECTIVE    4
#define EMPTY_MOVE_LIST      5
#define NO_USER_MOVE_YET     6
#define UNAVAILABLE_SWAP     7
#define MEMALLOC_ERROR       8
#define STATEFILE_ERROR      9
#define INVALID_DIMENSION   10
#define UNAVAILABLE_SUGGEST 11
#define UNAVAILABLE_CONT    12
#define INVALID_DIFFICULTY  13
