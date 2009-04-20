/*
 * loadepg.cpp
 *
 * Parse Sky AU/IT/UK and MediaHighway EPG data and generate an xmltv file.
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

#include "ctype.h"

#include "constants.h"
#include "loadepg.h"
#include "dvbtext.h"
#include "chanid.h"
#include "stats.h"
#include "log.h"

extern int format;
extern int sky_country;

sConfig *Config;
cTaskLoadepg *Task;

extern char conf[1024];

extern int adapter;
extern bool useshortxmlids;

int nThemes;
sTheme *lThemes;

int nChannels;
int nChannelUpdates;
sChannel *lChannels;

int nBouquets;
sBouquet *lBouquets;

int nTitles;
sTitle *lTitles;

int nSummaries;
sSummary *lSummaries;

unsigned char *bChannels;	// buffer data channels
unsigned char *bTitles;		// buffer data titles
unsigned char *bSummaries;	// buffer data summaries

int EpgTimeOffset;
int LocalTimeOffset;
int SatelliteTimeOffset;
int Yesterday;
int YesterdayEpoch;

// misc functions {{{

char *stripspace(char *s)
{
  if (s && *s) {
     for (char *p = s + strlen(s) - 1; p >= s; p--) {
         if (!isspace(*p))
            break;
         *p = 0;
         }
     }
  return s;
}

char *skipspace(const char *s)
{
  while (*s && isspace(*s))
        s++;
  return (char *)s;
}

char *compactspace(char *s)
{
  if (s && *s) {
     char *t = stripspace(skipspace(s));
     char *p = t;
     while (p && *p) {
           char *q = skipspace(p);
           if (q - p > 1)
              memmove(p + 1, q, strlen(q) + 1);
           p++;
           }
     if (t != s)
        memmove(s, t, strlen(t) + 1);
     }
  return s;
}

bool isempty(const char *s)
{
  return !(s && *skipspace(s));
}

static int BcdToInt(unsigned char Bcd)
{
    return (((Bcd & 0xf0) >> 4) * 10) + (Bcd & 0x0f);
}

static char *GetStringMJD(int MJD)
{
    int J, C, Y, M;
    int Day, Month, Year;
    char *Buffer;
    J = MJD + 2400001 + 68569;
    C = 4 * J / 146097;
    J = J - (146097 * C + 3) / 4;
    Y = 4000 * (J + 1) / 1461001;
    J = J - 1461 * Y / 4 + 31;
    M = 80 * J / 2447;
    Day = J - 2447 * M / 80;
    J = M / 11;
    Month = M + 2 - (12 * J);
    Year = 100 * (C - 49) + Y + J;
    asprintf(&Buffer, "%02i/%02i/%04i", Day, Month, Year);
    return Buffer;
}

// }}}

static int qsortChannels(const void *A, const void *B)
{
    sChannel *ChannelA = (sChannel *) A;
    sChannel *ChannelB = (sChannel *) B;
    if (ChannelA->ChannelId > ChannelB->ChannelId) {
	return 1;
    }
    if (ChannelA->ChannelId < ChannelB->ChannelId) {
	return -1;
    }
    if (ChannelA->ChannelId == ChannelB->ChannelId) {
	if (ChannelA->Nid > ChannelB->Nid) {
	    return 1;
	}
	if (ChannelA->Nid < ChannelB->Nid) {
	    return -1;
	}
	if (ChannelA->Nid == ChannelB->Nid) {
	    if (ChannelA->Tid > ChannelB->Tid) {
		return 1;
	    }
	    if (ChannelA->Tid < ChannelB->Tid) {
		return -1;
	    }
	    if (ChannelA->Tid == ChannelB->Tid) {
		if (ChannelA->Sid > ChannelB->Sid) {
		    return 1;
		}
		if (ChannelA->Sid < ChannelB->Sid) {
		    return -1;
		}
	    }
	}
    }
    return 0;
}

static int qsortChannelsBySkyNumber(const void *A, const void *B)
{
    sChannel *ChannelA = (sChannel *) A;
    sChannel *ChannelB = (sChannel *) B;
    if (ChannelA->SkyNumber > ChannelB->SkyNumber)
	return 1;
    if (ChannelA->SkyNumber < ChannelB->SkyNumber)
	return -1;
    // must be == so no test needed
    if (ChannelA->ChannelId > ChannelB->ChannelId) {
	return 1;
    }
    if (ChannelA->ChannelId < ChannelB->ChannelId) {
	return -1;
    }
    if (ChannelA->ChannelId == ChannelB->ChannelId) {
	if (ChannelA->Nid > ChannelB->Nid) {
	    return 1;
	}
	if (ChannelA->Nid < ChannelB->Nid) {
	    return -1;
	}
	if (ChannelA->Nid == ChannelB->Nid) {
	    if (ChannelA->Tid > ChannelB->Tid) {
		return 1;
	    }
	    if (ChannelA->Tid < ChannelB->Tid) {
		return -1;
	    }
	    if (ChannelA->Tid == ChannelB->Tid) {
		if (ChannelA->Sid > ChannelB->Sid) {
		    return 1;
		}
		if (ChannelA->Sid < ChannelB->Sid) {
		    return -1;
		}
	    }
	}
    }
    return 0;
}

static int qsortChannelsByChID(const void *A, const void *B)
{
    sChannel *ChannelA = (sChannel *) A;
    sChannel *ChannelB = (sChannel *) B;
    if (ChannelA->Nid > ChannelB->Nid) {
	return 1;
    }
    if (ChannelA->Nid < ChannelB->Nid) {
	return -1;
    }
    if (ChannelA->Nid == ChannelB->Nid) {
	if (ChannelA->Tid > ChannelB->Tid) {
	    return 1;
	}
	if (ChannelA->Tid < ChannelB->Tid) {
	    return -1;
	}
	if (ChannelA->Tid == ChannelB->Tid) {
	    if (ChannelA->Sid > ChannelB->Sid) {
		return 1;
	    }
	    if (ChannelA->Sid < ChannelB->Sid) {
		return -1;
	    }
	    if (ChannelA->Sid == ChannelB->Sid) {
		if (ChannelA->ChannelId > ChannelB->ChannelId) {
		    return 1;
		}
		if (ChannelA->ChannelId < ChannelB->ChannelId) {
		    return -1;
		}
	    }
	}
    }
    return 0;
}

static int bsearchChannelByChannelId(const void *A, const void *B)
{
    sChannel *ChannelA = (sChannel *) A;
    sChannel *ChannelB = (sChannel *) B;
    if (ChannelA->ChannelId > ChannelB->ChannelId) {
	return 1;
    }
    if (ChannelA->ChannelId < ChannelB->ChannelId) {
	return -1;
    }
    return 0;
}

static int bsearchChannelByChID(const void *A, const void *B)
{
    sChannel *ChannelA = (sChannel *) A;
    sChannel *ChannelB = (sChannel *) B;
    if (ChannelA->Nid > ChannelB->Nid) {
	return 1;
    }
    if (ChannelA->Nid < ChannelB->Nid) {
	return -1;
    }
    if (ChannelA->Nid == ChannelB->Nid) {
	if (ChannelA->Tid > ChannelB->Tid) {
	    return 1;
	}
	if (ChannelA->Tid < ChannelB->Tid) {
	    return -1;
	}
	if (ChannelA->Tid == ChannelB->Tid) {
	    if (ChannelA->Sid > ChannelB->Sid) {
		return 1;
	    }
	    if (ChannelA->Sid < ChannelB->Sid) {
		return -1;
	    }
	    if (ChannelA->Sid == ChannelB->Sid) {
		if (ChannelA->ChannelId > ChannelB->ChannelId) {
		    return 1;
		}
		if (ChannelA->ChannelId < ChannelB->ChannelId) {
		    return -1;
		}
	    }
	}
    }
    return 0;
}

static int bsearchChannelBySid(const void *A, const void *B)
{
    sChannel *ChannelA = (sChannel *) A;
    sChannel *ChannelB = (sChannel *) B;
    if (ChannelA->Nid > ChannelB->Nid) {
	return 1;
    }
    if (ChannelA->Nid < ChannelB->Nid) {
	return -1;
    }
    if (ChannelA->Nid == ChannelB->Nid) {
	if (ChannelA->Tid > ChannelB->Tid) {
	    return 1;
	}
	if (ChannelA->Tid < ChannelB->Tid) {
	    return -1;
	}
	if (ChannelA->Tid == ChannelB->Tid) {
	    if (ChannelA->Sid > ChannelB->Sid) {
		return 1;
	    }
	    if (ChannelA->Sid < ChannelB->Sid) {
		return -1;
	    }
	}
    }
    return 0;
}

static int qsortTitles(const void *A, const void *B)
{
    sTitle *TitleA = (sTitle *) A;
    sTitle *TitleB = (sTitle *) B;
    if (TitleA->ChannelId > TitleB->ChannelId) {
	return 1;
    }
    if (TitleA->ChannelId < TitleB->ChannelId) {
	return -1;
    }
    if (TitleA->ChannelId == TitleB->ChannelId) {
	if (TitleA->StartTime > TitleB->StartTime) {
	    return 1;
	}
	if (TitleA->StartTime < TitleB->StartTime) {
	    return -1;
	}
    }
    return 0;
}

static int qsortSummaries(const void *A, const void *B)
{
    sSummary *SummarieA = (sSummary *) A;
    sSummary *SummarieB = (sSummary *) B;
    if (SummarieA->ChannelId > SummarieB->ChannelId) {
	return 1;
    }
    if (SummarieA->ChannelId < SummarieB->ChannelId) {
	return -1;
    }
    if (SummarieA->ChannelId == SummarieB->ChannelId) {
	if (SummarieA->MjdTime > SummarieB->MjdTime) {
	    return 1;
	}
	if (SummarieA->MjdTime < SummarieB->MjdTime) {
	    return -1;
	}
	if (SummarieA->MjdTime == SummarieB->MjdTime) {
	    if (SummarieA->EventId > SummarieB->EventId) {
		return 1;
	    }
	    if (SummarieA->EventId < SummarieB->EventId) {
		return -1;
	    }
	}
    }
    return 0;
}

static int bsearchSummarie(const void *A, const void *B)
{
    sSummary *SummarieA = (sSummary *) A;
    sSummary *SummarieB = (sSummary *) B;
    if (SummarieA->ChannelId > SummarieB->ChannelId) {
	return 1;
    }
    if (SummarieA->ChannelId < SummarieB->ChannelId) {
	return -1;
    }
    if (SummarieA->ChannelId == SummarieB->ChannelId) {
	if (SummarieA->MjdTime > SummarieB->MjdTime) {
	    return 1;
	}
	if (SummarieA->MjdTime < SummarieB->MjdTime) {
	    return -1;
	}
	if (SummarieA->MjdTime == SummarieB->MjdTime) {
	    if (SummarieA->EventId > SummarieB->EventId) {
		return 1;
	    }
	    if (SummarieA->EventId < SummarieB->EventId) {
		return -1;
	    }
	}
    }
    return 0;
}

// }}}

// cTaskLoadepg {{{
// cTaskLoadepg Construction {{{
cTaskLoadepg::cTaskLoadepg(void)
#ifdef USETHREADS
    :  cThread("cTaskLoadepg")
#endif
{
}

cTaskLoadepg::~cTaskLoadepg()
{
    for (int i = 0; i < nActiveFilters; i++) {
	StopFilter(i);
    }
    if (lThemes) {
	free(lThemes);
	lThemes = NULL;
    }
    if (lChannels) {
	free(lChannels);
	lChannels = NULL;
    }
    if (lBouquets) {
	free(lBouquets);
	lBouquets = NULL;
    }
    if (lTitles) {
	free(lTitles);
	lTitles = NULL;
    }
    if (lSummaries) {
	free(lSummaries);
	lSummaries = NULL;
    }
    if (bChannels) {
	free(bChannels);
	bChannels = NULL;
    }
    if (bTitles) {
	free(bTitles);
	bTitles = NULL;
    }
    if (bSummaries) {
	free(bSummaries);
	bSummaries = NULL;
    }
#ifdef USETHREADS
    Cancel(2);
#endif
}

// }}}

// cTaskLoadepg Thread {{{
void cTaskLoadepg::Action(void)
{
    IsError = false;
    nThemes = 0;
    nChannels = 0;
    nChannelUpdates = 0;
    nBouquets = 0;
    nTitles = 0;
    nSummaries = 0;
    EndBAT = false;
    EndSDT = false;
    firstSDTChannel = NULL;
    EndThemes = false;
    EndChannels = false;
    pC = 0;
    pT = 0;
    pS = 0;
    EpgTimeOffset = 0;
#ifdef USETHREADS
    if (Running()) 
#endif
    {
	log_message(TRACE, "start task");
	switch (format) {
	    case DATA_FORMAT_SKYBOX:
	    case DATA_FORMAT_MHW_1:
	    case DATA_FORMAT_MHW_2:
		lThemes = (sTheme *) calloc(MAX_THEMES, sizeof(sTheme));
		if (!lThemes) {
		    log_message(ERROR, "failed to allocate memory for lThemes");
		    goto endrunning;
		}
		lChannels = (sChannel *) calloc(MAX_CHANNELS, sizeof(sChannel));
		if (!lChannels) {
		    log_message(ERROR, "failed to allocate memory for lChannels");
		    goto endrunning;
		}
		lBouquets = (sBouquet *) calloc(MAX_BOUQUETS, sizeof(sBouquet));
		if (!lBouquets) {
		    log_message(ERROR, "failed to allocate memory for lBouquets");
		    goto endrunning;
		}
		lTitles = (sTitle *) calloc(MAX_TITLES, sizeof(sTitle));
		if (!lTitles) {
		    log_message(ERROR, "failed to allocate memory for lTitles");
		    goto endrunning;
		}
		lSummaries = (sSummary *) calloc(MAX_SUMMARIES, sizeof(sSummary));
		if (!lSummaries) {
		    log_message(ERROR, "failed to allocate memory for lSummaries");
		    goto endrunning;
		}
		bChannels = (unsigned char *) calloc(MAX_BUFFER_SIZE_CHANNELS, sizeof(unsigned char));
		if (!bChannels) {
		    log_message(ERROR, "failed to allocate memory for bChannels");
		    goto endrunning;
		}
		bTitles = (unsigned char *) calloc(MAX_BUFFER_SIZE_TITLES, sizeof(unsigned char));
		if (!bTitles) {
		    log_message(ERROR, "failed to allocate memory for bTitles");
		    goto endrunning;
		}
		bSummaries = (unsigned char *) calloc(MAX_BUFFER_SIZE_SUMMARIES, sizeof(unsigned char));
		if (!bSummaries) {
		    log_message(ERROR, "failed to allocate memory for bSummaries");
		    goto endrunning;
		}
#ifdef USETHREADS
		cCondWait::SleepMs(2000);
#endif
		log_message(TRACE, "tuned transponder with adapter number=%i", adapter);
		LoadFromSatellite();
	    default:
		break;
	}
endrunning:;
	   if (lThemes) {
	       free(lThemes);
	       lThemes = NULL;
	   }
	   if (lChannels) {
	       free(lChannels);
	       lChannels = NULL;
	   }
	   if (lBouquets) {
	       free(lBouquets);
	       lBouquets = NULL;
	   }
	   if (lTitles) {
	       free(lTitles);
	       lTitles = NULL;
	   }
	   if (lSummaries) {
	       free(lSummaries);
	       lSummaries = NULL;
	   }
	   if (bChannels) {
	       free(bChannels);
	       bChannels = NULL;
	   }
	   if (bTitles) {
	       free(bTitles);
	       bTitles = NULL;
	   }
	   if (bSummaries) {
	       free(bSummaries);
	       bSummaries = NULL;
	   }
	   log_message(DEBUG, "end task");
    }
}
// }}}

// cTaskLoadepg Huffman decode {{{
int cTaskLoadepg::DecodeHuffmanCode(unsigned char *Data, int Length)
{
    int i;
    int p;
    int q;
    bool CodeError;
    bool IsFound;
    unsigned char Byte;
    unsigned char lastByte;
    unsigned char Mask;
    unsigned char lastMask;
    nH = &H;
    p = 0;
    q = 0;
    DecodeText[0] = '\0';
    DecodeErrorText[0] = '\0';
    CodeError = false;
    IsFound = false;
    lastByte = 0;
    lastMask = 0;
    for (i = 0; i < Length; i++) {
	Byte = Data[i];
	Mask = 0x80;
	if (i == 0) {
	    Mask = 0x20;
	    lastByte = i;
	    lastMask = Mask;
	}
loop1:;
      if (IsFound) {
	  lastByte = i;
	  lastMask = Mask;
	  IsFound = false;
      }
      if ((Byte & Mask) == 0) {
	  if (CodeError) {
	      DecodeErrorText[q] = 0x30;
	      q++;
	      goto nextloop1;
	  }
	  if (nH->P0 != NULL) {
	      nH = nH->P0;
	      if (nH->Value != NULL) {
		  // handle 0 code
		  if (nH->Value[0] == 0) {
		      memcpy(&DecodeText[p], nH->Value, 1);
		      p += 1;
		  } else {
		      memcpy(&DecodeText[p], nH->Value, strlen(nH->Value));
		      p += strlen(nH->Value);
		  }
		  nH = &H;
		  IsFound = true;
	      }
	  } else {
	      memcpy(&DecodeText[p], "<...?...>", 9);
	      p += 9;
	      i = lastByte;
	      Byte = Data[lastByte];
	      Mask = lastMask;
	      CodeError = true;
	      goto loop1;
	  }
      } else {
	  if (CodeError) {
	      DecodeErrorText[q] = 0x31;
	      q++;
	      goto nextloop1;
	  }
	  if (nH->P1 != NULL) {
	      nH = nH->P1;
	      if (nH->Value != NULL) {
		  memcpy(&DecodeText[p], nH->Value, strlen(nH->Value));
		  p += strlen(nH->Value);
		  nH = &H;
		  IsFound = true;
	      }
	  } else {
	      memcpy(&DecodeText[p], "<...?...>", 9);
	      p += 9;
	      i = lastByte;
	      Byte = Data[lastByte];
	      Mask = lastMask;
	      CodeError = true;
	      goto loop1;
	  }
      }
nextloop1:;
	  Mask = Mask >> 1;
	  if (Mask > 0) {
	      goto loop1;
	  }
    }
    DecodeText[p] = '\0';
    DecodeErrorText[q] = '\0';
    return p;
}

// }}}

// cTaskLoadepg::ReadFileDictionary {{{
bool cTaskLoadepg::ReadFileDictionary(void)
{
    char *FileName;
    FILE *FileDict;
    char *Line;
    char Buffer[256];

    switch (sky_country) {
    case SKY_AU:
        asprintf(&FileName, "%s/%s", Config->Directory, SKY_AU_DICT);
	break;
    case SKY_IT:
        asprintf(&FileName, "%s/%s", Config->Directory, SKY_IT_DICT);
	break;
    case SKY_UK:
        asprintf(&FileName, "%s/%s", Config->Directory, SKY_UK_DICT);
	break;
    }
    FileDict = fopen(FileName, "r");
    if (FileDict == NULL) {
	log_message(ERROR, "opening file '%s'. %s", FileName, strerror(errno));
	free(FileName);
	return false;
    } else {
	int i;
	int LenPrefix;
	int charcode;
	char string1[256];
	char string2[256];
	H.Value = NULL;
	H.P0 = NULL;
	H.P1 = NULL;
	while ((Line = fgets(Buffer, sizeof(Buffer), FileDict)) != NULL) {
	    if (!isempty(Line)) {
		memset(string1, 0, sizeof(string1));
		memset(string2, 0, sizeof(string2));
		if (sscanf(Line, "%c=%[^\n]\n", string1, string2) == 2) {
		    goto codingstart;
		} else if (sscanf(Line, "\\x%2x=%[^\n]\n", &charcode, string2)
			== 2) {
		    string1[0] = charcode;
		    goto codingstart;
		} else if (sscanf(Line, "%[^=]=%[^\n]\n", string1, string2)
			== 2) {
codingstart:;
	    nH = &H;
	    LenPrefix = strlen(string2);
	    for (i = 0; i < LenPrefix; i++) {
		switch (string2[i]) {
		    case '0':
			if (nH->P0 == NULL) {
			    nH->P0 = new sNodeH();
			    nH = nH->P0;
			    nH->Value = NULL;
			    nH->P0 = NULL;
			    nH->P1 = NULL;
			    if ((LenPrefix - 1) == i) {
				asprintf(&nH->Value, "%s", string1);
			    }
			} else {
			    nH = nH->P0;
			    if (nH->Value != NULL || (LenPrefix - 1) == i) {
				log_message(ERROR, "huffman prefix code already exists for \"%s\"=%s with '%s'", string1, string2, nH->Value);
			    }
			}
			break;
		    case '1':
			if (nH->P1 == NULL) {
			    nH->P1 = new sNodeH();
			    nH = nH->P1;
			    nH->Value = NULL;
			    nH->P0 = NULL;
			    nH->P1 = NULL;
			    if ((LenPrefix - 1) == i) {
				asprintf(&nH->Value, "%s", string1);
			    }
			} else {
			    nH = nH->P1;
			    if (nH->Value != NULL || (LenPrefix - 1) == i) {
				log_message(ERROR, "huffman prefix code already exists for \"%s\"=%s with '%s'", string1, string2, nH->Value);
			    }
			}
			break;
		    default:
			break;
		}
	    }
		}
	    }
	}
	fclose(FileDict);
    }

    // check tree huffman nodes
    FileDict = fopen(FileName, "r");
    if (FileDict) {
	int i;
	int LenPrefix;
	char string1[256];
	char string2[256];
	int charcode;
	while ((Line = fgets(Buffer, sizeof(Buffer), FileDict)) != NULL) {
	    if (!isempty(Line)) {
		memset(string1, 0, sizeof(string1));
		memset(string2, 0, sizeof(string2));
		if (sscanf(Line, "%c=%[^\n]\n", string1, string2) == 2) {
		    goto verifystart;
		} else if (sscanf(Line, "\\x%2x=%[^\n]\n", &charcode, string2)
			== 2) {
		    string1[0] = charcode;
		    goto verifystart;
		} else if (sscanf(Line, "%[^=]=%[^\n]\n", string1, string2)
			== 2) {
verifystart:;
	    nH = &H;
	    LenPrefix = strlen(string2);
	    for (i = 0; i < LenPrefix; i++) {
		switch (string2[i]) {
		    case '0':
			if (nH->P0 != NULL) {
			    nH = nH->P0;
			}
			break;
		    case '1':
			if (nH->P1 != NULL) {
			    nH = nH->P1;
			}
			break;
		    default:
			break;
		}
	    }
	    if (nH->Value != NULL) {
		if (memcmp(nH->Value, string1, strlen(nH->Value))
			!= 0) {
		    log_message(ERROR, "huffman prefix value '%s' not equal to '%s'", nH->Value, string1);
		}
	    } else {
		log_message(ERROR, "huffman prefix value is not exists for \"%s\"=%s", string1, string2);
	    }
		}
	    }
	}
	fclose(FileDict);
    }
    free(FileName);
    return true;
}

// }}}

// cTaskLoadepg::ReadFileThemes {{{
bool cTaskLoadepg::ReadFileThemes(void)
{
    char *FileName;
    FILE *FileThemes;
    char *Line;
    char Buffer[256];

    switch (sky_country) {
    case SKY_AU:
        asprintf(&FileName, "%s/%s", Config->Directory, SKY_AU_THEMES);
	break;
    case SKY_IT:
        asprintf(&FileName, "%s/%s", Config->Directory, SKY_IT_THEMES);
	break;
    case SKY_UK:
        asprintf(&FileName, "%s/%s", Config->Directory, SKY_UK_THEMES);
	break;
    }
    FileThemes = fopen(FileName, "r");
    if (FileThemes == NULL) {
	log_message(ERROR, "opening file '%s'. %s", FileName, strerror(errno));
	free(FileName);
	return false;
    } else {
	int id = 0;
	char string1[256];
	char string2[256];
	while ((Line = fgets(Buffer, sizeof(Buffer), FileThemes)) != NULL) {
	    memset(string1, 0, sizeof(string1));
	    memset(string2, 0, sizeof(string2));
	    if (!isempty(Line)) {
		sTheme *T = (lThemes + id);
		if (sscanf(Line, "%[^=] =%[^\n] ", string1, string2) == 2) {
		    snprintf((char *) T->Name, 255, "%s", string2);
		} else {
		    T->Name[0] = '\0';
		}
		id++;
	    }
	}
	fclose(FileThemes);
    }
    free(FileName);
    return true;
}

// }}}

// cTaskLoadepg::LoadFromSatellite {{{
void cTaskLoadepg::LoadFromSatellite(void)
{
    nFilters = 0;
    GetLocalTimeOffset();
    AddFilter(0x14, 0x70, 0xfc);	// TOT && TDT
    switch (format) {
	case DATA_FORMAT_SKYBOX:
	    AddFilter(0x11, 0x4a);
	    AddFilter(0x11, 0x46);
	    AddFilter(0x30, 0xa0, 0xfc);
	    AddFilter(0x31, 0xa0, 0xfc);
	    AddFilter(0x32, 0xa0, 0xfc);
	    AddFilter(0x33, 0xa0, 0xfc);
	    AddFilter(0x34, 0xa0, 0xfc);
	    AddFilter(0x35, 0xa0, 0xfc);
	    AddFilter(0x36, 0xa0, 0xfc);
	    AddFilter(0x37, 0xa0, 0xfc);
	    AddFilter(0x40, 0xa8, 0xfc);
	    AddFilter(0x41, 0xa8, 0xfc);
	    AddFilter(0x42, 0xa8, 0xfc);
	    AddFilter(0x43, 0xa8, 0xfc);
	    AddFilter(0x44, 0xa8, 0xfc);
	    AddFilter(0x45, 0xa8, 0xfc);
	    AddFilter(0x46, 0xa8, 0xfc);
	    AddFilter(0x47, 0xa8, 0xfc);
	    PollingFilters(10500);	// needs to be this due to TOT and TDT 10sec repeat
	    break;
	case DATA_FORMAT_MHW_1:
	    AddFilter(0xd2, 0x90);
	    AddFilter(0xd3, 0x90);
	    AddFilter(0xd3, 0x91);
	    AddFilter(0xd3, 0x92);
	    PollingFilters(10500);
	    break;
	case DATA_FORMAT_MHW_2:
	    AddFilter(0x231, 0xc8);
	    AddFilter(0x234, 0xe6);
	    AddFilter(0x236, 0x96);
	    PollingFilters(10500);
	    break;
	default:
	    IsError = true;
	    break;
    }
    if (!IsError) {
	CreateEpgXml();
    }
}

// }}}


char *get_channelident(sChannel * C)
{
    char *s;
    char *t;

    if (C == NULL)
	asprintf(&s, "undefined");
    else if (C->shortname != NULL && useshortxmlids)
	asprintf(&s, "%d.%s.%s.dvb.guide", C->Sid, C->shortname, C->providername);
    else {
	asprintf(&t, "%d.%d", C->Sid, C->SkyNumber);
	asprintf(&s, "%s", skyxmltvid(t, C->providername));
    }
    return s;
}

// cTaskLoadepg::CreateXmlChannels {{{
void cTaskLoadepg::CreateXmlChannels()
{
    char *ServiceName;
    qsort(lChannels, nChannels, sizeof(sChannel), &qsortChannelsBySkyNumber);
    for (int i = 0; i < nChannels; i++) {
	sChannel *C = (lChannels + i);
	if (C->Nid > 0 && C->Tid > 0 && C->Sid > 0) {
	    if (C->providername == NULL)
		continue;
	    if (strcmp(C->providername, "(null)") == 0)
		continue;
	    if (!C->IsEpg)
		continue;
	    if (!C->IsFound)
		continue;
	    if (C->name) {
		asprintf(&ServiceName, "%s", C->name);
	    } else {
		continue;
	    }
	    char *channelid = get_channelident(C);
	    if (channelid != NULL) {
		printf("<channel id=\"%s\">\n", channelid);
		printf("\t<display-name>%s</display-name>\n", ServiceName);
		printf("</channel>\n");
		free(channelid);
	    }
	    if (ServiceName) {
		free(ServiceName);
		ServiceName = NULL;
	    }
	}
    }
}

// }}}

// cTaskLoadepg Filter Control {{{
void cTaskLoadepg::AddFilter(unsigned short int Pid, unsigned char Tid, unsigned char Mask)
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

void cTaskLoadepg::StartFilter(int FilterId)
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

void cTaskLoadepg::StopFilter(int ActiveFilterId)
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

// cTaskLoadepg::PollingFilters {{{
void cTaskLoadepg::PollingFilters(int Timeout)
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

// cTaskLoadepg::Stop {{{
void cTaskLoadepg::Stop()
{
    log_message(TRACE, "stop");
    IsRunning = false;
#ifdef USETHREADS
    Cancel(2);
#endif
}
// }}}

// cTaskLoadepg::ReadBuffer {{{
void cTaskLoadepg::ReadBuffer(int FilterId, int Fd)
{
    unsigned char Buffer[2 * 4096];
    int Bytes;
    Bytes = read(Fd, Buffer, sizeof(Buffer));
    if (Bytes < 0) {
	if (errno != EOVERFLOW) {
	    log_message(ERROR, "failed to read filter for pid=0x%04x tid=0x%02x", Filters[FilterId].Pid, Filters[FilterId].Tid);
	    Filters[FilterId].Step = 2;
	} else {
	    log_message(ERROR, "buffer overflow to read filter for pid=0x%04x tid=0x%02x", Filters[FilterId].Pid, Filters[FilterId].Tid);
	}
    } else {
	if (Bytes > 3) {
	    switch (format) {
		case DATA_FORMAT_SKYBOX:
		    if (!SI::CRC32::isValid((const char *) Buffer, Bytes)) {
			return;
		    }
		    switch (Buffer[0]) {
			case 0x73:
			    GetSatelliteTimeOffset(FilterId, Buffer, Bytes);
			    break;
			case 0x46:
			    SupplementChannelsSKYBOX(FilterId, Buffer, Bytes - 4);
			    break;
			case 0x4a:
			    GetChannelsSKYBOX(FilterId, Buffer, Bytes - 4);
			    break;
			case 0xa0:
			case 0xa1:
			case 0xa2:
			case 0xa3:
			    GetTitlesSKYBOX(FilterId, Buffer, Bytes - 4);
			    break;
			case 0xa8:
			case 0xa9:
			case 0xaa:
			case 0xab:
			    GetSummariesSKYBOX(FilterId, Buffer, Bytes - 4);
			    break;
			default:
			    break;
		    }
		    break;
		case DATA_FORMAT_MHW_1:
		    if (Buffer[0] == 0x73) {
			GetSatelliteTimeOffset(FilterId, Buffer, Bytes);
		    }
		    if (Filters[FilterId].Pid == 0xd2) {
			switch (Buffer[0]) {
			    case 0x90:
				GetTitlesMHW1(FilterId, Buffer, Bytes);
				break;
			    default:
				break;
			}
		    }
		    if (Filters[FilterId].Pid == 0xd3) {
			switch (Buffer[0]) {
			    case 0x90:
				GetSummariesMHW1(FilterId, Buffer, Bytes);
				break;
			    case 0x91:
				GetChannelsMHW1(FilterId, Buffer, Bytes);
				break;
			    case 0x92:
				GetThemesMHW1(FilterId, Buffer, Bytes);
				break;
			    default:
				break;
			}
		    }
		    break;
		case DATA_FORMAT_MHW_2:
		    if (Buffer[0] == 0x73) {
			GetSatelliteTimeOffset(FilterId, Buffer, Bytes);
		    }
		    switch (Buffer[0]) {
			case 0xc8:
			    if (Buffer[3] == 0x00) {
				GetChannelsMHW2(FilterId, Buffer, Bytes);
			    }
			    if (Buffer[3] == 0x01) {
				GetThemesMHW2(FilterId, Buffer, Bytes);
			    }
			    break;
			case 0xe6:
			    GetTitlesMHW2(FilterId, Buffer, Bytes);
			    break;
			case 0x96:
			    GetSummariesMHW2(FilterId, Buffer, Bytes);
			    break;
			default:
			    break;
		    }
		    break;
		default:
		    break;
	    }
	}
    }
}

// }}}

// cTaskLoadepg Time Functions {{{
void cTaskLoadepg::GetLocalTimeOffset(void)
{
    time_t timeLocal;
    time_t timeUtc;
    struct tm *tmCurrent;

    timeLocal = time(NULL);
    tmCurrent = gmtime(&timeLocal);
    tmCurrent->tm_isdst = -1;
    timeUtc = mktime(tmCurrent);
    LocalTimeOffset = (int) difftime(timeLocal, timeUtc);
    timeLocal -= 86400;
    tmCurrent = gmtime(&timeLocal);
    Yesterday = tmCurrent->tm_wday;
    tmCurrent->tm_hour = 0;
    tmCurrent->tm_min = 0;
    tmCurrent->tm_sec = 0;
    tmCurrent->tm_isdst = -1;
    YesterdayEpoch = mktime(tmCurrent);
    log_message(INFO, "local time offset=[UTC]%+i", LocalTimeOffset / 3600);
}

void cTaskLoadepg::GetSatelliteTimeOffset(int FilterId, unsigned char *Data, int Length)
{
    if (Data[0] == 0x73) {
	int satMJD = (Data[3] << 8) | Data[4];
	int satH = BcdToInt(Data[5]);
	int satM = BcdToInt(Data[6]);
	int satS = BcdToInt(Data[7]);
	int DescriptorsLoopLength = ((Data[8] & 0x0f) << 8) | Data[9];
	int p1 = 10;
	while (DescriptorsLoopLength > 0) {
	    int DescriptorTag = Data[p1];
	    int DescriptorLength = Data[p1 + 1];
	    int SatelliteCountryRegionId;
	    int SatelliteTimeOffsetPolarity;
	    int SatelliteTimeOffsetH;
	    int SatelliteTimeOffsetM;
	    switch (DescriptorTag) {
		case 0x58:
		    unsigned char SatelliteCountryCode[4];
		    for (int i = 0; i < 3; i++) {
			SatelliteCountryCode[i] = Data[p1 + 2 + i];
		    }
		    SatelliteCountryCode[3] = '\0';
		    CleanString(SatelliteCountryCode);
		    SatelliteCountryRegionId = (Data[p1 + 5] & 0xfc) >> 6;
		    SatelliteTimeOffsetPolarity = (Data[p1 + 5] & 0x01);
		    SatelliteTimeOffsetH = BcdToInt(Data[p1 + 6]);
		    SatelliteTimeOffsetM = BcdToInt(Data[p1 + 7]);
		    if (SatelliteTimeOffsetPolarity == 1) {
			SatelliteTimeOffset = 0 - (SatelliteTimeOffsetH * 3600);
		    } else {
			SatelliteTimeOffset = SatelliteTimeOffsetH * 3600;
		    }
		    EpgTimeOffset = (LocalTimeOffset - SatelliteTimeOffset);
		    log_message(INFO, "satellite time offset=[UTC]%+i", SatelliteTimeOffset / 3600);
		    log_message(INFO, "EPG time offset=%+i seconds", EpgTimeOffset);
		    if (is_logging(DEBUG)) {
			log_message(DEBUG, "satellite time UTC: %s %02i:%02i:%02i", GetStringMJD(satMJD), satH, satM, satS);
			log_message(DEBUG, "satellite country code=%s", SatelliteCountryCode);
			log_message(DEBUG, "satellite country region ID=%i", SatelliteCountryRegionId);
			log_message(DEBUG, "satellite local time offset polarity=%i", SatelliteTimeOffsetPolarity);
			log_message(DEBUG, "satellite local time offset=%02i:%02i", SatelliteTimeOffsetH, SatelliteTimeOffsetM);
		    }
		    break;
		default:
		    //fprintf( stderr, "0x%02x\n", DescriptorTag );
		    break;
	    }
	    p1 += (DescriptorLength + 2);
	    DescriptorsLoopLength -= (DescriptorLength + 2);
	}
    }
    Filters[FilterId].Step = 2;
}

// }}}

// SKYBOX Stuff {{{
// cTaskLoadepg::SupplementChannelsSKYBOX {{{
void cTaskLoadepg::SupplementChannelsSKYBOX(int FilterId, unsigned char *Data, int Length)
{
    if (!EndBAT) {
	return;
    }

    if (EndSDT) {
	Filters[FilterId].Step = 2;
	log_message(TRACE, "endsdt");
	return;
    }

    SI::SDT sdt(Data, false);
    if (!sdt.CheckCRCAndParse())
	return;

    SI::SDT::Service SiSdtService;
    for (SI::Loop::Iterator it; sdt.serviceLoop.getNext(SiSdtService, it);) {

	sChannel Key, *C;
	Key.ChannelId = Key.Sid = SiSdtService.getServiceId();
	Key.Nid = sdt.getOriginalNetworkId();
	Key.Tid = sdt.getTransportStreamId();
	C = (sChannel *) bsearch(&Key, lChannels, nChannels, sizeof(sChannel), &bsearchChannelBySid);

	if (firstSDTChannel == NULL) {
	    firstSDTChannel = C;
	} else if (firstSDTChannel == C) {
	    if (nChannelUpdates == 0) {
		EndSDT = true;
	    } else
		nChannelUpdates = 0;
	}

	SI::Descriptor * d;
	for (SI::Loop::Iterator it2; (d = SiSdtService.serviceDescriptors.getNext(it2));) {
	    switch (d->getDescriptorTag()) {
		case SI::ServiceDescriptorTag:
		    {
			SI::ServiceDescriptor * sd = (SI::ServiceDescriptor *) d;
			switch (sd->getServiceType()) {
			    case 0x01:	// digital television service
			    case 0x02:	// digital radio sound service
			    case 0x04:	// NVOD reference service
			    case 0x05:	// NVOD time-shifted service
				{
				    char NameBuf[1024];
				    char ShortNameBuf[1024];
				    char ProviderNameBuf[1024];
				    log_message(TRACE, "B %02x %x-%x %x-%x %x-%x",
					    sd->getServiceType(), Key.Nid, lChannels[10].Nid, Key.Tid, lChannels[10].Tid, Key.Sid, lChannels[10].Sid);
				    sd->serviceName.getText(NameBuf, ShortNameBuf, sizeof(NameBuf), sizeof(ShortNameBuf));
				    char *pn = compactspace(NameBuf);
				    sd->providerName.getText(ProviderNameBuf, sizeof(ProviderNameBuf));
				    char *provname = compactspace(ProviderNameBuf);
				    if (C) {
					if (C->name == NULL) {
					    asprintf(&C->name, "%s", pn);
					    asprintf(&C->providername, "%s", provname);
					}
				    }
				}
				break;
			    default:
				break;
			}
		    }
		    break;
		case SI::MultilingualServiceNameDescriptorTag:
		    {
			if (C == NULL)
			    break;
			SI::MultilingualServiceNameDescriptor * md = (SI::MultilingualServiceNameDescriptor *) d;
			SI::MultilingualServiceNameDescriptor::Name n;
			for (SI::Loop::Iterator it2; (md->nameLoop.getNext(n, it2));) {
			    // languageCode char[4]
			    // name String
			    if (strncmp(n.languageCode, "aka", 3) == 0) {
				if (C->shortname == NULL) {
				    char b[100];
				    n.name.getText(b, sizeof(b));
				    C->shortname = strdup(b);
				    nChannelUpdates++;
				}
			    } else {
				if (!C->IsNameUpdated) {
				    if (C->name) {
					free(C->name);
					C->name = NULL;
				    }
				    char b[100];
				    n.name.getText(b, sizeof(b));
				    C->name = strdup(b);
				    C->IsNameUpdated = true;
				}
			    }
			}
		    }
		    break;
		default:
		    break;
	    }
	}
    }

}

// }}}

// cTaskLoadepg::GetChannelsSKYBOX {{{
void cTaskLoadepg::GetChannelsSKYBOX(int FilterId, unsigned char *Data, int Length)
{
    unsigned char SectionNumber = Data[6];
    unsigned char LastSectionNumber = Data[7];

    if (SectionNumber == 0x00 && nBouquets == 0) {
	return;
    }
    // Table BAT
    if (Data[0] == 0x4a) {
	if (EndBAT) {
	    Filters[FilterId].Step = 2;
	    log_message(TRACE, "endbat");
	    return;
	}
	unsigned short int BouquetId = (Data[3] << 8) | Data[4];
	int BouquetDescriptorsLength = ((Data[8] & 0x0f) << 8) | Data[9];
	int TransportStreamLoopLength = ((Data[BouquetDescriptorsLength + 10] & 0x0f) << 8) | Data[BouquetDescriptorsLength + 11];
	int p1 = (BouquetDescriptorsLength + 12);
	while (TransportStreamLoopLength > 0) {
	    unsigned short int Tid = (Data[p1] << 8) | Data[p1 + 1];
	    unsigned short int Nid = (Data[p1 + 2] << 8) | Data[p1 + 3];
	    int TransportDescriptorsLength = ((Data[p1 + 4] & 0x0f) << 8) | Data[p1 + 5];
	    int p2 = (p1 + 6);
	    p1 += (TransportDescriptorsLength + 6);
	    TransportStreamLoopLength -= (TransportDescriptorsLength + 6);
	    while (TransportDescriptorsLength > 0) {
		unsigned char DescriptorTag = Data[p2];
		int DescriptorLength = Data[p2 + 1];
		int p3 = (p2 + 2);
		p2 += (DescriptorLength + 2);
		TransportDescriptorsLength -= (DescriptorLength + 2);
		switch (DescriptorTag) {
		    case 0x41:	// service_list
			break;
		    case 0x5f:	// private data specifier indicates BSkyB
			break;
		    case 0x93:	// unknown private
			break;
		    case 0xb1:
			p3 += 2;
			DescriptorLength -= 2;
			while (DescriptorLength > 0) {
			    // 0x01 = Video Channel
			    // 0x02 = Audio Channel
			    // 0x05 = Other Channel
			    //if( Data[p3+2] == 0x01 || Data[p3+2] == 0x02 || Data[p3+2] == 0x05 )
			    //{
			    //
			    //full decoding from firmware left for ref
			    //
			    unsigned short Sid = (Data[p3] << 8) | Data[p3 + 1];
			    //unsigned char unk1 = Data[p3 + 2];
			    unsigned short ChannelId = (Data[p3 + 3] << 8) | Data[p3 + 4];
			    unsigned short SkyNumber = (Data[p3 + 5] << 8) | Data[p3 + 6];
			    //unsigned short Flags = (Data[p3 + 7] << 8) | Data[p3 + 8];
			    //unsigned short unkval = Flags >> 4;
			    //int unkflag1 = (Flags & 8) >> 3;
			    //int unkflag2 = (Flags & 4) >> 2;
			    //int unkflag3 = (Flags & 2) >> 1;
			    //int unkflag4 = (Flags & 1);
			    /*
			     * 
			     */
			    if (SkyNumber > 100 && SkyNumber < 1000) {
				if (ChannelId > 0) {
				    sChannel Key, *C;
				    Key.ChannelId = ChannelId;
				    Key.Nid = Nid;
				    Key.Tid = Tid;
				    Key.Sid = Sid;
				    C = (sChannel *) bsearch(&Key, lChannels, nChannels, sizeof(sChannel), &bsearchChannelByChID);
				    if (C == NULL) {
					C = (lChannels + nChannels);
					C->ChannelId = ChannelId;
					C->Nid = Nid;
					C->Tid = Tid;
					C->Sid = Sid;
					C->SkyNumber = SkyNumber;
					C->pData = 0;
					C->lenData = 0;
					C->IsFound = false;
					C->IsEpg = false;
					nChannels++;
					incr_stat("channels.count");
					if (nChannels >= MAX_CHANNELS) {
					    log_message(ERROR, "channels found more than %i", MAX_CHANNELS);
					    IsError = true;
					    return;
					}
					qsort(lChannels, nChannels, sizeof(sChannel), &qsortChannelsByChID);
				    }
				}
			    }
			    //}
			    p3 += 9;
			    DescriptorLength -= 9;
			}
			break;
		    default:
			log_message(ERROR, "unprocessed descriptor 0x%02x\n", DescriptorTag);
			break;
		}
	    }
	}
	sBouquet *B;
	for (int i = 0; i < nBouquets; i++) {
	    B = (lBouquets + i);
	    if (B->BouquetId == BouquetId) {
		goto CheckBouquetSections;
	    }
	}
	B = (lBouquets + nBouquets);
	B->BouquetId = BouquetId;
	for (int i = 0; i <= LastSectionNumber; i++) {
	    B->SectionNumber[i] = -1;
	}
	B->LastSectionNumber = LastSectionNumber;
	nBouquets++;
