#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hex.h"
#include "grid.h"
#include "directives.h"

extern game_t game;
extern clock_t timer;
extern double max_time;

int minimax(int depth, bool is_maximizing_plr, int a, int b, Move *best_move, int *critical) {
  if(!depth && calc_time(timer) < max_time)
    return static_evaluate(game.current_player);

  int eval;
  if(is_maximizing_plr) { /* Maximizing player's turn */
    int max_eval = -INF; /* Initially, any move is the best option for max */
    
    /* Regard every possible move the maximizing player can play as a next game state */
    for(int i = 0; i < game.dimension && calc_time(timer) < max_time; i++) {
      for(int j = 0; j < game.dimension; j++) {
        if(game.grid[i][j] == ' ' && calc_time(timer) < max_time) {
          game.grid[i][j] = (game.current_player == W) ? 'w' : 'b'; /* Simulate next game state */
          eval = minimax(depth-1, FALSE, a, b, best_move, critical);
          
          /* If the opponent has a winning move (in the next round), then */
          /* there is no need to search further, the priority is to block it */
          if(*critical == -INF) {
            game.grid[i][j] = ' '; /* Undo the simulation */
            return *critical;
          }

          if(max_eval < eval) {
            max_eval = eval;

            /* Update the best move only at the top level of the game tree */
            if(depth == game.difficulty) {
              best_move->row = i;
              best_move->col = j;

              if(max_eval == INF) {
                game.grid[i][j] = ' '; /* Undo the current simulation */
                *critical = INF; /* Notify the caller function that a winning move is available */
                return max_eval; /* No need to search further */
              }
            }
          }

          game.grid[i][j] = ' '; /* Undo the simulation */
          a = max(eval, a);
          if(a >= b) return max_eval;
        }
      }
    }

    return max_eval;
  }
  else { /* Minimizing player's turn */
    int min_eval = INF; /* Initially, any move is the worst option for min */

    /* Regard every possible move the minimizing player can play as a next game state */
    for(int i = 0; i < game.dimension && calc_time(timer) < max_time; i++) {
      for(int j = 0; j < game.dimension; j++) {
        if(game.grid[i][j] == ' ' && calc_time(timer) < max_time) {
          game.grid[i][j] = (game.current_player == W) ? 'b' : 'w'; /* Simulate next game state */
          eval = minimax(depth-1, TRUE, a, b, best_move, critical);

          if(min_eval > eval) {
            min_eval = eval;

            /* Update the best move if the opponent has a winning move in the next round */
            if(depth == game.difficulty-1 && min_eval == -INF) {
              /* Save the opponent's winning move to block it */
              best_move->row = i;
              best_move->col = j;

              game.grid[i][j] = ' '; /* Undo the current simulation */
              *critical = -INF; /* Notify the caller function that the opponent has a winning */
              return min_eval;  /* move (in the next round). No need to search further */
            }
          }

          game.grid[i][j] = ' '; /* Undo the simulation */
          b = min(eval, b);
          if(a >= b) return min_eval;
        }
      }
    } 

    return min_eval;
  }
}

/* Returns an evaluation that determines the quality of a game state for <player> */
int static_evaluate(Colour player) {
  /* Check whether either player has won, returning the corresponding evaluation in each case */
  if(game_finished(!PRINT_PATH, player))  return  INF;
  if(game_finished(!PRINT_PATH, !player)) return -INF;
  
  /* If neither has won, then compute the grid's quality based on a heuristic function */
  return hexes_needed_to_win_difference(player);
}

