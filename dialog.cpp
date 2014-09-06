#include "dialog.h"
#include <QFileDialog>
#include <QDir>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    proc(new QProcess)
{
    ui->setupUi(this);
    connect(ui->pathSearch, SIGNAL(clicked()), this, SLOT(selectOutputDirectory()));
    prepareDlButton();
}

Dialog::~Dialog()
{
    delete proc;
    delete ui;
}

void Dialog::selectOutputDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, trUtf8("Choisir une destination"), QDir::homePath());

    if (!dir.isEmpty())
        ui->pathInput->setText(dir);
}

void Dialog::download()
{
    QString program = "youtube-dl";
    QStringList args;
    args << ui->videoInput->text() << "-o" << (ui->pathInput->text()+"/%(id)s-%(title)s.%(ext)s");

    qDebug() << args;

    ui->logWindow->textCursor().movePosition(QTextCursor::End);

    connect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(handleError(QProcess::ProcessError)));
    connect(proc, SIGNAL(finished(int)), this, SLOT(handleEnd(int)));
    connect(proc, SIGNAL(readyReadStandardOutput()), this, SLOT(updateLog()));
    proc->blockSignals(false);
    proc->start(program, args, QIODevice::ReadOnly);

    ui->dlCommand->setText(trUtf8("Interrompre"));
    ui->dlCommand->disconnect(SIGNAL(clicked()));
    connect(ui->dlCommand, SIGNAL(clicked()), this, SLOT(stopDownloading()));
}

void Dialog::stopDownloading()
{
    if (proc->state() == QProcess::NotRunning) //proc is already finished
        return;

    proc->blockSignals(true);
    proc->disconnect();
    if (proc->state() != QProcess::NotRunning) {
        proc->terminate();
        proc->waitForFinished();
        if (proc->state() != QProcess::NotRunning)
            proc->kill();
        proc->waitForFinished();
    
        if (proc->state() == QProcess::Running) {
            QMessageBox::critical(this, trUtf8("Erreur critique"), trUtf8("Il est impossible de tuer le programme de téléchargement, il ne me reste plus à présent qu'à me tuer moi-même..."));
            qApp->exit(1);
        }

        ui->logWindow->append(trUtf8("*** Interrompu ***"));
        QMessageBox::information(this, trUtf8("Téléchargement interrompu"), trUtf8("Le téléchargement a été interrompu. Il peut y avoir des fichiers temporaires de téléchargement (avec l'extension \".part\") dans le dossier de téléchargement. Vous pouvez les supprimer ou bien les conserver, le téléchargement reprendra là où il s'est interrompu."));

        prepareDlButton();
    }
}

void Dialog::prepareDlButton()
{
    ui->dlCommand->setText(trUtf8("Télécharger"));
    ui->dlCommand->disconnect(SIGNAL(clicked()));
    connect(ui->dlCommand, SIGNAL(clicked()), this, SLOT(download()));
}

void Dialog::handleEnd(int status)
{
    QMessageBox *msg;
    if (status == 0)
    {
        msg = new QMessageBox(QMessageBox::Information, trUtf8("Téléchargement réussi"), trUtf8("Le téléchargement est terminé et tout s'est bien passé."),QMessageBox::Ok,this);
        ui->logWindow->append(trUtf8("*** Téléchargement terminé ***"));
    } else
    {
        ui->logWindow->setText(proc->readAllStandardError());
        msg = new QMessageBox(QMessageBox::Critical, trUtf8("Téléchargement échoué"), trUtf8("Le téléchargement a échoué. La raison est indiquée dans la fenêtre de téléchargement."),QMessageBox::Ok,this);
        ui->logWindow->append(trUtf8("*** Echec du téléchargement ***"));
    }

    msg->exec();
    delete msg;
    prepareDlButton();
    ui->videoInput->clear();
    ui->videoInput->setFocus();
    // leave pathInput intact, the user may want to download again in the same directory
}

void Dialog::handleError(QProcess::ProcessError err)
{
    QMessageBox msg(QMessageBox::Critical, trUtf8("Sérieux problème..."), trUtf8(""));
    switch (err){
    case QProcess::FailedToStart:
        msg.setText(trUtf8("Le programme de téléchargement n'a pas pu être lancé. Vérifiez que le programme \"youtube-dl.exe\" est présent. Je ne peux pas poursuire dans ces conditions."));
        break;
    case QProcess::Crashed:
        msg.setText(trUtf8("Le programme de téléchargement a rencontré un problème qui l'a forcé à s'arrêter. Je vais en faire de même."));
        break;
    case QProcess::Timedout:
        msg.setText(trUtf8("Le programme de téléchargement ne répond plus. Le mieux à faire est de recommencer à zéro. Je vais vous aider en crashant à mon tour."));
        break;
    default:
        msg.setText(trUtf8("Le programme de téléchargement ne se comporte pas comme prévu. Mieux vaut tout arrêter, quitte à réessayer."));
        break;
    }
    msg.exec();
    qApp->exit(2);
}

void Dialog::updateLog()
{
    ui->logWindow->textCursor().insertText(proc->readAll());
    ui->logWindow->ensureCursorVisible();
}
