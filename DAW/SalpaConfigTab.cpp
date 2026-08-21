// SalpaConfigTab.cpp

#include "SalpaConfigTab.h"
#include "ui_SalpaConfigTab.h"
#include <QVariant>

SalpaConfigTab::SalpaConfigTab(QWidget *tab): QObject(0) {
  ui = new Ui_SalpaConfigTab();
  ui->setupUi(tab);
  autoEnable();
  connect(ui->enable, &QCheckBox::toggled,
          this, &SalpaConfigTab::autoEnable);
  connect(ui->digitaltrigger, &QGroupBox::toggled,
          this, &SalpaConfigTab::autoEnable);
  connect(ui->autodetect, &QGroupBox::toggled,
          this, &SalpaConfigTab::autoEnable);

  connect(ui->detectthresholdmode, &QComboBox::currentIndexChanged,
          this, &SalpaConfigTab::configDetectThreshold);
  connect(ui->recoverythresholdmode, &QComboBox::currentIndexChanged,
          this, &SalpaConfigTab::configRecoveryThreshold);

  // externally report changes
  connect(ui->enable, &QCheckBox::toggled,
          this, &SalpaConfigTab::changed);
  connect(ui->digitaltrigger, &QGroupBox::toggled,
          this, &SalpaConfigTab::changed);
  connect(ui->autodetect, &QGroupBox::toggled,
          this, &SalpaConfigTab::changed);
  connect(ui->forcepeg, &QDoubleSpinBox::valueChanged,
          this, &SalpaConfigTab::changed);
  connect(ui->window, &QDoubleSpinBox::valueChanged,
          this, &SalpaConfigTab::changed);
  connect(ui->detectthreshold, &QDoubleSpinBox::valueChanged,
          this, &SalpaConfigTab::changed);
  connect(ui->detectthresholdmode, &QComboBox::currentIndexChanged,
          this, &SalpaConfigTab::changed);
  connect(ui->lookahead, &QDoubleSpinBox::valueChanged,
          this, &SalpaConfigTab::changed);
  connect(ui->recoverythreshold, &QDoubleSpinBox::valueChanged,
          this, &SalpaConfigTab::changed);
  connect(ui->recoverythresholdmode, &QComboBox::currentIndexChanged,
          this, &SalpaConfigTab::changed);
  connect(ui->estimator, &QDoubleSpinBox::valueChanged,
          this, &SalpaConfigTab::changed);
  connect(ui->blank, &QDoubleSpinBox::valueChanged,
          this, &SalpaConfigTab::changed);
  connect(ui->detectzerocrossing, &QCheckBox::toggled,
          this, &SalpaConfigTab::changed);

  connect(ui->reset, &QToolButton::clicked,
          this, &SalpaConfigTab::resetPressed);
  
}

SalpaConfigTab::~SalpaConfigTab() {
}

void SalpaConfigTab::autoEnable() {
  // bool enabled = ui->enable->isChecked();
  // ui->general->setEnabled(enabled);
  // ui->recovery->setEnabled(enabled);
  // ui->autodetect->setEnabled(enabled);
  // ui->autodetectbox->setEnabled(enabled && ui->autodetect->isChecked());
  // ui->digitaltrigger->setEnabled(enabled);
}
    

void SalpaConfigTab::toGUI(SalpaParams const &ppsalpa) {
  ui->enable->setChecked(ppsalpa.enable);
  ui->window->setValue(ppsalpa.window_ms);
  ui->autodetect->setChecked(ppsalpa.autodetect);
  ui->detectthreshold->setValue(ppsalpa.detect_threshold);
  ui->detectthresholdmode->setCurrentIndex(
           ppsalpa.detect_scaling == SalpaParams::Scaling::PercentRange ? 0
           : ppsalpa.detect_scaling == SalpaParams::Scaling::RMS ? 1
           : 2);
  ui->lookahead->setValue(ppsalpa.lookahead_ms);
  ui->digitaltrigger->setChecked(ppsalpa.digitaltrigger);
  ui->forcepeg->setValue(ppsalpa.forcepeg_ms);
  ui->recoverythreshold->setValue(ppsalpa.recovery_threshold);
  ui->recoverythresholdmode->setCurrentIndex(
           ppsalpa.recovery_scaling == SalpaParams::Scaling::RMS ? 0
           : 1);
  ui->estimator->setValue(ppsalpa.recovery_window_ms);  
  ui->blank->setValue(ppsalpa.recovery_blank_ms);  
  ui->detectzerocrossing->setChecked(ppsalpa.recovery_zero_crossing);  

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
  pp.forcepeg_ms = ui->forcepeg->value();
  pp.recovery_threshold = ui->recoverythreshold->value();
  idx = ui->recoverythresholdmode->currentIndex();
  pp.recovery_scaling = idx == 0 ? SalpaParams::Scaling::RMS
    : SalpaParams::Scaling::Absolute; // percent range not supported here
  pp.recovery_window_ms = ui->estimator->value();
  pp.recovery_blank_ms = ui->blank->value();
  pp.recovery_zero_crossing = ui->detectzerocrossing->isChecked();
  return pp;
}

void SalpaConfigTab::configDetectThreshold() {
  switch (ui->detectthresholdmode->currentIndex()) {
  case 0: // RMS
    ui->detectthreshold->setMinimum(5);
    ui->detectthreshold->setMaximum(50);
    ui->detectthreshold->setDecimals(1);
    ui->detectthreshold->setSingleStep(1);
    break;
  case 1: // Percent range
    ui->detectthreshold->setMinimum(10);
    ui->detectthreshold->setMaximum(90);
    ui->detectthreshold->setDecimals(0);
    ui->detectthreshold->setSingleStep(10);
    break;
  case 2: // Absolute
    ui->detectthreshold->setMinimum(100);
    ui->detectthreshold->setMaximum(30000);
    ui->detectthreshold->setDecimals(0);
    ui->detectthreshold->setSingleStep(100);
    break;
  }
}

void SalpaConfigTab::configRecoveryThreshold() {
  switch (ui->recoverythresholdmode->currentIndex()) {
  case 0: // RMS
    ui->recoverythreshold->setMinimum(1);
    ui->recoverythreshold->setMaximum(5);
    ui->recoverythreshold->setDecimals(1);
    ui->recoverythreshold->setSingleStep(0.1);
    break;
  case 1: // Absolute
    ui->recoverythreshold->setMinimum(100);
    ui->recoverythreshold->setMaximum(30000);
    ui->recoverythreshold->setDecimals(0);
    ui->recoverythreshold->setSingleStep(100);
    break;
  }
}
