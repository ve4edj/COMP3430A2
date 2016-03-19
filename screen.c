#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "screen.h"

#define TRUE 1
#define FALSE 0

#define NUM_COLOURS 8

typedef struct {
	char ch;
	short colour;
} ScreenChar;

typedef struct {
	int row, col;
} Pos;

static int curses_initialized = FALSE;
static ScreenChar screen[SCREEN_WIDTH][SCREEN_HEIGHT];

static int screen_initialized = FALSE;
#define QSIZE (SCREEN_WIDTH * 2 + SCREEN_HEIGHT * 2)
static Pos path_q[QSIZE], new_path_q[QSIZE];
static int path_q_size, new_path_q_size;

const int NUM_DIRS = 8;
const int DIRS[][2] = { {1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1},{0,1} };

static int on_screen(int col, int row);
static int may_movethrough(char ch);

void initialize_screen() {
	assert(!curses_initialized);

	initscr();
	start_color();
	for (int i = 0; i < NUM_COLOURS; i++) {
		init_pair(i, i, COLOR_BLACK);
	}
	
	curses_initialized = TRUE;
}

void update_screen() {
	assert(curses_initialized && screen_initialized);
	for (int row = 0; row < SCREEN_HEIGHT; row++) {
		for (int col = 0; col < SCREEN_WIDTH; col++) {
			attron(COLOR_PAIR(screen[col][row].colour));
			mvaddch(row, col, screen[col][row].ch);
		}
	}
	refresh();
}

int load_screen(char *path) {
	assert(NULL != path);
	
	FILE *in;
	int result = -1;
	int input, newl;
	int row, col, line;
	char default_ch;
	short default_colour;

	errno = 0;
	
	in = fopen(path, "r");
	if (in) {
		input = fgetc(in);
		if (EOF != input) {
			default_ch = (char)input;
			input = fgetc(in);
			if (EOF != input) {
				default_colour = ((char)input - '0');
				if (default_colour >= 0 && default_colour < NUM_COLOURS) {
					row = 0;
					col = 0;

					for (row = 0; row < SCREEN_HEIGHT; row++) {
						for (col = 0; col < SCREEN_WIDTH; col++) {
							screen[col][row].ch = default_ch;
							screen[col][row].colour = default_colour;
						}
					}
				
					newl = FALSE;
					line = 0;

					while (EOF != input && row < SCREEN_HEIGHT * 2) {
						if (!newl) {
							// consume to newline
							do {
								input = fgetc(in);
							} while (EOF != input && '\n' != input);
						}
						newl = FALSE;
					
						// read in a line
						col = 0;
						while (EOF != input && !newl && col < SCREEN_WIDTH) {
							input = fgetc(in);
							if ('\n' == input) {
								newl = TRUE;
								line++;
							} else if (EOF != input) {
								row = line / 2;
								if (line % 2) {
									if (input >= '0' && input < '0' + NUM_COLOURS)
										screen[col][row].colour = input - '0';
								} else {
									screen[col][row].ch = (char)input;
								}
								col++;
							}
						}
					}
					
					screen_initialized = true;
					result = 0;
				}
			}
		}
		fclose(in);
	}

	return result;
}

void set_screen_char(int col, int row, char ch) {
	assert(screen_initialized);
	assert(col >= 0 && col < SCREEN_WIDTH && row >= 0 && row < SCREEN_HEIGHT);
	screen[col][row].ch = ch;
}

char get_screen_char(int col, int row) {
	assert(screen_initialized);
	assert(col >= 0 && col < SCREEN_WIDTH && row >= 0 && row < SCREEN_HEIGHT);
	return screen[col][row].ch;
}

void blink_screen(char *charset) {
	assert(curses_initialized && screen_initialized);
	assert(NULL != charset);

	char ch;
	
	while (*charset) {
		ch = *charset;
		
		for (int row = 0; row < SCREEN_HEIGHT; row++) {
			for (int col = 0; col < SCREEN_WIDTH; col++) {
				if (ch == screen[col][row].ch) {
					screen[col][row].colour = (screen[col][row].colour + 1) % NUM_COLOURS;
				}
			}
		}
		
		charset++;
	}
}

void finish_screen() {
	assert(curses_initialized);
	endwin();
}

static int on_screen(int col, int row) {
	assert(screen_initialized);
	return col >= 0 && col < SCREEN_WIDTH && row >= 0 && row < SCREEN_HEIGHT;
}

