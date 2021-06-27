#include <stdio.h>
#include <stdlib.h>

#include "krclib.h"

typedef enum CellDirs {
	CELL_NORTH, CELL_EAST, CELL_WEST, CELL_SOUTH,
	STANDARD_ENUM_VALUES(CELL)
} CellDirs;

typedef struct maze_cell {
	int row, col;
	union {
		struct { struct maze_cell *north, *east, *west, *south; };
		struct maze_cell *at[4];
	} edges;
} maze_cell;

static inline int row_col_index(int row, int col, int ncols)
{
	return (row * ncols) + col;
}

typedef struct {
	int nrows, ncols;
	maze_cell cells[];
} maze_grid;

typedef TVector(int, row, col) rowcol;

typedef TVector(int, start, stop) range;

typedef struct {
	range first, second;
	rowcol cur;
} iter2d;

rowcol iter2d_start(iter2d *iter)
{
	return iter->cur = (rowcol){ .row = iter->first.start, .col = iter->second.start };
}

bool iter2d_done(const iter2d *iter)
{
	return iter->cur.row >= iter->first.stop;
}

rowcol iter2d_next(iter2d *iter)
{
	++iter->cur.col;
	if (iter->cur.col >= iter->second.stop) {
		iter->cur.col = iter->second.start;
		++iter->cur.row;
	}
	return iter->cur;
}

maze_grid *maze_grid_create(int nrows, int ncols)
{
	int num_cells = nrows * ncols;
	maze_grid *grid = malloc(sizeof(*grid) + (sizeof(*grid->cells) * num_cells));
	grid->nrows = nrows;
	grid->ncols = ncols;

	int i = 0;
	iter2d iter = { {0,nrows}, {0,ncols} };
	for (rowcol rc = iter2d_start(&iter); !iter2d_done(&iter); rc = iter2d_next(&iter))
		grid->cells[i++] = (maze_cell){ .row = rc.row, .col = rc.col };

	return grid;
}

maze_cell *maze_cell_at(maze_grid *grid, int row, int col)
{
	return &grid->cells[row_col_index(row, col, grid->ncols)];
}

void maze_grid_draw_ascii(maze_grid *grid)
{
	for (int row = 0; row < grid->nrows; ++row) {

		for (int col = 0; col < grid->ncols; ++col) {
			const maze_cell *cell = maze_cell_at(grid, row, col);
			if (cell->edges.north)
				printf(" | ");
			else
				printf("   ");
		}
		putchar('\n');

		for (int col = 0; col < grid->ncols; ++col) {
			const maze_cell *cell = maze_cell_at(grid, row, col);

			if (cell->edges.west)
				putchar('-');
			else
				putchar(' ');

			putchar('+');

			if (cell->edges.east)
				putchar('-');
			else
				putchar(' ');
		}
		putchar('\n');

		for (int col = 0; col < grid->ncols; ++col) {
			const maze_cell *cell = maze_cell_at(grid, row, col);
			if (cell->edges.south)
				printf(" | ");
			else
				printf("   ");
		}
		putchar('\n');
	}
}


int main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	puts("Hello maze");

	int size = 8;
	int seed = 432;

	srand(seed);

	maze_grid *grid = maze_grid_create(size, size);

	maze_cell *cell = &grid->cells[1];  // skip top-left cell
	maze_cell *end  = grid->cells + (grid->ncols * grid->nrows);
	while (cell != end) {

		CellDirs choices[2];
		int n = 0;

		if (cell->row > 0)  choices[n++] = CELL_NORTH;
		if (cell->col > 0)  choices[n++] = CELL_WEST;

		if (choices[rand()%n] == CELL_NORTH) {
			maze_cell *link = maze_cell_at(grid, cell->row-1, cell->col);
			cell->edges.at[CELL_NORTH] = link;
			link->edges.at[CELL_SOUTH] = cell;
		}
		else {
			maze_cell *link = maze_cell_at(grid, cell->row, cell->col-1);
			cell->edges.at[CELL_WEST] = link;
			link->edges.at[CELL_EAST] = cell;
		}
		++cell;
	}

	maze_grid_draw_ascii(grid);

	free(grid);
	return 0;
}
 

