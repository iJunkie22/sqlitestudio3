#ifndef ABSTRACTDB3_H
#define ABSTRACTDB3_H

#include "db/abstractdb.h"
#include "parser/lexer.h"
#include "common/utils_sql.h"
#include <sqlite3.h>
#include <QThread>
#include <QPointer>
#include <QDebug>

/**
 * @brief Complete implementation of SQLite 3 driver for SQLiteStudio.
 *
 * Inherit this when implementing Db for SQLite 3. In most cases you will only need
 * to create one public constructor, which forwards parameters to the AbstractDb constructor.
 * This be sufficient to implement SQLite 3 database plugin.
 * Just link it with proper SQLite library.
 *
 * The template parameter is currently not used for anything specific, so pass any unique type name.
 * The best would be to define empty class/structure just for this purpose.
 * The parameter is there, so this class becomes a template class.
 * We need a template class so we can provide common code base for all SQLite 3 plugins, while the
 * code isn't bound to the actual SQLite library. It's only bound to the specific library when the actual
 * linking is done.
 *
 * @see DbQt
 */
template <class T>
class AbstractDb3 : public AbstractDb
{
    public:
        /**
         * @brief Creates SQLite database object.
         * @param name Name for the database.
         * @param path File path of the database.
         * @param connOptions Connection options. See AbstractDb for details.
         *
         * All values from this constructor are just passed to AbstractDb constructor.
         */
        AbstractDb3(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions);
        ~AbstractDb3();

    protected:
        bool isOpenInternal();
        void interruptExecution();
        SqlResultsPtr execInternal(const QString& query, const QList<QVariant>& args);
        SqlResultsPtr execInternal(const QString& query, const QHash<QString, QVariant>& args);
        QString getErrorTextInternal();
        int getErrorCodeInternal();
        bool openInternal();
        bool closeInternal();
        void initAfterOpen();
        QString getTypeLabel();
        bool deregisterFunction(const QString& name, int argCount);
        bool registerScalarFunction(const QString& name, int argCount);
        bool registerAggregateFunction(const QString& name, int argCount);

    private:
        class Results : public SqlResults
        {
            public:
                class Row : public SqlResultsRow
                {
                    public:
                        int init(const QStringList& columns, sqlite3_stmt* stmt);

                    private:
                        int getValue(sqlite3_stmt* stmt, int col, QVariant& value);
                };

                Results(AbstractDb3<T>* db, sqlite3_stmt* stmt, bool error);
                ~Results();

                QString getErrorText();
                int getErrorCode();
                QStringList getColumnNames();
                int columnCount();
                qint64 rowsAffected();

            protected:
                SqlResultsRowPtr nextInternal();
                bool hasNextInternal();

            private:
                int fetchNext();

                QPointer<AbstractDb3<T>> db;
                sqlite3_stmt* stmt = nullptr;
                int errorCode = SQLITE_OK;
                QString errorMessage;
                int colCount = 0;
                QStringList colNames;
                int affected = 0;
                bool rowAvailable = true;
        };

        int prepareStmt(const QString& query, sqlite3_stmt** stmt);
        int bindParam(sqlite3_stmt* stmt, int paramIdx, const QVariant& value);
        bool isValid(sqlite3_stmt* stmt) const;
        void freeStatement(sqlite3_stmt* stmt);
        QString extractLastError();
        void cleanUp();

        /**
         * @brief Stores given result in function's context.
         * @param context Custom SQL function call context.
         * @param result Value returned from function execution.
         * @param ok true if the result is from a successful execution, or false if the result contains error message (QString).
         *
         * This method is called after custom implementation of the function was evaluated and it returned the result.
         * It stores the result in function's context, so it becomes the result of the function call.
         */
        static void storeResult(sqlite3_context* context, const QVariant& result, bool ok);

        /**
         * @brief Converts SQLite arguments into the list of argument values.
         * @param argCount Number of arguments.
         * @param args SQLite argument values.
         * @return Convenient Qt list with argument values as QVariant.
         *
         * This function does necessary conversions reflecting internal SQLite datatype, so if the type
         * was for example BLOB, then the QVariant will be a QByteArray, etc.
         */
        static QList<QVariant> getArgs(int argCount, sqlite3_value** args);

