#include "mainwindow.h"
#include <QApplication>
#include "flaw_demo.h"

int main(int argc, char *argv[])
{
    // 为了静态分析工具能检测到，我们在这里引用这些函数
    // 实际运行时请注释掉，否则会崩溃
    if (false) {
        memory_leak_demo();
        double_free_demo();
        null_pointer_demo();
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
