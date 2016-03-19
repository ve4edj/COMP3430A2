#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 24
#define MAX_RIDES 10
#define SPACE ' '

void initialize_screen();
void update_screen();
int load_screen(char *path);
void set_screen_char(int col, int row, char ch);
char get_screen_char(int col, int row);
void blink_screen(char *charset);
void finish_screen();

int find_target(char ch, int *col, int *row);
int move_to_target(int col, int row, int *to_col, int *to_row);
