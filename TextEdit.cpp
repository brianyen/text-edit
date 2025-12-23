#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

int main(int argc, char* argv[]) {
    vector<string> full_buff;
    int cursor[2] = {0, 0};
    int top_line = 0;
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
    
    return 0;
}

bool invalid_write(string f_name) {
    ofstream f(f_name, ios::app);
    if (f.file()) {
        cout << "File cannot be written to.";
        return true;
    }
    return false;
}

boolean handle_input(char next) {

}