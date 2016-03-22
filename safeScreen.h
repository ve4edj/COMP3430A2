/*
		Course:			COMP3430
		Assignment:		2 - Amusement Park Simulator
		Due date:		Tuesday, March 22 2016
		Instructor:		John Braico
		Programmed By:	Erik Johnson
		Student #:		7711697
*/

#ifndef _SAFESCREEN_H
#define _SAFESCREEN_H

void safe_update_screen();
char safe_get_screen_char(int multipart, int col, int row);
void safe_set_screen_char(int multipart, int col, int row, char ch);
void safe_blink_screen(char *charset);
int safe_find_target(int multipart, char ch, int *col, int *row);
int safe_move_to_target(int multipart, int col, int row, int *to_col, int *to_row);
void lockScreen();
void unlockScreen();

#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif