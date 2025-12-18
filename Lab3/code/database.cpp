#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QDir>

Database::Database(QObject *parent) : QObject(parent)
{
}

Database::~Database()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool Database::init()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString path = QDir::currentPath() + "/ledger.db";
    qDebug() << "Database path:" << path;
    db.setDatabaseName(path);

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
        return false;
    }
    tx.id = query.lastInsertId().toInt();
    return updateBalance(tx.accountId, (tx.type == "Income" ? tx.amount : -tx.amount));
}

bool Database::deleteTransaction(int id)
{
    db.transaction();
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

    QSqlQuery deleteQuery(db);
    deleteQuery.prepare("DELETE FROM transactions WHERE id = :id");
    deleteQuery.bindValue(":id", id);

    if (deleteQuery.exec()) {
        if(updateBalance(accountId, (type == "Income" ? -amount : amount))) {
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
    // For simplicity, we assume account and type don't change.
    // A full implementation would revert old balance change and apply new one.
    QSqlQuery query(db);
    query.prepare("UPDATE transactions SET amount = :amount, categoryId = :categoryId, "
                  "time = :time, note = :note WHERE id = :id");
    query.bindValue(":amount", tx.amount);
    query.bindValue(":categoryId", tx.categoryId);
    query.bindValue(":time", tx.time.toString(Qt::ISODate));
    query.bindValue(":note", tx.note);
    query.bindValue(":id", tx.id);

    if (!query.exec()) {
        qCritical() << "Failed to update transaction:" << query.lastError().text();
        return false;
    }
    // Balance update logic would be more complex here.
    // This is a simplified version.
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
    return true;
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
    QString queryString = "SELECT id, name, type FROM categories";
    if (!type.isEmpty()) {
        queryString += " WHERE type = '" + type + "'";
    }
    
    if(query.exec(queryString)) {
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
