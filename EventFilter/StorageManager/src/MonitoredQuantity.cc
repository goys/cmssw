// $Id: MonitoredQuantity.cc,v 1.11 2010/12/15 15:29:23 mommsen Exp $
/// @file: MonitoredQuantity.cc

#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"

#include <algorithm>
#include <math.h>

using namespace stor;


MonitoredQuantity::MonitoredQuantity
(
  utils::duration_t expectedCalculationInterval,
  utils::duration_t timeWindowForRecentResults
):
_enabled(true),
_expectedCalculationInterval(expectedCalculationInterval)
{
  setNewTimeWindowForRecentResults(timeWindowForRecentResults);
}

void MonitoredQuantity::addSample(const double& value)
{
  if (! _enabled) {return;}

  boost::mutex::scoped_lock sl(_accumulationMutex);

  if ( _lastCalculationTime.is_not_a_date_time() )
  {
    _lastCalculationTime = utils::getCurrentTime();
  }

  ++_workingSampleCount;
  _workingValueSum += value;
  _workingValueSumOfSquares += (value * value);

  if (value < _workingValueMin) _workingValueMin = value;
  if (value > _workingValueMax) _workingValueMax = value;

  _workingLastSampleValue = value;
}

void MonitoredQuantity::addSample(const int& value)
{
  addSample(static_cast<double>(value));
}

void MonitoredQuantity::addSample(const unsigned int& value)
{
  addSample(static_cast<double>(value));
}

void MonitoredQuantity::addSample(const long& value)
{
  addSample(static_cast<double>(value));
}

void MonitoredQuantity::addSample(const unsigned long& value)
{
  addSample(static_cast<double>(value));
}

void MonitoredQuantity::addSample(const long long& value)
{
  addSample(static_cast<double>(value));
}

void MonitoredQuantity::addSample(const unsigned long long& value)
{
  addSample(static_cast<double>(value));
}

void MonitoredQuantity::addSampleIfLarger(const double& value)
{
  if (value > _workingLastSampleValue)
    addSample(value);
}

