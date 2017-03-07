/* wmcellauto - Window Maker dockapp for displaying cellular automaton
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
int birth_mask = DEFAULT_BIRTH_MASK;
int survival_mask = DEFAULT_SURVIVAL_MASK;

char *alive_color = DEFAULT_ALIVE_COLOR;
char *dead_color = DEFAULT_DEAD_COLOR;
Pixmap pixmap;
GC alive_gc;
GC dead_gc;

void draw_grid(void);
void increment_gen(void);
void randomize_grid(int button, int state, int x, int y);

int main(int argc, char **argv)
{
	XGCValues values;
	DAProgramOption options[] = {
		{"-a", "--alivecolor",
		 "color of live cells (default: light sea green)",
		 DOString, False, {&alive_color}},
		{"-d", "--deadcolor",
		 "color of dead cells (default: black)",
		 DOString, False, {&dead_color}}
	};

	srand(time(NULL));

	DACallbacks eventCallbacks = {NULL,
				      randomize_grid,
				      NULL, NULL, NULL, NULL,
				      increment_gen};

	DAParseArguments(argc, argv, options, 2,
			 "Window Maker dockapp for displaying cellular "
			 "automaton",
			 PACKAGE_STRING);
	DAInitialize(NULL, PACKAGE_NAME, DOCKAPP_WIDTH, DOCKAPP_HEIGHT,
		     argc, argv);
	DASetCallbacks(&eventCallbacks);

	pixmap = DAMakePixmap();

	values.foreground = DAGetColor(alive_color);
	alive_gc = XCreateGC(DADisplay, pixmap, GCForeground, &values);

	values.foreground = DAGetColor(dead_color);
	dead_gc = XCreateGC(DADisplay, pixmap, GCForeground, &values);

	randomize_grid(0, 0, 0, 0);

	DASetTimeout(250);
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
}

void randomize_grid(int button, int state, int x, int y)
{
	int i,j;
	for (i = 0; i < GRID_WIDTH; i++) {
		for (j = 0; j < GRID_HEIGHT; j++) {
			current_gen[i][j] = rand() % 2;
		}
	}
	draw_grid();
}
