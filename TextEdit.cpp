#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <csignal>
#include <curses.h>

using namespace std;

bool invalid_write(string& f_name) {
    ofstream f(f_name, ios::app);
    if (f.fail()) {
        cout << "File cannot be written to.";
        return true;
    }
    return false;
}

void output(vector<string>& full_buff, int& top_line, int& left_line) {
    int w, h;
    getmaxyx(stdscr, w, h);
    int num_lines = min(h, (int) full_buff.size() - top_line);

    clear();
    for (int i = 0; i < num_lines; i++) {
        if (full_buff[top_line + i].size() >= left_line) {
            mvprintw(i, 0, full_buff[top_line + i].substr(left_line, w).c_str());
        }
    }
    refresh();
}

bool handle_input(int next) {
    return false;
}

int main(int argc, char* argv[]) {
    vector<string> full_buff;
    int cursor[2] = {0, 0};
    int top_line, left_col;
    bool insert = false;

    if (argc < 2) {
        cout << "Please provide a filename.";
        return 1;
    }
    string f_name = argv[1];
    ifstream f_in(f_name);
    if (f_in.is_open()) {
        string l;
        while (getline(f_in, l)) {
            cout << l << "\n";
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
        output(full_buff, top_line, left_col);
        next = getch();
        cont = handle_input(next);
    }
    endwin();
    return 0;
}