#include <QtTest>
#include <QTemporaryDir>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>

#include "../database.h"

class DatabaseTests : public QObject {
    Q_OBJECT

private slots:
    // -------- Setup / init edge cases --------
    void db_init_default_usesCwdLedgerDb();
    void db_init_invalidPath_fails();

    // -------- Unit tests: Accounts / Balance (>=10) --------
    void account_addAccount_setsId();
    void account_addAccount_duplicateName_fails();
    void account_getAllAccounts_returnsInserted();
    void account_updateAccount_updatesFields();
    void account_updateAccount_duplicateName_fails();
    void account_updateBalance_increases();
    void account_updateBalance_decreases();
    void account_updateBalance_nonexistentId_fails();
    void account_updateBalance_withoutInit_fails();
    void account_balance_matchesAfterTransaction_income();
    void account_balance_matchesAfterTransaction_expense();
    void account_deleteTransaction_revertsBalance();

    // -------- Unit tests: Transactions / Budgets / Spent (>=10) --------
    void tx_addTransaction_setsId();
    void tx_findTransactions_emptyFilter_returnsAll();
    void tx_findTransactions_filterByType_incomeOnly();
    void tx_findTransactions_filterByAccountId();
    void tx_findTransactions_invalidFilter_returnsEmpty();
    void tx_deleteTransaction_nonexistent_fails();
    void tx_addTransaction_transfer_noBalanceChange();
    void tx_addTransaction_unknownType_failsAndNoInsert();
    void tx_addTransaction_badAccount_rollsBackInsert();
    void tx_updateTransaction_adjustsBalance();
    void tx_calculateSpent_empty_returns0();
    void tx_calculateSpent_onlyExpenseAndMonthCounted();
    void category_addCategory_duplicate_fails();
    void category_getAllCategories_filterType_returnsOnlyMatches();
    void budget_setAndGetBudget_roundTrip();
    void budget_getBudget_missing_returnsSentinel();
    void tx_findTransactions_sortedByTimeDesc();

    // -------- Integration tests (>=2 groups) --------
    void it_endToEnd_budgetVsSpent();
    void it_endToEnd_multiAccountAndCategoryQueries();

private:
    struct TestEnv {
        QTemporaryDir tempDir;
        QString dbPath;
        Database db;

        TestEnv() : tempDir(), dbPath(), db(nullptr) {
            QVERIFY2(tempDir.isValid(), "Failed to create temp dir");
            dbPath = QDir(tempDir.path()).filePath("test_ledger.db");
            QVERIFY2(db.init(dbPath), "Failed to init SQLite DB");
        }
    };

    static Account makeAccount(const QString &name, const QString &type, double balance) {
        Account acc;
        acc.id = -1;
        acc.name = name;
        acc.type = type;
        acc.balance = balance;
        return acc;
    }

    static Category makeCategory(const QString &name, const QString &type) {
        Category cat;
        cat.id = -1;
        cat.name = name;
        cat.type = type;
        return cat;
    }

    static Transaction makeTx(double amount,
                              const QString &type,
                              int categoryId,
                              int accountId,
                              const QDateTime &time,
                              const QString &note = QString()) {
        Transaction tx;
        tx.id = -1;
        tx.amount = amount;
        tx.type = type;
        tx.categoryId = categoryId;
        tx.accountId = accountId;
        tx.time = time;
        tx.note = note;
        return tx;
    }
};

// -------------------- Init edge cases --------------------

void DatabaseTests::db_init_default_usesCwdLedgerDb() {
    QTemporaryDir dir;
    QVERIFY2(dir.isValid(), "Failed to create temp dir");

    const QString oldCwd = QDir::currentPath();
    QVERIFY(QDir::setCurrent(dir.path()));

    {
        Database db;
        QVERIFY(db.init());
    }

    const QString dbPath = QDir(dir.path()).filePath("ledger.db");
    QVERIFY2(QFileInfo::exists(dbPath), "Expected ledger.db to be created in current working directory");

    QVERIFY(QDir::setCurrent(oldCwd));
}

void DatabaseTests::db_init_invalidPath_fails() {
    Database db;
    QVERIFY(!db.init("/tmp/this/path/should/not/exist_12345/ledger.db"));
}

// -------------------- Accounts / Balance --------------------

void DatabaseTests::account_addAccount_setsId() {
    TestEnv env;
    Account acc = makeAccount("A1", "Cash", 0.0);
    QVERIFY(env.db.addAccount(acc));
    QVERIFY(acc.id > 0);
}

