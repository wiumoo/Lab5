/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QWidget *transactionsTab;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *addTransactionBox;
    QGridLayout *gridLayout;
    QLabel *label;
    QComboBox *transactionTypeBox;
    QLabel *label_2;
    QDoubleSpinBox *amountSpinBox;
    QLabel *label_3;
    QComboBox *categoryComboBox;
    QLabel *label_4;
    QComboBox *accountComboBox;
    QLabel *label_5;
    QDateTimeEdit *dateTimeEdit;
    QLabel *label_6;
    QLineEdit *noteLineEdit;
    QPushButton *addTransactionButton;
    QTableWidget *transactionsTable;
    QPushButton *deleteTransactionButton;
    QWidget *accountsTab;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *addAccountBox;
    QHBoxLayout *horizontalLayout;
    QLabel *label_7;
    QLineEdit *accountNameEdit;
    QLabel *label_8;
    QComboBox *accountTypeBox;
    QLabel *label_9;
    QDoubleSpinBox *initialBalanceSpinBox;
    QPushButton *addAccountButton;
    QTableWidget *accountsTable;
    QWidget *categoriesTab;
    QVBoxLayout *verticalLayout_4;
    QGroupBox *addCategoryBox;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_10;
    QLineEdit *categoryNameEdit;
    QLabel *label_11;
    QComboBox *categoryTypeBox;
    QPushButton *addCategoryButton;
    QTableWidget *categoriesTable;
    QWidget *budgetsTab;
    QVBoxLayout *verticalLayout_5;
    QGroupBox *setBudgetBox;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_12;
    QComboBox *budgetCategoryBox;
    QLabel *label_13;
    QDateTimeEdit *budgetMonthEdit;
    QLabel *label_14;
    QDoubleSpinBox *budgetLimitSpinBox;
    QPushButton *setBudgetButton;
    QTableWidget *budgetsTable;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName("tabWidget");
        transactionsTab = new QWidget();
        transactionsTab->setObjectName("transactionsTab");
        verticalLayout_2 = new QVBoxLayout(transactionsTab);
        verticalLayout_2->setObjectName("verticalLayout_2");
        addTransactionBox = new QGroupBox(transactionsTab);
        addTransactionBox->setObjectName("addTransactionBox");
        gridLayout = new QGridLayout(addTransactionBox);
        gridLayout->setObjectName("gridLayout");
        label = new QLabel(addTransactionBox);
        label->setObjectName("label");

        gridLayout->addWidget(label, 0, 0, 1, 1);

        transactionTypeBox = new QComboBox(addTransactionBox);
        transactionTypeBox->setObjectName("transactionTypeBox");

        gridLayout->addWidget(transactionTypeBox, 0, 1, 1, 1);

        label_2 = new QLabel(addTransactionBox);
        label_2->setObjectName("label_2");

        gridLayout->addWidget(label_2, 0, 2, 1, 1);

        amountSpinBox = new QDoubleSpinBox(addTransactionBox);
        amountSpinBox->setObjectName("amountSpinBox");

        gridLayout->addWidget(amountSpinBox, 0, 3, 1, 1);

        label_3 = new QLabel(addTransactionBox);
        label_3->setObjectName("label_3");

        gridLayout->addWidget(label_3, 1, 0, 1, 1);

        categoryComboBox = new QComboBox(addTransactionBox);
        categoryComboBox->setObjectName("categoryComboBox");

        gridLayout->addWidget(categoryComboBox, 1, 1, 1, 1);

        label_4 = new QLabel(addTransactionBox);
        label_4->setObjectName("label_4");

        gridLayout->addWidget(label_4, 1, 2, 1, 1);

        accountComboBox = new QComboBox(addTransactionBox);
        accountComboBox->setObjectName("accountComboBox");

        gridLayout->addWidget(accountComboBox, 1, 3, 1, 1);

        label_5 = new QLabel(addTransactionBox);
        label_5->setObjectName("label_5");

        gridLayout->addWidget(label_5, 2, 0, 1, 1);

        dateTimeEdit = new QDateTimeEdit(addTransactionBox);
        dateTimeEdit->setObjectName("dateTimeEdit");

        gridLayout->addWidget(dateTimeEdit, 2, 1, 1, 1);

        label_6 = new QLabel(addTransactionBox);
        label_6->setObjectName("label_6");

        gridLayout->addWidget(label_6, 2, 2, 1, 1);

        noteLineEdit = new QLineEdit(addTransactionBox);
        noteLineEdit->setObjectName("noteLineEdit");

        gridLayout->addWidget(noteLineEdit, 2, 3, 1, 1);

        addTransactionButton = new QPushButton(addTransactionBox);
        addTransactionButton->setObjectName("addTransactionButton");

        gridLayout->addWidget(addTransactionButton, 3, 3, 1, 1);


        verticalLayout_2->addWidget(addTransactionBox);

        transactionsTable = new QTableWidget(transactionsTab);
        transactionsTable->setObjectName("transactionsTable");

        verticalLayout_2->addWidget(transactionsTable);

        deleteTransactionButton = new QPushButton(transactionsTab);
        deleteTransactionButton->setObjectName("deleteTransactionButton");

        verticalLayout_2->addWidget(deleteTransactionButton);

        tabWidget->addTab(transactionsTab, QString());
        accountsTab = new QWidget();
        accountsTab->setObjectName("accountsTab");
        verticalLayout_3 = new QVBoxLayout(accountsTab);
        verticalLayout_3->setObjectName("verticalLayout_3");
        addAccountBox = new QGroupBox(accountsTab);
        addAccountBox->setObjectName("addAccountBox");
        horizontalLayout = new QHBoxLayout(addAccountBox);
        horizontalLayout->setObjectName("horizontalLayout");
        label_7 = new QLabel(addAccountBox);
        label_7->setObjectName("label_7");

        horizontalLayout->addWidget(label_7);

        accountNameEdit = new QLineEdit(addAccountBox);
        accountNameEdit->setObjectName("accountNameEdit");

        horizontalLayout->addWidget(accountNameEdit);

        label_8 = new QLabel(addAccountBox);
        label_8->setObjectName("label_8");

        horizontalLayout->addWidget(label_8);

        accountTypeBox = new QComboBox(addAccountBox);
        accountTypeBox->setObjectName("accountTypeBox");

        horizontalLayout->addWidget(accountTypeBox);

        label_9 = new QLabel(addAccountBox);
        label_9->setObjectName("label_9");

        horizontalLayout->addWidget(label_9);

        initialBalanceSpinBox = new QDoubleSpinBox(addAccountBox);
        initialBalanceSpinBox->setObjectName("initialBalanceSpinBox");

        horizontalLayout->addWidget(initialBalanceSpinBox);

        addAccountButton = new QPushButton(addAccountBox);
        addAccountButton->setObjectName("addAccountButton");

        horizontalLayout->addWidget(addAccountButton);


        verticalLayout_3->addWidget(addAccountBox);

        accountsTable = new QTableWidget(accountsTab);
        accountsTable->setObjectName("accountsTable");

        verticalLayout_3->addWidget(accountsTable);

        tabWidget->addTab(accountsTab, QString());
        categoriesTab = new QWidget();
        categoriesTab->setObjectName("categoriesTab");
        verticalLayout_4 = new QVBoxLayout(categoriesTab);
        verticalLayout_4->setObjectName("verticalLayout_4");
        addCategoryBox = new QGroupBox(categoriesTab);
        addCategoryBox->setObjectName("addCategoryBox");
        horizontalLayout_2 = new QHBoxLayout(addCategoryBox);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        label_10 = new QLabel(addCategoryBox);
        label_10->setObjectName("label_10");

        horizontalLayout_2->addWidget(label_10);

        categoryNameEdit = new QLineEdit(addCategoryBox);
        categoryNameEdit->setObjectName("categoryNameEdit");

        horizontalLayout_2->addWidget(categoryNameEdit);

        label_11 = new QLabel(addCategoryBox);
        label_11->setObjectName("label_11");

        horizontalLayout_2->addWidget(label_11);

        categoryTypeBox = new QComboBox(addCategoryBox);
        categoryTypeBox->setObjectName("categoryTypeBox");

        horizontalLayout_2->addWidget(categoryTypeBox);

        addCategoryButton = new QPushButton(addCategoryBox);
        addCategoryButton->setObjectName("addCategoryButton");

        horizontalLayout_2->addWidget(addCategoryButton);


        verticalLayout_4->addWidget(addCategoryBox);

        categoriesTable = new QTableWidget(categoriesTab);
        categoriesTable->setObjectName("categoriesTable");

        verticalLayout_4->addWidget(categoriesTable);

        tabWidget->addTab(categoriesTab, QString());
        budgetsTab = new QWidget();
        budgetsTab->setObjectName("budgetsTab");
        verticalLayout_5 = new QVBoxLayout(budgetsTab);
        verticalLayout_5->setObjectName("verticalLayout_5");
        setBudgetBox = new QGroupBox(budgetsTab);
        setBudgetBox->setObjectName("setBudgetBox");
        horizontalLayout_3 = new QHBoxLayout(setBudgetBox);
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        label_12 = new QLabel(setBudgetBox);
        label_12->setObjectName("label_12");

        horizontalLayout_3->addWidget(label_12);

        budgetCategoryBox = new QComboBox(setBudgetBox);
        budgetCategoryBox->setObjectName("budgetCategoryBox");

        horizontalLayout_3->addWidget(budgetCategoryBox);

        label_13 = new QLabel(setBudgetBox);
        label_13->setObjectName("label_13");

        horizontalLayout_3->addWidget(label_13);

        budgetMonthEdit = new QDateTimeEdit(setBudgetBox);
        budgetMonthEdit->setObjectName("budgetMonthEdit");

        horizontalLayout_3->addWidget(budgetMonthEdit);

        label_14 = new QLabel(setBudgetBox);
        label_14->setObjectName("label_14");

        horizontalLayout_3->addWidget(label_14);

        budgetLimitSpinBox = new QDoubleSpinBox(setBudgetBox);
        budgetLimitSpinBox->setObjectName("budgetLimitSpinBox");

        horizontalLayout_3->addWidget(budgetLimitSpinBox);

        setBudgetButton = new QPushButton(setBudgetBox);
        setBudgetButton->setObjectName("setBudgetButton");

        horizontalLayout_3->addWidget(setBudgetButton);


        verticalLayout_5->addWidget(setBudgetBox);

        budgetsTable = new QTableWidget(budgetsTab);
        budgetsTable->setObjectName("budgetsTable");

        verticalLayout_5->addWidget(budgetsTable);

        tabWidget->addTab(budgetsTab, QString());

        verticalLayout->addWidget(tabWidget);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 21));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Ledger App", nullptr));
        addTransactionBox->setTitle(QCoreApplication::translate("MainWindow", "Add Transaction", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Type:", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Amount:", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Category:", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "Account:", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "Date:", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "Note:", nullptr));
        addTransactionButton->setText(QCoreApplication::translate("MainWindow", "Add", nullptr));
        deleteTransactionButton->setText(QCoreApplication::translate("MainWindow", "Delete Selected Transaction", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(transactionsTab), QCoreApplication::translate("MainWindow", "Transactions", nullptr));
        addAccountBox->setTitle(QCoreApplication::translate("MainWindow", "Add Account", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow", "Name:", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow", "Type:", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow", "Initial Balance:", nullptr));
        addAccountButton->setText(QCoreApplication::translate("MainWindow", "Add Account", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(accountsTab), QCoreApplication::translate("MainWindow", "Accounts", nullptr));
        addCategoryBox->setTitle(QCoreApplication::translate("MainWindow", "Add Category", nullptr));
        label_10->setText(QCoreApplication::translate("MainWindow", "Name:", nullptr));
        label_11->setText(QCoreApplication::translate("MainWindow", "Type:", nullptr));
        addCategoryButton->setText(QCoreApplication::translate("MainWindow", "Add Category", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(categoriesTab), QCoreApplication::translate("MainWindow", "Categories", nullptr));
        setBudgetBox->setTitle(QCoreApplication::translate("MainWindow", "Set Budget", nullptr));
        label_12->setText(QCoreApplication::translate("MainWindow", "Category:", nullptr));
        label_13->setText(QCoreApplication::translate("MainWindow", "Month:", nullptr));
        budgetMonthEdit->setDisplayFormat(QCoreApplication::translate("MainWindow", "yyyy-MM", nullptr));
        label_14->setText(QCoreApplication::translate("MainWindow", "Limit:", nullptr));
        setBudgetButton->setText(QCoreApplication::translate("MainWindow", "Set Budget", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(budgetsTab), QCoreApplication::translate("MainWindow", "Budgets", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