CheckBouquetSections:;
		     B->SectionNumber[SectionNumber] = SectionNumber;
		     EndBAT = true;
		     for (int i = 0; i < nBouquets; i++) {
			 B = (lBouquets + i);
			 for (int ii = 0; ii <= B->LastSectionNumber; ii++) {
			     if (B->SectionNumber[ii] == -1) {
				 EndBAT = false;
				 break;
			     }
			 }
		     }
    }
}

// }}}

// cTaskLoadepg::GetTitlesSKYBOX {{{
void cTaskLoadepg::GetTitlesSKYBOX(int FilterId, unsigned char *Data, int Length)
{
    int p;
    unsigned short int ChannelId;
    unsigned short int MjdTime;
    int Len1;
    int Len2;

    if (Length < 20) {
	return;
    }
    if (memcmp(&InitialBuffer[FilterId][0], Data, 20) == 0) {
	Filters[FilterId].Step = 2;
	return;
    } else {
	if (InitialBuffer[FilterId][0] == 0) {
	    memcpy(&InitialBuffer[FilterId][0], Data, 20);
	}
	ChannelId = (Data[3] << 8) | Data[4];
	MjdTime = ((Data[8] << 8) | Data[9]);
	if (ChannelId > 0) {
	    if (MjdTime > 0) {
		p = 10;
loop1:;
      sTitle *T = (lTitles + nTitles);
      T->ChannelId = ChannelId;
      T->MjdTime = MjdTime;
      T->EventId = (Data[p] << 8) | Data[p + 1];
      Len1 = ((Data[p + 2] & 0x0f) << 8) | Data[p + 3];
      if (Data[p + 4] != 0xb5) {
	  log_message(WARNING, "data error signature for title");
	  goto endloop1;
      }
      if (Len1 > Length) {
	  log_message(WARNING, "data error length for title");
	  goto endloop1;
      }
      p += 4;
      Len2 = Data[p + 1] - 7;
      T->StartTime = ((MjdTime - 40587) * 86400) + ((Data[p + 2] << 9) | (Data[p + 3] << 1));
      T->Duration = ((Data[p + 4] << 9) | (Data[p + 5] << 1));
      T->ThemeId = Data[p + 6];
      T->pData = pT;
      T->lenData = Len2;
      if ((pT + Len2 + 2) > MAX_BUFFER_SIZE_TITLES) {
	  log_message(ERROR, "buffer overflow, titles size more than %i bytes", MAX_BUFFER_SIZE_TITLES);
	  IsError = true;
	  return;
      }
      memcpy(&bTitles[pT], &Data[p + 9], Len2);
      pT += (Len2 + 1);
      p += Len1;
      nTitles++;
      if (nTitles >= MAX_TITLES) {
	  log_message(ERROR, "titles found more than %i", MAX_TITLES);
	  IsError = true;
	  return;
      }
      if (p < Length) {
	  goto loop1;
      }
endloop1:;
	    }
	}
    }
}

