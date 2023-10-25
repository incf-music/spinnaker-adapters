/*
 *  This file is part of spinnaker-adapters
 *
 *  Copyright (C) 2017, 2018, 2019, 2021, 2022, 2023 Mikael Djurfeldt <mikael@djurfeldt.com>
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
#include "MusicInputAdapter.h"

/* sleep */
#include <unistd.h>



MusicInputAdapter::MusicInputAdapter (Setup* setup,
				      Runtime*& runtime,
				      double timestep,
				      double delay,
				      int maxBuffered,
				      double stoptime_,
				      std::string label_,
				      int nUnits,
				      std::string portName,
				      bool useBarrier,
				      double sync_)
  : clock (timestep), syncClock (sync_), isStopping (false), stoptime (stoptime_), label (label_), sync (sync_)
{
  if (pthread_mutex_init (&(this->music_mutex), NULL) == -1)
    throw std::runtime_error ("failed to initialize music mutex");
  if (pthread_mutex_init (&(this->start_mutex), NULL) == -1)
    throw std::runtime_error ("failed to initialize start mutex");
  if (pthread_cond_init (&(this->start_condition), NULL) == -1)
    throw std::runtime_error ("failed to initialize start condition");
  
  in = setup->publishEventInput (portName);
  LinearIndex indices (0, nUnits);
  eventHandler = new MIAEventHandler (spikes, delay);
  if (maxBuffered > 0)
    in->map (&indices, eventHandler, 0.0, maxBuffered);
  else
    in->map (&indices, eventHandler);
  if (useBarrier)
    MPI::COMM_WORLD.Barrier();
  runtime = new Runtime (setup, timestep);
  this->runtime = runtime;
}


MusicInputAdapter::~MusicInputAdapter ()
{
  delete eventHandler;
  delete runtime;
}

void
MusicInputAdapter::spikes_start (char *label,
				 SpynnakerLiveSpikesConnection *connection_)
{
  connection = connection_;
  
  pthread_mutex_lock (&(this->start_mutex));
  std::cerr << "MO: Starting the simulation\n";
  pthread_cond_signal (&(this->start_condition));
  pthread_mutex_unlock (&(this->start_mutex));
  std::cerr << "MO: Start signal sent\n";
}


void
MusicInputAdapter::waitForStart ()
{
  pthread_mutex_lock (&(this->start_mutex));
  std::cerr << "MO: Waiting for start\n";
  pthread_cond_wait (&(this->start_condition), &(this->start_mutex));
  pthread_mutex_unlock (&(this->start_mutex));
}


// Callback from SpiNNaker
void
MusicInputAdapter::spikes_stop (char *label,
				SpynnakerLiveSpikesConnection *connection)
{
  std::cerr << "MO: Stopping the simulation\n";
  pthread_mutex_lock (&(this->start_mutex));
  isStopping = true;
  pthread_cond_wait (&(this->start_condition), &(this->start_mutex));
  isStopping = false;
  pthread_mutex_unlock (&(this->start_mutex));
}


void
MusicInputAdapter::stop ()
{
  pthread_mutex_lock (&(this->start_mutex));
  pthread_cond_signal (&(this->start_condition));
  pthread_mutex_unlock (&(this->start_mutex));
  std::cerr << "MO: Stopped\n";
}

// Use templates instead

void MusicInputAdapter::main_loop() {
  if (sync <= 0.0)
    main_loop_nosync ();
  else
    main_loop_sync ();
  runtime->finalize ();
}

void MusicInputAdapter::main_loop_nosync() {
  clock.resetAndStop ();
  waitForStart ();
  clock.start ();
  while (clock.time () < stoptime)
    {
      clock.setNextTarget ();
      // Send all spikes until next target.

      struct timespec t;
      clock.getTime (&t);
      while (!clock.pastTarget (t))
	{
	  if (isStopping)
	    goto stop;
	  if (!spikes.empty () && clock.lessThanEql (spikes.top ().time (), &t))
	    {
	      connection->send_spike ((char *) label.c_str (), spikes.top ().id ());
	      //std::cerr << RTClock::secondsFromTimespec(*spikes.top().time()) << '\t' << clock.time() << '\n';
	      spikes.pop ();
	    }
	  else
	    sched_yield ();
	  clock.getTime (&t);
	}
      runtime->tick ();
      continue;
      
    stop:
      clock.stop ();
      stop ();
      break;
    }

}

void MusicInputAdapter::main_loop_sync() {
  clock.resetAndStop ();
  waitForStart ();
  clock.start ();
  while (clock.time () < stoptime)
    {
      clock.setNextTarget ();
      // Send all spikes until next target.

      struct timespec t;
      clock.getTime (&t);
      while (!clock.pastTarget (t))
	{
	  if (isStopping)
	    goto stop;
	  if (!spikes.empty () && clock.lessThanEql (spikes.top ().time (), &t))
	    {
	      connection->send_spike ((char *) label.c_str (), spikes.top ().id ());
	      //std::cerr << RTClock::secondsFromTimespec(*spikes.top().time()) << '\t' << clock.time() << '\n';
	      spikes.pop ();
	    }
	  else
	    sched_yield ();
	  clock.getTime (&t);
	}
      clock.stop ();
      runtime->tick ();
      usleep (1000);
      connection->continue_run ();
      clock.start ();
      continue;
      
    stop:
      clock.stop ();
      stop ();
      break;
    }

}
