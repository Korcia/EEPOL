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


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // Play button for output - initially disabled
    ui->playOutputButton->setEnabled(false);

    // Create three processes
    // 1.transcoding, 2.input play 3.output play
    mTranscodingProcess = new QProcess(this);
    mInputPlayProcess = new QProcess(this);
    mOutputPlayProcess = new QProcess(this);

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

// conversion start
void Dialog::on_startButton_clicked()
{
    //QString program = "C:/FFmpeg/bin/ffmpeg";
    QString program = "/usr/bin/ffmpeg";

    QStringList arguments;
    QString argumentos;

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
                     tr("ffmpeg"),tr("Output file not specified"));
        return;
    }

    QString fileName = ui->toLineEdit->text();
    qDebug() << "output file check " << fileName;
    qDebug() << "QFile::exists(fileName) = " << QFile::exists(fileName);
    if (QFile::exists(fileName)) {
         if (QMessageBox::question(this, tr("ffmpeg"),
                    tr("There already exists a file called %1 in "
                    "the current directory. Overwrite?").arg(fileName),
                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No)
             == QMessageBox::No)
             return;
         QFile::remove(fileName);
         while(QFile::exists(fileName)) {
             qDebug() << "output file still there";
         }
     }

    //arguments << "-i" << input << output;
    argumentos = "-f h264 -i " + input + " -s 640x480 -crf 14 " + output;

    qDebug() << argumentos;

    mTranscodingProcess->setProcessChannelMode(QProcess::MergedChannels);
    //mTranscodingProcess->start(program, arguments);
    //process->start(program, QStringList() << folder);
    //mTranscodingProcess->start(program, QStringList() << argumentos);
    mTranscodingProcess->start("ffmpeg -f h264 -i /home/jose/Documents/RR35_Interno_19_20160420_204500.dav -s 640x480 -crf 14 /home/jose/camara2.mp4");
    //mTranscodingProcess->waitForFinished();
    //mTranscodingProcess->close();
    //process.waitForFinished();
    //process.close();
}

void Dialog::readyReadStandardOutput()
{
    mOutputString.append(mTranscodingProcess->readAllStandardOutput());
    ui->textEdit->setText(mOutputString);

    // put the slider at the bottom
    ui->textEdit->verticalScrollBar()
            ->setSliderPosition(
                ui->textEdit->verticalScrollBar()->maximum());
}

void Dialog::encodingFinished()
{
    // Set the encoding status by checking output file's existence
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
}

// Browse... button clicked - this is for input file
void Dialog::on_fileOpenButton_clicked()
{
    QString fileName =
        QFileDialog::getOpenFileName(
                this,
                tr("Open File"),
                "/home/jose/Documents",
                tr("videos (*.dav *.mp4 *.mov *.avi)"));
    if (!fileName.isEmpty()) {
        ui->fromLineEdit->setText(fileName);
    }
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