        /**
         * @brief Evaluates requested function using defined implementation code and provides result.
         * @param context SQL function call context.
         * @param argCount Number of arguments passed to the function.
         * @param args Arguments passed to the function.
         *
         * This method is aware of the implementation language and the code defined for it,
         * so it delegates the execution to the proper plugin handling that language. Then it stores
         * result returned from the plugin in function's context, so it becomes function's result.
         *
         * This method is called for scalar functions.
         *
         * @see DbQt::evaluateScalar()
         */
        static void evaluateScalar(sqlite3_context* context, int argCount, sqlite3_value** args);

        /**
         * @brief Evaluates requested function using defined implementation code and provides result.
         * @param context SQL function call context.
         * @param argCount Number of arguments passed to the function.
         * @param args Arguments passed to the function.
         *
         * This method is called for aggregate functions.
         *
         * If this is the first call to this function using this context, then it will execute
         * both "initial" and then "per step" code for this function implementation.
         *
         * @see DbQt3::evaluateScalar()
         * @see DbQt::evaluateAggregateStep()
         */
        static void evaluateAggregateStep(sqlite3_context* context, int argCount, sqlite3_value** args);

        /**
         * @brief Evaluates "final" code for aggregate function.
         * @param context SQL function call context.
         *
         * This method is called for aggregate functions.
         *
         * It's called once, at the end of aggregate function evaluation.
         * It executes "final" code of the function implementation.
         */
        static void evaluateAggregateFinal(sqlite3_context* context);

        /**
         * @brief Destructor for function user data object.
         * @param dataPtr Pointer to the user data object.
         *
         * This is called by SQLite when the function is deregistered.
         */
        static void deleteUserData(void* dataPtr);

        /**
         * @brief Allocates and/or returns shared memory for the aggregate SQL function call.
         * @param context SQL function call context.
         * @return Pointer to the memory.
         *
         * It allocates exactly the number of bytes required to store pointer to a QHash.
         * The memory is released after the aggregate function is finished.
         */
        static void* getContextMemPtr(sqlite3_context* context);

        /**
         * @brief Allocates and/or returns QHash shared across all aggregate function steps.
         * @param context SQL function call context.
         * @return Shared hash table.
         *
         * The hash table is created before initial aggregate function step is made.
         * Then it's shared across all further steps (using this method to get it)
         * and then releases the memory after the last (final) step of the function call.
         */
        static QHash<QString,QVariant> getAggregateContext(sqlite3_context* context);

        /**
         * @brief Sets new value of the aggregate function shared hash table.
         * @param context SQL function call context.
         * @param aggregateContext New shared hash table value to store.
         *
         * This should be called after each time the context was requested with getAggregateContext() and then modified.
         */
        static void setAggregateContext(sqlite3_context* context, const QHash<QString,QVariant>& aggregateContext);

        /**
         * @brief Releases aggregate function shared hash table.
         * @param context SQL function call context.
         *
         * This should be called from final aggregate function step  to release the shared context (delete QHash).
         * The memory used to store pointer to the shared context will be released by the SQLite itself.
         */
        static void releaseAggregateContext(sqlite3_context* context);

        sqlite3* dbHandle = nullptr;
        QString lastError;
        int lastErrorCode = SQLITE_OK;
        QList<sqlite3_stmt*> preparedStatements;
};

//------------------------------------------------------------------------------------
// AbstractDb3
//------------------------------------------------------------------------------------

