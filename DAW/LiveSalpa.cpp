// LiveSalpa.cpp

#include "LiveSalpa.h"

#include "SalpaParams.h"
#include "CyclBuf.h"
#include "LocalFit.h"
#include "NoiseLevels.h"

#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <QMap>
#include <QDebug>

constexpr int NTRAININGCHUNKS = 10;

class LiveSalpa_Private {
public:
  SalpaParams pp;
  QMutex mutex;
  QMap<int, CyclBuf<raw_t> *> inbuf;
  QMap<int, CyclBuf<raw_t> *> outbuf;
  QMap<int, NoiseLevels *> noise;
  QMap<int, LocalFit *> fitters;
  timeref_t tfill;
  timeref_t dtlag;
  timeref_t dtforcepeg;
  bool forced = false;
  timeref_t tforcestart;
  timeref_t tforceend;
public:
  void updatefitter(int ic) {
    double sd = noise.contains(ic) ? noise[ic]->std() : 500;
    double pegthreshold =
      pp.detect_scaling == SalpaParams::Scaling::Absolute
      ? pp.detect_threshold
      : pp.detect_scaling == SalpaParams::Scaling::PercentRange
      ? pp.detect_threshold * 327.6
      : pp.detect_scaling == SalpaParams::Scaling::RMS
      ? pp.detect_threshold * sd
      : 5000;
    double recoverythreshold = 
      pp.recovery_scaling == SalpaParams::Scaling::Absolute
      ? pp.recovery_threshold
      : pp.recovery_scaling == SalpaParams::Scaling::PercentRange
      ? pp.recovery_threshold * 327.6
      : pp.recovery_scaling == SalpaParams::Scaling::RMS
      ? pp.recovery_threshold * sd
      : 1000;
    raw_t pegthr = 32767;
    if (pp.autodetect && pegthreshold < 32767)
      pegthr = raw_t(pegthreshold); 
    raw_t recoverythr = recoverythreshold < 32767
      ? raw_t(recoverythreshold)
      : raw_t(32767);
    fitters[ic]->setthreshold(recoverythr);
    fitters[ic]->setrail(-pegthr, pegthr);
  }
  
  void makefitter(int ic, double srate) {
    timeref_t start = tfill;
    timeref_t tau = timeref_t(srate * pp.window_ms / 2000.);
    timeref_t blankdepeg = timeref_t(srate * pp.recovery_blank_ms / 1000.);
    timeref_t ahead = timeref_t(srate * pp.lookahead_ms / 1000.);
    timeref_t tchi2 = timeref_t(srate * pp.recovery_window_ms / 1000.);
    dtforcepeg = timeref_t(srate * pp.forcepeg_ms / 1000.);
    fitters[ic] = new LocalFit(*inbuf[ic], *outbuf[ic],
                               start, 1000, tau,
                               blankdepeg, ahead,
                               tchi2);
    fitters[ic]->setusenegv(pp.recovery_zero_crossing);
    updatefitter(ic);
  }
};


template <class K, class V> void deleteAllExcept(QMap<K, V*> &map,
                                                 QSet<K> const &keep) {
  QSet<K> drop;
  for (auto it = map.begin(); it != map.end(); ++it)
    if (!keep.contains(it.key()))
      drop << it.key();
  for (int ic: drop) {
    delete map[ic];
    map.remove(ic);
  }
}

LiveSalpa::LiveSalpa(QObject *parent): QObject(parent) {
  d = new LiveSalpa_Private();
}

LiveSalpa::~LiveSalpa() {
  for (auto *lf: d->fitters)
    delete lf;
  for (auto *n: d->noise)
    delete n;
  for (auto *cb: d->inbuf)
    delete cb;
  for (auto *cb: d->outbuf)
    delete cb;
  delete d;
}

