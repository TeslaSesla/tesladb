#include <iostream>
#include <string>
#include <string.h>

#include "database.h"

using namespace std;

int main()
{
    database db;

    //cout << __func__ << endl;

    int ans = 0;
    int tempint = 0;
    int returnTemp = 0;
    string temp, temp1, temp2, temp3, temp4;

    vector<string> temparr;

    do
    {
        cout << "1)  Exit"              << endl;
        cout << "2)  Create DB"         << endl;
        cout << "3)  Create table"      << endl;
        cout << "4)  Select DB"         << endl;
        cout << "5)  Add entry"         << endl;
        cout << "6)  Delete entry"      << endl;
        cout << "7)  Search row"        << endl;
        cout << "8)  Search row array"  << endl;
        cout << "9)  Delete DB"         << endl;
        cout << "10) Test write time"   << endl;
        cout << "11) Check DB status"   << endl;
        cout << "12) Check table status"<< endl;
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
                returnTemp = db.createDB(temp);

                break;
            case 3:
                cout << "Enter table name: ";
                getline(cin, temp);
                cout << "Enter table structure: ";
                getline(cin, temp1);
                cout << "Enter table types: ";
                getline(cin, temp2);
                cout << "Enter increment column[0 - none]: ";
                getline(cin, temp3);

                returnTemp = db.createTable(temp, temp1, temp2, stoi(temp3));

                break;
            case 4:
                cout << "Enter DB name: ";
                getline(cin, temp);

                returnTemp = db.selectDB(temp);

                break;
            case 5:
                cout << "Enter table name: ";
                getline(cin, temp);
                cout << "Enter data: ";
                getline(cin, temp1);

                returnTemp = db.addEntry(temp, temp1);

                break;
            case 6:
                cout << "Enter table name: ";
                getline(cin, temp);
                cout << "Index where search data: ";
                getline(cin, temp1);
                cout << "Data: ";
                getline(cin, temp2);
                returnTemp = db.delEntry(temp, stoi(temp1), temp2);

                break;
            case 7:
                cout << "Enter table name: ";
                getline(cin, temp);
                cout << "Enter column name: ";
                getline(cin, temp4);
                cout << "Enter search data: ";
                getline(cin, temp3);
                returnTemp = db.getLineInTableByRow(temp, stoi(temp4), temp3, temp1, temp2);
                if (returnTemp == 0)
                {
                    cout << "Line: "  << temp1 << endl;
                    cout << "Types: " << temp2 << endl;
                }

                break;
            case 8:
                cout << "Enter table name: ";
                getline(cin, temp);
                cout << "Enter column name: ";
                getline(cin, temp4);
                cout << "Enter search data: ";
                getline(cin, temp3);

                db.getArrInTableByRow(temp, stoi(temp4), temp3, temparr, temp2);
                cout << "Lines: " << endl;
                tempint = temparr.size();
                for (int i = 0; i < tempint; i++)
                {
                    cout << i << ") " << temparr[i] << endl;
                }
                cout << "Types: " << temp2 << endl;

                break;
            case 9:
                cout << "Enter database name: ";
                getline(cin, temp1);
                returnTemp = db.delDB(temp1);

                break;
            case 10:
                cout << "Testing 10000 entries to write" << endl;
                for (int i = 0; i < 10000; i++)
                    db.addEntry("table", "hello, 50");

                break;
            case 11:
                cout << "Enter DB name: ";
                getline(cin, temp1);
                returnTemp = db.checkDbStatus(temp1);

                break;
            case 12:
                cout << "Enter DB name: ";
                getline(cin, temp1);
                cout << "Enter table name: ";
                getline(cin, temp2);
                returnTemp = db.checkTableStatus(temp1, temp2);
                break;

        }

        if (ans > 1 && ans <= 12)
        {
            if (returnTemp == 0)
                cout << "Successfull!" << endl;
            else
                cout << "Error: " << returnTemp << endl;

            cout << endl;
        }

    }
    while(ans != 1);


    return 0;
}
