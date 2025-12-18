#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "database.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_addTransactionButton_clicked();
    void on_transactionTypeBox_currentIndexChanged(int index);
    void on_addAccountButton_clicked();
    void on_addCategoryButton_clicked();
    void on_setBudgetButton_clicked();
    void on_transactionsTable_cellDoubleClicked(int row, int column);
    void on_deleteTransactionButton_clicked();


private:
    void setupUiElements();
    void loadInitialData();
    void refreshTransactionView();
    void refreshAccountView();
    void refreshCategoryView();
    void refreshBudgetView();
    void populateCategoryComboBox(const QString& type);
    void checkBudget(int categoryId);


    Ui::MainWindow *ui;
    Database m_db;
    QList<Transaction> m_currentTransactions;
};
#endif // MAINWINDOW_H
