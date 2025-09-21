#include <QApplication>
#include "pay.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    PayApp window;
    window.show();
    return app.exec();
}
