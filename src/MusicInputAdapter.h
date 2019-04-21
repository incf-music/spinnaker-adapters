/*
 *  This file is part of spinnaker-adapters
 *
 *  Copyright (C) 2017, 2018 Mikael Djurfeldt <mikael@djurfeldt.com>
 *
 *  libneurosim is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libneurosim is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MUSICOUTPUTADAPTER_H
#define MUSICOUTPUTADAPTER_H

#include "rtclock.h"
#include <SpynnakerLiveSpikesConnection.h>
#include <map>
#include <queue>
#include <set>
#include <pthread.h>
#include <music.hh>

using namespace MUSIC;

// Inner class used in priority queue
class TimeIdPair
{
 public:

  //TimeIdPair () { time_ = -1; };
  TimeIdPair (double time, MUSIC::GlobalIndex id) {
    time_ = RTClock::timespecFromSeconds (time);
    id_ = id;
  }

  bool operator< (const TimeIdPair& right) const {
    // Note that we use > here, since we want lowest items first
    return timespeccmp (&time_, right.time (), >);
  }

  const struct timespec* time () const { return &time_; }
  MUSIC::GlobalIndex id () const { return id_; }

 private:
  struct timespec time_;
  MUSIC::GlobalIndex id_;
};


class MIAEventHandler: public MUSIC::EventHandlerGlobalIndex {
public:
  MIAEventHandler (std::priority_queue<TimeIdPair>& spikes_)
    : spikes (spikes_) { }
  
  void operator () (double t, MUSIC::GlobalIndex id)
  {
    spikes.push (TimeIdPair (t, id));
  }

 private:
  std::priority_queue<TimeIdPair>& spikes;
};


class MusicInputAdapter
: public SpikesStartCallbackInterface,
  public SpikesPauseStopCallbackInterface
{
  
public:
    MusicInputAdapter (Setup* setup,
		       Runtime*& runtime,
		       double timestep,
		       double stopTime,
		       std::string label,
		       int nUnits,
		       std::string portName,
		       bool useBarrier = false);
    virtual ~MusicInputAdapter();
    
    void main_loop();
    virtual void spikes_start (char *label,
			       SpynnakerLiveSpikesConnection *connection);
    virtual void spikes_stop (char *label,
			      SpynnakerLiveSpikesConnection *connection);

private:

    void waitForStart ();
    void stop ();
    
    Runtime* runtime;
    EventInputPort* in;
    RTClock clock;
    bool isStopping;
    double stoptime;
    std::string label;
    
    pthread_mutex_t music_mutex;
    pthread_mutex_t start_mutex;
    pthread_cond_t start_condition;

    SpynnakerLiveSpikesConnection* connection;
    std::priority_queue<TimeIdPair> spikes;
    MIAEventHandler* eventHandler;
};

#endif /* MUSICINPUTADAPTER_H */
