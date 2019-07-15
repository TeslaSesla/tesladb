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
#include <boost/filesystem.hpp>

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
        int createDB(string); //Создание БД
        int delDB   (string); //Удаление БД
        int selectDB(string); //Выбор ранее созданной базы данных


        //Работа с таблицей
        int createTable      (string, string, string, int = 0); //Создание таблицы в выбранной БД
        int delTable         (string);                          //Удаление таблицы в выбранной БД
        int getLastTableLine (string, string&);                 //Возвращает последнюю найденную строку в таблице
        int addEntry         (string, string);                  //Добавление записи в таблицу
        int delEntry         (string, int,    string);          //Удаление одной записи из таблицы
        int delAllEntry      (string, int,    string);          //Удаление всех записей из таблицы
        int delLastEntry     (string, int);                     //Удаление X последних записей из таблицы

        //Поиск строк
        int getLineInTableByRow(string, int, string, string &,        string &); //Поиск первого подходящего элемента по содержанию и возврат элементов в векторе
        int getArrInTableByRow (string, int, string, vector<string>&, string &); //Поиск всех подходящих элементов по содержанию



        // TODO (nikolay#1#): Сделать функцию проверки БД на запрещённое имя


        //Проверка БД/таблицы
        int checkDbStatus       (string);   //Вернуть статус БД
        int checkTableStatus    (string, string);   //Вернуть статус таблицы
        int checkSystemDirStatus();         //Вернуть статус системной директории

        int getTableId(string); //Вернуть ID таблицы
        int getDbId(string);    //Вернуть ID БД

        int fixDb   (string);   //Починить БД
        int fixTable(string);   //Починить таблицу

        //Система
        int reloadConfigFile(); //Перезагрузить файл конфигураций
        int createConfigFile(); //Создать файл конфигураций, старый будет удалён

        int createSystemDir (); //Создать системную директорию (dbFiles)
        // 0 - создано
        //-1 - директория уже была создана

        // TODO (nikolay#3#): Сделать функцию поиска и удаления временных файлов (.tmp)

        int strCut(string, vector<string>&); //Разделяет строку на вектор

    protected:

    private:

        //Отладка и всё что с ней связано
        int addLog(string, short int = 0); //Создаём отчёт

        //Добавление данных в списки
        int addTableToList(string); //Добавить таблицу в список таблиц
        int addDBToList   (string); //Добавить базу данных в список баз данных

        int checkFileStructure(string, string); //Проверить структуру файла

        int getTableTypes     (string, string&); //Возвращает типы указанной таблицы
        int getIncrementColumn(string, int&);    //Возвращает индексный столбик

        //Удаление всего что связано с БД
        int delDbFromList        (string); //Удалить БД из списка
        int delTablesFromListByDb(string); //Удалить все таблицы из списка таблиц исходя из привязанной базы данных

        string selectedDb_ = "NONE";  //Выбранная база данных

        //Загруженные параметры
        bool enableLog_ = true;

};

#endif // DATABASE_H
