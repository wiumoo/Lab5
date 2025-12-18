#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    if (!m_db.init()) {
        QMessageBox::critical(this, "Error", "Failed to initialize database. The application will close.");
        QApplication::quit();
        return;
    }

    setupUiElements();
    loadInitialData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUiElements()
{
    // Main Tab setup
    ui->tabWidget->setCurrentIndex(0);

    // Transactions Tab
    ui->transactionTypeBox->addItems({"Expense", "Income"});
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    ui->transactionsTable->setColumnCount(6);
    ui->transactionsTable->setHorizontalHeaderLabels({"ID", "Time", "Type", "Category", "Amount", "Note"});
    ui->transactionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->transactionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);


    // Accounts Tab
    ui->accountsTable->setColumnCount(3);
    ui->accountsTable->setHorizontalHeaderLabels({"ID", "Name", "Balance"});
    ui->accountTypeBox->addItems({"Cash", "Bank", "CreditCard", "Alipay", "WeChat"});

    // Categories Tab
    ui->categoriesTable->setColumnCount(3);
    ui->categoriesTable->setHorizontalHeaderLabels({"ID", "Name", "Type"});
    ui->categoryTypeBox->addItems({"Expense", "Income"});

    // Budgets Tab
    ui->budgetMonthEdit->setDateTime(QDateTime::currentDateTime());
    ui->budgetsTable->setColumnCount(4);
    ui->budgetsTable->setHorizontalHeaderLabels({"Category", "Month", "Limit", "Spent"});
}

void MainWindow::loadInitialData()
{
    refreshTransactionView();
    refreshAccountView();
    refreshCategoryView();
    refreshBudgetView();
    populateCategoryComboBox("Expense");
}

void MainWindow::refreshTransactionView()
{
    m_currentTransactions = m_db.findTransactions("");
    ui->transactionsTable->setRowCount(m_currentTransactions.size());

    int row = 0;
    for (const auto &tx : m_currentTransactions) {
        ui->transactionsTable->setItem(row, 0, new QTableWidgetItem(QString::number(tx.id)));
        ui->transactionsTable->setItem(row, 1, new QTableWidgetItem(tx.time.toString("yyyy-MM-dd hh:mm")));
        ui->transactionsTable->setItem(row, 2, new QTableWidgetItem(tx.type));
        
        // Get category name from ID
        QList<Category> cats = m_db.getAllCategories("");
        QString catName = "N/A";
        for(const auto& cat : cats){
            if(cat.id == tx.categoryId) {
                catName = cat.name;
                break;
            }
        }
        ui->transactionsTable->setItem(row, 3, new QTableWidgetItem(catName));
        ui->transactionsTable->setItem(row, 4, new QTableWidgetItem(QString::number(tx.amount, 'f', 2)));
        ui->transactionsTable->setItem(row, 5, new QTableWidgetItem(tx.note));
        row++;
    }
    ui->transactionsTable->resizeColumnsToContents();
}

void MainWindow::refreshAccountView()
{
    QList<Account> accounts = m_db.getAllAccounts();
    ui->accountsTable->setRowCount(accounts.size());
    ui->accountComboBox->clear();

    int row = 0;
    for (const auto &acc : accounts) {
        ui->accountsTable->setItem(row, 0, new QTableWidgetItem(QString::number(acc.id)));
        ui->accountsTable->setItem(row, 1, new QTableWidgetItem(acc.name));
        ui->accountsTable->setItem(row, 2, new QTableWidgetItem(QString::number(acc.balance, 'f', 2)));
        ui->accountComboBox->addItem(acc.name, acc.id);
        row++;
    }
    ui->accountsTable->resizeColumnsToContents();
}

