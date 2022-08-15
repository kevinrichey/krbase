#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "krclib.h"


struct grid_cell
{
	int row, col;
};

struct grid
{
	int nrows, ncols;
	size_t cell_size;
};

struct grid *grid_create(size_t head_size, size_t elem_size, int nrows, int ncols, struct except_frame *xf)
{
	int num_cells = try_int_mult(nrows, ncols, xf, CURRENT_LOCATION);
	struct grid *grid = fam_alloc(head_size, elem_size, num_cells, xf);
	grid->nrows = nrows;
	grid->ncols = ncols;
	grid->cell_size = elem_size;
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
	row = CHECK(row, grid->nrows);
	col = CHECK(col, grid->ncols);
	return (row * grid->ncols) + col;
}

#define GRID_AT(Gridptr_, Row_, Col_)  ((Gridptr_)->cells[ grid_index( (struct grid*)(Gridptr_), (Row_), (Col_)) ])

int grid_cell_index(struct grid *grid, struct grid_cell cell)
{
	return grid_index(grid, cell.row, cell.col);
}

#define GRID_CELL_AT(Gridptr_, Cell_)  ((Gridptr_)->cells[ grid_cell_index( (struct grid*)(Gridptr_), (Cell_)) ])


struct grid_row
{
	void *front, *back;
	int row_size;
	int row_num;
};

struct grid_row row_start(struct grid *grid, void *cells)
{
	return 
		(struct grid_row)
		{
			.back = cells,
			.row_size = grid->cell_size * grid->ncols,
			.row_num = 0
		};
}

bool row_next(struct grid *grid, struct grid_row *row)
{
	if (row->row_num++ < grid->nrows)
	{
		row->front = row->back;
		(byte*)row->back += row->row_size;
		return true;
	}
	else return false;
}




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

struct maze
{
	struct grid grid;
	struct maze_cell cells[];
};

struct maze *maze_create(int height, int width, struct except_frame *xf)
{
	struct maze *maze = GRID_CREATE(struct maze, height, width, xf);
	for (struct grid_cell pos = {0,0};  pos.row < maze->grid.nrows;  ++pos.row)
		for (pos.col = 0; pos.col < maze->grid.ncols; ++pos.col)
			GRID_CELL_AT(maze, pos) = (struct maze_cell){ .pos = pos };
	return maze;
}


void maze_draw_ascii(struct maze *maze)
{
	struct grid_row row = row_start(&maze->grid, maze->cells);

	while (row_next(&maze->grid, &row))
	{
		for (struct maze_cell *cell = row.front;
		     cell != row.back; ++cell)
		{
			if (cell->north)
				printf(" | ");
			else
				printf("   ");
		}
		putchar('\n');

		for (struct maze_cell *cell = row.front;
		     cell != row.back; ++cell)
		{
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

		for (struct maze_cell *cell = row.front;
		     cell != row.back; ++cell)
		{
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

	struct maze *maze = NULL;
	if (!setjmp(xf.env))
	{
		maze = maze_create(options.height, options.width, &xf);
	}
	else
	{
		struct error *err = xf.error;
		error_fprint(stderr, &(struct error){ .status=STATUS_EXCEPTION, .source=CURRENT_LOCATION });
		error_fatal(err);
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

	maze_draw_ascii(maze);

	free(maze);
	except_dispose(&xf);
	return 0;
}

