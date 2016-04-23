// dialog.cpp

#include "dialog.h"
#include "ui_dialog.h"
#include <QDebug>
#include <QString>
#include <QProcess>
#include <QScrollBar>
#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QThread>
#include <QFileInfo>


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // Play button for output - initially disabled
    ui->playOutputButton->setEnabled(false);

    //Carpeta destino por defecto y modo sólo lectura
    ui->toLineEdit->setPlaceholderText(DIRDESTINO);
    ui->toLineEdit->setReadOnly(true);

    // Create three processes
    // 1.transcoding, 2.input play 3.output play
    mTranscodingProcess = new QProcess(this);
    mInputPlayProcess = new QProcess(this);
    mOutputPlayProcess = new QProcess(this);

    //Creo 4 procesos más en caso de necesitarlos
    //mTranscodingProcess2 = new QProcess(this);

    connect(mTranscodingProcess, SIGNAL(started()), this, SLOT(processStarted()));

    connect(mTranscodingProcess,SIGNAL(readyReadStandardOutput()),this,SLOT(readyReadStandardOutput()));
    connect(mTranscodingProcess, SIGNAL(finished(int)), this, SLOT(encodingFinished()));
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::processStarted()
{
    qDebug() << "processStarted()";
}

const QString Dialog::DIRDESTINO = "/home/jose/";

// conversion start
void Dialog::on_startButton_clicked()
{
    //QString program = "C:/FFmpeg/bin/ffmpeg";
    QString program = "/usr/bin/ffmpeg";

    QStringList arguments;
    QString arg;

    QString input = ui->fromLineEdit->text();
    if(input.isEmpty()) {
        qDebug() << "No input";
        QMessageBox::information(this,
                     tr("ffmpeg"),tr("Input file not specified"));
        return;
    }
    QString output = ui->toLineEdit->text();
    if(output.isEmpty()) {
        qDebug() << "No output";
        QMessageBox::information(this,
                     tr("ffmpeg"),tr("Nombre de fichero destino no especificado"));
        return;
    }

    QString fileName = ui->toLineEdit->text();
    qDebug() << "output file check " << fileName;
    qDebug() << "QFile::exists(fileName) = " << QFile::exists(fileName);
    if (QFile::exists(fileName)) {
         if (QMessageBox::question(this, tr("ffmpeg"),
                    tr("Existe un fichero %1 en "
                    "el directorio destino. ¿Sobreescribir?").arg(fileName),
                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No)
             == QMessageBox::No)
             return;
         QFile::remove(fileName);
         while(QFile::exists(fileName)) {
             qDebug() << "output file still there";
         }
     }

    //qDebug() << argumentos;

    if (!mComandos.isEmpty()) {
        arg = mComandos.dequeue();
    }

    mTranscodingProcess->setProcessChannelMode(QProcess::MergedChannels);
    mTranscodingProcess->start(arg);
}

void Dialog::readyReadStandardOutput()
{
    mOutputString.append(mTranscodingProcess->readAllStandardOutput());
    ui->textEdit->setText(mOutputString);

    // poner el slider abajo
    ui->textEdit->verticalScrollBar()
            ->setSliderPosition(
                ui->textEdit->verticalScrollBar()->maximum());
}

void Dialog::encodingFinished()
{
    // Resultado del proceso by checking output file's existence
    QString fileName = ui->toLineEdit->text();

    if (QFile::exists(fileName)) {
        ui->transcodingStatusLabel
                ->setText("Resultado Conversión: OK!");
        ui->transcodingStatusLabel->setStyleSheet("QLabel { background-color : red; color : blue; }");
        ui->playOutputButton->setEnabled(true);
    }
    else {
        ui->transcodingStatusLabel
                ->setText("Resultado Conversión: Error!");
    }

    if (mComandos.isEmpty()) {
        mTranscodingProcess->close();
    } else {
        QString arg = mComandos.dequeue();
        mTranscodingProcess->start(arg);
    }

}

//Compongo las command lines que llaman a ffmpeg con los nombres de los ficheros
void Dialog::crearComandos(QStringList nombreFicheros)
{
    foreach (QString var, nombreFicheros) {
        QFileInfo info(var);
        info.setCaching(false);
        QString base = info.completeBaseName();
        //delete info;
        QString tmp1 = "ffmpeg -f h264 -i ";
        QString tmp2 = tmp1 + var;
        QString tmp3 = " -s 640x480 -crf 14 ";
        QString tmp4 = DIRDESTINO + base + ".mp4";
        QString arg = tmp2 + tmp3 + tmp4;

        mComandos.enqueue(arg);
    }

}

// Browse... button clicked - this is for input file
void Dialog::on_fileOpenButton_clicked()
{
    QFileDialog dialogo(this);
    dialogo.setDirectory(QDir::homePath());
    dialogo.setFileMode(QFileDialog::ExistingFiles);
    dialogo.setNameFilter(trUtf8("Videos (*.dav *.h264)"));
    QString nombreFichero;
    if (dialogo.exec()) {
        QStringList fileNames = dialogo.selectedFiles();
        Dialog::crearComandos(fileNames);
        nombreFichero = fileNames.join(" ");
    }
    if (!nombreFichero.isEmpty()) {
        ui->fromLineEdit->setText(nombreFichero);
    }

    /* Esto sería para seleccionar un sólo fichero
    QString fileName =
        QFileDialog::getOpenFileName(
                this,
                tr("Open File"),
                "/home/jose/Documents",
                tr("videos (*.dav *.mp4 *.mov *.avi)"));
    if (!fileName.isEmpty()) {
        ui->fromLineEdit->setText(fileName);
    }
    */
}

void Dialog::on_playInputButton_clicked()
{
    QString program = "C:/FFmpeg/bin/ffplay";
    QStringList arguments;
    QString input = ui->fromLineEdit->text();
    arguments << input;
    mInputPlayProcess->start(program, arguments);
}

void Dialog::on_playOutputButton_clicked()
{
    //QString program = "C:/FFmpeg/bin/ffplay";
    QString program = "/usr/bin/ffplay";
    QStringList arguments;
    QString output = ui->toLineEdit->text();
    arguments << output;
    mInputPlayProcess->start(program, arguments);
}
