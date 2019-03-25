#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "directives.h"
#include "grid.h"
#include "hex.h"

extern game_t game, first_game;
Listptr first_move = NULL, last_move = NULL;

clock_t timer;
double max_time; /* Time limit for each move */

/* Processes a directive */
void process(char **directive) {
  int err_encountered; /* Determines error type, if one occurs */
  int dir_ind; /* Current directive index */
  
  Move current_move;
  static bool swap_occured = FALSE;

  switch(dir_ind = get_index(directive)) {
    case NEWGAME:
      if((err_encountered = newgame(directive))) {
        print_error(err_encountered);
        print_current_player();
        dealloc_char(MAX_WORDS, directive);
        process(next_directive());
      }
      else {
        delete_move_list(&first_move);
        first_move = last_move = NULL;
        dealloc_char(MAX_WORDS, directive);
        print_grid();
      }
      break;

    case PLAY:
      if((err_encountered = play(directive, &current_move))) {
        print_error(err_encountered);
        print_current_player();
        dealloc_char(MAX_WORDS, directive);
        process(next_directive());
      }
      else {
        print_grid();
        printf("Move played: %c%d\n", current_move.col+'A' ,current_move.row+1);
        dealloc_char(MAX_WORDS, directive);
      }
      break;

    case CONT:
      if((err_encountered = cont(directive, &current_move))) {
        print_error(err_encountered);
        print_current_player();
        dealloc_char(MAX_WORDS, directive);
        process(next_directive());
      }
      else {
        print_grid();
        printf("Move played: %c%d\n", current_move.col+'A' ,current_move.row+1);
        dealloc_char(MAX_WORDS, directive);
      }
      break;

    case UNDO:
      if((err_encountered = undo(directive)))
        print_error(err_encountered);
      else {
        if(swap_occured && first_move == NULL) {
          game.swap = ON;
          game.current_player = W;
        }
        else
          game.current_player = game.user;

        print_grid();
      }

      print_current_player();
      dealloc_char(MAX_WORDS, directive);
      process(next_directive());
      break;

    case SUGGEST:
      if((err_encountered = suggest(directive)))
        print_error(err_encountered);

      print_current_player();
      dealloc_char(MAX_WORDS, directive);
      process(next_directive());
      break;

    case LEVEL:
      if((err_encountered = level(directive)))
        print_error(err_encountered);

      print_current_player();
      dealloc_char(MAX_WORDS, directive);
      process(next_directive());
      break;

    case SWAP:
      if((err_encountered = swap(directive))) {
        print_error(err_encountered);
        print_current_player();
        dealloc_char(MAX_WORDS, directive);
        process(next_directive());
      }
      else {
        swap_occured = TRUE;
        print_grid();
        printf("Move played: swap\n");
        dealloc_char(MAX_WORDS, directive);
      }
      break;

    case SAVE:
      if((err_encountered = save(directive)))
        print_error(err_encountered);

      print_current_player();
      dealloc_char(MAX_WORDS, directive);
      process(next_directive());
      break;

    case LOAD:
      if((err_encountered = load(directive)))
        print_error(err_encountered);
      else
        print_grid();

      print_current_player();
      dealloc_char(MAX_WORDS,directive);
      process(next_directive());
      break;

    case SHOWSTATE:
      if(directive[1] != NULL)
        print_error(INVALID_DIRECTIVE);
      else
        print_grid();

      print_current_player();
      dealloc_char(MAX_WORDS, directive);
      process(next_directive());
      break;

    case QUIT:
      if(directive[1] != NULL) {
        print_error(INVALID_DIRECTIVE);
        print_current_player();
        dealloc_char(MAX_WORDS, directive);
        process(next_directive());
      }
      else {
        if(first_game.grid != game.grid)
          dealloc_char(first_game.dimension, first_game.grid);

        delete_move_list(&first_move);
        dealloc_char(game.dimension, game.grid);
        dealloc_char(MAX_WORDS, directive);
        exit(EXIT_SUCCESS);
      }
      break;

    default:
      print_error(INVALID_DIRECTIVE);
      print_current_player();
      dealloc_char(MAX_WORDS, directive);
      process(next_directive());
      break;
  }
}

