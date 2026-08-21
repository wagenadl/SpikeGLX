// Salpaconfigtab.h

#ifndef SALPACONFIGTAB_H

#define SALPACONFIGTAB_H

#include <QWidget>

#include "SalpaParams.h"

class SalpaConfigTab: public QObject {
  Q_OBJECT;
public:
  SalpaConfigTab(QWidget *tab);
  virtual ~SalpaConfigTab();
  void toGUI(SalpaParams const &);
  SalpaParams params() const;
signals:
  void changed();
  void resetPressed();
private slots:
  void autoEnable();
  void configDetectThreshold();
  void configRecoveryThreshold();
private:
  class Ui_SalpaConfigTab *ui;
};

#endif
