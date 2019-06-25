#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <thread>
#include <vector>
#include <cerrno>

#include <sys/stat.h>

#include "CSVread/csv.h"

using namespace std;


class database
{
    public:
        database();
        virtual ~database();

        //База данных
        int createDB(string);   //Создание базы данных
        int deleteDB(string);   //Удаление базы данных
        int selectDB(string);   //Выбор ранее созданной базы данных


        //Работа с таблицей
        int createTable(string, string, string, int = 0);   //Создание таблицы в выбранной бд (название таблицы, названия колонок [one, two, three, etc], названия типов [int, string, data])
        int deleteTable(string);                        //Удаление таблицы в выбранной бд
        int addEntry(string, string);                   //Добавление записи в таблицу (название таблицы, запись)
        int delEntry(string, int, string);              //Удаление записи из таблицы (название таблицы, номер столбца для поиска, содержание ячейки строку которой нужно удалить)
                                                        //Будет удалена первая попавшаяся запись
        int delAllEntry(string, int, string);           //Удаление всех записей из таблицы
        int delLastEntry(string, int);                  //Удаление X последних записей из таблицы


        template <class T>
        int searchRow(int, T&);   //Поиск строки по ID


        //Система
        int reloadConfigFile();   //Перезагрузить файл конфигураций
        int createConfigFile();   //Создать файл конфигураций, старый будет удалён
        int createSystemDir();    //Создать системную директорию (dbFiles)
        // 0 - создано
        //-1 - директория уже была создана

    protected:

    private:

        int addLog(string, short int = 0);          //Создаём отчёт
        int strCut(string, vector<string>&);    //Распознаём подстроки в строке

        int addTableToList(string);             //Добавить таблицу в список таблиц
        int addDBToList(string);                //Добавить базу данных в список баз данных

        int checkTableAvlb(string);             //Проверить наличие таблицы
        int checkDBAvlb(string);                //Проверить наличие базы данных

        string selectedDB = "NONE";  //Выбранная база данных

        //Загруженные параметры
        bool enableLog = true;

};

#endif // DATABASE_H
