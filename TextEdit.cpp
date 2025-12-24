#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <csignal>
#include <curses.h>

using namespace std;

const int KEY_ESCAPE = 27;
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

bool handle_command(vector<string>& full_buff, int& cursor_y, int& cursor_x, 
        int next, bool& write, bool& insert) {
    switch (next) {
        case ':':
            output_command(BASE_COMMAND);
            return handle_exit_route(write);
        case 'j':
            return false;
        case 'i':
            insert = true;
            break;
        case '\n':
        case '\r':
        case KEY_UP:
            if (cursor_y > 0) { 
                --cursor_y;
                cursor_x = min((int) full_buff[cursor_y].size(), cursor_x);
            } else {
                cursor_x = 0;
            }
            break;
        case KEY_DOWN:
            if (cursor_y < full_buff.size() - 1) {
                ++cursor_y;
                cursor_x = min((int) full_buff[cursor_y].size(), cursor_x);
            } else {
                cursor_x = full_buff[cursor_y].size();
            }
            break;
        case KEY_LEFT:
            if (cursor_x > 0) {
                --cursor_x;
            } else if (cursor_y > 0) {
                --cursor_y;
                cursor_x = full_buff[cursor_y].size();
            }
            break;
        case KEY_RIGHT:
            if (cursor_x < full_buff[cursor_y].size()) {
                ++cursor_x;
            } else if (cursor_y < full_buff.size() - 1) {
                ++cursor_y;
                cursor_x = 0;
            }
            break;
    }
    return true;
}

bool handle_insert(vector<string>& full_buff, int& cursor_y, int& cursor_x, 
        int next, bool& write, bool& insert) {
    return true;
}

bool handle_input(vector<string>& full_buff, int& cursor_y, int& cursor_x, 
        int next, bool& write, bool& insert) {
    if (next == KEY_ESCAPE) {
        insert = false;
        return true;
    }
    if (insert) {
        return handle_insert(full_buff, cursor_y, cursor_x, next, write, insert);
    } else {
        return handle_command(full_buff, cursor_y, cursor_x, next, write, insert);
    }
}

int main(int argc, char* argv[]) {
    vector<string> full_buff;
    int cursor[2] = {0, 0};
    int top_line, left_col;
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
        output(full_buff, top_line, left_col, cursor[0], cursor[1]);
        next = getch();
        cont = handle_input(full_buff, cursor[0], cursor[1], next, write, insert);
        if (write) {
            // should save to file
            cout << "Should write!\n";
            write = false;
        }
    }
    endwin();
    return 0;
}