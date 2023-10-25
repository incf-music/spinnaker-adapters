/*
 *  rtclock.cpp
 *
 *  Realtime clock
 *
 *  Copyright (C) 2015, 2017, 2018, 2019, 2021, 2022, 2023 Mikael Djurfeldt
 *
 *  rtclock is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
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

#include "rtclock.h"

RTClock::RTClock (double interval = 0.)
{
#ifndef CLOCK_GETTIME
  interval_ = timevalFromSeconds (interval);
#else
  interval_ = timespecFromSeconds (interval);
#endif
  reset ();
}

void
RTClock::reset ()
{
#ifndef CLOCK_GETTIME
  gettimeofday (&start_, 0);
#else
  if (clock_gettime (CLOCK_MONOTONIC, &start_) != 0)
    throw std::runtime_error (std::string ("gettime() failed: ")
			      + strerror (errno));  
#endif
  gridtime_ = start_;
}

void
RTClock::sync (RTClock& clock)
{
  start_ = clock.start_;
  gridtime_ = start_;
}

#ifndef CLOCK_GETTIME

void
RTClock::sleep (double t) const
{
  struct timespec req = timespecFromSeconds (t);
  nanosleep (&req, NULL);
}

void
RTClock::sleepNext ()
{
  timeradd (&gridtime_, &interval_, &gridtime_);
  struct timespec req = timespecFromTimeval (gridtime_);
  req.tv_sec = gridtime_.tv_sec;
  req.tv_nsec = 1000 * gridtime_.tv_usec;
  clock_nanosleep (CLOCK_REALTIME, TIMER_ABSTIME, &req, NULL);
}

void
RTClock::sleepUntil (double t) const
{
  struct timeval goal = timevalFromSeconds (t);
  timeradd (&start_, &goal, &goal);
  struct timespec req = timespecFromTimeval (goal);
  clock_nanosleep (CLOCK_REALTIME, TIMER_ABSTIME, &req, NULL);
}

#else

void
RTClock::resetAndStop ()
{
  start_.tv_sec = start_.tv_nsec = 0;
  gridtime_ = start_;
}

void
RTClock::stop ()
{
  if (clock_gettime (CLOCK_MONOTONIC, &offset_) != 0)
    throw std::runtime_error (std::string ("gettime() failed: ")
			      + strerror (errno));  
  timespecsub (&start_, &offset_, &start_);
  timespecsub (&gridtime_, &offset_, &gridtime_);
}

void
RTClock::start ()
{
  if (clock_gettime (CLOCK_MONOTONIC, &offset_) != 0)
    throw std::runtime_error (std::string ("gettime() failed: ")
			      + strerror (errno));  
  timespecadd (&start_, &offset_, &start_);
  timespecadd (&gridtime_, &offset_, &gridtime_);
}

void
RTClock::set (double time)
{
  if (clock_gettime (CLOCK_MONOTONIC, &start_) != 0)
    throw std::runtime_error (std::string ("gettime() failed: ")
			      + strerror (errno));
  struct timespec t = timespecFromSeconds (time);
  timespecsub (&start_, &t, &start_);
}

#endif