static int may_movethrough(char ch) {
	return SPACE == ch || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

int find_target(char ch, int *col, int *row) {
	assert(screen_initialized);
	assert(NULL != col && NULL != row);
	int euclid2, distance = -1;

	// do we have an origin?
	int origin_c = *col;
	int origin_r = *row;

	// default result
	*col = 0;
	*row = 0;

	// find the (nearest) instance of ch with (non-diagonal) space or moveable chars around it
	// this could be much more efficient!
	for (int r = 0; r < SCREEN_HEIGHT; r++) {
		for (int c = 0; c < SCREEN_WIDTH; c++) {

			euclid2 = (origin_c - c) * (origin_c - c) + (origin_r - r) * (origin_r - r);

			if (ch == screen[c][r].ch) {
				int has_exit = FALSE;
				
				for (int dir = 1; dir < NUM_DIRS && !has_exit; dir += 2) {
					char ch = screen[c + DIRS[dir][0]][r + DIRS[dir][1]].ch;
					if (may_movethrough(ch)) {
						if (distance < 0 || euclid2 < distance) {
							*col = c;
							*row = r;
							distance = euclid2;
						}
					}
				}
			}
		}
	}

	return distance < 0 ? FALSE : TRUE;
}

int move_to_target(int col, int row, int *to_col, int *to_row) {
	int target_c, target_r;
	
	assert(screen_initialized);
	assert(col >= 0 && col < SCREEN_WIDTH && row >= 0 && row < SCREEN_HEIGHT);
	assert(NULL != to_col && NULL != to_row);
	
	char ch = screen[col][row].ch;
	assert(SPACE != ch);
	
	target_c = *to_col;
	target_r = *to_row;
	assert(target_c >= 0 && target_r >= 0);

	// default: go nowhere
	*to_col = col;
	*to_row = row;

	// bfs
	int r, c;
	int scores[SCREEN_WIDTH][SCREEN_HEIGHT];
	for (r = 0; r < SCREEN_HEIGHT; r++) {
		for (c = 0; c < SCREEN_WIDTH; c++) {
			if (may_movethrough(screen[c][r].ch)) {
				scores[c][r] = SCREEN_WIDTH * SCREEN_HEIGHT; // max
			} else {
				scores[c][r] = -1;
			}
		}
	}
	
	int dist;
	int hit = FALSE;
	
	path_q[0].row = target_r;
	path_q[0].col = target_c;
	scores[target_c][target_r] = 0;
	path_q_size = 1;

	for (dist = 1; dist < QSIZE && !hit; dist++) {
		new_path_q_size = 0;
		for (int qpos = 0; qpos < path_q_size; qpos++) {
			for (int dir = 0; dir < NUM_DIRS; dir++) {
				int try_c = path_q[qpos].col + DIRS[dir][0];
				int try_r = path_q[qpos].row + DIRS[dir][1];
				if (try_c == col && try_r == row)
					hit = TRUE;
				else if (on_screen(try_c, try_r) && scores[try_c][try_r] > dist) {
					scores[try_c][try_r] = dist;
					new_path_q[new_path_q_size].row = try_r;
					new_path_q[new_path_q_size].col = try_c;
					new_path_q_size++;
				}
			}
		}
		memcpy(path_q, new_path_q, new_path_q_size * sizeof(Pos));
		path_q_size = new_path_q_size;
	}

	dist = QSIZE;
	int euclid2, max_euclid2 = 2 * QSIZE * QSIZE;
	int was_level = FALSE, is_level;
	for (int dir = 0; dir < NUM_DIRS; dir++) {
		int try_c = col + DIRS[dir][0];
		int try_r = row + DIRS[dir][1];
		if (on_screen(try_c, try_r)) {
			if (0 == scores[try_c][try_r])
				dist = 0;
			else if (SPACE == screen[try_c][try_r].ch && scores[try_c][try_r] <= dist) {
				// prefer non-diagonal, followed by shortest straight-line
				is_level = 0 == DIRS[dir][0] || 0 == DIRS[dir][1];
				euclid2 = (target_c - try_c) * (target_c - try_c) + (target_r - try_r) * (target_r - try_r);
				if (scores[try_c][try_r] < dist || (is_level && (!was_level || euclid2 < max_euclid2)) || (!was_level && euclid2 < max_euclid2)) {
					was_level = is_level;
					*to_col = try_c;
					*to_row = try_r;
					max_euclid2 = euclid2;
				}
				dist = scores[try_c][try_r];
			}
		}
	}

#ifdef DEBUG_PATHFINDING
	for (int row = 0; row < SCREEN_HEIGHT; row++) {
		for (int col = 0; col < SCREEN_WIDTH; col++) {
			if (scores[col][row] < 0) {
				attron(COLOR_PAIR(7));
				mvaddch(row, col, screen[col][row].ch);
			} else if (scores[col][row] > QSIZE) {
				mvaddch(row, col, ' ');
			} else {
				attron(COLOR_PAIR(4));
				mvaddch(row, col, 'A' + scores[col][row]);
			}
		}
	}
	refresh();
	getch();
	update_screen();
#endif

	return dist == 0;
}

#ifdef TEST_SCREEN
int main(int argc, char *argv[]) {
	char ch;
	Pos letters[26];
	Pos *current;
	int new_col, new_row;
	char targets[2*MAX_RIDES + 1] = "";
	const int CHARDIFF = '0' - '!';
	
	if (argc != 2) {
		fprintf(stderr, "Usage: %s mapfile.txt\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	initialize_screen();
	if (load_screen(argv[1]) < 0) {
		perror("Unable to load map file");
		finish_screen();
		return EXIT_FAILURE;
	}

	for (int i = 0; i < 26; i++) {
		do {
			letters[i].col = random() % SCREEN_WIDTH;
			letters[i].row = random() % SCREEN_HEIGHT;
		} while (SPACE != get_screen_char(letters[i].col, letters[i].row));
		set_screen_char(letters[i].col, letters[i].row, 'a' + i);
	}
	update_screen();
	
	while ((ch = getch()) != '`') {
		if (ch >= 'a' && ch <= 'z') {
			current = &letters[ch-'a'];
			if (current->col >= 0) {
				char target = '0' + ch % MAX_RIDES;
				new_col = current->col;
				new_row = current->row;
				find_target(target, &new_col, &new_row);
				if (move_to_target(current->col, current->row, &new_col, &new_row)) {
					set_screen_char(current->col, current->row, ' ');
					current->col = -1;
					current->row = -1;
					int oldlen = strlen(targets);
					targets[oldlen] = target - CHARDIFF;
					targets[oldlen+1] = target;
					targets[oldlen+2] = '\0';
				} else {
					set_screen_char(current->col, current->row, ' ');
					set_screen_char(new_col, new_row, ch);
					current->col = new_col;
					current->row = new_row;
				}
			}
		}
		blink_screen(targets);
		update_screen();
	}
	
	finish_screen();
	
	return 0;
}
#endif