// }}}

// cTaskLoadepg::GetSummariesSKYBOX {{{
void cTaskLoadepg::GetSummariesSKYBOX(int FilterId, unsigned char *Data, int Length)
{
    int p;
    unsigned short int ChannelId;
    unsigned short int MjdTime;
    int Len1;
    int Len2;

    if (Length < 20) {
	return;
    }
    if (memcmp(&InitialBuffer[FilterId][0], Data, 20) == 0) {
	Filters[FilterId].Step = 2;
	return;
    } else {
	if (InitialBuffer[FilterId][0] == 0) {
	    memcpy(&InitialBuffer[FilterId][0], Data, 20);
	}
	ChannelId = (Data[3] << 8) | Data[4];
	MjdTime = ((Data[8] << 8) | Data[9]);
	if (ChannelId > 0) {
	    if (MjdTime > 0) {
		p = 10;
loop1:;
      sSummary *S = (lSummaries + nSummaries);
      S->ChannelId = ChannelId;
      S->MjdTime = MjdTime;
      S->EventId = (Data[p] << 8) | Data[p + 1];
      Len1 = ((Data[p + 2] & 0x0f) << 8) | Data[p + 3];
      if (Data[p + 4] != 0xb9) {
	  log_message(WARNING, "data error signature for summary");
	  goto endloop1;
      }
      if (Len1 > Length) {
	  log_message(WARNING, "data error length for summary");
	  goto endloop1;
      }
      p += 4;
      Len2 = Data[p + 1];
      S->pData = pS;
      S->lenData = Len2;
      if ((pS + Len2 + 2) > MAX_BUFFER_SIZE_SUMMARIES) {
	  log_message(ERROR, "buffer overflow, summaries size more than %i bytes", MAX_BUFFER_SIZE_SUMMARIES);
	  IsError = true;
	  return;
      }
      memcpy(&bSummaries[pS], &Data[p + 2], Len2);
      pS += (Len2 + 1);
      p += Len1;
      nSummaries++;
      if (nSummaries >= MAX_SUMMARIES) {
	  log_message(ERROR, "summaries found more than %i", MAX_SUMMARIES);
	  IsError = true;
	  return;
      }
      if (p < Length) {
	  goto loop1;
      }
endloop1:;
	    }
	}
    }
}