/* Explores the grid (BFS) and returns the difference of */
/* the number of hexes that each player needs to win (heuristic) */
int hexes_needed_to_win_difference(Colour player) {
  int hexes_needed_for_white = INF;
  int hexes_needed_for_black = INF;

  int **cost_matrix;
  int trans_cost;

  /* Allocate memory for the cost matrix */
  if(!(cost_matrix = malloc(sizeof(int *) * game.dimension))) {
    print_error(MEMALLOC_ERROR);
    exit(EXIT_FAILURE);
  }
  for(int i = 0; i < game.dimension; i++)
    if(!(cost_matrix[i] = malloc(sizeof(int) * game.dimension))) {
      print_error(MEMALLOC_ERROR);
      exit(EXIT_FAILURE);
    }

  init_cost_matrix(cost_matrix, W);

  /* Compute the hexes needed for the white player to win (conducts an up-to-down BFS search) */
  for(int i = 0; i < game.dimension; i++) {
    for(int j = 0; j < game.dimension; j++) {
      if(game.grid[i][j] == 'b') continue; /* 'b' -> anything : infinite cost (for the W player), so skip 'b' */

      /* Right Neighbour */
      if((trans_cost = transition_cost(i, j+1, W)) != -1)
        cost_matrix[i][j+1] = min(cost_matrix[i][j+1], add(cost_matrix[i][j],trans_cost));
      
      /* Left Neighbour */
      if((trans_cost = transition_cost(i, j-1, W)) != -1)
        cost_matrix[i][j-1] = min(cost_matrix[i][j-1], add(cost_matrix[i][j],trans_cost));
      
      /* Down-Left Neighbour */
      if((trans_cost = transition_cost(i+1, j-1, W)) != -1)
        cost_matrix[i+1][j-1] = min(cost_matrix[i+1][j-1], add(cost_matrix[i][j],trans_cost));
      
      /* Down Neighbour */
      if((trans_cost = transition_cost(i+1, j, W)) != -1)
        cost_matrix[i+1][j] = min(cost_matrix[i+1][j], add(cost_matrix[i][j],trans_cost));
    }  
  }

  for(int j = 0; j < game.dimension; j++)
    hexes_needed_for_white = min(hexes_needed_for_white, cost_matrix[game.dimension-1][j]);

  init_cost_matrix(cost_matrix, B);

  /* Compute the hexes needed for the black player to win (conducts a left-to-right BFS search) */
  for(int j = 0; j < game.dimension; j++) {
    for(int i = 0; i < game.dimension; i++) {
      if(game.grid[i][j] == 'w') continue; /* 'w' -> anything : infinite cost (for the B player), so skip 'w' */

      /* Up Neighbour */
      if((trans_cost = transition_cost(i-1, j, B)) != -1)
        cost_matrix[i-1][j] = min(cost_matrix[i-1][j], add(cost_matrix[i][j],trans_cost));
      
      /* Down Neighbour */
      if((trans_cost = transition_cost(i+1, j, B)) != -1)
        cost_matrix[i+1][j] = min(cost_matrix[i+1][j], add(cost_matrix[i][j],trans_cost));
      
      /* Right Neighbour */
      if((trans_cost = transition_cost(i, j+1, B)) != -1)
        cost_matrix[i][j+1] = min(cost_matrix[i][j+1], add(cost_matrix[i][j],trans_cost));
      
      /* Up-Right Neighbour */
      if((trans_cost = transition_cost(i-1, j+1, B)) != -1)
        cost_matrix[i-1][j+1] = min(cost_matrix[i-1][j+1], add(cost_matrix[i][j],trans_cost));
    }  
  }

  for(int i = 0; i < game.dimension; i++) {
    hexes_needed_for_black = min(hexes_needed_for_black, cost_matrix[i][game.dimension-1]);
    free(cost_matrix[i]); 
  }
  free(cost_matrix);

  /* The evaluation returned is the <player>'s score for the given grid state */
  return (hexes_needed_for_black - hexes_needed_for_white) * ((player == W) ? 1 : -1);
}

/* Returns the difference of the maximum hex sequence lengths (heuristic) */
int max_seq_length_difference(Colour player) {
  int current_sequence_len;
  int white_max_len, black_max_len;
  char hex;

  bool **visited; /* Marks the hexes that have been visited (needed for DFS) */

  /* Allocate memory for visited */
  if(!(visited = malloc(sizeof(bool *) * game.dimension))) {
    print_error(MEMALLOC_ERROR);
    exit(EXIT_FAILURE);
  }
  for(int i = 0; i < game.dimension; i++)
    if(!(visited[i] = malloc(sizeof(bool) * game.dimension))) {
      print_error(MEMALLOC_ERROR);
      exit(EXIT_FAILURE);
    }

  /* Initialize visited */
  for(int i = 0; i < game.dimension; i++)
    for(int j = 0; j < game.dimension; j++)
      visited[i][j] = FALSE;

  black_max_len = white_max_len = 0;

  /* Loop through all hexes */
  for(int row = 0; row < game.dimension; row++) {
    for(int col = 0; col < game.dimension; col++) {
      current_sequence_len = 0;
      hex = game.grid[row][col];

      if(!visited[row][col] && hex != ' ') {
        current_sequence_len = compute_sequence_length(row, col, hex, visited);
        if(hex == 'b' && current_sequence_len > black_max_len)
          black_max_len = current_sequence_len;
        else if(hex == 'w' && current_sequence_len > white_max_len)
          white_max_len = current_sequence_len;
      }
    }
  }

  dealloc_bool(game.dimension, visited);

  /* The evaluation returned is the <player>'s score for the given grid state */
  return (white_max_len - black_max_len) * ((player == W) ? 1 : -1);
}

