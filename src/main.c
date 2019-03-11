#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "hex.h"
#include "grid.h"
#include "directives.h"

game_t game = {11, 1, W, W, OFF, NULL}; /* Default game settings */
game_t first_game; /* Saves the game settings passed from the command line */

extern Listptr first_move, last_move;

int main(int argc, char **argv) {
  process_CLA(argc, argv);
  init_grid();
  first_game = game;

  srand(2); /* A constant seed is used in order for the games to be able to be reproduced */

  /* The program never exits this loop: it either terminates when the */
  /* directive "quit" is given or when an error happens (eg. malloc error) */
  while(TRUE) {
    print_current_player();
    process(next_directive());

    /* Check if the current player has won (if he has, the winning path will be printed) */
    if(game_finished(PRINT_PATH, game.current_player)) {
      fflush(stdout);

      /* Since the game has finished, only the directives seen below are valid at this point */
      char **directive = next_directive();
      while(directive[0] != NULL
            && strcmp(directive[0], "newgame") != 0
            && strcmp(directive[0], "level") != 0
            && strcmp(directive[0], "quit") != 0)
      {
        print_error(INVALID_DIRECTIVE);
        dealloc_char(MAX_WORDS, directive);
        directive = next_directive();
      }

      /* Restart the game and continue by processing the next directive */
      game.current_player = W;
      empty_grid();
      delete_move_list(&first_move);
      first_move = last_move = NULL;
      process(directive);
    }

    game.current_player ^= 1; /* End of current player's move */
    fflush(stdout);
  }

  return 0;
}
