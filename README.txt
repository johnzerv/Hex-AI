*******************************************************

This project was created by:

• George Sittas

• John Zervakis

*******************************************************

To compile the source files and produce the game executable file,
run the command "make". The executable file is called "hex".

*******************************************************

HOW TO PLAY
-----------

The following command line options are available:

-n <size>

If the <size> option is given, the grid dimension is set to 4 <= <size> <= 26.
Otherwise, the grid dimension is set to 11 (default dimension).

-d <difficulty>

If the <difficulty> option is given, the game difficulty is set to
<difficulty> <= N^2, where N is the width/height of the grid.
Otherwise, the game difficulty is set to 1 (default difficulty).

-b

The player-user becomes the black player, so he makes the second move.

-s

The swap rule is activated.


The following directives are available during gameplay:

• newgame [white|black [swapoff|swapon [<size>]]]

Starts a new game, possibly with new settings, in the order that is shown,
eg. "newgame white swapon". If no parameters are given, the game settings
are reset to the default settings (always available).

• play <move>

The player-user makes a move, eg. "play A4" (available during the player-user's turn).

• cont

The player-computer makes a move (available during the player-computer's turn).

• undo

Deletes the player-user's last move. If the player-computer made the last move,
that move is also deleted (available only if the player-user has made at least one move).

• suggest

The program suggests a move to the player-user (available during the player-user's turn).

• level [<difficulty>]

Updates the game difficulty if the <difficulty> parameter is given, otherwise it only
prints the current game difficulty. The game difficulty cannot exceed N^2, where N is
the width/height of the hex grid (always available).

• swap

The swap rule is applied (available during the second move, only if it's the user's turn
and the rule is active).

• save <statefile>

Saves the current game state in the file <statefile> (always available).

• load <statefile>

Loads the game state saved in the file <statefile> (always available).

• showstate

Prints the current game state (always available).

• quit

Terminates the program (always available).