void MonitoredQuantity::calculateStatistics(const utils::time_point_t& currentTime)
{
  if (! _enabled) {return;}
  if (_lastCalculationTime.is_not_a_date_time()) {return;}
  if (currentTime - _lastCalculationTime < _expectedCalculationInterval) {return;}

  // create local copies of the working values to minimize the
  // time that we could block a thread trying to add a sample.
  // Also, reset the working values.
  long long latestSampleCount;
  double latestValueSum;
  double latestValueSumOfSquares;
  double latestValueMin;
  double latestValueMax;
  utils::duration_t latestDuration;
  utils::time_point_t latestSnapshotTime;
  double latestLastLatchedSampleValue;
  {
    boost::mutex::scoped_lock sl(_accumulationMutex);

    latestSampleCount = _workingSampleCount;
    latestValueSum = _workingValueSum;
    latestValueSumOfSquares = _workingValueSumOfSquares;
    latestValueMin = _workingValueMin;
    latestValueMax = _workingValueMax;
    latestDuration = currentTime - _lastCalculationTime;
    latestSnapshotTime = currentTime;
    latestLastLatchedSampleValue = _workingLastSampleValue;

    _lastCalculationTime = currentTime;
    _workingSampleCount = 0;
    _workingValueSum = 0.0;
    _workingValueSumOfSquares = 0.0;
    _workingValueMin =  INFINITY;
    _workingValueMax = -INFINITY;
  }

  // lock out any interaction with the results while we update them
  {
    boost::mutex::scoped_lock sl(_resultsMutex);
    _lastLatchedSampleValue = latestLastLatchedSampleValue;

    // we simply add the latest results to the full set
    _fullSampleCount += latestSampleCount;
    _fullValueSum += latestValueSum;
    _fullValueSumOfSquares += latestValueSumOfSquares;
    if (latestValueMin < _fullValueMin) {_fullValueMin = latestValueMin;}
    if (latestValueMax > _fullValueMax) {_fullValueMax = latestValueMax;}
    _fullDuration += latestDuration;

    // for the recent results, we need to replace the contents of
    // the working bin and re-calculate the recent values
    _binSampleCount[_workingBinId] = latestSampleCount;
    _binValueSum[_workingBinId] = latestValueSum;
    _binValueSumOfSquares[_workingBinId] = latestValueSumOfSquares;
    _binValueMin[_workingBinId] = latestValueMin;
    _binValueMax[_workingBinId] = latestValueMax;
    _binDuration[_workingBinId] = latestDuration;
    _binSnapshotTime[_workingBinId] = latestSnapshotTime;

    _lastLatchedValueRate = latestValueSum / utils::duration_to_seconds(latestDuration);

    _recentSampleCount = 0;
    _recentValueSum = 0.0;
    _recentValueSumOfSquares = 0.0;
    _recentValueMin =  INFINITY;
    _recentValueMax = -INFINITY;
    _recentDuration = boost::posix_time::seconds(0);

    for (unsigned int idx = 0; idx < _binCount; ++idx) {
      _recentSampleCount += _binSampleCount[idx];
      _recentValueSum += _binValueSum[idx];
      _recentValueSumOfSquares += _binValueSumOfSquares[idx];
      if (_binValueMin[idx] < _recentValueMin) {
        _recentValueMin = _binValueMin[idx];
      }
      if (_binValueMax[idx] > _recentValueMax) {
        _recentValueMax = _binValueMax[idx];
      }
      _recentDuration += _binDuration[idx];
    }

    // update the working bin ID here so that we are ready for
    // the next calculation request
    ++_workingBinId;
    if (_workingBinId >= _binCount) {_workingBinId = 0;}

    // calculate the derived full values
    const double fullDuration = utils::duration_to_seconds(_fullDuration);
    _fullSampleRate = _fullSampleCount / fullDuration;
    _fullValueRate = _fullValueSum / fullDuration;

    if (_fullSampleCount > 0) {
      _fullValueAverage = _fullValueSum / static_cast<double>(_fullSampleCount);

      double squareAvg = _fullValueSumOfSquares / static_cast<double>(_fullSampleCount);
      double avg = _fullValueSum / static_cast<double>(_fullSampleCount);
      double sigSquared = squareAvg - avg*avg;
      if(sigSquared > 0.0) {
        _fullValueRMS = sqrt(sigSquared);
      }
      else {
        _fullValueRMS = 0.0;
      }
    }
    else {
      _fullValueAverage = 0.0;
      _fullValueRMS = 0.0;
    }

    // calculate the derived recent values
    const double recentDuration = utils::duration_to_seconds(_recentDuration);
    if (recentDuration > 0) {
      _recentSampleRate = _recentSampleCount / recentDuration;
      _recentValueRate = _recentValueSum / recentDuration;
    }
    else {
      _recentSampleRate = 0.0;
      _recentValueRate = 0.0;
    }

    if (_recentSampleCount > 0) {
      _recentValueAverage = _recentValueSum / static_cast<double>(_recentSampleCount);

      double squareAvg = _recentValueSumOfSquares /
        static_cast<double>(_recentSampleCount);
      double avg = _recentValueSum / static_cast<double>(_recentSampleCount);
      double sigSquared = squareAvg - avg*avg;
      if(sigSquared > 0.0) {
        _recentValueRMS = sqrt(sigSquared);
      }
      else {
        _recentValueRMS = 0.0;
      }
    }
    else {
      _recentValueAverage = 0.0;
      _recentValueRMS = 0.0;
    }
  }
}

void MonitoredQuantity::_reset_accumulators()
{
  _lastCalculationTime = boost::posix_time::not_a_date_time;
  _workingSampleCount = 0;
  _workingValueSum = 0.0;
  _workingValueSumOfSquares = 0.0;
  _workingValueMin =  INFINITY;
  _workingValueMax = -INFINITY;
  _workingLastSampleValue = 0;
}

void MonitoredQuantity::_reset_results()
{
  _workingBinId = 0;
  for (unsigned int idx = 0; idx < _binCount; ++idx) {
    _binSampleCount[idx] = 0;
    _binValueSum[idx] = 0.0;
    _binValueSumOfSquares[idx] = 0.0;
    _binValueMin[idx] =  INFINITY;
    _binValueMax[idx] = -INFINITY;
    _binDuration[idx] = boost::posix_time::seconds(0);
    _binSnapshotTime[idx] = boost::posix_time::not_a_date_time;
  }

  _fullSampleCount = 0;
  _fullSampleRate = 0.0;
  _fullValueSum = 0.0;
  _fullValueSumOfSquares = 0.0;
  _fullValueAverage = 0.0;
  _fullValueRMS = 0.0;
  _fullValueMin =  INFINITY;
  _fullValueMax = -INFINITY;
  _fullValueRate = 0.0;
  _fullDuration = boost::posix_time::seconds(0);

  _recentSampleCount = 0;
  _recentSampleRate = 0.0;
  _recentValueSum = 0.0;
  _recentValueSumOfSquares = 0.0;
  _recentValueAverage = 0.0;
  _recentValueRMS = 0.0;
  _recentValueMin =  INFINITY;
  _recentValueMax = -INFINITY;
  _recentValueRate = 0.0;
  _recentDuration = boost::posix_time::seconds(0);
  _lastLatchedSampleValue = 0.0;
  _lastLatchedValueRate = 0.0;
}

