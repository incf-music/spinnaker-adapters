/* -*- C++ -*-
 *  rtclock.h
 *
 *  Realtime clock
 *
 *  Copyright (C) 2015, 2017, 2018 Mikael Djurfeldt
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

#ifndef RTCLOCK_H
#define RTCLOCK_H

#define CLOCK_GETTIME

#include <stdexcept>
#include <string>
#include <cstring>

#include <time.h>
#include <sys/time.h>

#if defined (CLOCK_GETTIME) && !defined (timespecadd)

# define timespeccmp(a, b, CMP) 				       	      \
  (((a)->tv_sec == (b)->tv_sec) ? 					      \
   ((a)->tv_nsec CMP (b)->tv_nsec) : 					      \
   ((a)->tv_sec CMP (b)->tv_sec))
# define timespecadd(a, b, result)	       				      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;			      \
    (result)->tv_nsec = (a)->tv_nsec + (b)->tv_nsec;			      \
    if ((result)->tv_nsec >= 1000000000)       				      \
      {									      \
	++(result)->tv_sec;						      \
	(result)->tv_nsec -= 1000000000;	       			      \
      }									      \
  } while (0)
# define timespecsub(a, b, result)     					      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;			      \
    (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec;			      \
    if ((result)->tv_nsec < 0) {					      \
      --(result)->tv_sec;						      \
      (result)->tv_nsec += 1000000000;					      \
    }									      \
  } while (0)

#endif

class RTClock {
public:
  /**
   * Create a new RTClock and note the starting wallclock time which
   * time () and sleepUntil (t) will use as a reference.
   *
   * If the optional interval is specified, this gives the sleeping
   * time for the method sleepNext ().
   */
  RTClock (double interval);
  
  /**
   * Reset starting time.
   *
   * This resets the reference wallclock time for time () and
   * sleepUntil (t).
   */
  void reset ();
  
  /**
   * Reset time and stop clock.
   *
   * Store time 0. This has the same effect as stop (), except that
   * the time stored is 0. The only valid call after this is start ().
   *
   */
  void resetAndStop ();

  /**
   * Stop clock.
   *
   * This also affects target time. The only valid call after this is
   * start ().
   */
  void stop ();

  /**
   * Start clock.
   *
   * This also affects target time.
   */
  void start ();
  
  /**
   * Return the time in seconds since starting time.
   */
  double time () const;

  /**
   * Set time.
   */
  void set (double time);

  /**
   * Set next target time to the current plus interval.
   */
  void setNextTarget ();

  /**
   * Return true if argument is less than target time
   */
  bool lessThanTarget (const struct timespec* t);
  
  /**
   * Check if we have reached target time.
   */
  bool pastTarget ();

#ifndef CLOCK_GETTIME

  /**
   * Sleep t seconds.
   */
  void sleep (double t) const;
  
  /**
   * Sleep until next interval counting from start time.
   *
   * If wallclock time has passed since last interval, sleep shorter
   * time such that we will wake up on a multiple of intervals from
   * start time.
   */
  void sleepNext ();
  
  /**
   * Sleep until time t counting from start time.
   */
  void sleepUntil (double t) const;
  
  /**
   * Convert a (relative) time t in seconds to a timeval
   */
  struct timeval timevalFromSeconds (double t) const;
  
  /**
   * Convert a (relative) time tv to seconds
   */
  double secondsFromTimeval (const struct timeval& tv) const;

  /**
   * Convert tv into a timespec struct
   */
  struct timespec timespecFromTimeval (const struct timeval& tv) const;

#endif /* !CLOCK_GETTIME */
  
  /**
   * Convert a (relative) time t in seconds to a timespec
   */
  static struct timespec timespecFromSeconds (double s);

#ifdef CLOCK_GETTIME

  inline double secondsFromTimespec (const struct timespec& tv) const;
  
#endif /* CLOCK_GETTIME */
  
private:
#ifndef CLOCK_GETTIME
  struct timeval start_;
  struct timeval gridtime_;
  struct timeval interval_;
#else
  struct timespec start_;
  struct timespec gridtime_;
  struct timespec interval_;
#endif
};

#ifndef CLOCK_GETTIME

inline double
RTClock::time () const
{
  struct timeval now;
  gettimeofday (&now, 0);
  timersub (&now, &start_, &now);
  return secondsFromTimeval (now);
}

inline struct timeval
RTClock::timevalFromSeconds (double t) const
{
  struct timeval tv;
  tv.tv_sec = t;
  tv.tv_usec = 1000000 * (t - tv.tv_sec);
  return tv;
}

inline double
RTClock::secondsFromTimeval (const struct timeval& tv) const
{
  return tv.tv_sec + 1e-6 * tv.tv_usec;
}

inline double
RTClock::secondsFromTimeval (const struct timeval& tv) const
{
  return tv.tv_sec + 1e-6 * tv.tv_usec;
}

inline struct timespec
RTClock::timespecFromTimeval (const struct timeval& tv) const
{
  struct timespec ts;
  ts.tv_sec = tv.tv_sec;
  ts.tv_nsec = 1000 * tv.tv_usec;
}

#endif /* !CLOCK_GETTIME */

inline struct timespec
RTClock::timespecFromSeconds (double t)
{
  struct timespec ts;
  ts.tv_sec = t;
  ts.tv_nsec = 1e9 * (t - ts.tv_sec);
  return ts;
}

#ifdef CLOCK_GETTIME

inline double
RTClock::time () const
{
  struct timespec now;
  if (clock_gettime (CLOCK_MONOTONIC, &now) != 0)
    throw std::runtime_error (std::string ("gettime() failed: ")
			      + strerror (errno));
  timespecsub (&now, &start_, &now);
  return secondsFromTimespec (now);
}

inline void
RTClock::setNextTarget ()
{
  timespecadd (&gridtime_, &interval_, &gridtime_);
}

inline bool
RTClock::lessThanTarget (const struct timespec* t)
{
  return timespeccmp (t, &gridtime_, <);
}

inline bool
RTClock::pastTarget ()
{
  struct timespec now;
  if (clock_gettime (CLOCK_MONOTONIC, &now) != 0)
    throw std::runtime_error (std::string ("gettime() failed: ")
			      + strerror (errno));  
  return timespeccmp (&now, &gridtime_, >=);
}

inline double
RTClock::secondsFromTimespec (const struct timespec& tv) const
{
  return tv.tv_sec + 1e-9 * tv.tv_nsec;
}

#endif /* CLOCK_GETTIME */

#endif /* RTCLOCK_H */
