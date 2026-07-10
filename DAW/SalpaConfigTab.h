// Salpaconfigtab.h

#ifndef SALPACONFIGTAB_H

#define SALPACONFIGTAB_H

#include <QWidget>

#include "Src-params/DAQ.h"
#include "SalpaParams.h"

class SalpaConfigTab: public QObject {
  Q_OBJECT;
public:
  SalpaConfigTab(QWidget *tab);
  virtual ~SalpaConfigTab();
  void toGUI(DAQ::Params const &pp, bool usingIM, bool usingNI);
  SalpaParams params() const;
private slots:
  void autoEnable();
  void deviceChange();
private:
  class Ui_SalpaConfigTab *ui;
  bool usingIM;
  bool usingNI;
  QMap<QString, QStringList> linesbydev;
};

#endif