// }}}
// }}}

// cTaskLoadepg::CleanString {{{
void cTaskLoadepg::CleanString(unsigned char *String)
{
    unsigned char *Src;
    unsigned char *Dst;
    int Spaces;
    int pC;
    Src = String;
    Dst = String;
    Spaces = 0;
    pC = 0;
    while (*Src) {
	// corrections
	if (*Src == 0x8c)	// iso-8859-2 LATIN CAPITAL LETTER S WITH ACUTE
	{
	    *Src = 0xa6;
	}
	if (*Src == 0x8f)	// iso-8859-2 LATIN CAPITAL LETTER Z WITH ACUTE
	{
	    *Src = 0xac;
	}

	if (*Src < 0x20) {
	    *Src = 0x20;
	}
	if (*Src == 0x20) {
	    Spaces++;
	    if (pC == 0) {
		Spaces++;
	    }
	} else {
	    Spaces = 0;
	}
	if (Spaces < 2) {
	    *Dst = *Src;
	    *Dst++;
	    pC++;
	}
	*Src++;
    }
    if (Spaces > 0) {
	Dst--;
	*Dst = 0;
    } else {
	*Dst = 0;
    }
}

// }}}

// MHW1 Stuff {{{
// cTaskLoadepg::GetThemesMHW1 {{{
void cTaskLoadepg::GetThemesMHW1(int FilterId, unsigned char *Data, int Length)
{
    if (Length > 19) {
	sThemeMHW1 *Theme = (sThemeMHW1 *) (Data + 19);
	nThemes = (Length - 19) / 15;
	if (nThemes > MAX_THEMES) {
	    log_message(ERROR, "themes found more than %i", MAX_THEMES);
	    IsError = true;
	    return;
	} else {
	    int ThemeId = 0;
	    int Offset = 0;
	    unsigned char *ThemesIndex = (Data + 3);
	    for (int i = 0; i < nThemes; i++) {
		if (ThemesIndex[ThemeId] == i) {
		    Offset = (Offset + 15) & 0xf0;
		    ThemeId++;
		}
		sTheme *T = (lThemes + Offset);
		memcpy(&T->Name, &Theme->Name, 15);
		CleanString(T->Name);
		Offset++;
		Theme++;
	    }
	    Filters[FilterId].Step = 2;
	}
    }
}