void DatabaseTests::account_addAccount_duplicateName_fails() {
    TestEnv env;
    Account a1 = makeAccount("Dup", "Cash", 0.0);
    QVERIFY(env.db.addAccount(a1));

    Account a2 = makeAccount("Dup", "Bank", 10.0);
    QVERIFY(!env.db.addAccount(a2));
}

void DatabaseTests::account_getAllAccounts_returnsInserted() {
    TestEnv env;
    Account a1 = makeAccount("A1", "Cash", 1.0);
    Account a2 = makeAccount("A2", "Bank", 2.0);
    QVERIFY(env.db.addAccount(a1));
    QVERIFY(env.db.addAccount(a2));

    const QList<Account> accounts = env.db.getAllAccounts();
    QCOMPARE(accounts.size(), 2);

    QStringList names;
    for (const auto &a : accounts) names << a.name;
    QVERIFY(names.contains("A1"));
    QVERIFY(names.contains("A2"));
}

void DatabaseTests::account_updateAccount_updatesFields() {
    TestEnv env;
    Account acc = makeAccount("Old", "Cash", 5.0);
    QVERIFY(env.db.addAccount(acc));

    acc.name = "New";
    acc.type = "Bank";
    acc.balance = 99.0;
    QVERIFY(env.db.updateAccount(acc));

    const QList<Account> accounts = env.db.getAllAccounts();
    QCOMPARE(accounts.size(), 1);
    QCOMPARE(accounts[0].name, QString("New"));
    QCOMPARE(accounts[0].type, QString("Bank"));
    QCOMPARE(accounts[0].balance, 99.0);
}

void DatabaseTests::account_updateAccount_duplicateName_fails() {
    TestEnv env;
    Account a1 = makeAccount("A1", "Cash", 0.0);
    Account a2 = makeAccount("A2", "Cash", 0.0);
    QVERIFY(env.db.addAccount(a1));
    QVERIFY(env.db.addAccount(a2));

    a2.name = a1.name;
    QVERIFY(!env.db.updateAccount(a2));
}

void DatabaseTests::account_updateBalance_increases() {
    TestEnv env;
    Account acc = makeAccount("A", "Cash", 10.0);
    QVERIFY(env.db.addAccount(acc));

    QVERIFY(env.db.updateBalance(acc.id, 2.5));
    const auto accounts = env.db.getAllAccounts();
    QCOMPARE(accounts[0].balance, 12.5);
}

void DatabaseTests::account_updateBalance_decreases() {
    TestEnv env;
    Account acc = makeAccount("A", "Cash", 10.0);
    QVERIFY(env.db.addAccount(acc));

    QVERIFY(env.db.updateBalance(acc.id, -3.0));
    const auto accounts = env.db.getAllAccounts();
    QCOMPARE(accounts[0].balance, 7.0);
}

void DatabaseTests::account_updateBalance_nonexistentId_fails() {
    TestEnv env;
    QVERIFY(!env.db.updateBalance(9999, 10.0));
    QCOMPARE(env.db.getAllAccounts().size(), 0);
}

void DatabaseTests::account_updateBalance_withoutInit_fails() {
    Database db;
    QVERIFY(!db.updateBalance(1, 1.0));
}

void DatabaseTests::account_balance_matchesAfterTransaction_income() {
    TestEnv env;
    Account acc = makeAccount("A", "Cash", 0.0);
    QVERIFY(env.db.addAccount(acc));

    Category cat = makeCategory("Salary", "Income");
    QVERIFY(env.db.addCategory(cat));

    Transaction tx = makeTx(100.0, "Income", cat.id, acc.id, QDateTime::currentDateTimeUtc());
    QVERIFY(env.db.addTransaction(tx));

    const auto accounts = env.db.getAllAccounts();
    QCOMPARE(accounts[0].balance, 100.0);
}

void DatabaseTests::account_balance_matchesAfterTransaction_expense() {
    TestEnv env;
    Account acc = makeAccount("A", "Cash", 50.0);
    QVERIFY(env.db.addAccount(acc));

    Category cat = makeCategory("Food", "Expense");
    QVERIFY(env.db.addCategory(cat));

    Transaction tx = makeTx(20.0, "Expense", cat.id, acc.id, QDateTime::currentDateTimeUtc());
    QVERIFY(env.db.addTransaction(tx));

    const auto accounts = env.db.getAllAccounts();
    QCOMPARE(accounts[0].balance, 30.0);
}