template <class T>
AbstractDb3<T>::AbstractDb3(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    AbstractDb(name, path, connOptions)
{
}

template <class T>
AbstractDb3<T>::~AbstractDb3()
{
    closeInternal();
}

template <class T>
bool AbstractDb3<T>::isOpenInternal()
{
    return dbHandle != nullptr;
}

template <class T>
void AbstractDb3<T>::interruptExecution()
{
    if (!isOpenInternal())
        return;

    sqlite3_interrupt(dbHandle);
}

template <class T>
SqlResultsPtr AbstractDb3<T>::execInternal(const QString& query, const QList<QVariant>& args)
{
    sqlite3_stmt* stmt;
    int res;
    int paramIdx;

    QList<QueryWithParamCount> queries = getQueriesWithParamCount(query, Dialect::Sqlite3);

    foreach (const QueryWithParamCount singleQuery, queries)
    {
        res = prepareStmt(singleQuery.first, &stmt);
        if (res != SQLITE_OK)
            return SqlResultsPtr(new Results(this, nullptr, true));

        for (paramIdx = 1; paramIdx <= singleQuery.second; paramIdx++)
        {
            res = bindParam(stmt, paramIdx, args[paramIdx]);
            if (res != SQLITE_OK)
            {
                extractLastError();
                return SqlResultsPtr(new Results(this, stmt, true));
            }
        }
    }
    return SqlResultsPtr(new Results(this, stmt, false));
}

template <class T>
SqlResultsPtr AbstractDb3<T>::execInternal(const QString& query, const QHash<QString, QVariant>& args)
{
    sqlite3_stmt* stmt;
    int res;
    int paramIdx;

    QList<QueryWithParamNames> queries = getQueriesWithParamNames(query, Dialect::Sqlite3);

    foreach (const QueryWithParamNames singleQuery, queries)
    {
        res = prepareStmt(singleQuery.first, &stmt);
        if (res != SQLITE_OK)
            return SqlResultsPtr(new Results(this, nullptr, true));

        paramIdx = 1;
        foreach (const QString& paramName, singleQuery.second)
        {
            if (!args.contains(paramName))
                return SqlResultsPtr(new Results(this, stmt, true));

            res = bindParam(stmt, paramIdx++, args[paramName]);
            if (res != SQLITE_OK)
            {
                extractLastError();
                return SqlResultsPtr(new Results(this, stmt, true));
            }
        }
    }

    return SqlResultsPtr(new Results(this, stmt, false));
}

template <class T>
QString AbstractDb3<T>::getErrorTextInternal()
{
    return lastError;
}

template <class T>
int AbstractDb3<T>::getErrorCodeInternal()
{
    return lastErrorCode;
}

template <class T>
bool AbstractDb3<T>::openInternal()
{
    sqlite3* handle;
    int res = sqlite3_open_v2(path.toUtf8().constData(), &handle, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, nullptr);
    if (!res == SQLITE_OK)
    {
        lastError = tr("Could not open database: %1").arg(extractLastError());
        lastErrorCode = res;
        return false;
    }
    dbHandle = handle;
    return true;
}

template <class T>
bool AbstractDb3<T>::closeInternal()
{
    if (!dbHandle)
        return false;

    cleanUp();

    int res = sqlite3_close(dbHandle);
    if (res != SQLITE_OK)
    {
        lastError = tr("Could not close database: %1").arg(extractLastError());
        lastErrorCode = res;
        qWarning() << "Error closing database. That's weird:" << lastError;
        return false;
    }
    dbHandle = nullptr;
    return true;
}

template <class T>
void AbstractDb3<T>::initAfterOpen()
{
    exec("PRAGMA foreign_keys = 1;", Flag::NO_LOCK);
    exec("PRAGMA recursive_triggers = 1;", Flag::NO_LOCK);
}

template <class T>
QString AbstractDb3<T>::getTypeLabel()
{
    return T::label;
}

template <class T>
bool AbstractDb3<T>::deregisterFunction(const QString& name, int argCount)
{
    if (!dbHandle)
        return false;

    sqlite3_create_function(dbHandle, name.toLatin1().data(), argCount, SQLITE_UTF8, 0, nullptr, nullptr, nullptr);
    return true;
}

template <class T>
bool AbstractDb3<T>::registerScalarFunction(const QString& name, int argCount)
{
    if (!dbHandle)
        return false;

    FunctionUserData* userData = new FunctionUserData;
    userData->db = this;
    userData->name = name;
    userData->argCount = argCount;

    int res = sqlite3_create_function_v2(dbHandle, name.toLatin1().data(), argCount, SQLITE_UTF8, userData,
                                         &AbstractDb3<T>::evaluateScalar,
                                         nullptr,
                                         nullptr,
                                         &AbstractDb3<T>::deleteUserData);

    return res == SQLITE_OK;
}

template <class T>
bool AbstractDb3<T>::registerAggregateFunction(const QString& name, int argCount)
{
    if (!dbHandle)
        return false;

    FunctionUserData* userData = new FunctionUserData;
    userData->db = this;
    userData->name = name;
    userData->argCount = argCount;

    int res = sqlite3_create_function_v2(dbHandle, name.toLatin1().data(), argCount, SQLITE_UTF8, userData,
                                         nullptr,
                                         &AbstractDb3<T>::evaluateAggregateStep,
                                         &AbstractDb3<T>::evaluateAggregateFinal,
                                         &AbstractDb3<T>::deleteUserData);

    return res == SQLITE_OK;
}

template <class T>
int AbstractDb3<T>::prepareStmt(const QString& query, sqlite3_stmt** stmt)
{
    const char* tail;
    QByteArray queryBytes = query.toUtf8();
    int res = sqlite3_prepare_v2(dbHandle, queryBytes.constData(), queryBytes.size(), stmt, &tail);
    if (res != SQLITE_OK)
    {
        extractLastError();
        return res;
    }

    preparedStatements << *stmt;

    if (tail && !QString::fromUtf8(tail).trimmed().isEmpty())
        qWarning() << "Executed query left with tailing contents:" << tail << ", while executing query:" << query;

    return SQLITE_OK;
}

template <class T>
int AbstractDb3<T>::bindParam(sqlite3_stmt* stmt, int paramIdx, const QVariant& value)
{
    if (value.isNull())
    {
        return sqlite3_bind_null(stmt, paramIdx);
    }

    switch (value.type())
    {
        case QVariant::ByteArray:
        {
            QByteArray ba = value.toByteArray();
            return sqlite3_bind_blob(stmt, paramIdx, ba.constData(), ba.size(), SQLITE_TRANSIENT);
        }
        case QVariant::Int:
        case QVariant::Bool:
        {
            return sqlite3_bind_int(stmt, paramIdx, value.toInt());
        }
        case QVariant::Double:
        {
            return sqlite3_bind_double(stmt, paramIdx, value.toDouble());
        }
        case QVariant::UInt:
        case QVariant::LongLong:
        {
            return sqlite3_bind_int64(stmt, paramIdx, value.toLongLong());
        }
        default:
        {
            // SQLITE_TRANSIENT makes sure that sqlite buffers the data
            QString str = value.toString();
            return sqlite3_bind_text16(stmt, paramIdx, str.utf16(), str.size() * sizeof(QChar), SQLITE_TRANSIENT);
        }
    }

    return SQLITE_MISUSE; // not going to happen
}

template <class T>
bool AbstractDb3<T>::isValid(sqlite3_stmt* stmt) const
{
    return preparedStatements.contains(stmt);
}

template <class T>
void AbstractDb3<T>::freeStatement(sqlite3_stmt* stmt)
{
    if (!isValid(stmt))
        return;

    sqlite3_finalize(stmt);
    preparedStatements.removeOne(stmt);
}

template <class T>
QString AbstractDb3<T>::extractLastError()
{
    lastErrorCode = sqlite3_extended_errcode(dbHandle);
    lastError = QString::fromUtf8(sqlite3_errmsg(dbHandle));
    return lastError;
}

template <class T>
void AbstractDb3<T>::cleanUp()
{
    foreach (sqlite3_stmt* stmt, preparedStatements)
        sqlite3_finalize(stmt);

    preparedStatements.clear();
}

template <class T>
void AbstractDb3<T>::storeResult(sqlite3_context* context, const QVariant& result, bool ok)
{
    if (!ok)
    {
        QString str = result.toString();
        sqlite3_result_error16(context, str.utf16(), str.size() * sizeof(QChar));
        return;
    }

    // Code below is a modified code from Qt (its SQLite plugin).
    if (result.isNull())
    {
        sqlite3_result_null(context);
        return;
    }

    switch (result.type())
    {
        case QVariant::ByteArray:
        {
            QByteArray ba = result.toByteArray();
            sqlite3_result_blob(context, ba.constData(), ba.size(), SQLITE_TRANSIENT);
            break;
        }
        case QVariant::Int:
        case QVariant::Bool:
        {
            sqlite3_result_int(context, result.toInt());
            break;
        }
        case QVariant::Double:
        {
            sqlite3_result_double(context, result.toDouble());
            break;
        }
        case QVariant::UInt:
        case QVariant::LongLong:
        {
            sqlite3_result_int64(context, result.toLongLong());
            break;
        }
        default:
        {
            // SQLITE_TRANSIENT makes sure that sqlite buffers the data
            QString str = result.toString();
            sqlite3_result_text16(context, str.utf16(), str.size() * sizeof(QChar), SQLITE_TRANSIENT);
            break;
        }
    }
}

template <class T>
QList<QVariant> AbstractDb3<T>::getArgs(int argCount, sqlite3_value** args)
{
    int dataType;
    QList<QVariant> results;
    QVariant value;

    // The loop below uses slightly modified code from Qt (its SQLite plugin) to extract values.
    for (int i = 0; i < argCount; i++)
    {
        dataType = sqlite3_value_type(args[i]);
        switch (dataType)
        {
            case SQLITE_INTEGER:
                value = sqlite3_value_int64(args[i]);
                break;
            case SQLITE_BLOB:
                value = QByteArray(
                            static_cast<const char*>(sqlite3_value_blob(args[i])),
                            sqlite3_value_bytes(args[i])
                            );
                break;
            case SQLITE_FLOAT:
                value = sqlite3_value_double(args[i]);
                break;
            case SQLITE_NULL:
                value = QVariant(QVariant::String);
                break;
            default:
                value = QString(
                            reinterpret_cast<const QChar*>(sqlite3_value_text16(args[i])),
                            sqlite3_value_bytes16(args[i]) / sizeof(QChar)
                            );
                break;
        }
        results << value;
    }
    return results;
}

template <class T>
void AbstractDb3<T>::evaluateScalar(sqlite3_context* context, int argCount, sqlite3_value** args)
{
    QList<QVariant> argList = getArgs(argCount, args);
    bool ok = true;
    QVariant result = AbstractDb::evaluateScalar(sqlite3_user_data(context), argList, ok);
    storeResult(context, result, ok);
}

template <class T>
void AbstractDb3<T>::evaluateAggregateStep(sqlite3_context* context, int argCount, sqlite3_value** args)
{
    void* dataPtr = sqlite3_user_data(context);
    QList<QVariant> argList = getArgs(argCount, args);
    QHash<QString,QVariant> aggregateContext = getAggregateContext(context);

    AbstractDb::evaluateAggregateStep(dataPtr, aggregateContext, argList);

    setAggregateContext(context, aggregateContext);
}

template <class T>
void AbstractDb3<T>::evaluateAggregateFinal(sqlite3_context* context)
{
    void* dataPtr = sqlite3_user_data(context);
    QHash<QString,QVariant> aggregateContext = getAggregateContext(context);

    bool ok = true;
    QVariant result = AbstractDb::evaluateAggregateFinal(dataPtr, aggregateContext, ok);

    storeResult(context, result, ok);
    releaseAggregateContext(context);
}

template <class T>
void AbstractDb3<T>::deleteUserData(void* dataPtr)
{
    if (!dataPtr)
        return;

    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);
    delete userData;
}

