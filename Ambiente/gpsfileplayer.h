#ifndef GPSFILEPLAYER_H
#define GPSFILEPLAYER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QDateTime>
#include "speedcontroller.h"
#include <memory>

class GpsFilePlayer : public QObject
{   Q_OBJECT

public:
    explicit GpsFilePlayer(QObject *parent = nullptr);

    ~GpsFilePlayer();

public slots:
    void startPlayback(const QString &filePath, int intervaMs = 100);

    void stopPlayback();



signals:

    void gpsDataUpdate(const GpsData& data);

    void playbackFinished();


protected slots:
  void processNextLine();


private:
    std::unique_ptr<QFile> m_file;
    std::unique_ptr<QTextStream> m_textStream;
    QTimer m_playbackTimer;


};

#endif // GPSFILEPLAYER_H