void MainWindow::refreshCategoryView()
{
    QList<Category> categories = m_db.getAllCategories("");
    ui->categoriesTable->setRowCount(categories.size());
    ui->budgetCategoryBox->clear();

    int row = 0;
    for (const auto &cat : categories) {
        ui->categoriesTable->setItem(row, 0, new QTableWidgetItem(QString::number(cat.id)));
        ui->categoriesTable->setItem(row, 1, new QTableWidgetItem(cat.name));
        ui->categoriesTable->setItem(row, 2, new QTableWidgetItem(cat.type));
        if (cat.type == "Expense") {
            ui->budgetCategoryBox->addItem(cat.name, cat.id);
        }
        row++;
    }
    ui->categoriesTable->resizeColumnsToContents();
    populateCategoryComboBox(ui->transactionTypeBox->currentText());
}

void MainWindow::refreshBudgetView()
{
    int month = ui->budgetMonthEdit->date().toString("yyyyMM").toInt();
    QList<Category> expenseCategories = m_db.getAllCategories("Expense");
    ui->budgetsTable->setRowCount(expenseCategories.size());

    int row = 0;
    for (const auto& cat : expenseCategories) {
        Budget budget = m_db.getBudget(cat.id, month);
        double spent = m_db.calculateSpent(cat.id, month);

        ui->budgetsTable->setItem(row, 0, new QTableWidgetItem(cat.name));
        ui->budgetsTable->setItem(row, 1, new QTableWidgetItem(QString::number(month)));
        ui->budgetsTable->setItem(row, 2, new QTableWidgetItem(budget.id != -1 ? QString::number(budget.limit, 'f', 2) : "Not Set"));
        ui->budgetsTable->setItem(row, 3, new QTableWidgetItem(QString::number(spent, 'f', 2)));
        row++;
    }
    ui->budgetsTable->resizeColumnsToContents();
}


void MainWindow::populateCategoryComboBox(const QString& type)
{
    ui->categoryComboBox->clear();
    QList<Category> categories = m_db.getAllCategories(type);
    for (const auto &cat : categories) {
        ui->categoryComboBox->addItem(cat.name, cat.id);
    }
}

void MainWindow::on_addTransactionButton_clicked()
{
    Transaction tx;
    tx.amount = ui->amountSpinBox->value();
    if (tx.amount <= 0) {
        QMessageBox::warning(this, "Invalid Input", "Amount must be positive.");
        return;
    }
    tx.type = ui->transactionTypeBox->currentText();
    tx.categoryId = ui->categoryComboBox->currentData().toInt();
    tx.accountId = ui->accountComboBox->currentData().toInt();
    tx.time = ui->dateTimeEdit->dateTime();
    tx.note = ui->noteLineEdit->text();

    if (tx.accountId == 0) {
        QMessageBox::warning(this, "Invalid Input", "Please select an account.");
        return;
    }
     if (tx.categoryId == 0) {
        QMessageBox::warning(this, "Invalid Input", "Please select a category.");
        return;
    }

    if (m_db.addTransaction(tx)) {
        QMessageBox::information(this, "Success", "Transaction added successfully.");
        refreshTransactionView();
        refreshAccountView();
        if (tx.type == "Expense") {
            checkBudget(tx.categoryId);
            refreshBudgetView();
        }
        // clear inputs
        ui->amountSpinBox->setValue(0);
        ui->noteLineEdit->clear();
    } else {
        QMessageBox::critical(this, "Error", "Failed to add transaction.");
    }
}

void MainWindow::on_transactionTypeBox_currentIndexChanged(int index)
{
    populateCategoryComboBox(ui->transactionTypeBox->currentText());
}

void MainWindow::on_addAccountButton_clicked()
{
    Account acc;
    acc.name = ui->accountNameEdit->text();
    acc.type = ui->accountTypeBox->currentText();
    acc.balance = ui->initialBalanceSpinBox->value();

    if (acc.name.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Account name cannot be empty.");
        return;
    }

    if (m_db.addAccount(acc)) {
        QMessageBox::information(this, "Success", "Account added successfully.");
        refreshAccountView();
        ui->accountNameEdit->clear();
        ui->initialBalanceSpinBox->setValue(0);
    } else {
        QMessageBox::critical(this, "Error", "Failed to add account. Does it already exist?");
    }
}