void DatabaseTests::account_deleteTransaction_revertsBalance() {
    TestEnv env;
    Account acc = makeAccount("A", "Cash", 50.0);
    QVERIFY(env.db.addAccount(acc));

    Category cat = makeCategory("Food", "Expense");
    QVERIFY(env.db.addCategory(cat));

    Transaction tx = makeTx(20.0, "Expense", cat.id, acc.id, QDateTime::currentDateTimeUtc());
    QVERIFY(env.db.addTransaction(tx));
    QCOMPARE(env.db.getAllAccounts()[0].balance, 30.0);

    QVERIFY(env.db.deleteTransaction(tx.id));
    QCOMPARE(env.db.getAllAccounts()[0].balance, 50.0);
}

// -------------------- Transactions / Budgets / Spent --------------------

void DatabaseTests::tx_addTransaction_setsId() {
    TestEnv env;
    Account acc = makeAccount("A", "Cash", 0.0);
    QVERIFY(env.db.addAccount(acc));
    Category cat = makeCategory("Salary", "Income");
    QVERIFY(env.db.addCategory(cat));

    Transaction tx = makeTx(1.0, "Income", cat.id, acc.id, QDateTime::currentDateTimeUtc());
    QVERIFY(env.db.addTransaction(tx));
    QVERIFY(tx.id > 0);
}

void DatabaseTests::tx_findTransactions_emptyFilter_returnsAll() {
    TestEnv env;
    Account acc = makeAccount("A", "Cash", 0.0);
    QVERIFY(env.db.addAccount(acc));
    Category cat = makeCategory("Food", "Expense");
    QVERIFY(env.db.addCategory(cat));

    Transaction t1 = makeTx(1.0, "Expense", cat.id, acc.id, QDateTime::currentDateTimeUtc().addSecs(-10));
    Transaction t2 = makeTx(2.0, "Expense", cat.id, acc.id, QDateTime::currentDateTimeUtc());
    QVERIFY(env.db.addTransaction(t1));
    QVERIFY(env.db.addTransaction(t2));

    const auto list = env.db.findTransactions(QString());
    QCOMPARE(list.size(), 2);
}

void DatabaseTests::tx_findTransactions_filterByType_incomeOnly() {
    TestEnv env;
    Account acc = makeAccount("A", "Cash", 0.0);
    QVERIFY(env.db.addAccount(acc));

    Category incomeCat = makeCategory("Salary", "Income");
    Category expenseCat = makeCategory("Food", "Expense");
    QVERIFY(env.db.addCategory(incomeCat));
    QVERIFY(env.db.addCategory(expenseCat));

    Transaction in = makeTx(10.0, "Income", incomeCat.id, acc.id, QDateTime::currentDateTimeUtc());
    Transaction ex = makeTx(5.0, "Expense", expenseCat.id, acc.id, QDateTime::currentDateTimeUtc());
    QVERIFY(env.db.addTransaction(in));
    QVERIFY(env.db.addTransaction(ex));

    const auto list = env.db.findTransactions("type = 'Income'");
    QCOMPARE(list.size(), 1);
    QCOMPARE(list[0].type, QString("Income"));
}

void DatabaseTests::tx_findTransactions_filterByAccountId() {
    TestEnv env;
    Account a1 = makeAccount("A1", "Cash", 0.0);
    Account a2 = makeAccount("A2", "Cash", 0.0);
    QVERIFY(env.db.addAccount(a1));
    QVERIFY(env.db.addAccount(a2));

    Category cat = makeCategory("Food", "Expense");
    QVERIFY(env.db.addCategory(cat));

    Transaction t1 = makeTx(1.0, "Expense", cat.id, a1.id, QDateTime::currentDateTimeUtc());
    Transaction t2 = makeTx(2.0, "Expense", cat.id, a2.id, QDateTime::currentDateTimeUtc());
    QVERIFY(env.db.addTransaction(t1));
    QVERIFY(env.db.addTransaction(t2));

    const auto list = env.db.findTransactions(QString("accountId = %1").arg(a1.id));
    QCOMPARE(list.size(), 1);
    QCOMPARE(list[0].accountId, a1.id);
}

void DatabaseTests::tx_findTransactions_invalidFilter_returnsEmpty() {
    TestEnv env;
    const auto list = env.db.findTransactions("THIS_IS_NOT_VALID_SQL");
    QCOMPARE(list.size(), 0);
}

void DatabaseTests::tx_deleteTransaction_nonexistent_fails() {
    TestEnv env;
    QVERIFY(!env.db.deleteTransaction(123456));
}

