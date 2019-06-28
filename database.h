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

struct rawStr
{
    string str;
    int typeInt;
};

class database
{
    public:
        database();
        virtual ~database();

        //База данных
        int createDB(string);   //Создание БД
        int deleteDB(string);   //Удаление БД
        int selectDB(string);   //Выбор ранее созданной базы данных


        //Работа с таблицей
        int createTable (string, string, string, int = 0);   //Создание таблицы в выбранной БД
        int deleteTable (string);                            //Удаление таблицы в выбранной БД
        int addEntry    (string, string);                    //Добавление записи в таблицу
        int delEntry    (string, int,    string);            //Удаление одной записи из таблицы
        int delAllEntry (string, int,    string);            //Удаление всех записей из таблицы
        int delLastEntry(string, int);                       //Удаление X последних записей из таблицы

        //Поиск строк
        int getLineInTableByRow(string, int, string, string &,        string &);    //Поиск первого подходящего элемента по содержанию и возврат элементов в векторе
        int getArrInTableByRow (string, int, string, vector<string>&, string &);    //Поиск всех подходящих элементов по содержанию


        //Система
        int reloadConfigFile();   //Перезагрузить файл конфигураций
        int createConfigFile();   //Создать файл конфигураций, старый будет удалён

        int createSystemDir();    //Создать системную директорию (dbFiles)
        // 0 - создано
        //-1 - директория уже была создана


        int strCut(string, vector<string>&);    //Разделяет строку на вектор

    protected:

    private:

        int addLog(string, short int = 0);      //Создаём отчёт

        int addTableToList(string);     //Добавить таблицу в список таблиц
        int addDBToList(string);        //Добавить базу данных в список баз данных

        int checkTableAvlb(string);     //Проверить наличие таблицы
        int checkDBAvlb(string);        //Проверить наличие базы данных

        int getTableTypes(string, string&);   //Возвращает типы указанной таблицы

        string selectedDB = "NONE";  //Выбранная база данных

        //Загруженные параметры
        bool enableLog = true;

};

#endif // DATABASE_H
