/* wmcellauto - Window Maker dockapp for displaying cellular automata
 * Copyright (C) 2017 Doug Torrance <dtorrance@piedmont.edu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libdockapp/dockapp.h>

#define CELL_WIDTH 2
#define CELL_HEIGHT 2
#define GRID_WIDTH 28
#define GRID_HEIGHT 28
#define DOCKAPP_WIDTH CELL_WIDTH * GRID_WIDTH
#define DOCKAPP_HEIGHT CELL_HEIGHT * GRID_HEIGHT
#define DEFAULT_ALIVE_COLOR "light sea green"
#define DEFAULT_DEAD_COLOR "black"
#define DEFAULT_GENERATION_TIME 250
#define DEFAULT_RESET -1

enum {
	BS_ZERO = 1 << 0,
	BS_ONE = 1 << 1,
	BS_TWO = 1 << 2,
	BS_THREE = 1 << 3,
	BS_FOUR = 1 << 4,
	BS_FIVE = 1 << 5,
	BS_SIX = 1 << 6,
	BS_SEVEN = 1 << 7,
	BS_EIGHT = 1 << 8
};

#define DEFAULT_BIRTH_MASK BS_THREE
#define DEFAULT_SURVIVAL_MASK BS_TWO | BS_THREE

int current_gen[GRID_WIDTH][GRID_HEIGHT];
int next_gen[GRID_WIDTH][GRID_HEIGHT];

int gen_count = 0;
int reset = DEFAULT_RESET;

int birth_mask = DEFAULT_BIRTH_MASK;
int survival_mask = DEFAULT_SURVIVAL_MASK;

char *birth_str = NULL;
char *survival_str = NULL;

char *alive_color = DEFAULT_ALIVE_COLOR;
char *dead_color = DEFAULT_DEAD_COLOR;
int generation_time = DEFAULT_GENERATION_TIME;
char *ruleset = NULL;
int density = 50;

Pixmap pixmap;
GC alive_gc;
GC dead_gc;

void draw_grid(void);
void increment_gen(void);
void randomize_grid(int button, int state, int x, int y);
void set_ruleset_masks(char *ruleset);
int str_to_mask(char *string);

int main(int argc, char **argv)
{
	XGCValues values;
	DAProgramOption options[] = {
		{"-a", "--alivecolor",
		 "color of live cells (default: light sea green)",
		 DOString, False, {&alive_color}},
		{"-d", "--deadcolor",
		 "color of dead cells (default: black)",
		 DOString, False, {&dead_color}},
		{"-t", "--time",
		 "time in ms between generations (default: 250)",
		 DONatural, False, {&generation_time}},
		{"-b", "--birth",
		 "custom ruleset - specify number of adjacent cells\n"
		 "\t\t\t\trequired for birth (default: 3)",
		 DOString, False, {&birth_str}},
		{"-s", "--survival",
		 "custom ruleset - specify number of adjacent cells\n"
		 "\t\t\t\trequired for survival (default: 23)",
		 DOString, False, {&survival_str}},
		{"-r", "--ruleset",
		 "select ruleset (life (default), 2x2, day & night,\n"
		 "\t\t\t\tflock, fredkin, highlife, life without death,\n"
		 "\t\t\t\tlive free or die, maze, mazectric, move,\n"
		 "\t\t\t\treplicator, seeds)",
		 DOString, False, {&ruleset}},
		{NULL, "--density",
		 "percentage of live cells in initial random grid\n"
		 "\t\t\t\t(default: 50)",
		 DONatural, False, {&density}},
		{"-R", "--reset",
		 "number of generations until grid resets\n"
		 "\t\t\t\t(default: -1, i.e., don't reset)",
		 DOInteger, False, {&reset}}
	};

	srand(time(NULL));

	DACallbacks eventCallbacks = {NULL,
				      randomize_grid,
				      NULL, NULL, NULL, NULL,
				      increment_gen};

	DAParseArguments(argc, argv, options, 8,
			 "Window Maker dockapp for displaying cellular "
			 "automata",
			 PACKAGE_STRING);
	DAInitialize(NULL, PACKAGE_NAME, DOCKAPP_WIDTH, DOCKAPP_HEIGHT,
		     argc, argv);
	DASetCallbacks(&eventCallbacks);

	pixmap = DAMakePixmap();

	values.foreground = DAGetColor(alive_color);
	alive_gc = XCreateGC(DADisplay, pixmap, GCForeground, &values);

	values.foreground = DAGetColor(dead_color);
	dead_gc = XCreateGC(DADisplay, pixmap, GCForeground, &values);

	if (ruleset)
		set_ruleset_masks(ruleset);

	if (birth_str)
		birth_mask = str_to_mask(birth_str);
	if (survival_str)
		survival_mask = str_to_mask(survival_str);

	randomize_grid(0, 0, 0, 0);

	DASetTimeout(generation_time);
	DAShow();
	DAEventLoop();

	return 0;
}

void draw_grid(void)
{
	int i,j;
	for (i = 0; i < GRID_WIDTH; i++) {
		for (j = 0; j < GRID_HEIGHT; j++) {
			if (current_gen[i][j] == 1)
				XFillRectangle(DADisplay, pixmap, alive_gc,
					       i*CELL_WIDTH, j*CELL_HEIGHT,
					       CELL_WIDTH, CELL_HEIGHT);
			else
				XFillRectangle(DADisplay, pixmap, dead_gc,
					       i*CELL_WIDTH, j*CELL_HEIGHT,
					       CELL_WIDTH, CELL_HEIGHT);

		}
	}
	DASetPixmap(pixmap);
}

void increment_gen(void)
{
	int i, j, adj_alive;
	for (i = 0; i < GRID_WIDTH; i++) {
		for (j = 0; j < GRID_HEIGHT; j++) {
			adj_alive = 0;
			if (i > 0 && j > 0)
				if (current_gen[i-1][j-1] == 1)
						      adj_alive++;
			if (i > 0)
				if (current_gen[i-1][j] == 1)
						      adj_alive++;
			if (i > 0 && j < GRID_HEIGHT-1)
				if (current_gen[i-1][j+1] == 1)
						      adj_alive++;
			if (j > 0)
				if (current_gen[i][j-1] == 1)
						      adj_alive++;
			if (j < GRID_HEIGHT-1)
				if (current_gen[i][j+1] == 1)
						      adj_alive++;
			if (i < GRID_WIDTH-1 && j > 0)
				if (current_gen[i+1][j-1] == 1)
						      adj_alive++;
			if (i < GRID_WIDTH-1 )
				if (current_gen[i+1][j] == 1)
						      adj_alive++;
			if (i < GRID_WIDTH-1 && j < GRID_HEIGHT-1)
				if (current_gen[i+1][j+1] == 1)
						      adj_alive++;
			if (current_gen[i][j] == 0) {
				if (birth_mask & (1 << adj_alive))
					next_gen[i][j] = 1;
				else
					next_gen[i][j] = 0;
			}
			if (current_gen[i][j] == 1) {
				if (survival_mask & (1 << adj_alive))
					next_gen[i][j] = 1;
				else
					next_gen[i][j] = 0;
			}
		}
	}
	for (i = 0; i < GRID_WIDTH; i++) {
		for (j = 0; j < GRID_HEIGHT; j++) {
			current_gen[i][j] = next_gen[i][j];
		}
	}
	draw_grid();
	gen_count++;
	if (gen_count == reset)
		randomize_grid(0, 0, 0, 0);
}

void randomize_grid(int button, int state, int x, int y)
{
	int i,j;
	for (i = 0; i < GRID_WIDTH; i++) {
		for (j = 0; j < GRID_HEIGHT; j++) {
			if (rand() % 100 <= density)
				current_gen[i][j] = 1;
			else
				current_gen[i][j] = 0;
		}
	}
	draw_grid();
	gen_count = 0;
}

void set_ruleset_masks(char *ruleset)
{
	if (strcmp(ruleset, "life") == 0) {
		/* default masks already set */
	} else if (strcmp(ruleset, "2x2") == 0) {
		birth_mask =  BS_THREE | BS_SIX;
		survival_mask = BS_ONE | BS_TWO | BS_FIVE;
	} else if (strcmp(ruleset, "day & night") == 0) {
		birth_mask =  BS_THREE | BS_SIX | BS_SEVEN | BS_EIGHT;
		survival_mask = BS_THREE | BS_FOUR | BS_SIX | BS_SEVEN |
			BS_EIGHT;
	} else if (strcmp(ruleset, "flock") == 0) {
		birth_mask =  BS_THREE;
		survival_mask = BS_ONE | BS_TWO;
	} else if (strcmp(ruleset, "fredkin") == 0) {
		birth_mask =  BS_ONE | BS_THREE | BS_FIVE | BS_SEVEN;
		survival_mask = BS_ZERO | BS_TWO | BS_FOUR | BS_SIX | BS_EIGHT;
	} else if (strcmp(ruleset, "highlife") == 0) {
		birth_mask =  BS_THREE | BS_SIX;
		survival_mask = BS_TWO | BS_THREE;
	} else if (strcmp(ruleset, "life without death") == 0) {
		birth_mask =  BS_THREE;
		survival_mask = BS_ZERO | BS_ONE | BS_TWO | BS_THREE | BS_FOUR |
			BS_FIVE | BS_SIX | BS_SEVEN | BS_EIGHT;
	} else if (strcmp(ruleset, "live free or die") == 0) {
		birth_mask =  BS_TWO;
		survival_mask = BS_ZERO;
	} else if (strcmp(ruleset, "maze") == 0) {
		birth_mask =  BS_THREE;
		survival_mask = BS_ONE | BS_TWO | BS_THREE | BS_FOUR | BS_FIVE;
	} else if (strcmp(ruleset, "mazectric") == 0) {
		birth_mask =  BS_THREE;
		survival_mask = BS_ONE | BS_TWO | BS_THREE | BS_FOUR;
	} else if (strcmp(ruleset, "move") == 0) {
		birth_mask =  BS_THREE | BS_SIX | BS_EIGHT;
		survival_mask = BS_TWO | BS_FOUR | BS_FIVE;
	} else if (strcmp(ruleset, "replicator") == 0) {
		birth_mask =  BS_ONE | BS_THREE | BS_FIVE | BS_SEVEN;
		survival_mask = BS_ONE | BS_THREE | BS_FIVE | BS_SEVEN;
	} else if (strcmp(ruleset, "seeds") == 0) {
		birth_mask =  BS_TWO;
		survival_mask = 0;
	} else {
		DAWarning("unknown ruleset '%s', defaulting to 'life'",
			  ruleset);
	}
}

int str_to_mask(char *string)
{
	int i, num, mask;

	mask = 0;
	i = 0;
	while (string[i]) {
		num = string[i] - '0';
		if (num < 0 || num > 8)
			DAWarning("'%c' not a number between 0 and 8, ignoring",
				  string[i]);
		else
			mask |= 1 << num;
		i++;
	}
	return mask;
}
