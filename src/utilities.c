#include <stdio.h>
#include <stdlib.h>

#include "hex.h"
#include "directives.h"

extern game_t game;
extern Listptr first_move;

/* Parses and processes Command Line Arguments */
void process_CLA(int argc, char **argv) {
  int argind;
  for(argind = 1; argind < argc && argv[argind][0] == '-'; argind++) {
    switch(argv[argind][1]) {
      case 'n':
        if(!argv[++argind]) {
          fprintf(stderr, "%s: Invalid arguments\n", argv[0]);
          exit(EXIT_FAILURE);
        }
        
        for(int i = 0; argv[argind][i] != '\0'; i++)
          if(!is_digit(argv[argind][i])) {
            fprintf(stderr, "%s: Invalid arguments\n", argv[0]);
            exit(EXIT_FAILURE);
          }

        game.dimension = atoi(argv[argind]);
        if(game.dimension < 4 || game.dimension > 26) {
          fprintf(stderr, "%s: Invalid arguments\n", argv[0]);
          exit(EXIT_FAILURE);
        }
        break;

      case 'd':
        if(!argv[++argind]) {
          fprintf(stderr, "%s: Invalid arguments\n", argv[0]);
          exit(EXIT_FAILURE);
        }

        for(int i = 0; argv[argind][i] != '\0'; i++)
          if(!is_digit(argv[argind][i])) {
            fprintf(stderr, "%s: Invalid arguments\n", argv[0]);
            exit(EXIT_FAILURE);
          }

        if((game.difficulty = atoi(argv[argind])) < 1) {
          fprintf(stderr, "%s: Invalid arguments\n", argv[0]);
          exit(EXIT_FAILURE);
        }
        break;

      case 'b':
        game.user = B;
        break;

      case 's':
        game.swap = ON;
        break;

      default:
        fprintf(stderr, "%s: Invalid arguments\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  /* Takes care of option-arguments not starting with '-' */
  if(argind < argc) {
    fprintf(stderr, "%s: Invalid arguments\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* The game difficulty cannot exceed the maximum game-tree depth: game.dimension^2 */
  if(game.difficulty > game.dimension*game.dimension) {
    fprintf(stderr, "%s: Invalid game difficulty\n", argv[0]);
    exit(EXIT_FAILURE);
  }
}

void print_current_player(void) {
  printf("%s player ", (game.current_player == W) ? "White" : "Black");
  printf("(%s) plays now\n", (game.current_player == game.user) ? "human" : "computer");
}

void skip_whitespace(void) {
  unsigned token;
  while(is_whitespace(token = getchar()));

  ungetc(token, stdin);
}

bool valid_coordinates(int row, int col) {
  return (row >= 0 && row < game.dimension && col >= 0 && col < game.dimension);
}

bool is_neighbour(int row, int col, char hex) {
  return (valid_coordinates(row, col) && game.grid[row][col] == hex);
}

/* Finds the transition (edge) cost from one hex to another */
int transition_cost(int row, int col, Colour player) {
  if(!valid_coordinates(row, col)) return -1;

  char unreachable_hex = (player == W) ? 'b' : 'w';
  return (game.grid[row][col] == unreachable_hex) ? INF : (game.grid[row][col] == ' ');
}

void init_cost_matrix(int **cost_matrix, Colour player) {
  for(int k = 0; k < game.dimension; k++) {
    if(player == W)
      cost_matrix[0][k] = transition_cost(0, k, W);
    else
      cost_matrix[k][0] = transition_cost(k, 0, B); 
  }
  
  for(int i = (player == W); i < game.dimension; i++)
    for(int j = (player == B); j < game.dimension; j++)
      cost_matrix[i][j] = INF;
}

void print_winner(int *path, int *p_ind) {
  int row, col;

  printf("%s player ", (game.current_player == W) ? "White" : "Black");
  printf("(%s) ", (game.current_player == game.user) ? "human" : "computer");
  printf("wins with path ");

  /* The winning move sequence is stored inside the path stack */
  while(--(*p_ind) >= 0) {
    row = path[*p_ind] / game.dimension;
    col = path[*p_ind] % game.dimension;

    printf("%c%d%c", col+'A', row+1, (*p_ind==0) ? '\n' : '-');
  }
}

/* Deallocates a dynamically allocated 2 dimensional char array */
void dealloc_char(int argc, char **argv) {
  for(int i = 0; i < argc; i++)
    free(argv[i]);
  free(argv);
}

/* Deallocates a dynamically allocated 2 dimensional bool array */
void dealloc_bool(int argc, bool **argv) {
  for(int i = 0; i < argc; i++)
    free(argv[i]);
  free(argv);
}

void print_error(int error) {
  switch(error) {
    case INVALID_MOVE:
      fprintf(stderr, "Invalid move\n");
      break;
    case WRONG_PLAYER:
      fprintf(stderr, "It's not your turn to make a move\n");
      break;
    case OCCUPIED_POSITION:
      fprintf(stderr, "Position occupied\n");
      break;
    case INVALID_DIRECTIVE:
      fprintf(stderr, "Invalid directive\n");
      break;
    case EMPTY_MOVE_LIST:
      fprintf(stderr, "No moves have been played yet\n");
      break;
    case NO_USER_MOVE_YET:
      fprintf(stderr, "You need to make a move first\n");
      break;
    case UNAVAILABLE_SWAP:
      fprintf(stderr, "swap directive is not available\n");
      break;
    case MEMALLOC_ERROR:
      fprintf(stderr, "Memory allocation error\n");
      break;
    case STATEFILE_ERROR:
      fprintf(stderr, "File cannot be opened\n");
      break;
    case INVALID_DIMENSION:
      fprintf(stderr, "Invalid dimension\n");
      break;
    case UNAVAILABLE_SUGGEST:
      fprintf(stderr, "suggest directive is not available\n");
      break;
    case UNAVAILABLE_CONT:
      fprintf(stderr, "cont directive is not available\n");
      break;
    case INVALID_DIFFICULTY:
      fprintf(stderr, "Invalid game difficulty\n");
      break;
  }
}

/* Reads input characters until '\n' is read (for error-handling) */
void input_flush(void) {
  unsigned token;
  while((token = getchar()) != '\n');
}

bool is_whitespace(unsigned token) {
  return (token == ' ' || token == '\t');
}

bool is_digit(unsigned token) {
  return (token >= '0' && token <= '9');
}

/* Adds two distances returning infinite if either is infinite (idea from: Orestis Polychroniou) */
int add(int a, int b) {
  return (a == INF || b == INF) ? INF : a + b;
}

/* Computes the minimum of two distances (handles infinity) */
int min(int a, int b) {
  return (b == INF || (a != INF && a < b)) ? a : b;
}

/* Computes the maximum of two distances (handles infinity) */
int max(int a, int b) {
  return (b == INF || (a != INF && a < b)) ? b : a;
}

/* Calculates the optimal time limit for each of the computer's moves */
double optimal_time_limit(double total_time_elapsed) {
  double total_time_remaining = TOTAL_TIME_LIMIT - total_time_elapsed;

  /* Checks whether the player-computer must start playing quickly or not. The */
  /* return value is simply the theoretical time limit for each move the */
  /* player-computer makes, in order for him to not get disqualified by exceeding */
  /* his total move time limit (which is equal to game.dimension/2 minutes) */

  if(total_time_remaining <= TIME_THRESHOLD)
    return game.dimension/(2.0*MOVE_TIME_LIMIT);

  /* For large grids, if the remaining moves are more than 1/3 of the total moves, */
  /* the player-computer will take less time to make each move. In all other cases, */
  /* he will take more time, taking a small time delay into consideration as well (eg. */
  /* the time needed to terminate the minimax search) */

  return ((game.dimension >= 11 && 2*game.dimension*game.dimension/3 >= move_count(first_move))
             ? MOVE_TIME_LIMIT / 3.0
             : MOVE_TIME_LIMIT - ((game.dimension > 11) ? 5.0 : 2.0));
}

/* Linked list utility functions */
void insert_at_end(Listptr *ptraddr, Listptr *last_move, int row, int col) {
  while(*ptraddr != NULL)
    ptraddr = &((*ptraddr)->next_move);

  if(!(*ptraddr = malloc(sizeof(Move)))) {
    print_error(MEMALLOC_ERROR);
    exit(EXIT_FAILURE);
  }

  (*ptraddr)->row = row;
  (*ptraddr)->col = col;
  (*ptraddr)->player_clr = game.current_player;
  (*ptraddr)->next_move = NULL;

  *last_move = *ptraddr;
}

void remove_last_node(Listptr *ptraddr, Listptr *last_move) {
  Listptr new_last_move = NULL;
  while((*ptraddr) != (*last_move)) {
    new_last_move = *ptraddr;
    ptraddr = &((*ptraddr)->next_move);
  }

  /* Delete last move */
  Listptr temp = *ptraddr;
  *ptraddr = (*ptraddr)->next_move;
  free(temp);

  *last_move = new_last_move;
}

void delete_move_list(Listptr *ptraddr) {
  if(!(*ptraddr))
    return;

  delete_move_list(&((*ptraddr) -> next_move));
  free(*ptraddr);
}

int move_count(Listptr list) {
  int count = 0;

  while(list) {
    count++;
    list = list->next_move;
  }

  return count;
}
