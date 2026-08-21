// LVE_salpa.cpp

#include "ToolBut.h"
#include "SalpaConfigTab.h"
#include <QString>
#include <QDebug>

LVE_salpa::LVE_salpa(QWidget *parent):
  LVEditor("Suppression of Artifacts by Local Polynomial Approximation",
           "Off", parent) {
  QWidget *container = new QWidget(this);
  m_salpaBox = new SalpaConfigTab(container);
  layout()->addWidget(container);
  connect(m_salpaBox, &SalpaConfigTab::changed,
          this, &LVE_salpa::report);
  connect(m_salpaBox, &SalpaConfigTab::resetPressed,
          this, &LVE_salpa::reset);
  
}

void LVE_salpa::setValue(const QString &) {
  // qDebug() << "LVE_salpa set value" << val;
}

void LVE_salpa::setParams(class SalpaParams const &ppsalpa) {
  qDebug() << "LVE_salpa set params";
  m_salpaBox->toGUI(ppsalpa);
}

void LVE_salpa::getParams(class SalpaParams &ppsalpa) const {
  SalpaParams pp = m_salpaBox->params();
  ppsalpa = pp;
}

void LVE_salpa::setFocus() {
}

void LVE_salpa::report() {
  SalpaParams pp = m_salpaBox->params();
  emit changed(pp.enable ? "On": "Off");
}

