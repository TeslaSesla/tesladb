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
}

int database::createSystemDir()
{
    int status;
    status = mkdir("dbFiles", S_IRWXU);

    //Если директория была только что создана
    if (status == 0)
    {
        //Директория была только что создана, создаём файлы
        ofstream logFile;
        logFile.open(LOG_FILE);
        logFile.close();
        addLog("Log file created.", 0);

        addLog("System dir created.", 0);

        ofstream tablesFile;
        tablesFile.open(TABLES_LIST_FILE);
        tablesFile << "ID, dbname, name" << endl;
        addLog("Tables file created.", 0);
        tablesFile.close();

        ofstream dbFile;
        dbFile.open(DB_LIST_FILE);
        dbFile << "ID, name" << endl;
        addLog("Database file created.", 0);
        dbFile.close();

        ofstream optionsFile;
        optionsFile.open(OPTIONS_FILE);
        optionsFile << "option, value" << endl;
        optionsFile << "enableLog, 1" << endl;
        addLog("Options file created.", 0);
        optionsFile.close();

        return 0;
    }
    else if (status == -1)
    {
        addLog("System dir already created", 0);
        return -1;
    }


    string errorOut = strerror(errno);
    addLog("Something went wrong when creating a system directory " + errorOut, 2);
    return status;
}

int database::createDB(string name)
{
    addLog("Creating database with name " + name + "...", 0);

    //Если не выбрана база данных
    if (selectedDB == "NONE")
    {
        addLog("No database selected", 2);
        return -4;
    }


    //Создаём временные переменные
    int it = 0;             //Итерации
    int SEARCH_id;          //Поиск БД: id
    string SEARCH_name;     //Поиск БД: имя БД

    //Ищем индекс последней базы данных
    io::CSVReader<2>in(DB_LIST_FILE);
    in.read_header(io::ignore_missing_column, "ID", "name");

    //Перебор таблицы со списком БД
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
    addLog("Database added to list", 1);

    //Создаём директорию
    int status;
    status = mkdir(name.c_str(), S_IRWXU);

    if (status == 0)
        addLog("Database created!", 0);
    else if (status == -1)
    {
        addLog("Can't create database directory, directory with this name already exists.", 2);
        return -2;  //Папка с названием базы данных уже существует
    }
    else
    {
        string errOut = strerror(errno);
        addLog("Can't create database directory (mkdir returned value " + to_string(status) + ")\n" + errOut, 2);
        return -3;  //mkdir вернула неизвестное значение
    }

    return 0;
}

int database::createTable(string name, string strNames, string strTypes, int indexColumnNumber)
{
    addLog("Creating table " + name + " in database " + selectedDB + "...", 0);

    //Если такая таблица уже есть
    if (checkTableAvlb(name) != 0) return -1;


    //Создаём файл структуры таблицы
    addLog("Creating structure file (str)", 0);
    ofstream tableFileConfig;
    tableFileConfig.open(selectedDB + "/" + name + ".str");
    tableFileConfig << strNames << endl;
    tableFileConfig << strTypes << endl;
    tableFileConfig.close();


    //Создаём файл конфигурации таблицы
    addLog("Creating table options file (" + name + "_options.csv)", 0);
    ofstream tableOptionsFile;
    tableOptionsFile.open(selectedDB + "/" + name + "_options.csv");            //Открываем файл конфигураций таблицы
    tableOptionsFile << "option, value" << endl;                                //Устанавливаем структуру файла конфигураций
    tableOptionsFile << "indexColumnNumber, " << indexColumnNumber << endl;     //Устанавливаем настройку "Индексный столбец"
    tableOptionsFile.close();


    //Создаём файл таблицы
    addLog("Creating table file (" + name + ".csv)", 0);
    ofstream tableFile;
    tableFile.open(selectedDB + "/" + name + ".csv");
    tableFile << strNames << endl;
    tableFile.close();

    addTableToList(name);


    return 0;
}

int database::addEntry(string name, string data)
{
    addLog("Adding a entry to table " + name + " in database " + selectedDB, 0);

    //Если таблица не существует
    if (checkTableAvlb(name) == -1)
        addLog("Table founded in table's list", 0);
    else
    {
        addLog("Table not created", 1);
        return -1;
    }

    //Сделать проверку в функции SelectDB()

    //Сделать проверку на правильность структуры
    //Сделать проверку на открытие файла
    addLog("Opening table file (" + selectedDB + "/" + name + ".csv)", 0);
    ofstream tableFile;
    tableFile.open(selectedDB + "/" + name + ".csv", ios::app);
    if (tableFile)
        addLog("Table file opened", 0);
    else
    {
        addLog("Can't open table file", 2);
        return -2;
    }
    tableFile << data << endl;
    tableFile.close();
    return 0;
}