// }}}

// cTaskLoadepg::GetChannelsMHW1 {{{
void cTaskLoadepg::GetChannelsMHW1(int FilterId, unsigned char *Data, int Length)
{
    sChannelMHW1 *Channel = (sChannelMHW1 *) (Data + 4);
    nChannels = (Length - 4) / sizeof(sChannelMHW1);
    set_stat("channels.count", nChannels);
    if (nChannels > MAX_CHANNELS) {
	log_message(ERROR, "channels found more than %i", MAX_CHANNELS);
	IsError = true;
	return;
    } else {
	for (int i = 0; i < nChannels; i++) {
	    sChannel *C = (lChannels + i);
	    C->ChannelId = i;
	    C->Nid = HILO16(Channel->NetworkId);
	    C->Tid = HILO16(Channel->TransportId);
	    C->Sid = HILO16(Channel->ServiceId);
	    C->SkyNumber = 0;
	    C->pData = pC;
	    C->lenData = 16;
	    C->IsFound = false;
	    C->IsEpg = true;
	    if ((pC + 18) > MAX_BUFFER_SIZE_CHANNELS) {
		log_message(ERROR, "buffer overflow, channels size more than %i bytes", MAX_BUFFER_SIZE_CHANNELS);
		IsError = true;
		return;
	    }
	    memcpy(&bChannels[pC], &Channel->Name, 16);
	    CleanString(&bChannels[pC]);
	    pC += 17;
	    Channel++;
	}
	Filters[FilterId].Step = 2;
    }
}