int newgame(char **directive) {
  game_t temp = first_game;

  /* This will become W in main.c, after process(next_directive()) */
  temp.current_player = B;

  /* If there are no parameters, restart the game with the default settings */
  if(!directive[1]) {
    if(game.grid != first_game.grid)
      dealloc_char(game.dimension, game.grid);

    game = temp;
    empty_grid();
    return NO_ERROR;
  }

  /* Otherwise, the first parameter must be either "black" or "white" */
  if(!strcmp(directive[1], "black"))
    temp.user = B;
  else if(!strcmp(directive[1], "white"))
    temp.user = W;
  else
    return INVALID_DIRECTIVE;

  /* If there isn't a second parameter, restart the game with the new settings */
  if(!directive[2]) {
    if(game.grid != first_game.grid)
      dealloc_char(game.dimension, game.grid);

    game = temp;
    empty_grid();
    return NO_ERROR;
  }

  /* Otherwise, the second parameter must be either "swapoff" or "swapon" */
  if(!strcmp(directive[2], "swapoff"))
    temp.swap = OFF;
  else if(!strcmp(directive[2], "swapon"))
    temp.swap = ON;
  else
    return INVALID_DIRECTIVE;

  /* If there isn't a third parameter, restart the game with the new settings */
  if(!directive[3]) {
    if(game.grid != first_game.grid)
      dealloc_char(game.dimension, game.grid);

    game = temp;
    empty_grid();
    return NO_ERROR;
  }

  /* Otherwise, the final parameter must be the grid's new dimension, in the range [4,26] */
  for(int i = 0; directive[3][i] != '\0'; i++)
    if(!is_digit(directive[3][i]))
      return INVALID_DIRECTIVE;

  temp.dimension = atoi(directive[3]);
  if(temp.dimension < 4 || temp.dimension > 26)
    return INVALID_DIMENSION;
  if(directive[4] != NULL)
    return INVALID_DIRECTIVE; /* newgame has received more than 3 arguments */

  if(game.grid != first_game.grid)
    dealloc_char(game.dimension, game.grid);

  /* Finally, restart the game with the new settings */
  game = temp;
  init_grid();
  return NO_ERROR;
}

int play(char **directive, Move *current_move) {
  /* play must receive exactly one parameter (the player's move) */
  if(directive[2] != NULL || !directive[1] || (directive[1][2] != '\0' &&  directive[1][3] != '\0'))
    return INVALID_DIRECTIVE;
  else if(game.current_player != game.user) /* .. and it should be used on the user's turn */
    return WRONG_PLAYER;

  int strind = 0;
  current_move->col = directive[1][strind++];
  if(current_move->col >= 'A' && current_move->col <= 'A' + (game.dimension-1)) {
    current_move->row = 0;

    if(is_digit(directive[1][strind])) {
      do {
        current_move->row = current_move->row*10 + (directive[1][strind++]-'0');
      } while((is_digit(directive[1][strind])));
    }
    else
      return INVALID_MOVE;

    if(current_move->row < 0 || current_move->row > game.dimension)
      return INVALID_MOVE;

    /* Translate the given move into array indeces */
    current_move->row -= 1;
    current_move->col -= 'A';

    if(game.grid[current_move->row][current_move->col] == ' ')
      game.grid[current_move->row][current_move->col] = (game.current_player == W) ? 'w' : 'b';
    else
      return OCCUPIED_POSITION;
  }
  else
    return INVALID_MOVE;

  insert_at_end(&first_move, &last_move, current_move->row, current_move->col);
  return NO_ERROR;
}

int cont(char **directive, Move *current_move) {
  if(directive[1] != NULL) /* cont must not receive any parameters */
    return INVALID_DIRECTIVE;
  if(game.current_player == game.user) /* .. and it should not be used on the user's turn */
    return UNAVAILABLE_CONT;

  static double total_time_elapsed = 0.0;
  int moves_played = move_count(first_move);

  /* The computer's opening move will be played around the center of the grid */
  if(game.dimension >= 5 && moves_played < 2) {
    current_move->row = game.dimension/2;
    current_move->col = current_move->row - (!(game.dimension % 2));

    if(game.grid[current_move->row][current_move->col] != ' ') {
      current_move->row -= 1 + (game.dimension > 5 && (game.dimension & 01));
      current_move->col++;
    }
  }  /* Plays some of the opening moves randomly, if the grid's dimension is large enough */
  else if(game.dimension > MAX_DIM && moves_played < game.dimension) {
    do {
      current_move->row = rand() % game.dimension;
      current_move->col = rand() % game.dimension;
    } while(game.grid[current_move->row][current_move->col] != ' ');
  }
  else { /* The "normal" case: initiates a minimax search to find the best move available */
    max_time = optimal_time_limit(total_time_elapsed);
    timer = clock();

    /* Iterative deepening is applied on top of minimax */
    int max_difficulty = game.difficulty;
    for(game.difficulty = 1; game.difficulty <= max_difficulty; game.difficulty++) {
      int eval = 0;
      minimax(game.difficulty, TRUE, -INF, INF, current_move, &eval);
      if(eval == INF || eval == -INF) break; /* Critical move found, stop the search */
    }

    total_time_elapsed += calc_time(timer);
    game.difficulty = max_difficulty;
  }

  game.grid[current_move->row][current_move->col] = (game.current_player == W) ? 'w' : 'b';
  insert_at_end(&first_move, &last_move, current_move->row, current_move->col);
  return NO_ERROR;
}