void DatabaseTests::tx_addTransaction_transfer_noBalanceChange() {
    TestEnv env;
    Account acc = makeAccount("A", "Cash", 10.0);
    QVERIFY(env.db.addAccount(acc));

    Category cat = makeCategory("TransferCat", "Expense");
    QVERIFY(env.db.addCategory(cat));

    Transaction tx = makeTx(5.0, "Transfer", cat.id, acc.id, QDateTime::currentDateTimeUtc());
    QVERIFY(env.db.addTransaction(tx));
    QCOMPARE(env.db.getAllAccounts()[0].balance, 10.0);

    QVERIFY(env.db.deleteTransaction(tx.id));
    QCOMPARE(env.db.getAllAccounts()[0].balance, 10.0);
}

void DatabaseTests::tx_addTransaction_unknownType_failsAndNoInsert() {
    TestEnv env;
    Account acc = makeAccount("A", "Cash", 10.0);
    QVERIFY(env.db.addAccount(acc));
    Category cat = makeCategory("Any", "Expense");
    QVERIFY(env.db.addCategory(cat));

    Transaction tx = makeTx(5.0, "WeirdType", cat.id, acc.id, QDateTime::currentDateTimeUtc());
    QVERIFY(!env.db.addTransaction(tx));
    QCOMPARE(env.db.findTransactions(QString()).size(), 0);
    QCOMPARE(env.db.getAllAccounts()[0].balance, 10.0);
}

void DatabaseTests::tx_addTransaction_badAccount_rollsBackInsert() {
    TestEnv env;
    Category cat = makeCategory("Any", "Expense");
    QVERIFY(env.db.addCategory(cat));

    Transaction tx = makeTx(5.0, "Expense", cat.id, 999999, QDateTime::currentDateTimeUtc());
    QVERIFY(!env.db.addTransaction(tx));
    QCOMPARE(env.db.findTransactions(QString()).size(), 0);
}

void DatabaseTests::tx_updateTransaction_adjustsBalance() {
    TestEnv env;
    Account acc = makeAccount("A", "Cash", 0.0);
    QVERIFY(env.db.addAccount(acc));
    Category cat = makeCategory("C", "Expense");
    QVERIFY(env.db.addCategory(cat));

    Transaction tx = makeTx(10.0, "Expense", cat.id, acc.id, QDateTime::currentDateTimeUtc());
    QVERIFY(env.db.addTransaction(tx));
    QCOMPARE(env.db.getAllAccounts()[0].balance, -10.0);

    tx.type = "Income";
    tx.amount = 10.0;
    QVERIFY(env.db.updateTransaction(tx));
    QCOMPARE(env.db.getAllAccounts()[0].balance, 10.0);
}

void DatabaseTests::tx_calculateSpent_empty_returns0() {
    TestEnv env;
    QCOMPARE(env.db.calculateSpent(1, 202512), 0.0);
}

void DatabaseTests::tx_calculateSpent_onlyExpenseAndMonthCounted() {
    TestEnv env;
    Account acc = makeAccount("A", "Cash", 0.0);
    QVERIFY(env.db.addAccount(acc));

    Category food = makeCategory("Food", "Expense");
    Category salary = makeCategory("Salary", "Income");
    QVERIFY(env.db.addCategory(food));
    QVERIFY(env.db.addCategory(salary));

    // Month under test: 2025-12
    const QDateTime dec10(QDate(2025, 12, 10), QTime(12, 0), Qt::UTC);
    const QDateTime jan05(QDate(2026, 1, 5), QTime(12, 0), Qt::UTC);

    Transaction e1 = makeTx(10.0, "Expense", food.id, acc.id, dec10);
    Transaction e2 = makeTx(7.0, "Expense", food.id, acc.id, dec10.addDays(1));
    Transaction in = makeTx(100.0, "Income", salary.id, acc.id, dec10);
    Transaction otherMonth = makeTx(20.0, "Expense", food.id, acc.id, jan05);

    QVERIFY(env.db.addTransaction(e1));
    QVERIFY(env.db.addTransaction(e2));
    QVERIFY(env.db.addTransaction(in));
    QVERIFY(env.db.addTransaction(otherMonth));

    QCOMPARE(env.db.calculateSpent(food.id, 202512), 17.0);
}

void DatabaseTests::category_addCategory_duplicate_fails() {
    TestEnv env;
    Category cat = makeCategory("DupCat", "Expense");
    QVERIFY(env.db.addCategory(cat));

    Category cat2 = makeCategory("DupCat", "Expense");
    QVERIFY(!env.db.addCategory(cat2));
}

void DatabaseTests::category_getAllCategories_filterType_returnsOnlyMatches() {
    TestEnv env;
    Category income = makeCategory("Salary", "Income");
    Category expense = makeCategory("Food", "Expense");
    QVERIFY(env.db.addCategory(income));
    QVERIFY(env.db.addCategory(expense));

    const auto incomeOnly = env.db.getAllCategories("Income");
    QCOMPARE(incomeOnly.size(), 1);
    QCOMPARE(incomeOnly[0].type, QString("Income"));
}

