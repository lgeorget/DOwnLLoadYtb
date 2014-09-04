#include "dialog.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("ISO-8851-1"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("ISO-8851-1"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("ISO-8851-1"));
    w.show();

    return a.exec();
}