template <class T>
void* AbstractDb3<T>::getContextMemPtr(sqlite3_context* context)
{
    return sqlite3_aggregate_context(context, sizeof(QHash<QString,QVariant>**));
}

template <class T>
QHash<QString, QVariant> AbstractDb3<T>::getAggregateContext(sqlite3_context* context)
{
    return AbstractDb::getAggregateContext(getContextMemPtr(context));
}

template <class T>
void AbstractDb3<T>::setAggregateContext(sqlite3_context* context, const QHash<QString, QVariant>& aggregateContext)
{
    AbstractDb::setAggregateContext(getContextMemPtr(context), aggregateContext);
}

template <class T>
void AbstractDb3<T>::releaseAggregateContext(sqlite3_context* context)
{
    AbstractDb::releaseAggregateContext(getContextMemPtr(context));
}

//------------------------------------------------------------------------------------
// Results
//------------------------------------------------------------------------------------

template <class T>
AbstractDb3<T>::Results::Results(AbstractDb3<T>* db, sqlite3_stmt* stmt, bool error) :
    db(db), stmt(stmt)
{
    if (error)
    {
        errorCode = db->lastErrorCode;
        errorMessage = db->lastError;
        return;
    }

    colCount = sqlite3_column_count(stmt);
    for (int i = 0; i < colCount; i++)
        colNames << QString::fromUtf8(sqlite3_column_name(stmt, i));

    affected = sqlite3_changes(db->dbHandle);

    fetchNext();
}

