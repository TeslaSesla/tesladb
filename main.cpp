#include <iostream>
#include <string>
#include <string.h>
#include <asm/boot.h>

#include "database.h"

using namespace std;

int main()
{
    database db;

    int ans = 0;
    int tempint = 0;
    string temp, temp1, temp2;

    do
    {
        cout << "1) Exit"           << endl;
        cout << "2) Create DB"      << endl;
        cout << "3) Create table"   << endl;
        cout << "4) Select DB"      << endl;
        cout << "5) Add entry"      << endl;
        cout << "6) Delete entry"   << endl;
        cout << "7) Search row"     << endl;
        getline(cin, temp);
        ans = stoi(temp);

        switch (ans)
        {
            case 1:
                cout << "Bye" << endl;
                break;
            case 2:
                cout << "Enter DB name: ";
                getline(cin, temp);
                cout << db.createDB(temp) << endl;
                break;
            case 3:
                cout << "Enter table name: ";
                getline(cin, temp);
                cout << "Enter table structure: ";
                getline(cin, temp1);
                cout << "Enter table types: ";
                getline(cin, temp2);
                cout << db.createTable(temp, temp1, temp2) << endl;
                break;
            case 4:
                cout << "Enter DB name: ";
                getline(cin, temp);
                cout << db.selectDB(temp) << endl;
                break;
            case 5:
                cout << "Enter table name: ";
                getline(cin, temp);
                cout << "Enter data: ";
                getline(cin, temp1);
                cout << db.addEntry(temp, temp1) << endl;
                break;
            case 6:
                cout << "Enter table name: ";
                getline(cin, temp);
                cout << "Index where search data: ";
                getline(cin, temp1);
                cout << "Data: ";
                getline(cin, temp2);
                cout << db.delEntry(temp, stoi(temp1), temp2) << endl;
                break;
            case 7:
                cout << "Enter table name: ";
                getline(cin, temp);
                db.searchLineInTableByRow(temp, 1, "15", temp1, temp2);
                cout << "Line: "  << temp1 << endl;
                cout << "Types: " << temp2 << endl;
                break;
        }
    }
    while(ans != 1);


    return 0;
}
