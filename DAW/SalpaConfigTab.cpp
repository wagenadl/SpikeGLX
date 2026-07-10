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

  // TODO: this doesn't update on “Devices:Detect”
  ui->device->clear();
  if (!pp.ni.dev1.isEmpty())
    ui->device->addItem(pp.ni.dev1, QVariant(1));
  if (!pp.ni.dev2.isEmpty() && pp.ni.dev2 != pp.ni.dev1)
    ui->device->addItem(pp.ni.dev2, QVariant(1));

  ui->enable->setChecked(pp.salpa.enable);
  ui->window->setValue(pp.salpa.window_ms);
  ui->autodetect->setChecked(pp.salpa.autodetect);
  ui->detectthreshold->setValue(pp.salpa.detect_threshold);
  ui->detectthresholdmode->setCurrentIndex(
           pp.salpa.detect_scaling == SalpaParams::Scaling::PercentRange ? 0
           : pp.salpa.detect_scaling == SalpaParams::Scaling::RMS ? 1
           : 2);
  ui->lookahead->setValue(pp.salpa.lookahead_ms);
  ui->digitaltrigger->setChecked(pp.salpa.digitaltrigger);
  for (int k = 0; k < ui->device->count(); k++)
    if (ui->device->itemText(k) == pp.salpa.trigger_device)
      ui->device->setCurrentIndex(k);

  deviceChange();

  for (int k = 0; k < ui->line->count(); k++)
    if (ui->line->itemText(k) == pp.salpa.trigger_line)
      ui->line->setCurrentIndex(k);
  ui->recoverythreshold->setValue(pp.salpa.recovery_threshold);
  ui->recoverythresholdmode->setCurrentIndex(
           pp.salpa.recovery_scaling == SalpaParams::Scaling::RMS ? 0
           : 1);
  ui->estimator->setValue(pp.salpa.recovery_window_ms);  
  ui->blank->setValue(pp.salpa.recovery_blank_ms);  
  ui->detectzerocrossing->setChecked(pp.salpa.recovery_zero_crossing);  

  autoEnable();
}

SalpaParams SalpaConfigTab::params() const {
  SalpaParams pp;
  pp.enable = ui->enable->isChecked();
  pp.window_ms = ui->window->value();
  pp.autodetect = ui->autodetect->isChecked();
  pp.detect_threshold = ui->detectthreshold->value();
  int idx = ui->detectthresholdmode->currentIndex();
  pp.detect_scaling = idx == 0 ? SalpaParams::Scaling::RMS
    : 1 ? SalpaParams::Scaling::PercentRange
    : SalpaParams::Scaling::Absolute;
  pp.lookahead_ms = ui->lookahead->value();
  pp.digitaltrigger = ui->digitaltrigger->isChecked();
  pp.trigger_device = ui->device->currentText();
  pp.trigger_line = ui->line->currentText();
  pp.recovery_threshold = ui->recoverythreshold->value();
  idx = ui->recoverythresholdmode->currentIndex();
  pp.recovery_scaling = idx == 0 ? SalpaParams::Scaling::RMS
    : SalpaParams::Scaling::Absolute; // percent range not supported here
  pp.recovery_window_ms = ui->estimator->value();
  pp.recovery_blank_ms = ui->blank->value();
  pp.recovery_zero_crossing = ui->detectzerocrossing->isChecked();
  return pp;
}
