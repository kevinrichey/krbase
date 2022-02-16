#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "krclib.h"

typedef TVector(int, row, col) rowcol;

enum cell_dirs 
{
	CELL_NORTH, CELL_EAST, CELL_WEST, CELL_SOUTH,
	STANDARD_ENUM_VALUES(CELL)
};

struct maze_cell 
{
	rowcol pos;
	struct 
	{ 
		struct maze_cell *north,
		                 *east,
		                 *west,
		                 *south;
	} edges;
};

static inline int row_col_index(int row, int col, int ncols)
{
	return (row * ncols) + col;
}

struct maze_grid
{
	int nrows, ncols;
	struct maze_cell cells[];
};

struct iter2d
{
	struct range first, second;
	rowcol cur;
};

rowcol iter2d_start(struct iter2d *iter)
{
	return iter->cur = (rowcol){ .row = iter->first.start, .col = iter->second.start };
}

bool iter2d_done(const struct iter2d *iter)
{
	return iter->cur.row >= iter->first.stop;
}

rowcol iter2d_next(struct iter2d *iter)
{
	if (++iter->cur.col >= iter->second.stop) 
	{
		iter->cur.col = iter->second.start;
		++iter->cur.row;
	}
	return iter->cur;
}

struct maze_grid *maze_grid_create(int nrows, int ncols)
{
	int num_cells = nrows * ncols;
	size_t grid_size = 0;
	struct maze_grid *grid = malloc(grid_size = sizeof(*grid) + (sizeof(*grid->cells) * num_cells));
	memset(grid, 0, grid_size);
	grid->nrows = nrows;
	grid->ncols = ncols;

	int i = 0;
	struct iter2d iter = { {0,nrows}, {0,ncols} };
	for (rowcol rc = iter2d_start(&iter); !iter2d_done(&iter); rc = iter2d_next(&iter))
		grid->cells[i++].pos = rc;

	return grid;
}

struct maze_cell *maze_cell_at(struct maze_grid *grid, int row, int col)
{
	return &grid->cells[row_col_index(row, col, grid->ncols)];
}

void maze_grid_draw_ascii(struct maze_grid *grid)
{
	for (int row = 0; row < grid->nrows; ++row) {

		for (int col = 0; col < grid->ncols; ++col) {
			const struct maze_cell *cell = maze_cell_at(grid, row, col);
			if (cell->edges.north)
				printf(" | ");
			else
				printf("   ");
		}
		putchar('\n');

		for (int col = 0; col < grid->ncols; ++col) {
			const struct maze_cell *cell = maze_cell_at(grid, row, col);

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
			const struct maze_cell *cell = maze_cell_at(grid, row, col);
			if (cell->edges.south)
				printf(" | ");
			else
				printf("   ");
		}
		putchar('\n');
	}
}


typedef struct {
	int width;
	int height;
	unsigned  seed;
} MazeOptions;


//unsigned parse_uint_option(int argi, int argc, char *argv[], const char *name)
unsigned parse_uint_option(int argi, str_span args, const char *name)
{
	if (argi >= SPAN_LENGTH(args)) {
		fprintf(stderr, "ERROR: missing value for argument %s\n", name);
		exit(0);
	}

	unsigned long n = strtoul(args.front[argi], NULL, 0);

	if (errno) {
		fprintf(stderr, "ERROR: argument %s: %s\n", name, strerror(errno));
		exit(0);
	}

	if (n == 0) {
		fprintf(stderr, "ERROR: %s must be a number greater than zero.\n", name);
		exit(0);
	} 

	if (n > (unsigned long)UINT_MAX) {
		fprintf(stderr, "ERROR: %s must be less than or equal to %u.\n", name, UINT_MAX);
		exit(0);
	}

	return n; 
}

void MazeOptions_read(MazeOptions *options, int argc, char *argv[])
{
	str_span args = SPAN_INIT_N(argv, argc);

	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-size"))
			options->height = options->width = parse_uint_option(++i, args, "-size");
		else if (!strcmp(argv[i], "-width"))
			options->width = parse_uint_option(++i, args, "-width");
		else if (!strcmp(argv[i], "-height"))
			options->height = parse_uint_option(++i, args, "-height");
		else if (!strcmp(argv[i], "-seed")) 
			options->seed = parse_uint_option(++i, args, "-seed");
		else {
			fprintf(stderr, "ERROR: unknown argument %s\n", argv[i]);
			exit(0);
		}
	}
}

int main(int argc, char *argv[])
{
	MazeOptions options = {
		.width = 8,
		.height = 8,
		.seed = 123456789
	};

	MazeOptions_read(&options, argc, argv);

	srand(options.seed);

	struct maze_grid *grid = maze_grid_create(options.height, options.width);

	struct maze_cell *cell = &grid->cells[1];  // skip top-left cell
	struct maze_cell *end  = grid->cells + (grid->ncols * grid->nrows);
	while (cell != end) {

		enum cell_dirs choices[2];
		int n = 0;

		if (cell->pos.row > 0)  choices[n++] = CELL_NORTH;
		if (cell->pos.col > 0)  choices[n++] = CELL_WEST;

		if (choices[rand()%n] == CELL_NORTH) {
			struct maze_cell *link = maze_cell_at(grid, cell->pos.row-1, cell->pos.col);
			cell->edges.north = link;
			link->edges.south = cell;
		}
		else {
			struct maze_cell *link = maze_cell_at(grid, cell->pos.row, cell->pos.col-1);
			cell->edges.west = link;
			link->edges.east = cell;
		}
		++cell;
	}

	maze_grid_draw_ascii(grid);

	free(grid);
	return 0;
}
 