int undo(char **directive) {
  if(directive[1] != NULL) /* undo must not receive any parameters */
    return INVALID_DIRECTIVE;

  /* It also cannot be used if the grid's empty, or the first player isn't the user */
  if(!first_move)
    return EMPTY_MOVE_LIST;
  else if(first_move == last_move && first_move->player_clr != game.user)
    return NO_USER_MOVE_YET;

  /* If the user played last, delete his move */
  if(last_move->player_clr == game.user) {
    game.grid[last_move->row][last_move->col] = ' ';
    remove_last_node(&first_move, &last_move);
    return NO_ERROR;
  }

  /* Otherwise, delete the last two moves */
  Listptr usr_move = first_move;
  while(usr_move->next_move != last_move)
    usr_move = usr_move->next_move;

  game.grid[last_move->row][last_move->col] = ' ';
  remove_last_node(&first_move, &last_move);

  game.grid[usr_move->row][usr_move->col] = ' ';
  remove_last_node(&first_move, &last_move);
  return NO_ERROR;
}

int suggest(char **directive) {
  if(directive[1] != NULL) /* suggest must not receive any parameters */
    return INVALID_DIRECTIVE;
  if(game.current_player != game.user) /* .. and it should be used on the user's turn */
    return UNAVAILABLE_SUGGEST;

  Move current_move;

  max_time = MOVE_TIME_LIMIT;
  timer = clock();

  /* Iterative deepening is applied on top of minimax */
  int max_difficulty = game.difficulty;
  for(game.difficulty = 1; game.difficulty <= max_difficulty; game.difficulty++) {
    int eval = 0;
    minimax(game.difficulty, TRUE, -INF, INF, &current_move, &eval);
    if(eval == INF || eval == -INF) break; /* Critical move found, stop the search */
  }

  game.difficulty = max_difficulty;

  printf("You may play at %c%d\n", current_move.col+'A', current_move.row+1);
  return NO_ERROR;
}

int level(char **directive) {
  if(!directive[1]) { /* If there are no parameters, just print the current difficulty */
    printf("Current game difficulty: %d\n", game.difficulty);
    return NO_ERROR;
  }

  /* Otherwise, level must only take one parameter, which is the new game's difficulty */
  if(directive[2] != NULL)
    return INVALID_DIRECTIVE;

  for(int i = 0; directive[1][i] != '\0'; i++)
    if(!is_digit(directive[1][i]))
      return INVALID_DIRECTIVE;

  /* The game difficulty cannot exceed the maximum game-tree depth: game.dimension^2 */
  game.difficulty = atoi(directive[1]);
  if(game.difficulty > game.dimension*game.dimension)
    return INVALID_DIFFICULTY;

  printf("New game difficulty: %d\n", game.difficulty);
  return NO_ERROR;
}

int swap(char **directive) {
  if(directive[1] != NULL) /* swap must not receive any parameters */
    return INVALID_DIRECTIVE;

  /* .. and it should be used on the user's turn, if available */
  if(game.swap == ON && first_move == last_move && first_move->player_clr != game.user) {
    /* Play the symmetric move for user */
    game.grid[first_move->row][first_move->col] = ' ';
    game.grid[first_move->col][first_move->row] = (game.user == W) ? 'w' : 'b';

    /* swap first_move's row and col values and update first_move's player_clr */
    XORSWAP(first_move->row, first_move->col);
    first_move->player_clr = game.user;

    game.swap = OFF;
    return NO_ERROR;
  }

  return UNAVAILABLE_SWAP;
}