void MainWindow::on_addCategoryButton_clicked()
{
    Category cat;
    cat.name = ui->categoryNameEdit->text();
    cat.type = ui->categoryTypeBox->currentText();

    if (cat.name.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Category name cannot be empty.");
        return;
    }

    if (m_db.addCategory(cat)) {
        QMessageBox::information(this, "Success", "Category added successfully.");
        refreshCategoryView();
        refreshBudgetView(); // New categories might need a budget
        ui->categoryNameEdit->clear();
    } else {
        QMessageBox::critical(this, "Error", "Failed to add category. Does it already exist?");
    }
}

void MainWindow::on_setBudgetButton_clicked()
{
    Budget budget;
    budget.categoryId = ui->budgetCategoryBox->currentData().toInt();
    budget.month = ui->budgetMonthEdit->date().toString("yyyyMM").toInt();
    budget.limit = ui->budgetLimitSpinBox->value();

    if (budget.categoryId == 0) {
        QMessageBox::warning(this, "Invalid Input", "Please select a category for the budget.");
        return;
    }
    if (budget.limit <= 0) {
        QMessageBox::warning(this, "Invalid Input", "Budget limit must be positive.");
        return;
    }

    if (m_db.setBudget(budget)) {
        QMessageBox::information(this, "Success", "Budget set successfully.");
        refreshBudgetView();
    } else {
        QMessageBox::critical(this, "Error", "Failed to set budget.");
    }
}

void MainWindow::checkBudget(int categoryId)
{
    int month = QDate::currentDate().toString("yyyyMM").toInt();
    Budget budget = m_db.getBudget(categoryId, month);
    if (budget.id != -1 && budget.limit > 0) {
        double spent = m_db.calculateSpent(categoryId, month);
        if (spent / budget.limit >= 0.8) {
            QList<Category> cats = m_db.getAllCategories("");
            QString catName = "";
            for(const auto& cat : cats){
                if(cat.id == categoryId) {
                    catName = cat.name;
                    break;
                }
            }
            QMessageBox::warning(this, "Budget Alert",
                                 QString("You have spent %1% of your budget for '%2'.")
                                 .arg(QString::number(spent / budget.limit * 100, 'f', 0))
                                 .arg(catName));
        }
    }
}

void MainWindow::on_transactionsTable_cellDoubleClicked(int row, int column)
{
    // Simple edit example: allow editing the note
    int txId = ui->transactionsTable->item(row, 0)->text().toInt();
    QString currentNote = ui->transactionsTable->item(row, 5)->text();

    bool ok;
    QString newNote = QInputDialog::getText(this, "Edit Note", "Enter new note:", QLineEdit::Normal, currentNote, &ok);

    if (ok && newNote != currentNote) {
        Transaction tx;
        // In a real app, you'd fetch the full transaction, update it, then save.
        // This is a simplified example.
        // We need to find the original transaction to keep other fields intact.
        for(const auto& transaction : m_currentTransactions) {
            if(transaction.id == txId) {
                tx = transaction;
                break;
            }
        }
        tx.note = newNote;
        if (m_db.updateTransaction(tx)) {
            QMessageBox::information(this, "Success", "Transaction updated.");
            refreshTransactionView();
        } else {
            QMessageBox::critical(this, "Error", "Failed to update transaction.");
        }
    }
}

void MainWindow::on_deleteTransactionButton_clicked()
{
    QList<QTableWidgetItem*> selectedItems = ui->transactionsTable->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select a transaction to delete.");
        return;
    }

    int row = ui->transactionsTable->row(selectedItems.first());
    int txId = ui->transactionsTable->item(row, 0)->text().toInt();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete this transaction? This will also update the account balance.",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (m_db.deleteTransaction(txId)) {
            QMessageBox::information(this, "Success", "Transaction deleted.");
            refreshTransactionView();
            refreshAccountView();
            refreshBudgetView();
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete transaction.");
        }
    }
}