void DatabaseTests::budget_setAndGetBudget_roundTrip() {
    TestEnv env;
    Category food = makeCategory("Food", "Expense");
    QVERIFY(env.db.addCategory(food));

    Budget b;
    b.id = -1;
    b.categoryId = food.id;
    b.month = 202512;
    b.limit = 123.45;

    QVERIFY(env.db.setBudget(b));

    const Budget loaded = env.db.getBudget(food.id, 202512);
    QVERIFY(loaded.id > 0);
    QCOMPARE(loaded.categoryId, food.id);
    QCOMPARE(loaded.month, 202512);
    QCOMPARE(loaded.limit, 123.45);
}

void DatabaseTests::budget_getBudget_missing_returnsSentinel() {
    TestEnv env;
    const Budget b = env.db.getBudget(999, 202512);
    QCOMPARE(b.id, -1);
    QCOMPARE(b.categoryId, -1);
    QCOMPARE(b.month, -1);
    QCOMPARE(b.limit, 0.0);
}

void DatabaseTests::tx_findTransactions_sortedByTimeDesc() {
    TestEnv env;
    Account acc = makeAccount("A", "Cash", 0.0);
    QVERIFY(env.db.addAccount(acc));
    Category cat = makeCategory("Food", "Expense");
    QVERIFY(env.db.addCategory(cat));

    Transaction oldTx = makeTx(1.0, "Expense", cat.id, acc.id, QDateTime::currentDateTimeUtc().addSecs(-100));
    Transaction newTx = makeTx(2.0, "Expense", cat.id, acc.id, QDateTime::currentDateTimeUtc());
    QVERIFY(env.db.addTransaction(oldTx));
    QVERIFY(env.db.addTransaction(newTx));

    const auto list = env.db.findTransactions(QString());
    QCOMPARE(list.size(), 2);
    QVERIFY(list[0].time >= list[1].time);
}

// -------------------- Integration tests --------------------

void DatabaseTests::it_endToEnd_budgetVsSpent() {
    TestEnv env;
    Account acc = makeAccount("Main", "Cash", 0.0);
    QVERIFY(env.db.addAccount(acc));

    Category food = makeCategory("Food", "Expense");
    QVERIFY(env.db.addCategory(food));

    Budget b;
    b.id = -1;
    b.categoryId = food.id;
    b.month = 202512;
    b.limit = 50.0;
    QVERIFY(env.db.setBudget(b));

    const QDateTime dec02(QDate(2025, 12, 2), QTime(8, 0), Qt::UTC);
    Transaction t1 = makeTx(10.0, "Expense", food.id, acc.id, dec02);
    Transaction t2 = makeTx(15.0, "Expense", food.id, acc.id, dec02.addDays(1));
    QVERIFY(env.db.addTransaction(t1));
    QVERIFY(env.db.addTransaction(t2));

    const double spent = env.db.calculateSpent(food.id, 202512);
    QCOMPARE(spent, 25.0);

    const Budget loaded = env.db.getBudget(food.id, 202512);
    QVERIFY(loaded.limit >= spent);
}

void DatabaseTests::it_endToEnd_multiAccountAndCategoryQueries() {
    TestEnv env;

    Account cash = makeAccount("Cash", "Cash", 0.0);
    Account bank = makeAccount("Bank", "Bank", 0.0);
    QVERIFY(env.db.addAccount(cash));
    QVERIFY(env.db.addAccount(bank));

    Category salary = makeCategory("Salary", "Income");
    Category food = makeCategory("Food", "Expense");
    QVERIFY(env.db.addCategory(salary));
    QVERIFY(env.db.addCategory(food));

    const QDateTime now = QDateTime::currentDateTimeUtc();
    Transaction s1 = makeTx(100.0, "Income", salary.id, bank.id, now.addSecs(-10));
    Transaction e1 = makeTx(20.0, "Expense", food.id, cash.id, now.addSecs(-5));
    Transaction e2 = makeTx(5.0, "Expense", food.id, cash.id, now);

    QVERIFY(env.db.addTransaction(s1));
    QVERIFY(env.db.addTransaction(e1));
    QVERIFY(env.db.addTransaction(e2));

    const auto expenses = env.db.findTransactions("type = 'Expense'");
    QCOMPARE(expenses.size(), 2);

    const auto cashOnly = env.db.findTransactions(QString("accountId = %1").arg(cash.id));
    QCOMPARE(cashOnly.size(), 2);
}

QTEST_MAIN(DatabaseTests)
#include "tst_database.moc"