// }}}

// cTaskLoadepg::GetTitlesMHW1 {{{
void cTaskLoadepg::GetTitlesMHW1(int FilterId, unsigned char *Data, int Length)
{
    sTitleMHW1 *Title = (sTitleMHW1 *) Data;
    if (Length == 46) {
	if (Title->ChannelId != 0xff) {
	    if (nTitles < MAX_TITLES) {
		if (memcmp(&InitialBuffer[FilterId][0], Data, 46) == 0) {
		    Filters[FilterId].Step = 2;
		} else {
		    if (InitialBuffer[FilterId][0] == 0) {
			memcpy(&InitialBuffer[FilterId][0], Data, 46);
		    }
		    sTitle *T = (lTitles + nTitles);
		    int Day = Title->Day;
		    int Hours = Title->Hours;
		    int Minutes = Title->Minutes;
		    if (Hours > 15) {
			Hours -= 4;
		    } else if (Hours > 7) {
			Hours -= 2;
		    } else {
			Day++;
		    }
		    if (Day > 6) {
			Day = Day - 7;
		    }
		    Day -= Yesterday;
		    if (Day < 1) {
			Day = 7 + Day;
		    }
		    if (Day == 1 && Hours < 6) {
			Day = 8;
		    }
		    int StartTime = (Day * 86400) + (Hours * 3600) + (Minutes * 60);
		    StartTime += YesterdayEpoch;
		    T->ChannelId = Title->ChannelId - 1;
		    T->ThemeId = Title->ThemeId;
		    T->MjdTime = 0;
		    T->EventId = HILO32(Title->ProgramId);
		    T->StartTime = StartTime;
		    T->Duration = HILO16(Title->Duration) * 60;
		    T->SummaryAvailable = Title->SummaryAvailable;
		    T->pData = pT;
		    T->lenData = 23;
		    if ((pT + 25) > MAX_BUFFER_SIZE_TITLES) {
			log_message(ERROR, "buffer overflow, titles size more than %i bytes", MAX_BUFFER_SIZE_TITLES);
			IsError = true;
			return;
		    }
		    memcpy(&bTitles[pT], &Title->Title, 23);
		    CleanString(&bTitles[pT]);
		    pT += 24;
		    nTitles++;
		}
	    } else {
		log_message(ERROR, "titles found more than %i", MAX_TITLES);
		IsError = true;
		return;
	    }
	}
    }
}

// }}}

