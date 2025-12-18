#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QList>
#include <QDateTime>

// Corresponds to domain.Transaction
struct Transaction {
    int id;
    double amount;
    QString type; // "Income", "Expense", "Transfer"
    int categoryId;
    int accountId;
    QDateTime time;
    QString note;
};

// Corresponds to domain.Account
struct Account {
    int id;
    QString name;
    QString type;
    double balance;
};

// Corresponds to domain.Category
struct Category {
    int id;
    QString name;
    QString type; // "Income", "Expense"
};

// Corresponds to domain.Budget
struct Budget {
    int id;
    int categoryId;
    int month; // YYYYMM format
    double limit;
};

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool init();
    bool init(const QString &dbFilePath);

    // Transaction management
    bool addTransaction(Transaction &tx);
    bool deleteTransaction(int id);
    bool updateTransaction(const Transaction &tx);
    QList<Transaction> findTransactions(const QString &filter);
    double calculateSpent(int categoryId, int month);

    // Account management
    bool addAccount(Account &acc);
    bool updateAccount(const Account &acc);
    QList<Account> getAllAccounts();
    bool updateBalance(int accountId, double amount);

    // Category management
    bool addCategory(Category &cat);
    QList<Category> getAllCategories(const QString &type);

    // Budget management
    bool setBudget(const Budget &budget);
    Budget getBudget(int categoryId, int month);


private:
    void createTables();
    QSqlDatabase db;
    QString connectionName;
};

#endif // DATABASE_H