int save(char **directive) {
  if(!directive[1] || directive[2] != NULL) /* save must receive exactly one parameter */
    return INVALID_DIRECTIVE;

  FILE *statefile;
  if(!(statefile = fopen(directive[1], "wb")))
    return STATEFILE_ERROR;

  putc(game.dimension, statefile);
  putc((game.current_player) ? 'w' : 'b', statefile);

  for(int i = 0; i < game.dimension; i++)
    for(int j = 0; j < game.dimension; j++)
      putc((game.grid[i][j] == ' ') ? 'n' : game.grid[i][j], statefile);

  fclose(statefile);
  return NO_ERROR;
}

int load(char **directive) {
  if(!directive[1] || directive[2]) /* load must receive exactly one parameter */
    return INVALID_DIRECTIVE;

  char token;
  game_t temp = game; /* Save current game settings, in case of error */

  FILE *statefile;
  if(!(statefile = fopen(directive[1], "rb")))
    return STATEFILE_ERROR;

  game.dimension = getc(statefile);
  if(game.dimension < 4 || game.dimension > 26) {
    game = temp;
    fclose(statefile);
    return INVALID_DIMENSION;
  }

  init_grid(); /* game.grid now points to a new address (temp still points to the last one) */
  if((token = getc(statefile)) == 'b')
    game.current_player = B;
  else if(token == 'w')
    game.current_player = W;
  else {
    fclose(statefile);
    dealloc_char(game.dimension, game.grid);
    game = temp;
    return STATEFILE_ERROR;
  }

  for(int i = 0; i < game.dimension; i++)
    for(int j = 0; j < game.dimension; j++) {
      token = getc(statefile);
      if(token == 'b' || token == 'w')
        game.grid[i][j] = token;
      else if(token == 'n')
        game.grid[i][j] = ' ';
      else {
        fclose(statefile);
        dealloc_char(game.dimension, game.grid);
        game = temp;
        return STATEFILE_ERROR;
      }
    }

  if((token = getc(statefile)) != EOF) {
    fclose(statefile);
    dealloc_char(game.dimension, game.grid);
    game = temp;
    return STATEFILE_ERROR;
  }

  if(temp.grid != first_game.grid)
    dealloc_char(temp.dimension, temp.grid);

  delete_move_list(&first_move);
  first_move = last_move = NULL;
  fclose(statefile);
  return NO_ERROR;
}

/* Parses a line into words, stores them in a string vector and then returns that vector */
char **next_directive(void) {
  char **directive;
  int w_count, ch_count, token;

  /* Allocate memory for the directive */
  if(!(directive = malloc(sizeof(char *) * MAX_WORDS))) {
    print_error(MEMALLOC_ERROR);
    exit(EXIT_FAILURE);
  }
  for(w_count = 0; w_count < MAX_WORDS; w_count++)
    if(!(directive[w_count] = malloc(sizeof(char) * MAX_WORD_SIZE))) {
      print_error(MEMALLOC_ERROR);
      exit(EXIT_FAILURE);
    }

  printf("> ");

  w_count = 0;
  skip_whitespace();

  while((token = getchar()) != '\n' && w_count < MAX_WORDS - 1) {
    ch_count = 0;

    /* Reads next word */
    do {
      directive[w_count][ch_count++] = token;
    } while(!is_whitespace(token = getchar()) && ch_count < MAX_WORD_SIZE - 1 && token != '\n');

    directive[w_count++][ch_count] = '\0';

    if(token == '\n')
      break;
    skip_whitespace();
  }

  if(token != '\n')
    input_flush();

  directive[w_count] = NULL;
  return directive;
}

/* Returns an index corresponding to the given directive, */
/* or -1, if the directive is invalid                     */
int get_index(char **directive) {
  if(!directive[0])
    return -1;

  char directives[][MAX_DIRECTIVE] = {
    "newgame",
    "play",
    "cont",
    "undo",
    "suggest",
    "level",
    "swap",
    "save",
    "load",
    "showstate",
    "quit"
  };

  int directive_count = sizeof(directives) / sizeof(directives[0]);
  for(int ind = NEWGAME; ind < directive_count; ind++)
    if(!strcmp(directive[0], directives[ind]))
      return ind;

  return -1;
}
