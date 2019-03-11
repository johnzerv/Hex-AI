#include <stdio.h>
#include <stdlib.h>

#include "hex.h"
#include "grid.h"
#include "directives.h"

extern game_t game;

void print_grid(void) {
  int i, j;
  int indent = 2;
  char hex_cell[] = "   ";

  /* Data concerning the message "BLACK" */
  int flag = game.dimension & 01; /* Determines when "BLACK" will be printed */
  int B_pos = (game.dimension-1)/2; /* Determines where 'B' will be printed */
  int K_pos = B_pos + 2; /* Determines where 'K' will be printed */
  int black_ind = 0;
  char *black = "BLACK";

  putchar('\n');

  space_pad((game.dimension-1)*2);
  printf("W H I T E\n");

  space_pad(4);
  for(i = 0; i < game.dimension; i++) /* Prints the letters */
    printf("%c%s", ('A' + i), (i == game.dimension-1) ? "\n" : "   ");

  space_pad(4);
  for(i = 0; i < game.dimension; i++) /* Prints the first row of underscores */
    printf("_%s", (i == game.dimension-1) ? "\n" : "   ");

  space_pad(3);
  for(i = 0; i < game.dimension; i++) /* Prints the first row of / \_/ \_ .. etc*/
    printf("/ \\%c", (i == game.dimension-1) ? '\n' : '_');

  /* Prints the main hex grid */
  for(i = 0; i < game.dimension; i++, indent += 2) { /* i+1 represents the row index */
    space_pad(indent - ((i+1 >= 10) ? 3 : 2)); /* Takes care of 2-digit nums */
    
    /* Prints the lines containing the vertical bar '|' */
    printf("%d ", i+1); /* Prints the left row index */
    for(j = 0; j <= game.dimension; j++) {
      hex_cell[1] = game.grid[i][j];
      printf("|%s", (j == game.dimension) ? " " : hex_cell);
    }
    printf("%d", i+1); /* Prints the right row index */
    
    /* Prints the next letter of "BLACK", if needed */
    if((i+1) >= B_pos && (i+1) <= K_pos && flag) {
      space_pad(((i+1) >= 10) ? 2 : 3); /* Takes care of 2-digit nums */
      printf("%c", black[black_ind++]);
    }
    putchar('\n');

    space_pad(indent);
    for(j = 0; j < game.dimension; j++) /* Prints the lines containing \_/ \_/ .. etc */
      printf(" \\_/");
    printf(" %s", (i == game.dimension-1) ? "" : "\\");
    
    /* Prints the next letter of "BLACK", if needed */
    if((i+1) >= B_pos && (i+1) <= K_pos) {
      flag = 1;
      if(black_ind <= 4) {
        space_pad(5);
        printf("%c", black[black_ind++]);
      }
    }

    putchar('\n');
  }

  space_pad(indent);
  for(i = 0; i < game.dimension; i++) /* Prints the letters */
    printf("%c%s", ('A' + i), (i == game.dimension-1) ? "\n" : "   ");

  putchar('\n');
}

/* Allocates memory for the game grid and initializes it with empty_grid() */
void init_grid(void) {
  if(!(game.grid = malloc(sizeof(char *) * game.dimension))) {
    print_error(MEMALLOC_ERROR);
    exit(EXIT_FAILURE);
  }
  for(int i = 0; i < game.dimension; i++)
    if(!(game.grid[i] = malloc(sizeof(char) * game.dimension))) {
      print_error(MEMALLOC_ERROR);
      exit(EXIT_FAILURE);
    }

  empty_grid();
}

/* Fills the game grid with spaces (denoting empty hex cells) */
void empty_grid(void) {
  for(int i = 0; i < game.dimension; i++)
    for(int j = 0; j < game.dimension; j++)
      game.grid[i][j] = ' ';
}

/* Prints a specified number of space characters */
void space_pad(unsigned indent) {
  while(indent--)
    putchar(' ');
}
