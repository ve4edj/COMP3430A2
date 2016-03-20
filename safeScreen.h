#ifndef _SAFESCREEN_H
#define _SAFESCREEN_H

void safe_update_screen();
void safe_set_screen_char(int col, int row, char ch);
char safe_get_screen_char(int col, int row);
void safe_blink_screen(char *charset);
int safe_find_target(char ch, int *col, int *row);
int safe_move_to_target(int col, int row, int *to_col, int *to_row);

#endif