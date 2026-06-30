// SalpaConfigTab.cpp

#include "SalpaConfigTab.h"
#include "ui_SalpaConfigTab.h"
#include <QVariant>

SalpaConfigTab::SalpaConfigTab(QWidget *tab): QObject(0) {
  ui = new Ui_SalpaConfigTab();
  ui->setupUi(tab);
  usingIM = false;
  usingNI = false;
  autoEnable();
  connect(ui->enable, &QCheckBox::toggled,
          this, &SalpaConfigTab::autoEnable);
  connect(ui->digitaltrigger, &QGroupBox::toggled,
          this, &SalpaConfigTab::autoEnable);
  connect(ui->autodetect, &QGroupBox::toggled,
          this, &SalpaConfigTab::autoEnable);
  connect(ui->device, &QComboBox::currentIndexChanged,
          this, &SalpaConfigTab::deviceChange);
}

SalpaConfigTab::~SalpaConfigTab() {
}

void SalpaConfigTab::deviceChange() {
  qDebug() <<"salpaconfig: devicechange";
  ui->line->clear();
  for (QString const &line: linesbydev[ui->device->currentText()])
    ui->line->addItem(line);
}

void SalpaConfigTab::autoEnable() {
  qDebug() << "salpaconfig: autoenable";
  ui->enable->setEnabled(usingIM);

  bool enabled = usingIM && ui->enable->isChecked();
  ui->general->setEnabled(enabled);
  ui->recovery->setEnabled(enabled);
  ui->autodetect->setEnabled(enabled);
  ui->digitaltrigger->setEnabled(enabled && usingNI);
}
    

void SalpaConfigTab::toGUI(DAQ::Params const &pp,
                           bool usingIM1, bool usingNI1) {
  usingIM = usingIM1;
  usingNI = usingNI1;
  linesbydev.clear();
  for (QString const &devline: pp.ni.getAllDOLines()) {
    QStringList bits = devline.split("/");
    QString dev = bits.takeFirst();
    linesbydev[dev] << bits.join("/");
  }

  autoEnable();

  ui->device->clear();
  if (!pp.ni.dev1.isEmpty())
    ui->device->addItem(pp.ni.dev1, QVariant(1));
  if (!pp.ni.dev2.isEmpty() && pp.ni.dev2 != pp.ni.dev1)
    ui->device->addItem(pp.ni.dev2, QVariant(1));

  deviceChange();
}
