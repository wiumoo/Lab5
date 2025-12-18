#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QDate>
#include <QDir>
#include <QUuid>

namespace {
enum class TxType {
    Income,
    Expense,
    Transfer,
    Unknown,
};

TxType parseTxType(const QString &type) {
    if (type == QStringLiteral("Income")) return TxType::Income;
    if (type == QStringLiteral("Expense")) return TxType::Expense;
    if (type == QStringLiteral("Transfer")) return TxType::Transfer;
    return TxType::Unknown;
}

double balanceDeltaFor(TxType type, double amount) {
    switch (type) {
    case TxType::Income:
        return amount;
    case TxType::Expense:
        return -amount;
    case TxType::Transfer:
        return 0.0;
    case TxType::Unknown:
        return 0.0;
    }
    return 0.0;
}
}

Database::Database(QObject *parent) : QObject(parent)
{
}

Database::~Database()
{
    if (db.isOpen()) {
        db.close();
    }

    // Ensure the connection is removed after the QSqlDatabase handle is closed.
    // Important: Qt requires that all QSqlDatabase handles referencing a
    // connection are destroyed before removeDatabase() is called.
    // Resetting db releases the handle and prevents warnings in unit tests.
    if (!connectionName.isEmpty()) {
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
        connectionName.clear();
    }
}

bool Database::init()
{
    const QString path = QDir::currentPath() + "/ledger.db";
    return init(path);
}

