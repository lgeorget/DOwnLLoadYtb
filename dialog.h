#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QProcess>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    Ui::Dialog *ui;
    QProcess *proc;
    void prepareDlButton();

private slots:
    void selectOutputDirectory();
    void download();
    void handleEnd(int status);
    void handleError(QProcess::ProcessError err);
    void updateLog();
    void stopDownloading();
};

#endif // DIALOG_H