int database::delEntry(string tableName, int columnNumber, string columnText)
{
    if (checkTableAvlb(tableName) != -1) return -1;

    addLog("Creating temp table file (" + tableName + ".tmp)", 0);

    ofstream tableTempFile;
    tableTempFile.open(selectedDB + "/" + tableName + ".tmp", ios::app);
    if (!tableTempFile)
    {
        addLog("Can't create table temp file", 2);
        return -2;
    }

    addLog("Temp file created", 0);
    addLog("Moving table from " + tableName + ".csv to " + tableName + ".tmp", 0);

    //Инициализируем временные переменные
    string str;                 //Строка при переборе файла
    vector<string> arr;         //Вектор для разделения строки
    bool founded = false;


    io::LineReader in(selectedDB + "/" + tableName + ".csv");
    while(char*line = in.next_line())
    {
        str = line;         //Обновляем текущую строку
        strCut(str, arr);   //Разделяем строку и помещаем в вектор

        //Если строка не подходит по критерию ИЛИ уже найдена подходящая строка ТО добавляем строку в новый файл
        if (arr[columnNumber-1] != columnText || founded == true)
            tableTempFile << str << endl;
        else
            founded = true;
    }

    tableTempFile.close();

    string newstr = selectedDB + "/" + tableName;
    newstr += ".csv";
    string oldstr = selectedDB + "/" + tableName;
    oldstr += ".tmp";

    //Удаляем файл таблицы
    if(remove(newstr.c_str()) != 0)
        addLog("Error removing file (" + newstr + "): " + strerror(errno), 2);
    else
        addLog("Successful file remove (" + newstr + ")", 0);

    //Переименовываем временный файл в файл таблицы
    if(rename(oldstr.c_str(), newstr.c_str()) != 0)
        addLog("Error rename file (" + oldstr + "): " + strerror(errno), 2);
    else
        addLog("Successful file rename (" + oldstr + ")", 0);


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


    io::LineReader in2(selectedDB + "/" + tableName + ".csv");
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


    io::LineReader in2(selectedDB + "/" + tableName + ".csv");
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
    io::LineReader in(selectedDB + "/" + tableName + ".str");
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

int database::checkTableAvlb(string name)
{
    //Проверяем наличие таблицы в файле таблиц
    addLog("Checking the availability of the table " + name + " in table's list (tables.csv)", 0);

    try
    {
        io::CSVReader<3>in(TABLES_LIST_FILE);
        in.read_header(io::ignore_missing_column, "ID", "dbname", "name");
        int SEARCH_id; string SEARCH_dbname, SEARCH_name;
        while(in.read_row(SEARCH_id, SEARCH_dbname, SEARCH_name))
        {
            if (name == SEARCH_name)
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

    addLog("Table with name " + name + " not founded", 0);
    return 0;
}

int database::checkDBAvlb(string name)
{
    //Проверяем наличие базы данных в файле баз данных
    addLog("Checking the availability of the database " + name + " in database's list (databases.csv)", 0);
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
    addLog("Adding table to list", 1);
    int lastTableId = 0;
    int it = 0;
    try
    {
        //Чтение таблиц
        addLog("Reading tables indexes", 0);
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

    ofstream tableConfigFile;
    tableConfigFile.open(TABLES_LIST_FILE, std::ios::app);

    if (it == 0)
        tableConfigFile << it << ", " << selectedDB << ", " << name << endl;
    else
        tableConfigFile << lastTableId + 1 << ", " << selectedDB << ", " << name << endl;

    tableConfigFile.close();
    addLog("Table added!", 0);
    return 0;
}

int database::addLog(string message, short int signal)
{
    /*
        0 - INFO
        1 - WARN
        2 - ERROR
    */

    if (enableLog == false) return -2;

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char date[25];
    strftime(date, sizeof(date), "[%d.%m.%C | %H:%M:%S]", tm);

    ofstream logFile;
    logFile.open(LOG_FILE, std::ios::app);
    if (logFile)    logFile << date << " [Database: " << selectedDB << "] - " << message;
        else return -1;

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
    selectedDB = name;
    addLog("Database " + name + " selected", 0);
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
                enableLog = SEARCH_value;
        }
    }
}

database::~database()
{
    //dtor
}
