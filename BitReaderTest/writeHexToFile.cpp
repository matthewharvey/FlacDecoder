#include <cstdint>
#include <fstream>
#include <cassert>
#include <iostream>
using namespace std;

uint8_t charToInt(char a)
{
    if (a == '0')
        return 0;
    else if (a == '1')
        return 1;
    else if (a == '2')
        return 2;
    else if (a == '3')
        return 3;
    else if (a == '4')
        return 4;
    else if (a == '5')
        return 5;
    else if (a == '6')
        return 6;
    else if (a == '7')
        return 7;
    else if (a == '8')
        return 8;
    else if (a == '9')
        return 9;
    else if (a == 'A')
        return 10;
    else if (a == 'B')
        return 11;
    else if (a == 'C')
        return 12;
    else if (a == 'D')
        return 13;
    else if (a == 'E')
        return 14;
    else if (a == 'F')
        return 15;
    else
    {
        cerr << "Found a " << (int)a << " character which is incorrect" << endl;
        assert(false);
    }
}

int main(int argc, char** argv)
{
    uint8_t cur;
    assert(argc == 2);
    ofstream outfile;
    outfile.open(argv[1], ios::binary | ios::out);
    while ((cin.eof() == false) && (cin.peek() != -1))
    {
        uint8_t high = charToInt((char)cin.get());
        uint8_t low = charToInt((char)cin.get());
        cur = high << 4 | low;
        outfile.write((char*)&cur, 1);
    }
    outfile.close();
    return 0;
}
