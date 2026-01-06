#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <csignal>
#include <curses.h>

using namespace std;

const int KEY_ESCAPE = 27;
const int KEY_TAB = 9;
const int KEY_BACK = 127;
string BASE_COMMAND = ":";

class state_node {
    public:
        int start;
        int length;
        list<string> old_text;
};

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
        if (i + top_line == cursor_y) {
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

void window_update(int& cursor_y, int& cursor_x, int& top_line, int& left_col) {
    int w, h;
    getmaxyx(stdscr, h, w);
    h -= 3;
    w -= 1;
    top_line = max(min(top_line, cursor_y), cursor_y - h);
    left_col = max(min(left_col, cursor_x), cursor_x - h);
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

bool handle_command(vector<string>& full_buff, int& cursor_y, int& cursor_x, 
        int& col_mem, int& next, bool& write, bool& insert, state_node& undo_block, vector<state_node>& undo_stack) {
    switch (next) {
        case ':':
            output_command(BASE_COMMAND);
            return handle_exit_route(write);
        case 'i':
            insert = true;
            undo_block.start = cursor_y;
            undo_block.length = -1;
            undo_block.old_text.push_back(full_buff[cursor_y]);
            break;
        case '0':
            cursor_x = 0;
            break;
        case '$':
            cursor_x = full_buff[cursor_y].size();
            break;
        case 'u':
            if (undo_stack.size() > 0) {
                state_node prev_block = undo_stack.back();
                undo_stack.pop_back();
                full_buff.erase(full_buff.begin() + prev_block.start, full_buff.begin() + prev_block.start + prev_block.length);
                int size = prev_block.old_text.size();
                for (int i = 0; i < size; ++i) {
                    full_buff.insert(full_buff.begin() + prev_block.start + i, prev_block.old_text.front());
                    prev_block.old_text.pop_front();
                }
                cursor_x = 0;
                cursor_y = prev_block.start;
            }
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

bool handle_insert(vector<string>& full_buff, int& cursor_y, int& cursor_x, int& col_mem, 
        int& next, bool& write, bool& insert, state_node& undo_block, vector<state_node>& undo_stack) {
    string to_keep;
    string to_shift;
    switch (next) {
        case '\n':
            if (undo_block.length < 0) {
                undo_block.length = 1;
            }
            if (cursor_y < undo_block.start) {
                int diff = undo_block.start - cursor_y;
                for (int i = undo_block.start - 1; i >= undo_block.start - diff; --i) {
                    undo_block.old_text.push_front(full_buff[i]);
                }
                undo_block.start = cursor_y;
                undo_block.length += diff + 1;
            } else if (cursor_y >= undo_block.start + undo_block.length) {
                int diff = 1 + cursor_y - undo_block.start - undo_block.length;
                for (int i = 0; i < diff; ++i) {
                    int idx = undo_block.start + undo_block.length + i;
                    undo_block.old_text.push_back(full_buff[idx]);
                }
                undo_block.length += diff + 1;
            } else if (cursor_y >= undo_block.start && cursor_y < undo_block.start + undo_block.length) {
                undo_block.length += 1;
            } 
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
            if (undo_block.length < 0) {
                undo_block.length = 1;
            }
            if (cursor_x == 0) {
                if (cursor_y > 0) {
                    if (cursor_y < undo_block.start) {
                        int diff = undo_block.start - cursor_y;
                        for (int i = undo_block.start - 1; i >= undo_block.start - diff - 1; --i) {
                            undo_block.old_text.push_front(full_buff[i]);
                        }
                        undo_block.start = cursor_y - 1;
                        undo_block.length += diff;
                    } else if (cursor_y >= undo_block.start + undo_block.length) {
                        int diff = 1 + cursor_y - undo_block.start - undo_block.length;
                        for (int i = 0; i < diff; ++i) {
                            int idx = undo_block.start + undo_block.length + i;
                            undo_block.old_text.push_back(full_buff[idx]);
                        }
                        undo_block.length += diff - 1;
                    } else {
                        undo_block.old_text.push_front(full_buff[cursor_y - 1]);
                        undo_block.start -= 1;
                    }
                    --cursor_y;
                    cursor_x = full_buff[cursor_y].size();
                    full_buff[cursor_y] += full_buff[cursor_y + 1];
                    full_buff.erase(full_buff.begin() + cursor_y + 1);
                }
            } else {
                if (cursor_y < undo_block.start) {
                    int diff = undo_block.start - cursor_y;
                    for (int i = undo_block.start - 1; i >= undo_block.start - diff; --i) {
                        undo_block.old_text.push_front(full_buff[i]);
                    }
                    undo_block.start = cursor_y;
                    undo_block.length += diff;
                } else if (cursor_y >= undo_block.start + undo_block.length) {
                    int diff = cursor_y - undo_block.start - undo_block.length + 1;
                    for (int i = 0; i < diff; ++i) {
                        int idx = undo_block.start + undo_block.length + i;
                        undo_block.old_text.push_back(full_buff[idx]);
                    }
                    undo_block.length = cursor_y - undo_block.start + 1;
                }
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
            if (isprint(next)) {
                if (undo_block.length < 0) {
                    undo_block.length = 1;
                }
                if (cursor_y < undo_block.start) {
                    int diff = undo_block.start - cursor_y;
                    for (int i = undo_block.start - 1; i >= undo_block.start - diff; ++i) {
                        undo_block.old_text.push_front(full_buff[i]);
                    }
                    undo_block.start = cursor_y;
                    undo_block.length += diff;
                } else if (cursor_y >= undo_block.start + undo_block.length) {
                    int diff = cursor_y - undo_block.start - undo_block.length + 1;
                    for (int i = 0; i < diff; ++i) {
                        int idx = undo_block.start + undo_block.length + i;
                        undo_block.old_text.push_back(full_buff[idx]);
                    }
                    undo_block.length = cursor_y - undo_block.start + 1;
                }
                full_buff[cursor_y].insert(cursor_x, 1, next);
                ++cursor_x;
            }
    }
    return true;
}

bool handle_input(vector<string>& full_buff, int& cursor_y, int& cursor_x, int& col_mem, 
        int& next, bool& write, bool& insert, state_node& undo_block, vector<state_node>& undo_stack) {
    if (next == KEY_ESCAPE) {
        insert = false;
        state_node new_block = undo_block;
        if (new_block.length >= 0) {
            undo_stack.push_back(new_block);
        }
        undo_block.old_text.clear();
        return true;
    }
    if (insert) {
        return handle_insert(full_buff, cursor_y, cursor_x, col_mem, next, write, insert, undo_block, undo_stack);
    } else {
        return handle_command(full_buff, cursor_y, cursor_x, col_mem, next, write, insert, undo_block, undo_stack);
    }
}

int main(int argc, char* argv[]) {
    vector<string> full_buff;
    vector<state_node> undo_stack;
    state_node undo_block;
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
        cont = handle_input(full_buff, cursor_y, cursor_x, col_mem, next, write, insert, undo_block, undo_stack);
        window_update(cursor_y, cursor_x, top_line, left_col);
        if (write) {
            ofstream f_out(f_name);
            if (!f_out.fail()) {
                for (vector<string>::iterator it = full_buff.begin(); it != full_buff.end(); ++it) {
                    f_out << *it << "\n";
                }
            } else {
                cout << "Cannot write to file";
            }
            f_out.close();
            write = false;
        }
    }
    endwin();
    return 0;
}