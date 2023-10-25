/*
 *  This file is part of spinnaker-adapters
 *
 *  Copyright (C) 2017, 2018, 2022, 2023 Mikael Djurfeldt <mikael@djurfeldt.com>
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
#include <deque>
#include <set>
#include <pthread.h>
#include <music.hh>

using namespace MUSIC;

class MusicOutputAdapter
: public SpikeReceiveCallbackInterface,
  public SpikesStartCallbackInterface,
  public SpikesPauseStopCallbackInterface
{
  
public:
    MusicOutputAdapter (Setup* setup,
			Runtime*& runtime,
			double timestep,
			double delay,
			double stopTime,
			std::string label,
			int nUnits,
			std::string portName,
			bool useBarrier = false);
    void main_loop();
    virtual void spikes_start (char *label,
			       SpynnakerLiveSpikesConnection *connection);
    virtual void spikes_stop (char *label,
			      SpynnakerLiveSpikesConnection *connection);
    virtual void receive_spikes(char *label,
				int time,
				int n_spikes,
				int* spikes);
    virtual ~MusicOutputAdapter();

private:

    void waitForStart ();
    void stop ();
    
    Runtime* runtime;
    EventOutputPort* out;
    RTClock clock;
    double delay;
    bool isStopping;
    double stoptime;

    pthread_mutex_t music_mutex;
    pthread_mutex_t start_mutex;
    pthread_cond_t start_condition;
};

#endif /* MUSICOUTPUTADAPTER_H */
