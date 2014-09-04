#include "dialog.h"
#include <QFileDialog>
#include <QDir>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    connect(ui->pathSearch, SIGNAL(clicked()), this, SLOT(selectOutputDirectory()));
    connect(ui->dlCommand, SIGNAL(clicked()), this, SLOT(download()));
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::selectOutputDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choisir une destination"), QDir::homePath());

    if (!dir.isEmpty())
        ui->pathInput->setText(dir);
}

void Dialog::download()
{
    QString program = "youtube-dl";
    QStringList args;
    args << ui->videoInput->text() << "-o" << (ui->pathInput->text()+"/%(id)s-%(title)s.%(ext)s");

    qDebug() << args;

    proc = new QProcess();
    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(handleEnd(int)));
    connect(proc, SIGNAL(readyRead()), this, SLOT(updateLog()));
    proc->start(program, args, QIODevice::Text);
}

void Dialog::handleEnd(int status)
{
    ui->logWindow->clear();

    QMessageBox *msg;
    if (status == 0)
    {
        msg = new QMessageBox(QMessageBox::Information, "T�l�chargement r�ussi", "Le t�l�chargement est termin� et tout s'est bien pass�.",QMessageBox::Ok,this);
    } else
    {
        ui->logWindow->setText(proc->readAllStandardError());
        msg = new QMessageBox(QMessageBox::Critical, "T�l�chargement �chou�", "Le t�l�chargement a �chou�. La raison est indiqu�e dans la fen�tre de t�l�chargement.",QMessageBox::Ok,this);
    }

    msg->exec();
    ui->videoInput->clear();
    ui->videoInput->setFocus();
    // leave pathInput intact, the user may want to download again in the same directory
}

void Dialog::updateLog()
{
    while (!proc->atEnd())
        ui->logWindow->append(proc->readLine());
}
