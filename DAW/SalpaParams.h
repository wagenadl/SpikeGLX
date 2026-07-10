// SalpaParams.h

#ifndef SALPAPARAMS_H

#define SALPAPARAMS_H

#include <QString>

struct SalpaParams {
  enum class Scaling {
    Absolute,
    PercentRange,
    RMS,
  };
  bool enable = false;
  float window_ms = 3.0;
  bool autodetect = true;
  float detect_threshold = 80;
  Scaling detect_scaling = Scaling::PercentRange;
  float lookahead_ms = 0.20;
  bool digitaltrigger = false;
  QString trigger_device;
  QString trigger_line;
  float recovery_threshold = 3.0;
  Scaling recovery_scaling = Scaling::RMS;
  float recovery_window_ms = 0.20;
  float recovery_blank_ms = 0.40;
  bool recovery_zero_crossing = true;
public:
  SalpaParams();
  void saveSettings(class QSettings &settings) const;
  void loadSettings(class QSettings &settings);
};

#endif
