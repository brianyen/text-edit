#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <csignal>
#include <curses.h>

using namespace std;

const int KEY_ESCAPE = 27;
const int KEY_TAB = 9;
const int KEY_BACK = 127;
string BASE_COMMAND = ":";

bool invalid_write(string& f_name) {
    ofstream f(f_name, ios::app);
    if (f.fail()) {
        cout << "File cannot be written to.";
        return true;
    }
    return false;
}

void output(vector<string>& full_buff, int& top_line, int& left_col, 
        int& cursor_y, int& cursor_x) {
    int w, h;
    getmaxyx(stdscr, h, w);
    h -= 2;
    int num_lines = min(h, (int) full_buff.size() - top_line);

    clear();
    for (int i = 0; i < num_lines; i++) {
        if (full_buff[top_line + i].size() >= left_col) {
            mvprintw(i, 0, full_buff[top_line + i].substr(left_col, w).c_str());
        }
        if (i == cursor_y) {
            mvprintw(i, cursor_x - left_col, "_");
        }
    }
    refresh();
}

void output_command(string& command) {
    int w, h;
    getmaxyx(stdscr, h, w);
    move(--h, 0);
    clrtoeol();
    mvprintw(h, 0, command.c_str());
    refresh();
}

// eventually add dirty flag for override
bool handle_exit_route(bool& write) {
    int next = getch();
    string command = ":";
    while (next != KEY_ENTER && next != '\n' && next != '\r') {
        if (next < KEY_MIN && isprint(next)) {
            command += (char) next;
        }
        output_command(command);
        next = getch();
    }

    if (command == ":wq") {
        write = true;
        return false;
    } else if (command == ":q") {
        return false;
    } else if (command == ":w") {
        write = true;
        return true;
    } else {
        return true;
    }
}

void move_up(vector<string>& full_buff, int& cursor_y, int& cursor_x, int& col_mem) {
    if (cursor_y > 0) { 
        --cursor_y;
        cursor_x = min((int) full_buff[cursor_y].size(), max(col_mem, cursor_x));
    } else {
        cursor_x = 0;
        col_mem = cursor_x;
    }
}

void move_down(vector<string>& full_buff, int& cursor_y, int& cursor_x, int& col_mem) {
    if (cursor_y < full_buff.size() - 1) {
        ++cursor_y;
        cursor_x = min((int) full_buff[cursor_y].size(), max(col_mem, cursor_x));
    } else {
        cursor_x = full_buff[cursor_y].size();
        col_mem = cursor_x;
    }
}

void move_left(vector<string>& full_buff, int& cursor_y, int& cursor_x, int& col_mem) {
    if (cursor_x > 0) {
        --cursor_x;
    } else if (cursor_y > 0) {
        --cursor_y;
        cursor_x = full_buff[cursor_y].size();
    }
    col_mem = cursor_x;
}

void move_right(vector<string>& full_buff, int& cursor_y, int& cursor_x, int& col_mem) {
    if (cursor_x < full_buff[cursor_y].size()) {
        ++cursor_x;
    } else if (cursor_y < full_buff.size() - 1) {
        ++cursor_y;
        cursor_x = 0;
    }
    col_mem = cursor_x;
}

void insert_char(vector<string>& full_buff, int& cursor_y, int& cursor_x, 
        int& col_mem, int& next) {

}

bool handle_command(vector<string>& full_buff, int& cursor_y, int& cursor_x, 
        int& col_mem, int& next, bool& write, bool& insert) {
    switch (next) {
        case ':':
            output_command(BASE_COMMAND);
            return handle_exit_route(write);
        case 'i':
            insert = true;
            break;
        case '0':
            cursor_x = 0;
            break;
        case '$':
            cursor_x = full_buff[cursor_y].size();
            break;
        case 'k':
        case KEY_UP:
            move_up(full_buff, cursor_y, cursor_x, col_mem);
            break;
        case 'j':
        case KEY_DOWN:
            move_down(full_buff, cursor_y, cursor_x, col_mem);
            break;
        case 'h':
        case KEY_LEFT:
            move_left(full_buff, cursor_y, cursor_x, col_mem);
            break;
        case 'l':
        case KEY_RIGHT:
            move_right(full_buff, cursor_y, cursor_x, col_mem);
            break;
    }
    return true;
}

bool handle_insert(vector<string>& full_buff, int& cursor_y, int& cursor_x, 
        int& col_mem, int& next, bool& write, bool& insert) {
    string to_keep;
    string to_shift;
    switch (next) {
        case '\n':
            if (cursor_x == full_buff[cursor_y].size()) {
                full_buff.insert(full_buff.begin() + cursor_y + 1, "");
            } else if (cursor_x == 0) {
                full_buff.insert(full_buff.begin() + cursor_y, "");
            } else {
                to_keep = full_buff[cursor_y].substr(0, cursor_x);
                to_shift = full_buff[cursor_y].substr(cursor_x);
                full_buff[cursor_y] = to_keep;
                full_buff.insert(full_buff.begin() + cursor_y + 1, to_shift);
            }
            ++cursor_y;
            cursor_x = 0;
            col_mem = 0;
            break;
        case KEY_BACK:
            if (cursor_x == 0) {
                if (cursor_y > 0) {
                    --cursor_y;
                    cursor_x = full_buff[cursor_y].size();
                    full_buff[cursor_y] += full_buff[cursor_y + 1];
                    full_buff.erase(full_buff.begin() + cursor_y + 1);
                }
            } else {
                full_buff[cursor_y].erase(cursor_x - 1, 1);
                --cursor_x;
                col_mem = cursor_x;
            }
            break;
        case KEY_UP:
            move_up(full_buff, cursor_y, cursor_x, col_mem);
            break;
        case KEY_DOWN:
            move_down(full_buff, cursor_y, cursor_x, col_mem);
            break;
        case KEY_LEFT:
            move_left(full_buff, cursor_y, cursor_x, col_mem);
            break;
        case KEY_RIGHT:
            move_right(full_buff, cursor_y, cursor_x, col_mem);
            break;
        case KEY_TAB:
            next = '\t';
        default:
            cout << next;
            if (isprint(next)) {
                insert_char(full_buff, cursor_y, cursor_x, col_mem, next);
            }
    }
    return true;
}

bool handle_input(vector<string>& full_buff, int& cursor_y, int& cursor_x, 
        int& col_mem, int& next, bool& write, bool& insert) {
    if (next == KEY_ESCAPE) {
        insert = false;
        return true;
    }
    if (insert) {
        return handle_insert(full_buff, cursor_y, cursor_x, col_mem, next, write, insert);
    } else {
        return handle_command(full_buff, cursor_y, cursor_x, col_mem, next, write, insert);
    }
}

int main(int argc, char* argv[]) {
    vector<string> full_buff;
    int cursor_x, cursor_y;
    int top_line, left_col;
    int col_mem = 0;
    bool insert = false;
    bool write = false;

    if (argc < 2) {
        cout << "Please provide a filename.";
        return 1;
    }
    string f_name = argv[1];
    ifstream f_in(f_name);
    if (f_in.is_open()) {
        string l;
        while (getline(f_in, l)) {
            full_buff.push_back(l);
        }
    }
    f_in.close();

    if (invalid_write(f_name)) return 1;
    
    bool cont = true;    
    int next;
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    while (cont) {
        output(full_buff, top_line, left_col, cursor_y, cursor_x);
        next = getch();
        cont = handle_input(full_buff, cursor_y, cursor_x, col_mem, next, write, insert);
        if (write) {
            // should save to file
            cout << "Should write!\n";
            write = false;
        }
    }
    endwin();
    return 0;
}