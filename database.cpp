/*
    Copyright: (2019) Nikolay Tetus <nikolaytetus@gmail.com>
    License: GNU GPLv3

    All rights reserved.


    This file is part of Tesladb.

    TeslaDB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Tesladb is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Tesladb.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "database.h"

#define DB_LIST_FILE            "dbFiles/datebases.csv"
#define TABLES_LIST_FILE        "dbFiles/tables.csv"
#define LOG_FILE                "dbFiles/log.txt"
#define OPTIONS_FILE            "dbFiles/options.csv"

database::database()
{
    //Конструктор

    //Создаём системную директорию
    createSystemDir();

    //Перезагружаем файл конфигураций
    reloadConfigFile();

    addLog("Database initializated", 0);
}

int database::createSystemDir()
{
    //Если директория была только что создана
    if (boost::filesystem::create_directory("dbFiles") == true)
    {
        //Директория была только что создана, создаём файлы
        ofstream logFile;
        logFile.open(LOG_FILE);
        logFile.close();

        ofstream tablesFile;
        tablesFile.open(TABLES_LIST_FILE);
        tablesFile << "ID, dbname, name" << endl;
        tablesFile.close();

        ofstream dbFile;
        dbFile.open(DB_LIST_FILE);
        dbFile << "ID, name" << endl;
        dbFile.close();

        ofstream optionsFile;
        optionsFile.open(OPTIONS_FILE);
        optionsFile << "option, value" << endl;
        optionsFile << "enableLog, 1" << endl;
        optionsFile.close();

        return 0;
    }

    //Ветвление не выполнилось, директория уже существует
    addLog("Directory already created", 0);
    return -1;
}

int database::createDB(string name)
{
    // TODO (nikolay#1#): Перевести эту функцию на использование функции checkDbStatus

    //Создаём временные переменные
    int it = 0;             //Счётчик итераций
    int SEARCH_id;          //Поиск БД: id
    string SEARCH_name;     //Поиск БД: имя БД


    //Заменить на функцию checkBdStatus

    //Ищем индекс последней базы данных
    io::CSVReader<2>in(DB_LIST_FILE);
    in.read_header(io::ignore_missing_column, "ID", "name");
    while(in.read_row(SEARCH_id, SEARCH_name))
    {
        if (name == SEARCH_name)
        {
            addLog("Can't create database, founded database with same name.", 2);
            return -1;
        }
        it++;
    }


    //Добавляем базу данных в список баз данных
    ofstream dbFile;
    dbFile.open(DB_LIST_FILE, ios::app);
    dbFile << it << ", " << name << endl;
    dbFile.close();


    //Создаём директорию
    if (boost::filesystem::create_directory(name) == false)
    {
        //Папка с названием базы данных уже существует
        addLog("Can't create database directory", 2);
        return -2;
    }

    return 0;
}

int database::createTable(string name, string strNames, string strTypes, int incrementColumnNumber)
{

    //Если такая таблица уже есть
    if (checkTableAvlb(name) != 0) return -1;


    //Если не выбрана база данных
    if (selectedDb_ == "NONE")
    {
        addLog("No database selected", 2);
        return -5;
    }


    //Создаём файл структуры таблицы
    ofstream tableFileConfig;
    try
    {
        tableFileConfig.open(selectedDb_ + "/" + name + ".str");
        tableFileConfig << strNames << endl;    //Записываем структуру имён (названия столбиков)
        tableFileConfig << strTypes << endl;    //Записываем структуру типов данных
    }
    catch (...)
    {
        string errOut = strerror(errno);
        addLog("Can't create structure file " + errOut, 2);
        return -2;
    }
    tableFileConfig.close();


    //Создаём файл конфигурации таблицы
    ofstream tableOptionsFile;
    try
    {
        tableOptionsFile.open(selectedDb_ + "/" + name + "_options.csv");                //Открываем файл конфигураций таблицы
        tableOptionsFile << "option, value" << endl;                                    //Устанавливаем структуру файла конфигураций
        tableOptionsFile << "incrementColumnNumber, " << incrementColumnNumber << endl; //Устанавливаем настройку "Столбец типа AUTO_INCREMENT"
    }
    catch (...)
    {
        string errOut = strerror(errno);
        addLog("Can't create options file " + errOut, 2);
        return -3;
    }
    tableOptionsFile.close();


    //Создаём файл таблицы
    ofstream tableFile;
    try
    {
        tableFile.open(selectedDb_ + "/" + name + ".csv");
        tableFile << strNames << endl;  //Записываем названия столбиков
    }
    catch (...)
    {
        string errOut = strerror(errno);
        addLog("Can't create table file " + errOut, 2);
        return -4;
    }
    tableFile.close();

    //Добавляем таблицу в список таблиц
    addTableToList(name);


    return 0;
}

int database::getIncrementColumn(string tableName, int & columnNumber)
{
    //Ищем параметр incrementColumnNumber
    io::CSVReader<2> in(selectedDb_ + "/" + tableName + "_options.csv");
    in.read_header(io::ignore_missing_column, "option", "value");

    string searchStr = "incrementColumnNumber"; //Строка которую нужно найти
    string SEARCH_option; int SEARCH_value; //Переменные для поиска

    //Перебор таблицы со списком БД
    while(in.read_row(SEARCH_option, SEARCH_value))
    {
        if (SEARCH_option == searchStr)
        {
            columnNumber = SEARCH_value;
            return 0;
        }
    }

    addLog("Option incrementColumnNumber not founded", 1);
    return -1;
}

int database::addEntry(string name, string data)
{
    //Если таблица не существует
    if (checkTableAvlb(name) != -1)
    {
        addLog("Table already created", 1);
        return -1;
    }

    //Сделать проверку в функции SelectDB()

    //Сделать проверку на правильность структуры
    //Сделать проверку на открытие файла
    addLog("Opening table file (" +  name + ".csv)", 0);
    ofstream tableFile;
    tableFile.open(selectedDb_ + "/" + name + ".csv", ios::app);
    if (tableFile)
        addLog("Table file opened", 0);
    else
    {
        addLog("Can't open table file", 2);
        return -2;
    }

    int incrementColumn = 0;
    getIncrementColumn(name, incrementColumn);

    if (incrementColumn == 0)
    {
        //Индексный столбик отстутствует - просто добавляем запись
        tableFile << data << endl;
    }
    else
    {

        vector<string> arr;     //Вектор для разделения строки
        string lastLine;        //Последняя строка
        int lastincrement = 0;      //Последний индекс
        string lastIncrementTemp;   //Индекс последней найденной записи

        vector<string> dataarr; //Выходной вектор для объеденения индекса и записи
        int incrementToInsert = 0;  //Индекс который будет добавлен в новую запись
        string outStr = "";     //Выходная строка


        //Если это не первая строка
        if (getLastTableLine(name, lastLine) == 0)
        {
            strCut(lastLine, arr);  //Разделяем последнюю строку

            lastIncrementTemp = arr[incrementColumn - 1];
            lastincrement = stoi(lastIncrementTemp); //Получаем последний индекс

            incrementToInsert = lastincrement + 1;  //Индекс новой строки

        }


        strCut(data, dataarr);  //Разделяем исходную строку
        dataarr.insert(dataarr.begin() + incrementColumn - 1, to_string(incrementToInsert));


        int dataArrSize = dataarr.size();
        for (int i = 0; i < dataArrSize; i++)
        {
            outStr += dataarr[i];
            if (i != dataArrSize - 1)
                outStr += ", ";
        }

        tableFile << outStr << endl;


    }

    tableFile.close();
    return 0;
}

int database::getLastTableLine(string tableName, string & strOut)
{

    io::LineReader in2(selectedDb_ + "/" + tableName + ".csv");
    int it = 0;
    while(char*line = in2.next_line())
    {
        strOut = line;
        it++;
    }

    if (it < 2)
    {
        addLog("Founded file have only 1 line", 1);
        return -2;  //В файле только 1 строка
    }

    return 0;
}

int database::delEntry(string tableName, int columnNumber, string columnText)
{
    if (checkTableAvlb(tableName) != -1) return -1;


    ofstream tableTempFile;
    tableTempFile.open(selectedDb_ + "/" + tableName + ".tmp", ios::app);
    if (!tableTempFile)
    {
        addLog("Can't create table temp file", 2);
        return -2;
    }


    //Инициализируем временные переменные
    string str;                 //Строка при переборе файла
    vector<string> arr;         //Вектор для разделения строки
    bool isFounded = false;


    io::LineReader in(selectedDb_ + "/" + tableName + ".csv");
    while(char*line = in.next_line())
    {
        str = line;         //Обновляем текущую строку
        strCut(str, arr);   //Разделяем строку и помещаем в вектор

        //Если строка не подходит по критерию ИЛИ уже найдена подходящая строка ТО добавляем строку в новый файл
        if (arr[columnNumber-1] != columnText || isFounded == true)
            tableTempFile << str << endl;
        else
            isFounded = true;
    }

    tableTempFile.close();


    if (boost::filesystem::remove(selectedDb_ + "/" + tableName + ".csv") == false)
        addLog("Can't delete file", 2);

    boost::filesystem::rename(selectedDb_ + "/" + tableName + ".tmp", selectedDb_ + "/" + tableName + ".csv");



    return 0;
}

int database::delDB(string dbName)
{

    if (delDbFromList(dbName) != 0)
        return -1;

    if (delTablesFromListByDb(dbName))
        return -2;


    //Удаляем папку с базой данных
    if (boost::filesystem::remove_all(dbName) == false)
        addLog("Can't delete database directory", 1);

    return 0;
}

int database::delDbFromList(string dbName)
{
    ofstream databasesTempFile;
    databasesTempFile.open("dbFiles/databases.tmp", ios::app);
    if (!databasesTempFile)
    {
        addLog("Can't create databases.csv temp file", 2);
        return -1;
    }


    databasesTempFile << "ID, name" << endl;
    io::CSVReader<2> in(DB_LIST_FILE);
    in.read_header(io::ignore_missing_column, "ID", "name");
    int SEARCH_id; string SEARCH_name;
    while(in.read_row(SEARCH_id, SEARCH_name))
    {
        if (dbName != SEARCH_name)
        {
            databasesTempFile << SEARCH_id << ", " << SEARCH_name << endl;
        }
    }
    databasesTempFile.close();

    boost::filesystem::remove(DB_LIST_FILE);
    boost::filesystem::rename("dbFiles/databases.tmp", DB_LIST_FILE);


    return 0;
}

int database::delTablesFromListByDb(string dbName)
{
    ofstream tablesTempFile;
    tablesTempFile.open("dbFiles/tables.tmp", ios::app);
    if (!tablesTempFile)
    {
        addLog("Can't create tables.csv temp file", 2);
        return -1;
    }


    tablesTempFile << "ID, dbname, name" << endl;
    io::CSVReader<3> in(TABLES_LIST_FILE);
    in.read_header(io::ignore_missing_column, "ID", "dbname", "name");
    int SEARCH_id; string SEARCH_dbname, SEARCH_name;
    while(in.read_row(SEARCH_id, SEARCH_dbname, SEARCH_name))
    {
        //Если имя базы данных (таблицы которой нужно удалить) не совпадает с именем найденной базы данных - записываем
        if (dbName != SEARCH_dbname)
            tablesTempFile << SEARCH_id << ", " << SEARCH_dbname << ", " << SEARCH_name << endl;
    }
    tablesTempFile.close();

    boost::filesystem::remove(TABLES_LIST_FILE);
    boost::filesystem::rename("dbFiles/tables.tmp", TABLES_LIST_FILE);

    return 0;
}

int database::getLineInTableByRow(string tableName, int columnNumber, string columnData, string & strOut, string & typeNames)
{

    //Получаем запись типов столбиков
    getTableTypes(tableName, typeNames);


    //Инициализируем временные переменные
    string str;             //Строка при переборе файла
    vector<string> arr;     //Вектор для разделения строки
    int strCount = 0;       //Кол-во строк найденных в файле


    io::LineReader in2(selectedDb_ + "/" + tableName + ".csv");
    while(char*line = in2.next_line())
    {
        strCount++;
        if (strCount != 1)
        {
            //Обновляем переменные
            str = line;         //Обновляем текущую строку
            strCut(str, arr);   //Разделяем строку и помещаем в вектор

            //Если строка подходит по критерию ТО возвращаем строку
            if (arr[columnNumber-1] == columnData)
            {
                //Приравниваем выходной параметр к текущей строке
                strOut = line;
                return 0;
            }
        }
    }

    return -1;  //Ничего не найдено
}

int database::getArrInTableByRow(string tableName, int columnNumber, string columnData, vector<string> & strOut, string & typeNames)
{

    //Получаем запись типов столбиков
    getTableTypes(tableName, typeNames);


    //Инициализируем временные переменные
    string str;             //Строка при переборе файла
    vector<string> arr;     //Вектор для разделения строки
    int strCount   = 0;     //Всего строк
    int strFounded = 0;     //Найденных строк


    io::LineReader in2(selectedDb_ + "/" + tableName + ".csv");
    while(char*line = in2.next_line())
    {
        strCount++;
        if (strCount != 1)
        {
            str = line;         //Обновляем текущую строку
            strCut(str, arr);   //Разделяем строку и помещаем в вектор

            //Если строка подходит по критерию ТО возвращаем строку
            if (arr[columnNumber - 1] == columnData)
            {
                strFounded++;
                strOut.resize(strFounded);
                strOut[strFounded - 1] = line;
            }
        }
    }
    return -1;  //Ничего не найдено

}

int database::getTableTypes(string tableName, string & typeOut)
{
    //Добавить проверку на нарушение целостности
    int it = 0;
    io::LineReader in(selectedDb_ + "/" + tableName + ".str");
    while(char*line = in.next_line())
    {
        it++;
        if (it == 2)
        {
            typeOut = line;
            return 0;
        }
    }
    return -1;
}

int database::checkTableAvlb(string tableName, string dbName)
{
    if (dbName == "NOT_SELECTED")
    {
        if (selectedDb_ == "NONE")
        {
            addLog("No database selected", 2);
            return -3;
        }
        else
            dbName = selectedDb_;
    }

    try
    {
        //Проверяем наличие таблицы в файле таблиц
        io::CSVReader<3>in(TABLES_LIST_FILE);
        in.read_header(io::ignore_missing_column, "ID", "dbname", "name");

        int SEARCH_id;          //Поиск таблицы: идентификатор
        string SEARCH_dbname,   //Поиск таблицы: имя базы данных
               SEARCH_name;     //Поиск таблицы: имя таблицы

        while(in.read_row(SEARCH_id, SEARCH_dbname, SEARCH_name))
        {
            if (tableName == SEARCH_name && dbName == SEARCH_dbname)
            {
                addLog("Founded table with same name", 0);
                return -1;
            }
        }
    }
    catch (...)
    {
        addLog("Can't read tables list file", 2);
        return -2;
    }

    addLog("Table with name " + tableName + " not founded", 0);
    return 0;
}

int database::checkDBAvlb(string name)
{
    //Проверяем наличие базы данных в файле баз данных
    try
    {
        io::CSVReader<2>in(DB_LIST_FILE);
        in.read_header(io::ignore_missing_column, "ID", "name");
        int SEARCH_id; string SEARCH_name;
        while(in.read_row(SEARCH_id, SEARCH_name))
        {
            if (name == SEARCH_name)
            {
                addLog("Founded database with same name", 0);
                return -1;
            }
        }
    }
    catch(...)
    {
        addLog("Can't read database list file", 2);
        return -2;
    }
    addLog("Database with name " + name + " not founded", 0);
    return 0;
}

int database::checkFileStructure(string fileName, string fileStructure)
{
    int    it = 0;  //Счётчик итераций
    string tmpStr;  //Временная строка

    try
    {
        io::LineReader in2(fileName);
        while(char*line = in2.next_line())
        {
            it++;
            if (it == 1)
            {
                tmpStr = line;
                if (fileStructure == tmpStr)
                {
                    return 0;
                }
                else
                {
                    return -2;
                }
            }
        }
    }
    catch(...)
    {
        addLog("Cheking file structure error");
    }

    return -1;
}

int database::checkSystemDirStatus()
{
    addLog("Checking system directory");
    if (boost::filesystem::is_directory("dbFiles") == false)
    {
        addLog("Can't find system directory");
        return -1;
    }

    // TODO (nikolay#1#): Сделать проверку файла с логами


    //Состояние структур файлов в системной директории
    int  tablesListStatus    = checkFileStructure(TABLES_LIST_FILE, "ID, dbname, name");    //Статус файла таблиц
    int  dbListStatus        = checkFileStructure(DB_LIST_FILE,     "ID, name");            //Статус файла баз данных
    int  optionsListStatus   = checkFileStructure(OPTIONS_FILE,     "option, value");       //Статус файла параметров

    if (tablesListStatus != 0)
    {
        addLog("Tables list structure error", 2);
        return -2;
    }
    if (dbListStatus != 0)
    {
        addLog("Database list structure error", 2);
        return -3;
    }
    if (optionsListStatus != 0)
    {
        addLog("Options list structure error", 2);
        return -4;
    }

    return 0;
}

int database::checkDbStatus(string dbName)
{
    addLog("Checking database status...");

    //Проверяем имя бд на запрещённое
    if (dbName == "NONE" || dbName == "NOT_SELECTED" || dbName == "")
    {
        addLog("Selected banned or empty DB name", 1);
        return -6;
    }

    bool isDirCreated          = false; //Создана ли директория
    bool isFounded             = false; //Найдена ли БД в списке БД
    bool isMultipleDeclaration = false; //Присутствует ли множественное объявление БД в файле баз данных

    //Ищем директорию базы данных
    if (boost::filesystem::is_directory(dbName) == true)
    {
        addLog("Database directory: OK");
        isDirCreated = true;
    }
    else
    {
        addLog("Database directory not founded");
        isDirCreated = false;
    }


    //Если в системной директории всё нормас и все файлы присутствуют
    if (checkSystemDirStatus() == 0)
    {
        io::CSVReader<2> in(DB_LIST_FILE);
        in.read_header(io::ignore_missing_column, "ID", "name");
        int SEARCH_id; string SEARCH_name;
        while(in.read_row(SEARCH_id, SEARCH_name))
        {
            if (dbName == SEARCH_name)
            {
                if (isFounded == true)
                {
                    addLog("Founded multiple database declaration (dbFiles/datebases.csv)");
                    isMultipleDeclaration = true;
                }
                else
                {
                    addLog("Database avilable in list: OK");
                    isFounded = true;
                }

            }
        }
    }
    else
    {
        //Проблемы с системной директорией
        return -5;
    }


    addLog("DB STATUS: isDirCreated=" + to_string(isDirCreated) + ", isFounded=" + to_string(isFounded) + ", isMultipleDeclaration=" + to_string(isMultipleDeclaration));

    if (isDirCreated == false && isFounded == false)
    {
        addLog("Database not created");
        return -1;
    }
    if (isDirCreated == false)
    {
        addLog("Dir not created", 1);
        return -2;
    }
    if (isFounded == false)
    {
        addLog("DB not founded in DB list", 1);
        return -3;
    }
    if (isMultipleDeclaration == true)
    {
        addLog("Founded multiple declaration of DB", 1);
        return -4;
    }


    //Проблем с БД не найдено
    return 0;
}

int database::checkTableStatus(string dbName, string tableName)
{
    addLog("Checking table status...");


    if (checkDbStatus(dbName) == -1)
        return -5;  //БД связанной с таблицей не существует
    else if (checkDbStatus(dbName) != 0)
        return -6;  //Проблемы с БД


    //Переменные показатели
    bool isFounded          = false;    //Найдена ли таблица в списке таблиц
    bool isTableFileOk      = true;     //Состояние файла таблицы
    bool isTableStructureOk = true;     //Состояние файла структуры таблицы
    bool isTableOptionsOk   = true;     //Состояние файла параметров таблицы

    string tableStructure;      //Структура таблицы взятая из tableName.str
    string optionsStructure;    //Структура файла параметров
    int it = 0;                 //Счётчик итераций

    try
    {
        io::CSVReader<3> in0(TABLES_LIST_FILE);
        in0.read_header(io::ignore_missing_column, "ID", "dbname", "name");

        int SEARCH_id;          //Поиск таблицы: идентификатор
        string SEARCH_dbname,   //Поиск таблицы: имя базы данных
               SEARCH_name;     //Поиск таблицы: имя таблицы

        while(in0.read_row(SEARCH_id, SEARCH_dbname, SEARCH_name))
        {
            if (tableName == SEARCH_name && dbName == SEARCH_dbname)
            {
                isFounded = true;
            }
        }
    }
    catch (...)
    {
        isFounded = false;
    }

    //Проверка файла структуры таблицы
    try
    {
        io::LineReader in(dbName + "/" + tableName + ".str");
        while(char*line = in.next_line())
        {
            it++;

            if (it == 1)
                tableStructure = line;
        }
    }
    catch (...)
    {
        isTableStructureOk = false;
    }

    //Проверка файла таблицы
    try
    {
        io::LineReader in2(dbName + "/" + tableName + ".csv");
        while(char*line = in2.next_line())
        {
            it++;
            if (it == 1)
                tableStructure = line;
        }
    }
    catch (...)
    {
        isTableFileOk = false;
    }

    //Проверка файла параметров таблицы
    it = 0;
    try
    {
        io::LineReader in3(dbName + "/" + tableName + "_options.csv");
        while(char*line = in3.next_line())
        {
            it++;
            if (it == 1)
            {
                optionsStructure = line;
            }
        }

        if (optionsStructure != "option, value")
            isTableOptionsOk = false;
    }
    catch (...)
    {
        isTableOptionsOk = false;
    }


    if (isTableFileOk == false && isTableOptionsOk == false && isTableStructureOk == false && isFounded == false)
    {
        addLog("Table not founded", 1);
        return -1;
    }

    if (isTableFileOk == false)
    {
        addLog("Table have problems with table file", 1);
        return -2;
    }
    if (isTableOptionsOk == false)
    {
        addLog("Table have problems with options file", 1);
        return -3;
    }
    if (isTableStructureOk == false)
    {
        addLog("Table have problems with structure file", 1);
        return -4;
    }

    //Всё нормально - таблица найдена и исправна
    return 0;
}

int database::getTableId(string tableName)
{
    io::CSVReader<3> in(TABLES_LIST_FILE);
    in.read_header(io::ignore_missing_column, "ID", "dbName", "name");
    int SEARCH_id; string SEARCH_dbName, SEARCH_name;
    while(in.read_row(SEARCH_id, SEARCH_dbName, SEARCH_name))
    {
        //Если имя таблицы соответсвует найденному - возвращаем ID
        if (tableName == SEARCH_name)
            return SEARCH_id;
    }

    //Таблица не найдена
    addLog("Table id not found (Can't find table)", 1);
    return -1;
}

int database::getDbId(string dbName)
{
    io::CSVReader<2> in(DB_LIST_FILE);
    in.read_header(io::ignore_missing_column, "ID", "name");
    int SEARCH_id; string SEARCH_name;
    while(in.read_row(SEARCH_id, SEARCH_name))
    {
        //Если имя БД соответсвует найденному - возвращаем ID
        if (dbName == SEARCH_name)
            return SEARCH_id;
    }

    //БД не найдена
    return 0;
}

int database::strCut(string inStr, vector<string> & substringArr)
{
    int lastSeparatorPos = 0;   //Позиция последнего разделителя
    int nextSeparatorPos = 0;   //Позиция следующего разделителя
    int it               = 0;   //Кол-во итераций (разделителей)
    int substrLen        = 0;   //Длина текущей подстроки
    string separator     = ","; //Разделитель

    while(true)
    {
        it++;   //Увеличиваем кол-во итераций

        if (inStr.find(separator, 0) == string::npos)
        {//Разделителей нет
            substringArr.resize(it);  //Увеличиваем размер вектора
            substringArr[it-1] = inStr;
            break;
        }

        if (it == 1)
            //Первый элемент. Записываем только адрес первого разделителя
            substrLen = inStr.find(separator, lastSeparatorPos);
        else
            //Не первый элемент. Берём индекс следующего указателя и вычитаем индекс текущего указателя
            substrLen = inStr.find(separator, nextSeparatorPos) - inStr.find(separator, lastSeparatorPos);

        substringArr.resize(it);  //Увеличиваем размер вектора

        if (it == 1)
            //Первая подстрока. Копируем до разделителя
            substringArr[it-1] = inStr.substr(lastSeparatorPos, substrLen);
        else
            //Не первая подстрока. Копируем между разделителей и избавляемся от них
            substringArr[it-1] = inStr.substr(lastSeparatorPos + 2, substrLen - 2);

        //Обновляем индексы
        lastSeparatorPos = inStr.find(separator, lastSeparatorPos + 1);     //Отмечаем последний разделитель в строке
        nextSeparatorPos = inStr.find(separator, lastSeparatorPos + 1);     //Отмечаем следующий разделитель в строке

        if (inStr.find(separator, nextSeparatorPos) == string::npos)
        {//Следующая строка последняя. Обрабатываем её сейчас
            it++;
            substringArr.resize(it);                                    //Увеличиваем размер вектора
            substringArr[it-1] = inStr.substr(lastSeparatorPos + 2);    //Создаём подстроку с последнего разделителя до конца строки

            break;
        }

    }
    return 0;
}

int database::addTableToList(string name)
{
    //Ищем индекс последней таблицы
    int lastTableId = 0;    //Идентификатор последней таблицы
    int it = 0;             //Кол-во таблиц (итераций)
    try
    {
        //Проходим по всему файлу
        io::CSVReader<3>in(TABLES_LIST_FILE);
        in.read_header(io::ignore_missing_column, "ID", "dbname", "name");
        int SEARCH_id; string SEARCH_dbname, SEARCH_name;
        while(in.read_row(SEARCH_id, SEARCH_dbname, SEARCH_name))
        {
            it++;
            lastTableId = SEARCH_id;
        }
    }
    catch (...)
    {
        addLog("Can't read tables file", 2);
        return -1;
    }

    //Добавляем таблицу в список
    ofstream tableListFile;

    //Открываем список таблиц с параметром записи в конец
    tableListFile.open(TABLES_LIST_FILE, ios::app);

    if (it == 0)
        //Если это первая таблица в списке
        tableListFile << it << ", " << selectedDb_ << ", " << name << endl;
    else
        //Если это не первая таблица в списке
        tableListFile << lastTableId + 1 << ", " << selectedDb_ << ", " << name << endl;


    //Закрываем файл со списком таблиц
    tableListFile.close();

    return 0;
}

int database::addLog(string message, short int signal)
{
    /*
        0 - INFO
        1 - WARN
        2 - ERROR
    */

    if (enableLog_ == false) return -2;

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char date[25];
    strftime(date, sizeof(date), "[%d.%m.%C | %H:%M:%S]", tm);

    ofstream logFile;
    logFile.open(LOG_FILE, ios::app);
    if (logFile)
        logFile << date << " [Database: " << selectedDb_ << "] - " << message;
    else
        return -1;

    switch (signal)
    {
        case 0:
            logFile << " [INFO]";
            break;
        case 1:
            logFile << " [WARN]";
            break;
        case 2:
            logFile << " [ERROR]";
            break;
        default:
            logFile << " [UNKNOWN SIGNAL]";
            break;
    }
    logFile << endl;

    logFile.close();
    return 0;
}

int database::selectDB(string name)
{
    if (checkDBAvlb(name) != -1)
    {
        addLog("Can't select database " + name, 1);
        return -1;
    }
    selectedDb_ = name;
    return 0;
}

int database::reloadConfigFile()
{
    string search1 = "enableLog";

    io::CSVReader<2>in(OPTIONS_FILE);
    in.read_header(io::ignore_missing_column, "option", "value");
    string SEARCH_option; int SEARCH_value;
    while(in.read_row(SEARCH_option, SEARCH_value))
    {
        if (SEARCH_option == search1)
        {
            if (SEARCH_value == 0 || SEARCH_value == 1)
            {
                enableLog_ = SEARCH_value;
            }
        }
    }

    return 0;
}

database::~database()
{
    //dtor
}
