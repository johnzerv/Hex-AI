#include <limits.h>
#define INF INT_MAX /* Represents infinity */

#define PRINT_PATH 1 /* Determines whether game_finished() will print the winning path or not */

typedef enum {B, W} Colour;
typedef unsigned bool;

#define TRUE  1
#define FALSE 0

typedef struct game_t {
  int dimension;
  int difficulty;
  Colour user;
  Colour current_player;
  enum {OFF, ON} swap;
  char **grid;
} game_t; /* Contains info about the game's settings */

typedef struct move_list *Listptr;
typedef struct move_list {
  int row, col;
  Colour player_clr;
  Listptr next_move;
} Move; /* Linked list containing info about the game's moves sequence */

int minimax(int, bool, int, int, Move *, int *);

#define MOVE_TIME_LIMIT 30.0 /* Maximum time the computer's moves are allowed to take */
#define TOTAL_TIME_LIMIT (60.0*game.dimension/2.0) /* Max total time the computer's moves can take */
#define TIME_THRESHOLD 20.0 /* Determines when the computer will start playing very quickly */

#define calc_time(a) ((double) (clock() - (a)) / CLOCKS_PER_SEC)

double optimal_time_limit(double); /* Computes the time limit for each of the computer's moves */

int static_evaluate(Colour); /* Evaluates the quality of a grid state for a player */

int hexes_needed_to_win_difference(Colour);
int transition_cost(int, int, Colour);
void init_cost_matrix(int **, Colour);

int add(int, int);
int min(int, int);
int max(int, int);

int max_seq_length_difference(Colour);
int compute_sequence_length(int, int, char, bool **);

bool game_finished(bool, Colour); /* Checks whether a player's sides are connected */

/* Checks whether a hex at one side is connected to the opposing side (DFS) */
bool evaluate_game(int, int, char, bool **, int *, int *);

bool is_whitespace(unsigned);
bool valid_coordinates(int, int);
bool is_neighbour(int, int, char);
bool is_digit(unsigned);

void process_CLA(int argc, char **argv); /* Parses and processes Command Line Arguments */
void skip_whitespace(void);
void input_flush(void); /* Reads input characters until '\n' is read (for error-handling) */

void print_current_player(void);
void print_winner(int *, int *);
void print_error(int);

/* Utility functions for deallocating two-dimensional arrays */
void dealloc_char(int, char **);
void dealloc_bool(int, bool **);

/* Linked list utility functions */
void insert_at_end(Listptr *, Listptr *, int, int);
void remove_last_node(Listptr *, Listptr *);
void delete_move_list(Listptr *);
int  move_count(Listptr);
