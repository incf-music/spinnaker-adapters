/*
 *  This file is part of spinnaker-adapters
 *
 *  Copyright (C) 2017 Mikael Djurfeldt <mikael@djurfeldt.com>
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "MusicOutputAdapter.h"

/* sleep */
#include <unistd.h>

MusicOutputAdapter::MusicOutputAdapter (Setup* setup,
					Runtime*& runtime,
					double timestep,
					double stoptime_,
					std::string label,
					int nUnits,
					std::string portName)
  : clock (timestep), isStopping (false), stoptime (stoptime_)
{
  if (pthread_mutex_init (&(this->music_mutex), NULL) == -1)
    throw std::runtime_error ("failed to initialize music mutex");
  if (pthread_mutex_init (&(this->start_mutex), NULL) == -1)
    throw std::runtime_error ("failed to initialize start mutex");
  if (pthread_cond_init (&(this->start_condition), NULL) == -1)
    throw std::runtime_error ("failed to initialize start condition");
  
  out = setup->publishEventOutput (portName);
  LinearIndex indices (0, nUnits);
  out->map (&indices, MUSIC::Index::GLOBAL);
  runtime = new Runtime (setup, timestep);
  this->runtime = runtime;
}


void
MusicOutputAdapter::spikes_start (char *label,
				  SpynnakerLiveSpikesConnection *connection)
{
  pthread_mutex_lock (&(this->start_mutex));
  std::cerr << "Starting the simulation\n";
  pthread_cond_signal (&(this->start_condition));
  pthread_mutex_unlock (&(this->start_mutex));
  std::cerr << "Start signal sent\n";
}


void
MusicOutputAdapter::waitForStart ()
{
  pthread_mutex_lock (&(this->start_mutex));
  pthread_cond_wait (&(this->start_condition), &(this->start_mutex));
  pthread_mutex_unlock (&(this->start_mutex));
}


void
MusicOutputAdapter::spikes_stop (char *label,
				 SpynnakerLiveSpikesConnection *connection)
{
  std::cerr << "Stopping the simulation\n";
  pthread_mutex_lock (&(this->start_mutex));
  isStopping = true;
  pthread_cond_wait (&(this->start_condition), &(this->start_mutex));
  isStopping = false;
  pthread_mutex_unlock (&(this->start_mutex));
}


void
MusicOutputAdapter::stop ()
{
  std::cerr << "Stopping the simulation\n";
  pthread_mutex_lock (&(this->start_mutex));
  pthread_cond_signal (&(this->start_condition));
  pthread_mutex_unlock (&(this->start_mutex));
  std::cerr << "Stop signal sent\n";
}


void
MusicOutputAdapter::receive_spikes (char *label,
				    int time,
				    int n_spikes,
				    int *spikes)
{
  double t = 1e-3 * time;
  clock.set (t); // synchronize with SpiNNaker
  pthread_mutex_lock (&(this->music_mutex));
  for (int i = 0; i < n_spikes; i++)
    {
      int id = spikes[i];
      out->insertEvent (t, MUSIC::GlobalIndex (id));
    }
  pthread_mutex_unlock (&(this->music_mutex));
}


void MusicOutputAdapter::main_loop() {
  clock.resetAndStop ();
  waitForStart ();
  clock.start ();
  while (clock.time () < stoptime)
    {
      clock.setNextTarget ();
      while (!clock.pastTarget ())
	{
	  if (isStopping)
	    goto stop;
	  sched_yield ();
	}
      pthread_mutex_lock (&(this->music_mutex));
      runtime->tick ();
      pthread_mutex_unlock (&(this->music_mutex));
      continue;
      
    stop:
      clock.stop ();
      stop ();
      waitForStart ();
      clock.start ();
    }
}


MusicOutputAdapter::~MusicOutputAdapter()
{
}