template <class T>
AbstractDb3<T>::Results::~Results()
{
    if (db.isNull())
        return;

    db->freeStatement(stmt);
}

template <class T>
QString AbstractDb3<T>::Results::getErrorText()
{
    return errorMessage;
}

template <class T>
int AbstractDb3<T>::Results::getErrorCode()
{
    return errorCode;
}

template <class T>
QStringList AbstractDb3<T>::Results::getColumnNames()
{
    return colNames;
}

template <class T>
int AbstractDb3<T>::Results::columnCount()
{
    return colCount;
}

template <class T>
qint64 AbstractDb3<T>::Results::rowsAffected()
{
    return affected;
}

template <class T>
SqlResultsRowPtr AbstractDb3<T>::Results::nextInternal()
{
    Row* row = new Row;
    int res = row->init(colNames, stmt);
    if (res != SQLITE_OK)
    {
        delete row;
        errorCode = res;
        errorMessage = QString::fromUtf8(sqlite3_errmsg(db->dbHandle));
        return SqlResultsRowPtr();
    }

    fetchNext();
    return SqlResultsRowPtr(row);
}

template <class T>
bool AbstractDb3<T>::Results::hasNextInternal()
{
    return rowAvailable && !db.isNull() && db->isValid(stmt);
}

template <class T>
int AbstractDb3<T>::Results::fetchNext()
{
    if (db.isNull() || !db->isValid(stmt))
        rowAvailable = false;

    if (!rowAvailable)
        return SQLITE_MISUSE;

    rowAvailable = false;
    int res;
    int secondsSpent = 0;
    while ((res = sqlite3_step(stmt)) == SQLITE_BUSY && secondsSpent < db->getTimeout())
    {
        QThread::sleep(1);
        if (db->getTimeout() >= 0)
            secondsSpent++;
    }

    switch (res)
    {
        case SQLITE_ROW:
            rowAvailable = true;
            break;
        case SQLITE_DONE:
            // Empty pointer as no more results are available.
            break;
        default:
            errorCode = res;
            errorMessage = QString::fromUtf8(sqlite3_errmsg(db->dbHandle));
            return SQLITE_ERROR;
    }
    return SQLITE_OK;
}

