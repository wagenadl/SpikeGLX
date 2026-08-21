// LiveSalpa.h

#ifndef LIVESALPA_H

#define LIVESALPA_H

#include <QObject>

class LiveSalpa: public QObject {
  Q_OBJECT;
public:
  LiveSalpa(QObject *parent=0);
  virtual ~LiveSalpa();
  void setParams(class SalpaParams const &);
  void process(qint16 *data, int nNu, int nC, int ntpts,
               QVector<int> const &ic2iy, double srate);
  void resetTraining();
private:
  class LiveSalpa_Private *d;
};

#endif
