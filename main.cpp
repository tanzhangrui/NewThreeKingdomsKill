#include <QApplication>
#include <QFontDatabase>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("ThreeKingdomsKill");
    a.setOrganizationName("SKG");
    a.setStyle("Fusion");

    a.setStyleSheet(
        "QWidget{"
        "  font-family:'Microsoft YaHei','SimHei','Noto Sans CJK SC',sans-serif;"
        "}"
        "QToolTip{"
        "  background:#1E143A;color:#D4AF37;"
        "  border:2px solid #D4AF37;"
        "  font-size:16px;padding:10px;border-radius:6px;"
        "}"
        "QMessageBox{background:#0D0A1A;}"
        "QMessageBox QLabel{color:#D4AF37;font-size:18px;}"
    );

    MainWindow w;
    w.show();
    return a.exec();
}