void MonitoredQuantity::reset()
{
  {
    boost::mutex::scoped_lock sl(_accumulationMutex);
    _reset_accumulators();
  }

  {
    boost::mutex::scoped_lock sl(_resultsMutex);
    _reset_results();
  }
}

void MonitoredQuantity::enable()
{
  if (! _enabled) {
    reset();
    _enabled = true;
  }
}

void MonitoredQuantity::disable()
{
  // It is faster to just set _enabled to false than to test and set
  // it conditionally.
  _enabled = false;
}

void MonitoredQuantity::setNewTimeWindowForRecentResults(const utils::duration_t& interval)
{
  // lock the results objects since we're dramatically changing the
  // bins used for the recent results
  {
    boost::mutex::scoped_lock sl(_resultsMutex);

    _intervalForRecentStats = interval;

    // determine how many bins we should use in our sliding window
    // by dividing the input time window by the expected calculation
    // interval and rounding to the nearest integer.
    // In case that the calculation interval is larger then the 
    // interval for recent stats, keep the last one.
    _binCount = std::max(1U,
      static_cast<unsigned int>(
        (_intervalForRecentStats.total_nanoseconds() / _expectedCalculationInterval.total_nanoseconds()) + 0.5
      )      
    );

    // create the vectors for the binned quantities
    _binSampleCount.reserve(_binCount);
    _binValueSum.reserve(_binCount);
    _binValueSumOfSquares.reserve(_binCount);
    _binValueMin.reserve(_binCount);
    _binValueMax.reserve(_binCount);
    _binDuration.reserve(_binCount);
    _binSnapshotTime.reserve(_binCount);

    _reset_results();
  }

  {
    boost::mutex::scoped_lock sl(_accumulationMutex);
    _reset_accumulators();
  }

  // call the reset method to populate the correct initial values
  // for the internal sample data
  //reset();
}

void
MonitoredQuantity::getStats(Stats& s) const
{
  boost::mutex::scoped_lock results(_resultsMutex);

  s.fullSampleCount = _fullSampleCount;
  s.fullSampleRate = _fullSampleRate;
  s.fullValueSum = _fullValueSum;
  s.fullValueSumOfSquares = _fullValueSumOfSquares;
  s.fullValueAverage = _fullValueAverage;
  s.fullValueRMS = _fullValueRMS;
  s.fullValueMin = _fullValueMin;
  s.fullValueMax = _fullValueMax;
  s.fullValueRate = _fullValueRate;
  s.fullDuration = _fullDuration;

  s.recentSampleCount = _recentSampleCount;
  s.recentSampleRate = _recentSampleRate;
  s.recentValueSum = _recentValueSum;
  s.recentValueSumOfSquares = _recentValueSumOfSquares;
  s.recentValueAverage = _recentValueAverage;
  s.recentValueRMS = _recentValueRMS;
  s.recentValueMin = _recentValueMin;
  s.recentValueMax = _recentValueMax;
  s.recentValueRate = _recentValueRate;
  s.recentDuration = _recentDuration;

  s.recentBinnedSampleCounts.resize(_binCount);
  s.recentBinnedValueSums.resize(_binCount);
  s.recentBinnedDurations.resize(_binCount);
  s.recentBinnedSnapshotTimes.resize(_binCount);
  uint32_t sourceBinId = _workingBinId;
  for (uint32_t idx = 0; idx < _binCount; ++idx) {
    if (sourceBinId >= _binCount) {sourceBinId = 0;}
    s.recentBinnedSampleCounts[idx] = _binSampleCount[sourceBinId];
    s.recentBinnedValueSums[idx] = _binValueSum[sourceBinId];
    s.recentBinnedDurations[idx] = _binDuration[sourceBinId];
    s.recentBinnedSnapshotTimes[idx] = _binSnapshotTime[sourceBinId];
    ++sourceBinId;
  }

  s.lastSampleValue = _lastLatchedSampleValue;
  s.lastValueRate = _lastLatchedValueRate;
  s.enabled = _enabled;
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -