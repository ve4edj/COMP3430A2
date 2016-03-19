#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "screen.h"

typedef struct {
	char ch;
	short colour;
} ScreenChar;

typedef struct {
	int row, col;
} Pos;

void loadPark(char * parkFile) {
	initialize_screen();
	if (load_screen(parkFile) < 0) {
		perror("Unable to load map file");
		finish_screen();
		exit(EXIT_FAILURE);
	}
}

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
	loadPark(argv[1]);

	for (int i = 0; i < 26; i++) {
		do {
			letters[i].col = random() % SCREEN_WIDTH;
			letters[i].row = 0;
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