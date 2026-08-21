// SalpaParams.cpp

#include "SalpaParams.h"
#include <QSettings>

SalpaParams::SalpaParams() {
}

void SalpaParams::loadSettings(QSettings &settings) {
  settings.beginGroup("SALPA");
  enable = settings.value("enable", false).toBool();
  window_ms = settings.value("window_ms", 3.0).toFloat();
  autodetect = settings.value("autodetect", true).toBool();
  detect_threshold = settings.value("detect_threshold", 80.).toFloat();
  QString idx = settings.value("detect_scaling", "range").toString();
  detect_scaling = idx == "range" ? Scaling::PercentRange
    : idx == "rms" ? Scaling::RMS
    : Scaling::Absolute;
  lookahead_ms = settings.value("lookahead_ms", 0.20).toFloat();
  digitaltrigger = settings.value("digitaltrigger", false).toBool();
  forcepeg_ms = settings.value("forcepeg_ms", 5.0).toFloat();
  recovery_threshold = settings.value("recovery_threshold", 3.0).toFloat();
  idx = settings.value("recovery_scaling", "rms").toString();
  recovery_scaling = idx == "rms" ? Scaling::RMS
    : Scaling::Absolute;
  recovery_window_ms = settings.value("recovery_window_ms", 0.20).toFloat();
  recovery_blank_ms = settings.value("recovery_blank_ms", 0.40).toFloat();
  recovery_zero_crossing = settings.value("recovery_zero_crossing", true).toBool();
  settings.endGroup();
}

void SalpaParams::saveSettings(QSettings &settings) const {
  settings.beginGroup("SALPA");
  settings.setValue("enable", enable);
  settings.setValue("window_ms", QString::number(window_ms, 'f', 2));
  settings.setValue("autodetect", autodetect);
  settings.setValue("detect_threshold",  QString::number(detect_threshold, 'f', 2));
  settings.setValue("detect_scaling",
                    detect_scaling == Scaling::PercentRange ? "range"
                    : detect_scaling == Scaling::RMS ? "rms"
                    : "abs");
  settings.setValue("lookahead_ms", QString::number(lookahead_ms, 'f', 2));
  settings.setValue("digitaltrigger", digitaltrigger);
  settings.setValue("forcepeg_ms", QString::number(forcepeg_ms, 'f', 2));
  settings.setValue("recovery_threshold", QString::number(recovery_threshold, 'f', 2));
  settings.setValue("recovery_scaling",
                    recovery_scaling == Scaling::RMS ? "rms"
                    : "abs");
  settings.setValue("recovery_window_ms", QString::number(recovery_window_ms, 'f', 2));
  settings.setValue("recovery_blank_ms", QString::number(recovery_blank_ms, 'f', 2));
  settings.setValue("recovery_zero_crossing", recovery_zero_crossing);
  settings.endGroup();
}

