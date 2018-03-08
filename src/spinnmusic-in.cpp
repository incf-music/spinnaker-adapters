/*
 *  spinnmusic_out.cpp
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

#include <mpi.h>

#include <iostream>
#include <string>

extern "C" {
#include <unistd.h>
#include <getopt.h>
}

#include <music.hh>

#include "MusicOutputAdapter.h"

using namespace MUSIC;

const double DEFAULT_TIMESTEP = 1e-2;

void
usage (int rank)
{
  if (rank == 0)
    {
      std::cerr << "Usage: spinnmusic-in [OPTION...]\n"
		<< "`spinnmusic_in' receives spikes from a labelled population on SpiNNaker\n"
		<< "hardware and relays them through a MUSIC port.\n\n"
		<< "  -l, --label LABEL       population label\n"
		<< "  -r, --range N           population size\n"
		<< "  -o, --out PORTNAME      output port name (default: out)\n"
		<< "  -p, --port N            database notification port\n"
		<< "  -t, --timestep TIMESTEP time between tick() calls (default " << DEFAULT_TIMESTEP << " s)\n"
		<< "  -b, --maxbuffered TICKS maximal amount of data buffered\n"
		<< "  -h, --help              print this help message\n";
    }
  exit (1);
}

string label;
string portName ("out");
int dbNotificationPort = 19999;
int    nUnits;
double timestep = DEFAULT_TIMESTEP;
int    maxbuffered = 1;


void
getargs (int rank, int argc, char* argv[])
{
  opterr = 0; // handle errors ourselves
  while (1)
    {
      static struct option longOptions[] =
	{
	  {"label",       required_argument, 0, 'l'},
	  {"range",       required_argument, 0, 'r'},
	  {"port",        required_argument, 0, 'p'},
	  {"timestep",    required_argument, 0, 't'},
	  {"maxbuffered", required_argument, 0, 'b'},
	  {"help",        no_argument,       0, 'h'},
	  {"out",	  required_argument, 0, 'o'},
	  {0, 0, 0, 0}
	};
      /* `getopt_long' stores the option index here. */
      int option_index = 0;

      // the + below tells getopt_long not to reorder argv
      int c = getopt_long (argc, argv, "+l:r:t:b:h",
			   longOptions, &option_index);

      /* detect the end of the options */
      if (c == -1)
	break;

      switch (c)
	{
	case 'l':
	  label = optarg;
	  continue;
	case 'r':
	  nUnits = atoi (optarg);
	  continue;
	case 't':
	  timestep = atof (optarg); // NOTE: could do error checking
	  continue;
	case 'b':
	  maxbuffered = atoi (optarg);
	  continue;
	case 'o':
	  portName = optarg;
	  continue;
	case 'p':
	  dbNotificationPort = atoi (optarg);
	  continue;
	case '?':
	  break; // ignore unknown options
	case 'h':
	  usage (rank);

	default:
	  abort ();
	}
    }

  if (argc < optind + 0 || argc > optind + 0)
    usage (rank);
}


int
main (int argc, char* argv[])
{
  Setup* setup = new Setup (argc, argv);
  Runtime* runtime;

  MPI::Intracomm comm = setup->communicator ();
  int rank = comm.Get_rank ();
  getargs (rank, argc, argv);

  double stoptime;
  setup->config ("stoptime", &stoptime);

  char* receive_labels[1] = { (char*) label.c_str () };
  char const* local_host = NULL;
  SpynnakerLiveSpikesConnection connection =
    SpynnakerLiveSpikesConnection(1,
				  receive_labels,
				  0,
				  NULL,
				  (char*) local_host,
				  dbNotificationPort);
  
  MusicOutputAdapter musicOutput (setup, runtime, timestep, stoptime, label, nUnits, portName);

  connection.add_start_callback ((char*) label.c_str (), &musicOutput);
  connection.add_pause_stop_callback ((char*) label.c_str (), &musicOutput);
  connection.add_receive_callback ((char*) label.c_str (), &musicOutput);

  musicOutput.main_loop ();

  runtime->finalize ();

  return 0;
}