// cTaskLoadepg::GetSummariesMHW1 {{{
void cTaskLoadepg::GetSummariesMHW1(int FilterId, unsigned char *Data, int Length)
{
    sSummaryMHW1 *Summary = (sSummaryMHW1 *) Data;
    if (Length > 11) {
	if (Summary->NumReplays < 10) {
	    if (Length > (11 + (Summary->NumReplays * 7))) {
		if (Summary->Byte7 == 0xff && Summary->Byte8 && Summary->Byte9 == 0xff) {
		    if (nSummaries < MAX_SUMMARIES) {
			if (memcmp(&InitialBuffer[FilterId][0], Data, 20)
				== 0) {
			    Filters[FilterId].Step = 2;
			} else {
			    if (InitialBuffer[FilterId][0] == 0) {
				memcpy(&InitialBuffer[FilterId][0], Data, 20);
			    }
			    int SummaryOffset = 11 + (Summary->NumReplays * 7);
			    int SummaryLength = Length - SummaryOffset;
			    sSummary *S = (lSummaries + nSummaries);
			    S->ChannelId = 0;
			    S->MjdTime = 0;
			    S->EventId = HILO32(Summary->ProgramId);
			    S->pData = pS;
			    S->lenData = SummaryLength;
			    if ((pS + SummaryLength + 2) > MAX_BUFFER_SIZE_SUMMARIES) {
				log_message(ERROR, "buffer overflow, summaries size more than %i bytes", MAX_BUFFER_SIZE_SUMMARIES);
				IsError = true;
				return;
			    }
			    memcpy(&bSummaries[pS], &Data[SummaryOffset], SummaryLength);
			    CleanString(&bSummaries[pS]);
			    pS += (SummaryLength + 1);
			    nSummaries++;
			}
		    } else {
			log_message(ERROR, "summaries found more than %i", MAX_SUMMARIES);
			IsError = true;
			return;
		    }
		}
	    }
	}
    }
}

// }}}
// }}}

// MHW2 Stuff {{{
// cTaskLoadepg::GetThemesMHW2 {{{
void cTaskLoadepg::GetThemesMHW2(int FilterId, unsigned char *Data, int Length)
{
    int p1;
    int p2;
    int pThemeName = 0;
    int pSubThemeName = 0;
    int lenThemeName = 0;
    int lenSubThemeName = 0;
    int pThemeId = 0;
    if (EndThemes && EndChannels) {
	Filters[FilterId].Step = 2;
	return;
    }
    if (Length > 4) {
	for (int i = 0; i < Data[4]; i++) {
	    p1 = ((Data[5 + i * 2] << 8) | Data[6 + i * 2]) + 3;
	    if (Length > p1) {
		for (int ii = 0; ii <= (Data[p1] & 0x3f); ii++) {
		    p2 = ((Data[p1 + 1 + ii * 2] << 8) | Data[p1 + 2 + ii * 2]) + 3;
		    if (Length > p2) {
			if (ii == 0) {
			    pThemeName = p2 + 1;
			    lenThemeName = Data[p2] & 0x1f;
			    lenSubThemeName = 0;
			} else {
			    pSubThemeName = p2 + 1;
			    lenSubThemeName = Data[p2] & 0x1f;
			}
			if (Length >= (pThemeName + lenThemeName)) {
			    pThemeId = ((i & 0x3f) << 6) | (ii & 0x3f);
			    if ((lenThemeName + 2) < 256) {
				memcpy((lThemes + pThemeId)->Name, &Data[pThemeName], lenThemeName);
				if (Length >= (pSubThemeName + lenSubThemeName)) {
				    if (lenSubThemeName > 0) {
					if ((lenThemeName + lenSubThemeName + 2) < 256) {
					    (lThemes + pThemeId)->Name[lenThemeName] = ' ';
					    memcpy(&(lThemes + pThemeId)->Name[lenThemeName + 1], &Data[pSubThemeName], lenSubThemeName);
					}
				    }
				}
				CleanString((lThemes + pThemeId)->Name);
				nThemes++;
				if (nThemes > MAX_THEMES) {
				    log_message(ERROR, "themes found more than %i", MAX_THEMES);
				    IsError = true;
				    return;
				}
			    }
			} else {
			    return;
			}
		    } else {
			return;
		    }
		}
	    } else {
		return;
	    }
	}
	EndThemes = true;
    }
}

// }}}

// cTaskLoadepg::GetChannelsMHW2 {{{
void cTaskLoadepg::GetChannelsMHW2(int FilterId, unsigned char *Data, int Length)
{
    if (EndThemes && EndChannels) {
	Filters[FilterId].Step = 2;
	return;
    }
    if (Length > 119) {
	nChannels = Data[119];
	set_stat("channels.count", nChannels);
	if (nChannels > MAX_CHANNELS) {
	    log_message(ERROR, "channels found more than %i", MAX_CHANNELS);
	    IsError = true;
	    return;
	} else {
	    int pName = ((nChannels * 8) + 120);
	    if (Length > pName) {
		sChannelMHW2 *Channel = (sChannelMHW2 *) (Data + 120);
		for (int i = 0; i < nChannels; i++) {
		    sChannel *C = (lChannels + i);
		    C->ChannelId = i;
		    C->Nid = HILO16(Channel->NetworkId);
		    C->Tid = HILO16(Channel->TransportId);
		    C->Sid = HILO16(Channel->ServiceId);
		    C->SkyNumber = 0;
		    int lenName = Data[pName] & 0x0f;
		    if ((pC + lenName + 2) > MAX_BUFFER_SIZE_CHANNELS) {
			log_message(ERROR, "buffer overflow, channels size more than %i bytes", MAX_BUFFER_SIZE_CHANNELS);
			IsError = true;
			return;
		    }
		    memcpy(&bChannels[pC], &Data[pName + 1], lenName);
		    pName += (lenName + 1);
		    C->pData = pC;
		    C->lenData = lenName;
		    C->IsFound = false;
		    C->IsEpg = true;
		    pC += (lenName + 1);
		    Channel++;
		}
		EndChannels = true;
	    }
	}
    }
}

// }}}

// cTaskLoadepg::GetTitlesMHW2 {{{
void cTaskLoadepg::GetTitlesMHW2(int FilterId, unsigned char *Data, int Length)
{
    if (Length > 18) {
	int Pos = 18;
	int Len = 0;
	bool Check = false;
	while (Pos < Length) {
	    Check = false;
	    Pos += 7;
	    if (Pos < Length) {
		Pos += 3;
		if (Pos < Length) {
		    if (Data[Pos] > 0xc0) {
			Pos += (Data[Pos] - 0xc0);
			Pos += 4;
			if (Pos < Length) {
			    if (Data[Pos] == 0xff) {
				Pos += 1;
				Check = true;
			    }
			}
		    }
		}
	    }
	    if (Check == false) {
		return;
	    }
	}
	if (memcmp(&InitialBuffer[FilterId][0], Data, 16) == 0) {
	    Filters[FilterId].Step = 2;
	} else {
	    if (InitialBuffer[FilterId][0] == 0) {
		memcpy(&InitialBuffer[FilterId][0], Data, 16);
	    }
	    Pos = 18;
	    while (Pos < Length) {
		sTitle *T = (lTitles + nTitles);
		T->ChannelId = Data[Pos];
		T->MjdTime = (Data[Pos + 3] << 8) | Data[Pos + 4];
		T->StartTime = ((T->MjdTime - 40587) * 86400)
		    + (((((Data[Pos + 5] & 0xf0) >> 4) * 10) + (Data[Pos + 5] & 0x0f)) * 3600)
		    + (((((Data[Pos + 6] & 0xf0) >> 4) * 10) + (Data[Pos + 6] & 0x0f)) * 60);
		T->Duration = (((Data[Pos + 8] << 8) | Data[Pos + 9]) >> 4) * 60;
		Len = Data[Pos + 10] & 0x3f;
		T->pData = pT;
		T->lenData = Len;
		if ((pT + Len + 2) > MAX_BUFFER_SIZE_TITLES) {
		    log_message(ERROR, "buffer overflow, titles size more than %i bytes", MAX_BUFFER_SIZE_TITLES);
		    IsError = true;
		    return;
		}
		memcpy(&bTitles[pT], &Data[Pos + 11], Len);
		CleanString(&bTitles[pT]);
		pT += (Len + 1);
		Pos += Len + 11;
		T->ThemeId = ((Data[7] & 0x3f) << 6) | (Data[Pos] & 0x3f);
		T->EventId = (Data[Pos + 1] << 8) | Data[Pos + 2];
		T->SummaryAvailable = 1;
		Pos += 4;
		nTitles++;
		if (nTitles > MAX_TITLES) {
		    log_message(ERROR, "titles found more than %i", MAX_TITLES);
		    IsError = true;
		    return;
		}
	    }
	}
    }
}

// }}}

