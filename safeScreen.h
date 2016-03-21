#ifndef _SAFESCREEN_H
#define _SAFESCREEN_H

void safe_update_screen();
char safe_get_screen_char(int multipart, int col, int row);
void safe_set_screen_char(int multipart, int col, int row, char ch);
void safe_blink_screen(char *charset);
int safe_find_target(int multipart, char ch, int *col, int *row);
int safe_move_to_target(int multipart, int col, int row, int *to_col, int *to_row);

#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif