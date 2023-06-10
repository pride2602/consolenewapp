#include <iostream>
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

using namespace std;


int main() {

    constexpr auto SQL_RESULT_LEN = 240;
    constexpr auto SQL_RETURN_CODE_LEN = 1024;

    SQLHANDLE sqlConnHandle{ nullptr }; // дескриптор для соединения с базой данных
    SQLHANDLE sqlEnvHandle{ nullptr }; // дескриптор окружения базы данных
    SQLHANDLE sqlStmtHandle{ nullptr };  // дескриптор для выполнения запросов к базе данных
    SQLWCHAR retconstring[SQL_RETURN_CODE_LEN]{}; // строка для кода возврата из функций API ODBC


    // Выделяем дескриптор для базы данных
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvHandle))
        goto COMPLETED;

    if (SQL_SUCCESS != SQLSetEnvAttr(sqlEnvHandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
        goto COMPLETED;

    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvHandle, &sqlConnHandle))
        goto COMPLETED;

    cout << "Attempting connection to SQL Server...\n";

    // Устанавливаем соединение с сервером  
    switch (SQLDriverConnect(sqlConnHandle,
        GetDesktopWindow(),
        (SQLWCHAR*)L"DRIVER={MySQL ODBC 8.0 ANSI Driver};SERVER=localhost;PORT=3306;DATABASE=testdb;UID=root;PWD=root;",
        SQL_NTS,
        retconstring,
        1024,
        NULL,
        SQL_DRIVER_COMPLETE)) {


    case SQL_SUCCESS:
    case SQL_SUCCESS_WITH_INFO:
        cout << "Successfully connected to SQL Server\n";
        break;

    case SQL_INVALID_HANDLE:
    case SQL_ERROR:
        cout << "Could not connect to SQL Server\n";
        goto COMPLETED;

    default:
        break;
    }

    // Если соединение не установлено, то выходим из программы
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle))
        goto COMPLETED;

    cout << "\nExecuting T-SQL query...\n";

    // Если выполнение запроса с ошибками, то выходим из программы
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)L"SELECT * from test", SQL_NTS)) {
        cout << "Error querying SQL Server \n";
        goto COMPLETED;
    }
    else {

        //Объявление структуры данных для результата запроса версии SQL
        SQLCHAR sqlVersion[SQL_RESULT_LEN];
        SQLLEN sql_str_length;
        // Переменная для хранения числа столбцов
        SQLSMALLINT    V_OD_colanz, V_OD_rowcount;
        SQLINTEGER   V_OD_err, V_OD_id;
        SQLCHAR     V_OD_buffer[200];

        V_OD_err = SQLBindCol(sqlStmtHandle, 1, SQL_INTEGER, &V_OD_id, sizeof(V_OD_id), nullptr);
        V_OD_err = SQLBindCol(sqlStmtHandle, 2, SQL_CHAR, &V_OD_buffer, SQL_RESULT_LEN, &sql_str_length);

        // Получим значение числа столбцов
        V_OD_err = SQLNumResultCols(sqlStmtHandle, &V_OD_colanz);
        cout << "Col count: " << V_OD_colanz << endl;

        V_OD_err = SQLRowCount(sqlStmtHandle, &sql_str_length);
        cout << "Row count: " << sql_str_length << endl;

        while (SQLFetch(sqlStmtHandle) != SQL_NO_DATA) {
            //Выведем на экран версию SQL          
            cout << "Id: " << V_OD_id << ", name: " << V_OD_buffer << endl;
        }
    }

    // Закрываем соединение и выходим из программы
COMPLETED:
    SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
    SQLDisconnect(sqlConnHandle);
    SQLFreeHandle(SQL_HANDLE_DBC, sqlConnHandle);
    SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvHandle);

    // Пауза перед закрытием консольного приложения
    cout << "\nPress any key to exit...";
    getchar();
}