bool Database::init(const QString &dbFilePath)
{
    // Use a unique connection name per Database instance.
    // If we used the default connection, multiple Database objects in a single
    // process (like unit tests) would overwrite each other.
    if (connectionName.isEmpty()) {
        connectionName = QStringLiteral("ledger_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    }

    db = QSqlDatabase::addDatabase("QSQLITE", connectionName);

    qDebug() << "Database path:" << dbFilePath;
    db.setDatabaseName(dbFilePath);

    if (!db.open()) {
        qCritical() << "Database connection failed:" << db.lastError().text();
        return false;
    }

    createTables();
    return true;
}

void Database::createTables()
{
    QSqlQuery query(db);
    QStringList tables = db.tables();

    if (!tables.contains("transactions")) {
        if(!query.exec("CREATE TABLE transactions ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "amount REAL NOT NULL, "
                      "type TEXT NOT NULL, "
                      "categoryId INTEGER, "
                      "accountId INTEGER NOT NULL, "
                      "time TEXT NOT NULL, "
                      "note TEXT)")) {
            qCritical() << "Failed to create table transactions:" << query.lastError().text();
        }
    }

    if (!tables.contains("accounts")) {
        if(!query.exec("CREATE TABLE accounts ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "name TEXT NOT NULL UNIQUE, "
                      "type TEXT NOT NULL, "
                      "balance REAL NOT NULL DEFAULT 0)")) {
            qCritical() << "Failed to create table accounts:" << query.lastError().text();
        }
    }

    if (!tables.contains("categories")) {
        if(!query.exec("CREATE TABLE categories ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "name TEXT NOT NULL UNIQUE, "
                      "type TEXT NOT NULL)")) {
            qCritical() << "Failed to create table categories:" << query.lastError().text();
        }
    }

    if (!tables.contains("budgets")) {
        if(!query.exec("CREATE TABLE budgets ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "categoryId INTEGER NOT NULL, "
                      "month INTEGER NOT NULL, "
                      "limit_amount REAL NOT NULL, "
                      "UNIQUE(categoryId, month))")) {
            qCritical() << "Failed to create table budgets:" << query.lastError().text();
        }
    }
}

bool Database::addTransaction(Transaction &tx)
{
    const TxType txType = parseTxType(tx.type);
    if (txType == TxType::Unknown) {
        qCritical() << "Unsupported transaction type:" << tx.type;
        return false;
    }

    if (!db.transaction()) {
        qCritical() << "Failed to start DB transaction:" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO transactions (amount, type, categoryId, accountId, time, note) "
                  "VALUES (:amount, :type, :categoryId, :accountId, :time, :note)");
    query.bindValue(":amount", tx.amount);
    query.bindValue(":type", tx.type);
    query.bindValue(":categoryId", tx.categoryId);
    query.bindValue(":accountId", tx.accountId);
    query.bindValue(":time", tx.time.toString(Qt::ISODate));
    query.bindValue(":note", tx.note);

    if (!query.exec()) {
        qCritical() << "Failed to add transaction:" << query.lastError().text();
        db.rollback();
        return false;
    }
    tx.id = query.lastInsertId().toInt();

    const double delta = balanceDeltaFor(txType, tx.amount);
    if (!updateBalance(tx.accountId, delta)) {
        db.rollback();
        return false;
    }

    if (!db.commit()) {
        qCritical() << "Failed to commit addTransaction:" << db.lastError().text();
        db.rollback();
        return false;
    }
    return true;
}

bool Database::deleteTransaction(int id)
{
    if (!db.transaction()) {
        qCritical() << "Failed to start DB transaction:" << db.lastError().text();
        return false;
    }
    QSqlQuery selectQuery(db);
    selectQuery.prepare("SELECT amount, type, accountId FROM transactions WHERE id = :id");
    selectQuery.bindValue(":id", id);
    if (!selectQuery.exec() || !selectQuery.next()) {
        qCritical() << "Failed to retrieve transaction for deletion:" << selectQuery.lastError().text();
        db.rollback();
        return false;
    }

    double amount = selectQuery.value("amount").toDouble();
    QString type = selectQuery.value("type").toString();
    int accountId = selectQuery.value("accountId").toInt();

    const TxType txType = parseTxType(type);
    if (txType == TxType::Unknown) {
        qCritical() << "Unsupported transaction type:" << type;
        db.rollback();
        return false;
    }

    QSqlQuery deleteQuery(db);
    deleteQuery.prepare("DELETE FROM transactions WHERE id = :id");
    deleteQuery.bindValue(":id", id);

    if (deleteQuery.exec() && deleteQuery.numRowsAffected() == 1) {
        const double deltaApplied = balanceDeltaFor(txType, amount);
        if(updateBalance(accountId, -deltaApplied)) {
            db.commit();
            return true;
        }
    }
    
    qCritical() << "Failed to delete transaction:" << deleteQuery.lastError().text();
    db.rollback();
    return false;
}

bool Database::updateTransaction(const Transaction &tx)
{
    if (!db.transaction()) {
        qCritical() << "Failed to start DB transaction:" << db.lastError().text();
        return false;
    }

    QSqlQuery oldQuery(db);
    oldQuery.prepare("SELECT amount, type, accountId FROM transactions WHERE id = :id");
    oldQuery.bindValue(":id", tx.id);
    if (!oldQuery.exec() || !oldQuery.next()) {
        qCritical() << "Failed to retrieve old transaction:" << oldQuery.lastError().text();
        db.rollback();
        return false;
    }

    const double oldAmount = oldQuery.value("amount").toDouble();
    const QString oldTypeStr = oldQuery.value("type").toString();
    const int oldAccountId = oldQuery.value("accountId").toInt();

    const TxType oldType = parseTxType(oldTypeStr);
    const TxType newType = parseTxType(tx.type);
    if (oldType == TxType::Unknown || newType == TxType::Unknown) {
        qCritical() << "Unsupported transaction type:" << oldTypeStr << tx.type;
        db.rollback();
        return false;
    }

    const double oldDelta = balanceDeltaFor(oldType, oldAmount);
    const double newDelta = balanceDeltaFor(newType, tx.amount);

    QSqlQuery query(db);
    query.prepare("UPDATE transactions SET amount = :amount, type = :type, categoryId = :categoryId, "
                  "accountId = :accountId, time = :time, note = :note WHERE id = :id");
    query.bindValue(":amount", tx.amount);
    query.bindValue(":type", tx.type);
    query.bindValue(":categoryId", tx.categoryId);
    query.bindValue(":accountId", tx.accountId);
    query.bindValue(":time", tx.time.toString(Qt::ISODate));
    query.bindValue(":note", tx.note);
    query.bindValue(":id", tx.id);

    if (!query.exec() || query.numRowsAffected() != 1) {
        qCritical() << "Failed to update transaction:" << query.lastError().text();
        db.rollback();
        return false;
    }

    if (tx.accountId == oldAccountId) {
        if (!updateBalance(oldAccountId, newDelta - oldDelta)) {
            db.rollback();
            return false;
        }
    } else {
        if (!updateBalance(oldAccountId, -oldDelta)) {
            db.rollback();
            return false;
        }
        if (!updateBalance(tx.accountId, newDelta)) {
            db.rollback();
            return false;
        }
    }

    if (!db.commit()) {
        qCritical() << "Failed to commit updateTransaction:" << db.lastError().text();
        db.rollback();
        return false;
    }

    return true;
}

QList<Transaction> Database::findTransactions(const QString &filter)
{
    QList<Transaction> transactions;
    QSqlQuery query(db);
    QString queryString = "SELECT id, amount, type, categoryId, accountId, time, note FROM transactions";
    if (!filter.isEmpty()) {
        queryString += " WHERE " + filter;
    }
    queryString += " ORDER BY time DESC";

    if(query.exec(queryString)) {
        while (query.next()) {
            Transaction tx;
            tx.id = query.value("id").toInt();
            tx.amount = query.value("amount").toDouble();
            tx.type = query.value("type").toString();
            tx.categoryId = query.value("categoryId").toInt();
            tx.accountId = query.value("accountId").toInt();
            tx.time = QDateTime::fromString(query.value("time").toString(), Qt::ISODate);
            tx.note = query.value("note").toString();
            transactions.append(tx);
        }
    } else {
        qCritical() << "Failed to find transactions:" << query.lastError().text();
    }
    return transactions;
}

double Database::calculateSpent(int categoryId, int month)
{
    QSqlQuery query(db);
    QDate date = QDate::fromString(QString::number(month) + "01", "yyyyMMdd");
    QString startDate = date.toString("yyyy-MM-01T00:00:00");
    QString endDate = date.addMonths(1).toString("yyyy-MM-01T00:00:00");

    query.prepare("SELECT SUM(amount) FROM transactions WHERE type = 'Expense' AND categoryId = :categoryId "
                  "AND time >= :startDate AND time < :endDate");
    query.bindValue(":categoryId", categoryId);
    query.bindValue(":startDate", startDate);
    query.bindValue(":endDate", endDate);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    qCritical() << "Failed to calculate spent:" << query.lastError().text();
    return 0.0;
}


bool Database::addAccount(Account &acc)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO accounts (name, type, balance) VALUES (:name, :type, :balance)");
    query.bindValue(":name", acc.name);
    query.bindValue(":type", acc.type);
    query.bindValue(":balance", acc.balance);

    if (!query.exec()) {
        qCritical() << "Failed to add account:" << query.lastError().text();
        return false;
    }
    acc.id = query.lastInsertId().toInt();
    return true;
}

bool Database::updateAccount(const Account &acc)
{
    QSqlQuery query(db);
    query.prepare("UPDATE accounts SET name = :name, type = :type, balance = :balance WHERE id = :id");
    query.bindValue(":name", acc.name);
    query.bindValue(":type", acc.type);
    query.bindValue(":balance", acc.balance);
    query.bindValue(":id", acc.id);

    if (!query.exec()) {
        qCritical() << "Failed to update account:" << query.lastError().text();
        return false;
    }
    return true;
}

QList<Account> Database::getAllAccounts()
{
    QList<Account> accounts;
    QSqlQuery query("SELECT id, name, type, balance FROM accounts", db);
    while (query.next()) {
        Account acc;
        acc.id = query.value("id").toInt();
        acc.name = query.value("name").toString();
        acc.type = query.value("type").toString();
        acc.balance = query.value("balance").toDouble();
        accounts.append(acc);
    }
    return accounts;
}

bool Database::updateBalance(int accountId, double amount)
{
    QSqlQuery query(db);
    query.prepare("UPDATE accounts SET balance = balance + :amount WHERE id = :id");
    query.bindValue(":amount", amount);
    query.bindValue(":id", accountId);
    if (!query.exec()) {
        qCritical() << "Failed to update balance:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() == 1;
}

bool Database::addCategory(Category &cat)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO categories (name, type) VALUES (:name, :type)");
    query.bindValue(":name", cat.name);
    query.bindValue(":type", cat.type);
    if (!query.exec()) {
        qCritical() << "Failed to add category:" << query.lastError().text();
        return false;
    }
    cat.id = query.lastInsertId().toInt();
    return true;
}

QList<Category> Database::getAllCategories(const QString &type)
{
    QList<Category> categories;
    QSqlQuery query(db);
    if (type.isEmpty()) {
        query.prepare("SELECT id, name, type FROM categories");
    } else {
        query.prepare("SELECT id, name, type FROM categories WHERE type = :type");
        query.bindValue(":type", type);
    }

    if(query.exec()) {
        while (query.next()) {
            Category cat;
            cat.id = query.value("id").toInt();
            cat.name = query.value("name").toString();
            cat.type = query.value("type").toString();
            categories.append(cat);
        }
    } else {
        qCritical() << "Failed to get categories:" << query.lastError().text();
    }
    return categories;
}

bool Database::setBudget(const Budget &budget)
{
    QSqlQuery query(db);
    // Use INSERT OR REPLACE to handle both new and existing budgets
    query.prepare("INSERT OR REPLACE INTO budgets (categoryId, month, limit_amount) "
                  "VALUES (:categoryId, :month, :limit_amount)");
    query.bindValue(":categoryId", budget.categoryId);
    query.bindValue(":month", budget.month);
    query.bindValue(":limit_amount", budget.limit);

    if (!query.exec()) {
        qCritical() << "Failed to set budget:" << query.lastError().text();
        return false;
    }
    return true;
}

Budget Database::getBudget(int categoryId, int month)
{
    Budget budget = {-1, -1, -1, 0.0};
    QSqlQuery query(db);
    query.prepare("SELECT id, limit_amount FROM budgets WHERE categoryId = :categoryId AND month = :month");
    query.bindValue(":categoryId", categoryId);
    query.bindValue(":month", month);

    if (query.exec() && query.next()) {
        budget.id = query.value("id").toInt();
        budget.categoryId = categoryId;
        budget.month = month;
        budget.limit = query.value("limit_amount").toDouble();
    }
    return budget;
}