//------------------------------------------------------------------------------------
// Row
//------------------------------------------------------------------------------------

template <class T>
int AbstractDb3<T>::Results::Row::init(const QStringList& columns, sqlite3_stmt* stmt)
{
    int res;
    QVariant value;
    for (int i = 0; i < columns.size(); i++)
    {
        res = getValue(stmt, i, value);
        if (res != SQLITE_OK)
            return res;

        values << value;
        valuesMap[columns[i]] = value;
    }
    return res;
}

template <class T>
int AbstractDb3<T>::Results::Row::getValue(sqlite3_stmt* stmt, int col, QVariant& value)
{
    int dataType = sqlite3_column_type(stmt, col);
    switch (dataType)
    {
        case SQLITE_INTEGER:
            value = sqlite3_column_int64(stmt, col);
            break;
        case SQLITE_BLOB:
            value = QByteArray(
                        static_cast<const char*>(sqlite3_column_blob(stmt, col)),
                        sqlite3_column_bytes(stmt, col)
                        );
            break;
        case SQLITE_FLOAT:
            value = sqlite3_column_double(stmt, col);
            break;
        case SQLITE_NULL:
            value = QVariant(QVariant::String);
            break;
        default:
            value = QString(
                            reinterpret_cast<const QChar*>(sqlite3_column_text16(stmt, col)),
                            sqlite3_column_bytes16(stmt, col) / sizeof(QChar)
                        );
            break;
    }
    return SQLITE_OK;
}

#endif // ABSTRACTDB3_H
