#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "krclib.h"


typedef struct grid_cell
{
	int row, col;
} rowcol;

struct grid
{
	int nrows, ncols;
};

struct grid *grid_create(size_t head_size, size_t elem_size, int nrows, int ncols, struct except_frame *xf)
{
	size_t num_cells = try_size_mult((size_t)nrows, (size_t)ncols, xf, CURRENT_LOCATION);
	struct grid *grid = fam_alloc(head_size, elem_size, num_cells, xf);
	grid->nrows = nrows;
	grid->ncols = ncols;
	return grid;
}

#define GRID_CREATE(GridType_, Rows_, Cols_, Xf_)  \
	(GridType_*)grid_create(sizeof(GridType_), sizeof(*(GridType_){}.cells), (Rows_), (Cols_), (Xf_))

int grid_num_cells(struct grid *grid)
{
	return grid->nrows * grid->ncols;
}

#define GRID_END(Grid_)  ((Grid_)->cells + grid_num_cells((struct grid*)(Grid_)))

int grid_index(struct grid *grid, int row, int col)
{
	CHECK(row, grid->nrows);
	CHECK(col, grid->ncols);
	return (row * grid->ncols) + col;
}

int grid_cell_index(struct grid *grid, struct grid_cell cell)
{
	return grid_index(grid, cell.row, cell.col);
//	CHECK(cell.row, grid->nrows);
//	CHECK(cell.col, grid->ncols);
//	return (cell.row * grid->ncols) + cell.col;
}

#define GRID_CELL_AT(Gridptr_, Cell_)  ((Gridptr_)->cells[ grid_cell_index( (struct grid*)(Gridptr_), (Cell_)) ])

#define GRID_AT(Gridptr_, Row_, Col_)  ((Gridptr_)->cells[ grid_index( (struct grid*)(Gridptr_), (Row_), (Col_)) ])





struct maze_cell 
{
	struct grid_cell pos;
	struct maze_cell *north,
					 *south,
					 *east,
					 *west;
};

void maze_cell_link(struct maze_cell *a, struct maze_cell *b)
{
	if      (a->pos.row > b->pos.row)  a->north = b, b->south = a;
	else if (a->pos.row < b->pos.row)  a->south = b, b->north = a;
	else if (a->pos.col > b->pos.col)  a->west  = b, b->east  = a;
	else if (a->pos.col < b->pos.col)  a->east  = b, b->west  = a;
}

struct maze_grid
{
	struct grid grid;
	struct maze_cell cells[];
};

struct maze_grid *maze_create(int height, int width, struct except_frame *xf)
{
	struct maze_grid *maze = GRID_CREATE(struct maze_grid, height, width, xf);
	for (struct grid_cell pos = {0,0};  pos.row < maze->grid.nrows;  ++pos.row)
		for (pos.col = 0; pos.col < maze->grid.ncols; ++pos.col)
			GRID_CELL_AT(maze, pos) = (struct maze_cell){ .pos = pos };
	return maze;
}



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


void maze_grid_draw_ascii(struct maze_grid *grid)
{
	struct grid_cell p;
	for (p.row = 0; p.row < grid->grid.nrows; ++p.row) 
	{
		for (p.col = 0; p.col < grid->grid.ncols; ++p.col) 
		{
			if (GRID_CELL_AT(grid, p).north)
				printf(" | ");
			else
				printf("   ");
		}
		putchar('\n');

		for (p.col = 0; p.col < grid->grid.ncols; ++p.col) 
		{
			const struct maze_cell *cell = &GRID_CELL_AT(grid, p);

			if (cell->west)
				putchar('-');
			else
				putchar(' ');

			putchar('+');

			if (cell->east)
				putchar('-');
			else
				putchar(' ');
		}
		putchar('\n');

		for (p.col = 0; p.col < grid->grid.ncols; ++p.col) 
		{
			const struct maze_cell *cell = &GRID_CELL_AT(grid, p);
			if (cell->south)
				printf(" | ");
			else
				printf("   ");
		}
		putchar('\n');
	}
}


typedef struct
{
	int width;
	int height;
	unsigned  seed;
} MazeOptions;


unsigned parse_uint_option(int argi, int argc, char *argv[], const char *name)
{
	if (argi >= argc) {
		fprintf(stderr, "ERROR: missing value for argument %s\n", name);
		exit(0);
	}

	unsigned long n = strtoul(argv[argi], NULL, 0);

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
	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-size"))
			options->height = options->width = parse_uint_option(++i, argc, argv, "-size");
		else if (!strcmp(argv[i], "-width"))
			options->width = parse_uint_option(++i, argc, argv, "-width");
		else if (!strcmp(argv[i], "-height"))
			options->height = parse_uint_option(++i, argc, argv, "-height");
		else if (!strcmp(argv[i], "-seed")) 
			options->seed = parse_uint_option(++i, argc, argv, "-seed");
		else {
			fprintf(stderr, "ERROR: unknown argument %s\n", argv[i]);
			exit(0);
		}
	}
}


int main(int argc, char *argv[])
{
	struct except_frame xf = {0};

	MazeOptions options = {
		.width = 8,
		.height = 8,
		.seed = 123456789
	};

	MazeOptions_read(&options, argc, argv);

	srand(options.seed);

	struct maze_grid *maze = NULL;
	if (!setjmp(xf.env))
	{
		maze = maze_create(options.height, options.width, &xf);
	}
	else
	{
		struct error *err = xf.error;
		error_fprint(stderr, &(struct error){ .status=STATUS_EXCEPTION, .source=CURRENT_LOCATION });
		error_fatal(err, "");
	}


	struct maze_cell *end = GRID_END(maze);
	for (struct maze_cell *cell = &maze->cells[1]; cell != end; ++cell)
	{
		int n = 0;
		struct maze_cell *neighbors[2];
		if (cell->pos.row > 0)  neighbors[n++] = &GRID_AT(maze, cell->pos.row-1, cell->pos.col);
		if (cell->pos.col > 0)  neighbors[n++] = &GRID_AT(maze, cell->pos.row,   cell->pos.col-1);
		maze_cell_link(cell, neighbors[rand()%n]);
	}

	maze_grid_draw_ascii(maze);

	free(maze);
	except_dispose(&xf);
	return 0;
}

