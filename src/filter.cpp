/*
 * filter.cpp
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "constants.h"
#include "filter.h"
#include "log.h"

#define TIMEOUT_FILTER 5000 // ms

cFilter::cFilter(const char* demux)
    : 
        nFilters(0),
        nActiveFilters(0),
        started(false),
        demux(demux),
        IsError(false),
        pollindex(-1)
{
}

cFilter::~cFilter()
{
    // clean and stop filters
    for (int i = 0; i < nActiveFilters; i++)
    {
        StopFilter(i);
    }
}

// cFilter Filter Control {{{
void cFilter::AddFilter(unsigned short int Pid, unsigned char Tid, unsigned char Mask)
{
    if (nFilters >= MAX_FILTERS) {
        log_message(ERROR, "numbers of filters is greater than %i, can't add filter pid=0x%04x tid=0x%02x", MAX_FILTERS, Pid, Tid);
        return;
    }
    Filters[nFilters].Step = 0;
    Filters[nFilters].Pid = Pid;
    Filters[nFilters].Tid = Tid;
    Filters[nFilters].Mask = Mask;
    nFilters++;
}

void cFilter::StartFilter(int FilterId)
{
    dmx_sct_filter_params sctFilterParams;
    memset(&sctFilterParams, 0, sizeof(sctFilterParams));
    sctFilterParams.pid = Filters[FilterId].Pid;
    sctFilterParams.timeout = TIMEOUT_FILTER;
    sctFilterParams.flags = DMX_IMMEDIATE_START;
    sctFilterParams.filter.filter[0] = Filters[FilterId].Tid;
    sctFilterParams.filter.mask[0] = Filters[FilterId].Mask;
    if (ioctl(Filters[FilterId].Fd, DMX_SET_FILTER, &sctFilterParams) >= 0) {
        Filters[FilterId].Step = 1;
    } else {
        log_message(ERROR, "can't starting filter pid=0x%04x tid=0x%02x", Filters[FilterId].Pid, Filters[FilterId].Tid);
        Filters[FilterId].Step = 3;
    }
}

void cFilter::StopFilter(int ActiveFilterId)
{
    if (ActiveFilters[ActiveFilterId].Fd >= 0) {
        if (ioctl(ActiveFilters[ActiveFilterId].Fd, DMX_STOP) < 0) {
            log_message(ERROR, "ioctl DMX_STOP failed");
            perror(strerror(errno));
        }
        if (close(ActiveFilters[ActiveFilterId].Fd) == 0) {
            ActiveFilters[ActiveFilterId].Fd = -1;
        }
    }
}

// }}}

bool cFilter::Start()
{
    int File;

    for (int i = 0; i < nFilters; i++) {
        if (nActiveFilters < MAX_ACTIVE_FILTERS2) {
            File = open(demux, O_RDWR | O_NONBLOCK);
            if (File > 0) {
                ActiveFilters[nActiveFilters].Fd = File;
                ActiveFilters[nActiveFilters].FilterId = -1;
                ActiveFilters[nActiveFilters].IsBusy = false;
                nActiveFilters++;
            } else {
                log_message(ERROR, "can't open filter handle on '%s'", demux);
                IsError = true;
            }
        } else {
            break;
        }
    }

    started = true;
    return true;
}

int cFilter::Poll(int Timeout, int * pid)
{
    bool IsRunning = false;
    int Status;

    if (!started)
        Start();

    if (pollindex >= 0)
    {
        for (; pollindex < nActiveFilters; pollindex++) {
            if (PFD[pollindex].revents & POLLIN) 
            {
                if (pid)
                    *pid = Filters[ActiveFilters[pollindex].FilterId].Pid;
                return PFD[pollindex++].fd;
            }
        }
        pollindex = -1;
    }

    // checking end reading filter
    for (int i = 0; i < nFilters; i++) {
        if (Filters[i].Step == 2) {
            Filters[i].Step = 3;
            for (int ii = 0; ii < nActiveFilters; ii++) {
                if (ActiveFilters[ii].FilterId == i) {
                    ActiveFilters[ii].FilterId = -1;
                    ActiveFilters[ii].IsBusy = false;
                    break;
                }
            }
        }
    }

    // preparing active filters
    for (int i = 0; i < nActiveFilters; i++) {
        if (!ActiveFilters[i].IsBusy) {
            for (int ii = 0; ii < nFilters; ii++) {
                if (Filters[ii].Step == 0) {
                    Filters[ii].Fd = ActiveFilters[i].Fd;
                    StartFilter(ii);
                    if (Filters[ii].Step == 1) {
                        ActiveFilters[i].FilterId = ii;
                        ActiveFilters[i].IsBusy = true;
                        break;
                    }
                }
            }
        }
        if (ActiveFilters[i].IsBusy) {
            IsRunning = true;
        }
    }
    if (IsRunning)
    {
        for (int i = 0; i < nActiveFilters; i++) {
            PFD[i].fd = ActiveFilters[i].Fd;
            PFD[i].events = POLLIN;
            PFD[i].revents = 0;
        }

        pollindex = -1;
        Status = poll(PFD, nActiveFilters, Timeout);
        if (Status > 0) {
            pollindex = 0;
            for (; pollindex < nActiveFilters; pollindex++) {
                if (PFD[pollindex].revents & POLLIN) 
                {
                    if (pid)
                        *pid = Filters[ActiveFilters[pollindex].FilterId].Pid;
                    return PFD[pollindex++].fd;
                }
            }
            pollindex = -1;
        }
        else if (Status == 0)
        {
            // timeout
            return 0;
        }
    }

    return -1;
}

#if 0
// cTaskLoadepg::PollingFilters {{{
void cFilter::PollingFilters(int Timeout)
{
    char *FileName;
    int File;
    int Status;

    IsError = false;
    IsRunning = true;
    asprintf(&FileName, DVB_DEVICE_DEMUX, adapter);
    nActiveFilters = 0;
    for (int i = 0; i < nFilters; i++) {
        if (nActiveFilters < MAX_ACTIVE_FILTERS) {
            File = open(FileName, O_RDWR | O_NONBLOCK);
            if (File) {
                ActiveFilters[nActiveFilters].Fd = File;
                ActiveFilters[nActiveFilters].FilterId = -1;
                ActiveFilters[nActiveFilters].IsBusy = false;
                nActiveFilters++;
            } else {
                log_message(ERROR, "can't open filter handle on '%s'", FileName);
                IsError = true;
            }
        } else {
            break;
        }
    }
    if (FileName) {
        free(FileName);
    }
    for (int i = 0; i < MAX_FILTERS; i++) {
        memset(&InitialBuffer[i], 0, 32);
    }

    if (!IsError) {
        if (nActiveFilters > 0) {
            while (IsRunning) {
                // checking end reading filter
                for (int i = 0; i < nFilters; i++) {
                    if (Filters[i].Step == 2) {
                        Filters[i].Step = 3;
                        for (int ii = 0; ii < nActiveFilters; ii++) {
                            if (ActiveFilters[ii].FilterId == i) {
                                ActiveFilters[ii].FilterId = -1;
                                ActiveFilters[ii].IsBusy = false;
                                break;
                            }
                        }
                    }
                }

                // preparing active filters
                IsRunning = false;
                for (int i = 0; i < nActiveFilters; i++) {
                    if (!ActiveFilters[i].IsBusy) {
                        for (int ii = 0; ii < nFilters; ii++) {
                            if (Filters[ii].Step == 0) {
                                Filters[ii].Fd = ActiveFilters[i].Fd;
                                StartFilter(ii);
                                if (Filters[ii].Step == 1) {
                                    ActiveFilters[i].FilterId = ii;
                                    ActiveFilters[i].IsBusy = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (ActiveFilters[i].IsBusy) {
                        IsRunning = true;
                    }
                }
                for (int i = 0; i < nActiveFilters; i++) {
                    PFD[i].fd = ActiveFilters[i].Fd;
                    PFD[i].events = POLLIN;
                    PFD[i].revents = 0;
                }

                // exit from polling
                if (!IsRunning) {
                    break;
                }
                // running poll filters
                Status = poll(PFD, nActiveFilters, Timeout);
                if (Status > 0) {
                    for (int i = 0; i < nActiveFilters; i++) {
                        if (PFD[i].revents & POLLIN) {
                            if (ActiveFilters[i].IsBusy) {
                                ReadBuffer(ActiveFilters[i].FilterId, PFD[i].fd);
                            }
                        }
                        if (IsError) {
                            log_message(ERROR, "unknown");
                            IsRunning = false;
                        }
                    }
                } else if (Status == 0) {
                    log_message(ERROR, "timeout polling filter");
                    IsError = true;
                    IsRunning = false;
                } else {
                    log_message(ERROR, "polling filter");
                    IsError = true;
                    IsRunning = false;
                }
            }
        }
    }
    // clean and stop filters
    for (int i = 0; i < nActiveFilters; i++) {
        StopFilter(i);
    }
}
// }}}
#endif


