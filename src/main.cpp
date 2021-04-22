#include "main_control_panel.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.setWindowTitle("Bone Marrow Puncture Auto Guider");
    return a.exec();
}
