/*
 * misc.cpp
 *
 * Miscellaneous vdr compatability routines.
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

#include "include/vdr/channels.h"
#include "include/vdr/device.h"
#include <ctype.h>
#include <linux/dvb/ca.h>
int cChannel::Transponder() const {return sid;}
int cChannel::Transponder(int Frequency, char Polarization)
{
  // some satellites have transponders at the same frequency, just with different polarization:
  switch (tolower(Polarization)) {
    case 'h': Frequency += 100000; break;
    case 'v': Frequency += 200000; break;
    case 'l': Frequency += 300000; break;
    case 'r': Frequency += 400000; break;
    }
  return Frequency;
}
cChannel* cChannels::GetByNumber(int a, int b) {
  cChannel *ch = new cChannel;
  ch->SetId(0,0,a);
  return ch;
}
void cChannel::SetId(int Nid, int Tid, int Sid, int Rid) {
  sid = Sid;
  tid = Tid;
}
void cChannel::SetCaIds(const int *CaIds) {
}
bool cChannel::SetSatTransponderData(int Source, int Frequency, char Polarization, int Srate, int CoderateH) {
     source = Source;
     frequency = Frequency;
     polarization = Polarization;
     srate = Srate;
     coderateH = CoderateH;
     return true;
}
void cChannel::SetPids(int Vpid, int Ppid, int *Apids, char ALangs[][MAXLANGCODE2], int *Dpids, char DLangs[][MAXLANGCODE2], int Tpid)
{
  vpid = Vpid;
  ppid = Ppid;
  tpid = Tpid;
  for (int i = 0; i < MAXAPIDS; i++)
      apids[i] = Apids[i];
  apids[MAXAPIDS] = 0;
  for (int i = 0; i < MAXDPIDS; i++) {
     dpids[i] = Dpids[i];
  }
  dpids[MAXDPIDS] = 0;
}

cChannel::cChannel() {
  caids[0]=0x0101;
  caids[1]=0;
  source=1;
  sid=1;
  groupSep=0;
  pmtlen = 0;
}
cChannel::~cChannel() {
}

void cChannel::SetPMTBuf(const unsigned char *buf, int len)
{
  memcpy(pmtbuf, buf, len);
  pmtlen = len;
}
int cChannel::GetPMTBuf(unsigned char *buf) {
  memcpy(buf, pmtbuf, pmtlen);
  return pmtlen;
}