// cTaskLoadepg::GetSummariesMHW2 {{{
void cTaskLoadepg::GetSummariesMHW2(int FilterId, unsigned char *Data, int Length)
{
    if (Length > (Data[14] + 17)) {
	if (memcmp(&InitialBuffer[FilterId][0], Data, 16) == 0) {
	    Filters[FilterId].Step = 2;
	} else {
	    if (InitialBuffer[FilterId][0] == 0) {
		memcpy(&InitialBuffer[FilterId][0], Data, 16);
	    }
	    int lenText = Data[14];
	    int lenSummary = lenText;
	    int Pos = 15;
	    int Loop = Data[Pos + lenSummary] & 0x0f;
	    if ((pS + lenText + 2) < MAX_BUFFER_SIZE_SUMMARIES) {
		sSummary *S = (lSummaries + nSummaries);
		S->ChannelId = 0;
		S->MjdTime = 0;
		S->EventId = (Data[3] << 8) | Data[4];
		S->pData = pS;
		memcpy(&bSummaries[S->pData], &Data[Pos], lenText);
		bSummaries[S->pData + lenSummary] = '|';
		lenSummary += 1;
		Pos += (lenText + 1);
		if (Loop > 0) {
		    while (Loop > 0) {
			lenText = Data[Pos];
			Pos += 1;
			if ((Pos + lenText) < Length) {
			    if ((pS + lenSummary + lenText + 2) > MAX_BUFFER_SIZE_SUMMARIES) {
				log_message(ERROR, "buffer overflow, summaries size more than %i bytes", MAX_BUFFER_SIZE_SUMMARIES);
				IsError = true;
				return;
			    }
			    memcpy(&bSummaries[S->pData + lenSummary], &Data[Pos], lenText);
			    lenSummary += lenText;
			    if (Loop > 1) {
				bSummaries[S->pData + lenSummary] = '|';
				lenSummary += 1;
			    }
			} else {
			    break;
			}
			Pos += lenText;
			Loop--;
		    }
		}
		CleanString(&bSummaries[S->pData]);
		S->lenData = lenSummary;
		pS += (lenSummary + 1);
		nSummaries++;
	    } else {
		log_message(ERROR, "buffer overflow, summaries size more than %i", MAX_BUFFER_SIZE_SUMMARIES);
		IsError = true;
		return;
	    }
	}
    }
}

// }}}
// }}}

// cTaskLoadepg::CreateEpgXml {{{
void cTaskLoadepg::CreateEpgXml(void)
{
    log_message(INFO, "found %i themes", nThemes);
    log_message(INFO, "found %i channels", nChannels);
    log_message(INFO, "found %i titles", nTitles);
    log_message(INFO, "found %i summaries", nSummaries);
    qsort(lChannels, nChannels, sizeof(sChannel), &qsortChannels);
    qsort(lTitles, nTitles, sizeof(sTitle), &qsortTitles);
    qsort(lSummaries, nSummaries, sizeof(sSummary), &qsortSummaries);

    if (print_stats) {
	printf("<!-- channels.count=\"%d\" -->\n", get_stat("channels.count"));
    }

    if (format == DATA_FORMAT_SKYBOX) {
	// SKYBOX
	if (ReadFileDictionary()) {
	    ReadFileThemes();
	    if (nChannels > 0) {
		if (nTitles > 0) {
		    if (nSummaries > 0) {
			int i;
			int prev_i;
			int EventId;
			unsigned short int ChannelId;
			bool IsChannel;
			sChannel KeyC, *C;
			i = 0;
			prev_i = 0;
			EventId = 1;
			ChannelId = 0;
			IsChannel = false;

			while (i < nTitles) {
			    char date_strbuf[256];
			    time_t StartTime;

			    sTitle *T = (lTitles + i);

			    KeyC.ChannelId = T->ChannelId;
			    C = (sChannel *) bsearch(&KeyC, lChannels, nChannels, sizeof(sChannel), &bsearchChannelByChannelId);
			    if (C) {
				C->IsEpg = true;
				C->IsFound = true;
			    }

			    char *channelIdent = get_channelident(C);
			    printf("<programme channel=\"%s\" ", channelIdent);
			    StartTime = (T->StartTime + EpgTimeOffset);
			    strftime(date_strbuf, sizeof(date_strbuf), "start=\"%Y%m%d%H%M%S %z\"", localtime(&StartTime));
			    printf("%s ", date_strbuf);
			    StartTime = (T->StartTime + T->Duration + EpgTimeOffset);
			    strftime(date_strbuf, sizeof(date_strbuf), "stop=\"%Y%m%d%H%M%S %z\"", localtime(&StartTime));
			    printf("%s>\n ", date_strbuf);

			    //printf("\t<EventID>%i</EventID>\n", HILO(evt->event_id));
			    //printf("\t<RunningStatus>%i</RunningStatus>\n", evt->running_status);
			    //1 Airing, 2 Starts in a few seconds, 3 Pausing, 4 About to air

			    if (DecodeHuffmanCode(&bTitles[T->pData], T->lenData)) {
				CleanString(DecodeText);
				//printf("\t<title lang=\"%s\">%s</title>\n", xmllang(&evtdesc->lang_code1), xmlify(evt));
				printf("\t<title lang=\"%s\">%s</title>\n",
					/* xmllang(&evtdesc->lang_code1) */
					"en", xmlify((const char *) DecodeText));
			    }
			    sSummary KeyS, *S;
			    KeyS.ChannelId = T->ChannelId;
			    KeyS.MjdTime = T->MjdTime;
			    KeyS.EventId = T->EventId;
			    S = (sSummary *) bsearch(&KeyS, lSummaries, nSummaries, sizeof(sSummary), &bsearchSummarie);
			    if (S) {
				if (DecodeHuffmanCode(&bSummaries[S->pData], S->lenData)) {
				    CleanString(DecodeText);
				    char *d = xmlify((const char *) DecodeText);
				    if (d && *d) {
					//printf("\t<sub-title lang=\"%s\">%s</sub-title>\n", 
					//   /*xmllang(&evtdesc->lang_code1)*/ "en", d);
					char *colon = strrchr(d, ':');
					if (colon != NULL) {
					    *colon = 0;
					    printf("\t<subtitle lang=\"%s\">%s</subtitle>\n",
						    /*xmllang(&levt->lang_code1) */
						    "en", d);
					    d = colon + 2;
					}
					printf("\t<desc lang=\"%s\">",
						/*xmllang(&levt->lang_code1) */
						"en");
					printf("%s", d);
					printf("</desc>\n");
				    }
				}
			    }
			    delete channelIdent;
			    printf("</programme>\n");
			    i++;
			    EventId++;
			}
		    }
		}
	    }
	}
    } else {
	// MHW_1 && MHW_2
	int i;
	int prev_i;
	unsigned char ChannelId;
	sChannel KeyC, *C;
	bool IsChannel;
	i = 0;
	prev_i = 0;
	ChannelId = 0xff;
	IsChannel = false;
	while (i < nTitles) {
	    sTitle *T = (lTitles + i);
	    if (ChannelId != T->ChannelId) {
		IsChannel = false;
		KeyC.ChannelId = T->ChannelId;
		C = (sChannel *) bsearch(&KeyC, lChannels, nChannels, sizeof(sChannel), &bsearchChannelByChannelId);
		ChannelId = T->ChannelId;
	    }
	    if (IsChannel) {
		log_message(TRACE, "E %u %u %u 01 FF", T->EventId, (T->StartTime + EpgTimeOffset), T->Duration);
		log_message(TRACE, "T %s", &bTitles[T->pData]);
		if ((lThemes + T->ThemeId)->Name[0] != '\0') {
		    if (is_logging(TRACE)) {
			time_t StartTime;
			char *DateTime;
			StartTime = (T->StartTime + EpgTimeOffset);
			asprintf(&DateTime, "%s", ctime(&StartTime));
			log_message(TRACE, "S %s - %d\' - %s", (lThemes + T->ThemeId)->Name, T->Duration / 60, DateTime);
		    }
		}
		sSummary KeyS, *S;
		KeyS.ChannelId = 0;
		KeyS.MjdTime = 0;
		KeyS.EventId = T->EventId;
		S = (sSummary *) bsearch(&KeyS, lSummaries, nSummaries, sizeof(sSummary), &bsearchSummarie);
		if (S) {
		    log_message(TRACE, "D %s", &bSummaries[S->pData]);
		}
		log_message(TRACE, "e");
	    }
	    i++;
	}
	if (IsChannel) {
	    log_message(TRACE, "c");
	}
    }
    CreateXmlChannels();
}

// }}}

// EPGGrabber {{{
EPGGrabber::EPGGrabber()
{
    Config = new sConfig();
    asprintf(&Config->Directory, "%s", conf);
    Config->EnableOsdMessages = false;
    Task = NULL;
}

EPGGrabber::~EPGGrabber()
{
    // Clean up after yourself!
    if (Config) {
	if (Config->Directory) {
	    free(Config->Directory);
	}
	free(Config);
    }
#ifdef notdef
    if (lProviders) {
	for (int i = 0; i < nProviders; i++) {
	    if ((lProviders + i)->Title) {
		free((lProviders + i)->Title);
	    }
	    if ((lProviders + i)->Parm1) {
		free((lProviders + i)->Parm1);
	    }
	    if ((lProviders + i)->Parm2) {
		free((lProviders + i)->Parm2);
	    }
	}
	free(lProviders);
    }
    if (lEquivChannels) {
	free(lEquivChannels);
    }
#endif
    if (Task) {
	delete(Task);
    }
}

// }}}

// GrabEPG {{{
void EPGGrabber::Grab()
{
    Task = new cTaskLoadepg();
    if (Task) {
#ifdef USETHREADS
	Task->Start();
	time_t starttime = time(NULL);
	while (Task->Active()) {
	    usleep(1000000);
	    time_t now = time(NULL);
	    time_t diff = now - starttime;
	    if (Task->IsLoopRunning()) {
		if (is_logging(DEBUG)) {
		    log_message(DEBUG, "%d found %d channels %d titles %d summaries", diff, nChannels, nTitles, nSummaries);
		} else {
		    log_raw_message(INFO, "\r%d found %d channels %d titles %d summaries\r", diff, nChannels, nTitles, nSummaries);
		}
	    }
	    if (diff >= 60) {
		log_message(ERROR, "timed out");
		break;
	    }
	}
	Task->Stop();
#else
	Task->Action();
#endif
	// stop load epg
	delete(Task);
	Task = NULL;
    }
}

// }}}

// vim: foldmethod=marker ts=8 sw=4