/* Explores the grid (DFS) and computes the length of a hex sequence */
int compute_sequence_length(int row, int col, char hex, bool **visited) {
  visited[row][col] = TRUE;

  int sequence_len = 1; /* A hex forms a hex sequence of length 1 */
  if(is_neighbour(row+1, col, hex) && !visited[row+1][col]) /* Down */
    sequence_len += compute_sequence_length(row+1, col, hex, visited);

  if(is_neighbour(row, col+1, hex) && !visited[row][col+1]) /* Right */
    sequence_len += compute_sequence_length(row, col+1, hex, visited);

  if(is_neighbour(row, col-1, hex) && !visited[row][col-1]) /* Left */
    sequence_len += compute_sequence_length(row, col-1, hex, visited);

  if(is_neighbour(row+1, col-1, hex) && !visited[row+1][col-1]) /* Down-left */
    sequence_len += compute_sequence_length(row+1, col-1, hex, visited);

  if(is_neighbour(row-1, col, hex) && !visited[row-1][col]) /* Up */
    sequence_len += compute_sequence_length(row-1, col, hex, visited);

  if(is_neighbour(row-1, col+1, hex) && !visited[row-1][col+1]) /* Up-right */
    sequence_len += compute_sequence_length(row-1, col+1, hex, visited);

  return sequence_len;
}

/* Checks whether <player> has won or not */
bool game_finished(bool print_path, Colour player) {
  int starting_hex, row, col;
  int step = (player == W) ? 1 : game.dimension; /* Determines the starting-hex sequence */
  char hex = (player == W) ? 'w' : 'b';
  bool player_has_won = FALSE;

  int *path, p_ind;
  bool **visited; /* Marks the hexes that have been visited (needed for DFS) */

  /* Allocate memory for visited */
  if(!(visited = malloc(sizeof(bool *) * game.dimension))) {
    print_error(MEMALLOC_ERROR);
    exit(EXIT_FAILURE);
  }
  for(int i=0; i < game.dimension; i++)
    if(!(visited[i] = malloc(sizeof(bool) * game.dimension))) {
      print_error(MEMALLOC_ERROR);
      exit(EXIT_FAILURE);
    }

  /* Allocate memory for the path, which consists of at most (game.dimension^2)/2 hexes */
  if(!(path = malloc(sizeof(int) * ((game.dimension*game.dimension)/2)))) {
    print_error(MEMALLOC_ERROR);
    exit(EXIT_FAILURE);
  }

  /* Initialize visited */
  for(int i=0; i < game.dimension; i++)
    for(int j=0; j < game.dimension; j++)
      visited[i][j] = FALSE;

  /* Check if opposite-side hexes are connected */
  for(starting_hex = 0; starting_hex < game.dimension; starting_hex++) {
    /* Loop through row 1/column A hexes if <player> is white/black */
    row = (starting_hex*step) / game.dimension;
    col = (starting_hex*step) % game.dimension;

    p_ind = 0;
    if(game.grid[row][col] == hex && !visited[row][col]) {
      if((player_has_won = evaluate_game(row, col, hex, visited, path, &p_ind))) {
        if(print_path)
          print_winner(path, &p_ind);

        free(path);
        dealloc_bool(game.dimension, visited);
        return TRUE;
      }
    }
  }

  free(path);
  dealloc_bool(game.dimension, visited);
  return FALSE;
}

/* Explores the grid (DFS), in order to find if two opposing sides are connected */
bool evaluate_game(int row, int col, char hex, bool **visited, int *path, int *p_ind) {
  visited[row][col] = TRUE;

  /* Check whether the current hex lies on a finishing side */
  bool won = (hex == 'w' && row == game.dimension-1) ||
             (hex == 'b' && col == game.dimension-1);

  if(won)
    path[(*p_ind)++] = row*game.dimension + col;

  /* Search for unvisited neighbouring hexes of the same colour */
  if(!won && is_neighbour(row+1, col, hex) && !visited[row+1][col]) /* Down */
    if((won = evaluate_game(row+1, col, hex, visited, path, p_ind)))
      path[(*p_ind)++] = row*game.dimension + col;

  if(!won && is_neighbour(row, col+1, hex) && !visited[row][col+1]) /* Right */
    if((won = evaluate_game(row, col+1, hex, visited, path, p_ind)))
      path[(*p_ind)++] = row*game.dimension + col;

  if(!won && is_neighbour(row, col-1, hex) && !visited[row][col-1]) /* Left */
    if((won = evaluate_game(row, col-1, hex, visited, path, p_ind)))
      path[(*p_ind)++] = row*game.dimension + col;

  if(!won && is_neighbour(row+1, col-1, hex) && !visited[row+1][col-1]) /* Down-left */
    if((won = evaluate_game(row+1, col-1, hex, visited, path, p_ind)))
      path[(*p_ind)++] = row*game.dimension + col;

  if(!won && is_neighbour(row-1, col, hex) && !visited[row-1][col]) /* Up */
    if((won = evaluate_game(row-1, col, hex, visited, path, p_ind)))
      path[(*p_ind)++] = row*game.dimension + col;

  if(!won && is_neighbour(row-1, col+1, hex) && !visited[row-1][col+1]) /* Up-right */
    if((won = evaluate_game(row-1, col+1, hex, visited, path, p_ind)))
      path[(*p_ind)++] = row*game.dimension + col;

  return won;
}
