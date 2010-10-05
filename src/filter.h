#ifndef __FILTER_H__
#define __FILTER_H__

/*
 * filter.h
 *
 * Routines to set up DVB PID filters.
 *
 * Only used by the DVB./Freesat code, but should be used by the Sky code
 * as well.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <poll.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>
#include "constants.h"

#define MAX_ACTIVE_FILTERS2 MAX_FILTERS

class cFilter 
{
private:
    typedef struct
    {
      int Fd;
      int Step;
      unsigned short int Pid;
      unsigned char Tid;
      unsigned char Mask;
    } sFilter;

    typedef struct
    {
      int Fd;
      int FilterId;
      bool IsBusy;
    } sActiveFilter;

    struct pollfd PFD[MAX_FILTERS];
    int nFilters;
    sFilter Filters[MAX_FILTERS];
    int nActiveFilters;
    sActiveFilter ActiveFilters[MAX_ACTIVE_FILTERS2];

    bool started;
    const char* demux;
    bool IsError;
    int pollindex;

public:
    cFilter(const char* demux);
    ~cFilter();

    void AddFilter( unsigned short int Pid, unsigned char Tid = 0x00, unsigned char Mask = 0xff );
    void StartFilter( int FilterId );
    void StopFilter( int ActiveFilterId );
    int Poll(int Timeout, int * pid);  // returns filterid or -1 for done

private:
    bool Start();
 };
 
#endif