void LiveSalpa::setParams(SalpaParams const &pp) {
  QMutexLocker lock(&d->mutex);
  d->pp = pp;
  for (auto *lf: d->fitters)
    delete lf;
  d->fitters.clear();
  for (auto *n: d->noise)
    delete n;
  d->noise.clear();
  for (auto *cb: d->inbuf)
    delete cb;
  d->inbuf.clear();
  for (auto *cb: d->outbuf)
    delete cb;
  d->outbuf.clear();
  d->tfill = 0;
  d->dtlag = 0;
}

void LiveSalpa::process(qint16 *data, int nNu, int nC, int ntpts,
                        QVector<int> const &ic2iy, double srate) {
  qDebug() << "salpa process" << nNu << nC << ntpts << srate;
  QMutexLocker lock(&d->mutex);
  if (!d->pp.enable)
    return;

  timeref_t lag1 = timeref_t((d->pp.window_ms / 2 + d->pp.lookahead_ms)
                             * srate / 1000);
  timeref_t lag2 = timeref_t(d->pp.recovery_window_ms * srate / 1000);
  timeref_t lag = lag1 > lag2 ? lag1 : lag2;
  d->dtlag = lag;
  int minsize = ntpts + lag + 512;
  int minlog = 0;
  while ((1 << minlog) < minsize)
    ++minlog;

  // construct set of channels
  QSet<int> channels;
  for (int ic=0; ic<nNu; ic++)
    if (ic2iy[ic] >= 0)
      channels << ic;

  // train noise regardless of current use
  for (int ic=0; ic<nNu; ic++) {
    if (!d->noise.contains(ic))
      d->noise[ic] = new NoiseLevels();
    if (!d->noise[ic]->isready()) {
      d->noise[ic]->train(data + ic, ntpts, nC);
      if (d->noise[ic]->chunks() >= NTRAININGCHUNKS) {
        d->noise[ic]->makeready();
        if (ic==0)
          qDebug() << "noise " << ic << d->noise[ic]->mean() << d->noise[ic]->std();
        if (d->fitters.contains(ic))
          d->updatefitter(ic);
      }
    }      
  }

  std::uint32_t start = d->tfill;
  std::uint32_t end = start + ntpts;

  if (d->pp.digitaltrigger && !d->forced && nC > nNu) {
    // look for new forcing event
    qint16 *digi = data + nNu;
    for (int dt = 0; dt < ntpts; dt++) {
      if (*digi) {
        d->tforcestart = start + dt; // this can be past end - lag!
        d->tforceend = d->tforcestart + d->dtforcepeg;
        d->forced = true;
        break;
      }
      digi += nC;
    }
  }
  
  for (int ic: channels) {
    if (d->inbuf.contains(ic))
      d->inbuf[ic]->grow(minlog);
    else
      d->inbuf[ic] = new CyclBuf<raw_t>(minlog);
    d->inbuf[ic]->fill(data + ic, ntpts, nC,
                       start, d->noise[ic]->mean());
    
    if (d->outbuf.contains(ic))
      d->outbuf[ic]->grow(minlog);
    else
      d->outbuf[ic] = new CyclBuf<raw_t>(minlog);

    if (!d->fitters.contains(ic) && d->noise[ic]->isready())
      d->makefitter(ic, srate);

    if (d->fitters.contains(ic)) {
      timeref_t limit = end >= lag ? end - lag : 0u;
      if (d->forced && d->tforcestart < limit) {
        timeref_t end = d->tforceend;
        if (end > limit) {
          d->fitters[ic]->forcepeg(start, limit);
          // forced peg continues into next iteration
        } else {
          d->fitters[ic]->forcepeg(start, end);
          d->forced = false;
          d->fitters[ic]->process(limit);
        }
      } else {
        d->fitters[ic]->process(end - lag);
      }
      d->outbuf[ic]->grab(data + ic, ntpts, nC, start - lag);
    } else {
      d->inbuf[ic]->grab(data + ic, ntpts, nC, start - lag);
    }
  }
  d->tfill = end;

  // remove unused
  deleteAllExcept(d->inbuf, channels);
  deleteAllExcept(d->outbuf, channels);
  deleteAllExcept(d->fitters, channels);
}

void LiveSalpa::resetTraining() {
  for (auto *n: d->noise)
    n->reset();
}
