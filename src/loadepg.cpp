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

#include "constants.h"
#include "loadepg.h"
#include "dvbtext.h"
#include "chanid.h"

static const char *VERSION        = "0.2.1-20080915";
#if APIVERSNUM >= 10507
static const char *DESCRIPTION    = trNOOP( "Load EPG type MediaHighWay, SkyBox, from script or file" );
static const char *MAINMENUENTRY  = trNOOP( "LoadEPG" );
#else
static const char *DESCRIPTION    = "Load EPG type MediaHighWay, SkyBox, from script or file";
static const char *MAINMENUENTRY  = "LoadEPG";
#endif

#undef esyslog
#define esyslog(x, args...) fprintf(stderr, x "\n", ##args)
#define Dprintf(x, args...) fprintf(stderr, x, ##args)

#if 1
#undef DEBUG
#define DEBUG 1
#endif

sConfig *Config;
cTaskLoadepg *Task;

extern char *ProgName;
extern char conf[1024];

extern int adapter;
extern int vdrmode;
extern bool useshortxmlids;

int CurrentProvider;
int nProviders;
sProvider *lProviders;

int nEquivChannels;
sEquivChannel *lEquivChannels;

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

unsigned char *bChannels; // buffer data channels
unsigned char *bTitles; // buffer data titles
unsigned char *bSummaries; // buffer data summaries

int EpgTimeOffset;
int LocalTimeOffset;
int SatelliteTimeOffset;
int Yesterday;
int YesterdayEpoch;

// misc functions {{{
static int BcdToInt( unsigned char Bcd )
{
  return ( ( ( Bcd & 0xf0 ) >> 4 ) * 10 ) + ( Bcd & 0x0f );
}

static char *GetStringMJD( int MJD )
{
  int J, C, Y, M;
  int Day, Month, Year;
  char *Buffer;
  J = MJD + 2400001 + 68569;
  C = 4 * J / 146097;
  J = J - ( 146097 * C + 3 ) / 4;
  Y = 4000 * ( J + 1 ) / 1461001;
  J = J - 1461 * Y / 4 + 31;
  M = 80 * J / 2447;
  Day = J - 2447 * M / 80;
  J = M / 11;
  Month = M + 2 - ( 12 * J );
  Year = 100 * ( C - 49 ) + Y + J;
  asprintf( &Buffer, "%02i/%02i/%04i", Day, Month, Year );
  return Buffer;
}
// }}}

// ReadConfigLoadepg {{{
static void ReadConfigLoadepg( void )
{
  char Buffer[1024];
  char *Line;
  FILE *File;
  char *FileName;
  bool IsSkyThemesNull = false;
  if( lProviders )
  {
    if( nProviders > 0 )
    {
      for( int i = 0; i < nProviders; i ++ )
      {
        if( ( lProviders + i )->Title )
	{
	  free( ( lProviders + i )->Title );
	}
	if( ( lProviders + i )->Parm1 )
	{
	  free( ( lProviders + i )->Parm1 );
	}
	if( ( lProviders + i )->Parm2 )
	{
	  free( ( lProviders + i )->Parm2 );
	}
	if( ( lProviders + i )->Parm3 )
	{
	  free( ( lProviders + i )->Parm3 );
	}
      }
    }
    free( lProviders );
    lProviders = NULL;
  }
  if( lEquivChannels )
  {
    free( lEquivChannels );
    lEquivChannels = NULL;
  }
  lProviders = ( sProvider * ) calloc( MAX_PROVIDERS, sizeof( sProvider ) );
  lEquivChannels = ( sEquivChannel * ) calloc( MAX_CHANNELS, sizeof( sEquivChannel ) );
  nProviders = 0;
  nEquivChannels = 0;
  
  // Read loadepg.conf
  Dprintf( "Config->Directory=\"%s\"\n", Config->Directory);
  asprintf( &FileName, "%s/%s", Config->Directory, LOADEPG_FILE_CONF );
  Dprintf( "Readconfig %s\n", FileName);
  File = fopen( FileName, "r" );
  if( File )
  {
    char string1[256];
    char string2[256];
    char string3[256];
    char string4[256];
    char char1;
    int int1;
    int int2;
    memset( Buffer, 0, sizeof( Buffer ) );
    while( ( Line = fgets( Buffer, sizeof( Buffer ), File ) ) != NULL )
    {
      Line = compactspace( skipspace( stripspace( Line ) ) );
      //Dprintf( "line is '%s'\n", Line);
      if( ! isempty( Line ) )
      {
	if ( Line[0] == '#' )
	  continue;
        if( nProviders < MAX_PROVIDERS )
	{
          if( sscanf( Line, "SKYBOX=%[^:] :%i :%c :%[^:] :%i :%[^:] :%s ", string1, &int1, &char1, string2, &int2, string3, string4 ) == 7 )
	  {
	    Dprintf( "is SKYBOX\n");
	    asprintf( &( lProviders + nProviders )->Title, "%s", string1 );
	    ( lProviders + nProviders )->DataFormat = DATA_FORMAT_SKYBOX;
	    ( lProviders + nProviders )->SourceId = cSource::FromString( string2 );
	    asprintf( &( lProviders + nProviders )->Parm1, "%i:%c:%s:%i", int1, char1, string2, int2 );
	    asprintf( &( lProviders + nProviders )->Parm2, "%s", string3 );
	    asprintf( &( lProviders + nProviders )->Parm3, "%s", string4 );
	    nProviders ++;
	  }
          else if( sscanf( Line, "SKYBOX=%[^:] :%i :%c :%[^:] :%i :%s ", string1, &int1, &char1, string2, &int2, string3 ) == 6 )
	  {
	    asprintf( &( lProviders + nProviders )->Title, "%s", string1 );
	    ( lProviders + nProviders )->DataFormat = DATA_FORMAT_SKYBOX;
	    ( lProviders + nProviders )->SourceId = cSource::FromString( string2 );
	    asprintf( &( lProviders + nProviders )->Parm1, "%i:%c:%s:%i", int1, char1, string2, int2 );
	    asprintf( &( lProviders + nProviders )->Parm2, "%s", string3 );
	    asprintf( &( lProviders + nProviders )->Parm3, '\0' );
	    nProviders ++;
	    IsSkyThemesNull = true;
	  }
          else if( sscanf( Line, "MHW_1=%[^:] :%i :%c :%[^:] :%i ", string1, &int1, &char1, string2, &int2 ) == 5 )
	  {
	    asprintf( &( lProviders + nProviders )->Title, "%s", string1 );
	    ( lProviders + nProviders )->DataFormat = DATA_FORMAT_MHW_1;
	    ( lProviders + nProviders )->SourceId = cSource::FromString( string2 );
	    asprintf( &( lProviders + nProviders )->Parm1, "%i:%c:%s:%i", int1, char1, string2, int2 );
	    asprintf( &( lProviders + nProviders )->Parm2, '\0' );
	    asprintf( &( lProviders + nProviders )->Parm3, '\0' );
	    nProviders ++;
	  }
          else if( sscanf( Line, "MHW_2=%[^:] :%i :%c :%[^:] :%i ", string1, &int1, &char1, string2, &int2 ) == 5 )
	  {
	    asprintf( &( lProviders + nProviders )->Title, "%s", string1 );
	    ( lProviders + nProviders )->DataFormat = DATA_FORMAT_MHW_2;
	    ( lProviders + nProviders )->SourceId = cSource::FromString( string2 );
	    asprintf( &( lProviders + nProviders )->Parm1, "%i:%c:%s:%i", int1, char1, string2, int2 );
	    asprintf( &( lProviders + nProviders )->Parm2, '\0' );
	    asprintf( &( lProviders + nProviders )->Parm3, '\0' );
	    nProviders ++;
	  }
          else if( sscanf( Line, "FILE=%[^:] :%s ", string1, string2 ) == 2 )
	  {
	    asprintf( &( lProviders + nProviders )->Title, "%s", string1 );
	    ( lProviders + nProviders )->DataFormat = DATA_FORMAT_FILE;
	    ( lProviders + nProviders )->SourceId = 0;
	    asprintf( &( lProviders + nProviders )->Parm1, "%s", string2 );
	    asprintf( &( lProviders + nProviders )->Parm2, '\0' );
	    asprintf( &( lProviders + nProviders )->Parm3, '\0' );
	    nProviders ++;
	  }
          else if( sscanf( Line, "SCRIPT=%[^:] :%[^:] :%s ", string1, string2, string3 ) == 3 )
	  {
	    asprintf( &( lProviders + nProviders )->Title, "%s", string1 );
	    ( lProviders + nProviders )->DataFormat = DATA_FORMAT_SCRIPT;
	    ( lProviders + nProviders )->SourceId = 0;
	    asprintf( &( lProviders + nProviders )->Parm1, "%s", string2 );
	    asprintf( &( lProviders + nProviders )->Parm2, "%s", string3 );
	    asprintf( &( lProviders + nProviders )->Parm3, '\0' );
	    nProviders ++;
	  }
	}
      }
      memset( Buffer, 0, sizeof( Buffer ) );
    }
    fclose( File );
#if DEBUG
    for( int i = 0; i < nProviders; i ++ )
    {
      fprintf( stderr, "%s|%i|%s|%s|%s\n", ( lProviders + i )->Title, ( lProviders + i )->SourceId, ( lProviders + i )->Parm1, ( lProviders + i )->Parm2, ( lProviders + i )->Parm3 );
    }
#endif
  }
  
  // IsSkyThemes (IT)
  if( IsSkyThemesNull )
  {
    File = fopen( FileName, "w" );
    if( File )
    {
      for( int i = 0; i < nProviders; i ++ )
      {
        switch( ( lProviders + i )->DataFormat )
	{
	  case DATA_FORMAT_SKYBOX:
	    if( strcasestr( ( lProviders +i )->Parm2, "sky_it.dict" ) != NULL )
	    {
	      fprintf( File, "SKYBOX=%s:%s:%s:sky_it.themes\n", ( lProviders +i )->Title, ( lProviders +i )->Parm1, ( lProviders +i )->Parm2 );
	    }
	    if( strcasestr( ( lProviders +i )->Parm2, "sky_uk.dict" ) != NULL )
	    {
	      fprintf( File, "SKYBOX=%s:%s:%s:sky_uk.themes\n", ( lProviders +i )->Title, ( lProviders +i )->Parm1, ( lProviders +i )->Parm2 );
	    }
	    if( strcasestr( ( lProviders +i )->Parm2, "sky_au.dict" ) != NULL )
	    {
	      fprintf( File, "SKYBOX=%s:%s:%s:sky_au.themes\n", ( lProviders +i )->Title, ( lProviders +i )->Parm1, ( lProviders +i )->Parm2 );
	    }
	    break;
	  case DATA_FORMAT_MHW_1:
	    fprintf( File, "MHW_1=%s:%s\n", ( lProviders +i )->Title, ( lProviders +i )->Parm1 );
	    break;
	  case DATA_FORMAT_MHW_2:
	    fprintf( File, "MHW_2=%s:%s\n", ( lProviders +i )->Title, ( lProviders +i )->Parm1 );
	    break;
	  case DATA_FORMAT_FILE:
	    fprintf( File, "FILE=%s:%s\n", ( lProviders +i )->Title, ( lProviders +i )->Parm1 );
	    break;
	  case DATA_FORMAT_SCRIPT:
	    fprintf( File, "SCRIPT=%s:%s:%s\n", ( lProviders +i )->Title, ( lProviders +i )->Parm1, ( lProviders +i )->Parm2 );
	    break;
	  default:
	    break;
	};
      }
      fclose( File );
    }
  }
  free( FileName );
  
  // Read loadepg.equiv
  asprintf( &FileName, "%s/%s", Config->Directory, LOADEPG_FILE_EQUIV );
  File = fopen( FileName, "r" );
  if( File )
  {
    memset( Buffer, 0, sizeof( Buffer ) );
    char string1[256];
    char string2[256];
    char string3[256];
    int int1;
    int int2;
    int int3;
    int int4;
    while( ( Line = fgets( Buffer, sizeof( Buffer ), File ) ) != NULL )
    {
      Line = compactspace( skipspace( stripspace( Line ) ) );
      if( ! isempty( Line ) )
      {
        if( sscanf( Line, "%[^ ] %[^ ] %[^\n]\n", string1, string2, string3 ) == 3 )
	{
	  if( string1[0] != '#' && string1[0] != ';' )
	  {
	    int1 = 0;
	    int2 = 0;
	    int3 = 0;
	    int4 = 0;
	    if( sscanf( string1, "%[^-]-%i -%i -%i ", string3, &int1, &int2, &int3 ) == 4 )if( sscanf( string2, "%[^-]-%i -%i -%i ", string3, &int1, &int2, &int3 ) == 4 )
	    {
	      if( sscanf( string1, "%[^-]-%i -%i -%i -%i ", string3, &int1, &int2, &int3, &int4 ) != 5 )
	      {
	        int4 = 0;
	      }
	      tChannelID OriginalChID = tChannelID( cSource::FromString( string3 ), int1, int2, int3, int4 );
	      cChannel *OriginalChannel = NULL; // Channels.GetByChannelID( OriginalChID, false );
	      if( OriginalChannel )
	      {
		if( sscanf( string2, "%[^-]-%i -%i -%i ", string3, &int1, &int2, &int3 ) == 4 )
		{
		  if( sscanf( string2, "%[^-]-%i -%i -%i -%i ", string3, &int1, &int2, &int3, &int4 ) != 5 )
		  {
		    int4 = 0;
		  }
		  tChannelID EquivChID = tChannelID( cSource::FromString( string3 ), int1, int2, int3, int4 );
		  //cChannel *EquivChannel = Channels.GetByChannelID( EquivChID, false );
		  cChannel *EquivChannel = NULL; //Channels.GetByChannelID( EquivChID, false );
		  if( EquivChannel )
		  {
	            ( lEquivChannels + nEquivChannels )->OriginalSourceId = OriginalChannel->Source();
	            ( lEquivChannels + nEquivChannels )->OriginalNid = OriginalChannel->Nid();
	            ( lEquivChannels + nEquivChannels )->OriginalTid = OriginalChannel->Tid();
	            ( lEquivChannels + nEquivChannels )->OriginalSid = OriginalChannel->Sid();
	            ( lEquivChannels + nEquivChannels )->OriginalRid = OriginalChannel->Rid();
	            ( lEquivChannels + nEquivChannels )->EquivSourceId = EquivChannel->Source();
		    ( lEquivChannels + nEquivChannels )->EquivNid = EquivChannel->Nid();
		    ( lEquivChannels + nEquivChannels )->EquivTid = EquivChannel->Tid();
		    ( lEquivChannels + nEquivChannels )->EquivSid = EquivChannel->Sid();
		    ( lEquivChannels + nEquivChannels )->EquivRid = EquivChannel->Rid();
		    nEquivChannels ++;
		  }
		  else
		  {
		    esyslog( "LoadEPG: warning, not found equivalent channel \'%s\' in channels.conf", string2 );
		  }
		}
	      }
	      else
	      {
	        esyslog( "LoadEPG: warning, not found epg channel \'%s\' in channels.conf", string1 );
	      }
	    }
	  }
	}
      }
    }
    fclose( File );
  }
  free( FileName );
}
// }}}

// Sort Functions {{{
static int qsortEquivChannels( const void *A, const void *B )
{
  sEquivChannel *ChannelA = ( sEquivChannel * ) A;
  sEquivChannel *ChannelB = ( sEquivChannel * ) B;
  if( ChannelA->OriginalSourceId > ChannelB->OriginalSourceId )
  {
    return 1;
  }
  if( ChannelA->OriginalSourceId < ChannelB->OriginalSourceId )
  {
    return -1;
  }
  if( ChannelA->OriginalSourceId == ChannelB->OriginalSourceId )
  {
    if( ChannelA->OriginalNid > ChannelB->OriginalNid )
    {
      return 1;
    }
    if( ChannelA->OriginalNid < ChannelB->OriginalNid )
    {
      return -1;
    }
    if( ChannelA->OriginalNid == ChannelB->OriginalNid )
    {
      if( ChannelA->OriginalTid > ChannelB->OriginalTid )
      {
        return 1;
      }
      if( ChannelA->OriginalTid < ChannelB->OriginalTid )
      {
        return -1;
      }
      if( ChannelA->OriginalTid == ChannelB->OriginalTid )
      {
        if( ChannelA->OriginalSid > ChannelB->OriginalSid )
	{
	  return 1;
	}
        if( ChannelA->OriginalSid < ChannelB->OriginalSid )
	{
	  return -1;
	}
      }
    }
  }
  return 0;
}

static int bsearchEquivChannel( const void *A, const void *B )
{
  sEquivChannel *ChannelA = ( sEquivChannel * ) A;
  sEquivChannel *ChannelB = ( sEquivChannel * ) B;
  if( ChannelA->OriginalSourceId > ChannelB->OriginalSourceId )
  {
    return 1;
  }
  if( ChannelA->OriginalSourceId < ChannelB->OriginalSourceId )
  {
    return -1;
  }
  if( ChannelA->OriginalSourceId == ChannelB->OriginalSourceId )
  {
    if( ChannelA->OriginalNid > ChannelB->OriginalNid )
    {
      return 1;
    }
    if( ChannelA->OriginalNid < ChannelB->OriginalNid )
    {
      return -1;
    }
    if( ChannelA->OriginalNid == ChannelB->OriginalNid )
    {
      if( ChannelA->OriginalTid > ChannelB->OriginalTid )
      {
        return 1;
      }
      if( ChannelA->OriginalTid < ChannelB->OriginalTid )
      {
        return -1;
      }
      if( ChannelA->OriginalTid == ChannelB->OriginalTid )
      {
        if( ChannelA->OriginalSid > ChannelB->OriginalSid )
	{
	  return 1;
	}
        if( ChannelA->OriginalSid < ChannelB->OriginalSid )
	{
	  return -1;
	}
      }
    }
  }
  return 0;
}

static int qsortChannels( const void *A, const void *B )
{
  sChannel *ChannelA = ( sChannel * ) A;
  sChannel *ChannelB = ( sChannel * ) B;
  if( ChannelA->ChannelId > ChannelB->ChannelId )
  {
    return 1;
  }
  if( ChannelA->ChannelId < ChannelB->ChannelId )
  {
    return -1;
  }
  if( ChannelA->ChannelId == ChannelB->ChannelId )
  {
    if( ChannelA->Nid > ChannelB->Nid )
    {
      return 1;
    }
    if( ChannelA->Nid < ChannelB->Nid )
    {
      return -1;
    }
    if( ChannelA->Nid == ChannelB->Nid )
    {
      if( ChannelA->Tid > ChannelB->Tid )
      {
        return 1;
      }
      if( ChannelA->Tid < ChannelB->Tid )
      {
        return -1;
      }
      if( ChannelA->Tid == ChannelB->Tid )
      {
        if( ChannelA->Sid > ChannelB->Sid )
	{
	  return 1;
	}
	if( ChannelA->Sid < ChannelB->Sid )
	{
	  return -1;
	}
      }
    }
  }
  return 0;
}

static int qsortChannelsBySkyNumber( const void *A, const void *B )
{
  sChannel *ChannelA = ( sChannel * ) A;
  sChannel *ChannelB = ( sChannel * ) B;
  if( ChannelA->SkyNumber > ChannelB->SkyNumber )
    return 1;
  if( ChannelA->SkyNumber < ChannelB->SkyNumber )
    return -1;
  // must be == so no test needed
  if( ChannelA->ChannelId > ChannelB->ChannelId )
  {
    return 1;
  }
  if( ChannelA->ChannelId < ChannelB->ChannelId )
  {
    return -1;
  }
  if( ChannelA->ChannelId == ChannelB->ChannelId )
  {
    if( ChannelA->Nid > ChannelB->Nid )
    {
      return 1;
    }
    if( ChannelA->Nid < ChannelB->Nid )
    {
      return -1;
    }
    if( ChannelA->Nid == ChannelB->Nid )
    {
      if( ChannelA->Tid > ChannelB->Tid )
      {
        return 1;
      }
      if( ChannelA->Tid < ChannelB->Tid )
      {
        return -1;
      }
      if( ChannelA->Tid == ChannelB->Tid )
      {
        if( ChannelA->Sid > ChannelB->Sid )
	{
	  return 1;
	}
	if( ChannelA->Sid < ChannelB->Sid )
	{
	  return -1;
	}
      }
    }
  }
  return 0;
}

static int qsortChannelsByChID( const void *A, const void *B )
{
  sChannel *ChannelA = ( sChannel * ) A;
  sChannel *ChannelB = ( sChannel * ) B;
  if( ChannelA->Nid > ChannelB->Nid )
  {
    return 1;
  }
  if( ChannelA->Nid < ChannelB->Nid )
  {
    return -1;
  }
  if( ChannelA->Nid == ChannelB->Nid )
  {
    if( ChannelA->Tid > ChannelB->Tid )
    {
      return 1;
    }
    if( ChannelA->Tid < ChannelB->Tid )
    {
      return -1;
    }
    if( ChannelA->Tid == ChannelB->Tid )
    {
      if( ChannelA->Sid > ChannelB->Sid )
      {
        return 1;
      }
      if( ChannelA->Sid < ChannelB->Sid )
      {
        return -1;
      }
      if( ChannelA->Sid == ChannelB->Sid )
      {
        if( ChannelA->ChannelId > ChannelB->ChannelId )
	{
	  return 1;
	}
        if( ChannelA->ChannelId < ChannelB->ChannelId )
	{
	  return -1;
	}
      }
    }
  }
  return 0;
}

static int bsearchChannelByChannelId( const void *A, const void *B )
{
  sChannel *ChannelA = ( sChannel * ) A;
  sChannel *ChannelB = ( sChannel * ) B;
  if( ChannelA->ChannelId > ChannelB->ChannelId )
  {
    return 1;
  }
  if( ChannelA->ChannelId < ChannelB->ChannelId )
  {
    return -1;
  }
  return 0;
}

static int bsearchChannelByChID( const void *A, const void *B )
{
  sChannel *ChannelA = ( sChannel * ) A;
  sChannel *ChannelB = ( sChannel * ) B;
  if( ChannelA->Nid > ChannelB->Nid )
  {
    return 1;
  }
  if( ChannelA->Nid < ChannelB->Nid )
  {
    return -1;
  }
  if( ChannelA->Nid == ChannelB->Nid )
  {
    if( ChannelA->Tid > ChannelB->Tid )
    {
      return 1;
    }
    if( ChannelA->Tid < ChannelB->Tid )
    {
      return -1;
    }
    if( ChannelA->Tid == ChannelB->Tid )
    {
      if( ChannelA->Sid > ChannelB->Sid )
      {
        return 1;
      }
      if( ChannelA->Sid < ChannelB->Sid )
      {
        return -1;
      }
      if( ChannelA->Sid == ChannelB->Sid )
      {
        if( ChannelA->ChannelId > ChannelB->ChannelId )
	{
	  return 1;
	}
        if( ChannelA->ChannelId < ChannelB->ChannelId )
	{
	  return -1;
	}
      }
    }
  }
  return 0;
}

static int bsearchChannelBySid( const void *A, const void *B )
{
  sChannel *ChannelA = ( sChannel * ) A;
  sChannel *ChannelB = ( sChannel * ) B;
  if( ChannelA->Nid > ChannelB->Nid )
  {
    return 1;
  }
  if( ChannelA->Nid < ChannelB->Nid )
  {
    return -1;
  }
  if( ChannelA->Nid == ChannelB->Nid )
  {
    if( ChannelA->Tid > ChannelB->Tid )
    {
      return 1;
    }
    if( ChannelA->Tid < ChannelB->Tid )
    {
      return -1;
    }
    if( ChannelA->Tid == ChannelB->Tid )
    {
      if( ChannelA->Sid > ChannelB->Sid )
      {
        return 1;
      }
      if( ChannelA->Sid < ChannelB->Sid )
      {
        return -1;
      }
    }
  }
  return 0;
}

static int qsortTitles( const void *A, const void *B )
{
  sTitle *TitleA = ( sTitle * ) A;
  sTitle *TitleB = ( sTitle * ) B;
  if( TitleA->ChannelId > TitleB->ChannelId )
  {
    return 1;
  }
  if( TitleA->ChannelId < TitleB->ChannelId )
  {
    return -1;
  }
  if( TitleA->ChannelId == TitleB->ChannelId )
  {
    if( TitleA->StartTime > TitleB->StartTime )
    {
      return 1;
    }
    if( TitleA->StartTime < TitleB->StartTime )
    {
      return -1;
    }
  }
  return 0;
}

static int qsortSummaries( const void *A, const void *B )
{
  sSummary *SummarieA = ( sSummary * ) A;
  sSummary *SummarieB = ( sSummary * ) B;
  if( SummarieA->ChannelId > SummarieB->ChannelId )
  {
    return 1;
  }
  if( SummarieA->ChannelId < SummarieB->ChannelId )
  {
    return -1;
  }
  if( SummarieA->ChannelId == SummarieB->ChannelId )
  {
    if( SummarieA->MjdTime > SummarieB->MjdTime )
    {
      return 1;
    }
    if( SummarieA->MjdTime < SummarieB->MjdTime )
    {
      return -1;
    }
    if( SummarieA->MjdTime == SummarieB->MjdTime )
    {
      if( SummarieA->EventId > SummarieB->EventId )
      {
        return 1;
      }
      if( SummarieA->EventId < SummarieB->EventId )
      {
        return -1;
      }
    }
  }
  return 0;
}

static int bsearchSummarie( const void *A, const void *B )
{
  sSummary *SummarieA = ( sSummary * ) A;
  sSummary *SummarieB = ( sSummary * ) B;
  if( SummarieA->ChannelId > SummarieB->ChannelId )
  {
    return 1;
  }
  if( SummarieA->ChannelId < SummarieB->ChannelId )
  {
    return -1;
  }
  if( SummarieA->ChannelId == SummarieB->ChannelId )
  {
    if( SummarieA->MjdTime > SummarieB->MjdTime )
    {
      return 1;
    }
    if( SummarieA->MjdTime < SummarieB->MjdTime )
    {
      return -1;
    }
    if( SummarieA->MjdTime == SummarieB->MjdTime )
    {
      if( SummarieA->EventId > SummarieB->EventId )
      {
        return 1;
      }
      if( SummarieA->EventId < SummarieB->EventId )
      {
        return -1;
      }
    }
  }
  return 0;
}
// }}}

// cTaskLoadepg {{{
// cTaskLoadepg Construction {{{
cTaskLoadepg::cTaskLoadepg( void )
:cThread( "cTaskLoadepg" )
{
  SetupUpdateChannels = Setup.UpdateChannels;
}

cTaskLoadepg::~cTaskLoadepg()
{
  Setup.UpdateChannels = SetupUpdateChannels;
  if( EpgDevice )
  {
    if( HasSwitched )
    {
      EpgDevice->SwitchChannel( VdrChannel, true );
      HasSwitched = false;
    }
  }
  EpgDevice = NULL;
  for( int i = 0; i < nActiveFilters; i ++ )
  {
    StopFilter( i );
  }
  if( lThemes )
  {
    free( lThemes );
    lThemes = NULL;
  }
  if( lChannels )
  {
    free( lChannels );
    lChannels = NULL;
  }
  if( lBouquets )
  {
    free( lBouquets );
    lBouquets = NULL;
  }
  if( lTitles )
  {
    free( lTitles );
    lTitles = NULL;
  }
  if( lSummaries )
  {
    free( lSummaries );
    lSummaries = NULL;
  }
  if( bChannels )
  {
    free( bChannels );
    bChannels = NULL;
  }
  if( bTitles )
  {
    free( bTitles );
    bTitles = NULL;
  }
  if( bSummaries )
  {
    free( bSummaries );
    bSummaries = NULL;
  }
  Cancel( 2 );
}
// }}}

// cTaskLoadepg Thread {{{
void cTaskLoadepg::Action( void )
{
  int Frequency;
  char Polarization;
  char SourceName[32];
  int SymbolRate;
  int TimeoutRotor;
  
  SetupUpdateChannels = Setup.UpdateChannels;
  IsError = false;
  VdrChannel = NULL;
  EpgChannel = NULL;
  HasSwitched = false;
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
  if( Running() )
  {
    if( DEBUG )
    {
      fprintf( stderr, "LoadEPG: Start task\n" );
    }
    esyslog( "LoadEPG: Start task" );
    switch( ( lProviders + CurrentProvider )->DataFormat )
    {
      case DATA_FORMAT_SKYBOX:
      case DATA_FORMAT_MHW_1:
      case DATA_FORMAT_MHW_2:
        SetupUpdateChannels = Setup.UpdateChannels;
        Setup.UpdateChannels = 0;
        lThemes = ( sTheme * ) calloc( MAX_THEMES, sizeof( sTheme ) );
	if( ! lThemes )
	{
	  esyslog( "LoadEPG: Error, failed to allocate memory for lThemes" );
	  goto endrunning;
	}
	lChannels = ( sChannel * ) calloc( MAX_CHANNELS, sizeof( sChannel ) );
        if( ! lChannels )
        {
          esyslog( "LoadEPG: Error, failed to allocate memory for lChannels" );
          goto endrunning;
        }
	lBouquets = ( sBouquet * ) calloc( MAX_BOUQUETS, sizeof( sBouquet ) );
        if( ! lBouquets )
        {
          esyslog( "LoadEPG: Error, failed to allocate memory for lBouquets" );
          goto endrunning;
        }
        lTitles = ( sTitle * ) calloc( MAX_TITLES, sizeof( sTitle ) );
        if( ! lTitles )
        {
          esyslog( "LoadEPG: Error, failed to allocate memory for lTitles" );
          goto endrunning;
        }
        lSummaries = ( sSummary * ) calloc( MAX_SUMMARIES, sizeof( sSummary ) );
        if( ! lSummaries )
        {
          esyslog( "LoadEPG: Error, failed to allocate memory for lSummaries" );
          goto endrunning;
        }
	bChannels = ( unsigned char * ) calloc( MAX_BUFFER_SIZE_CHANNELS, sizeof( unsigned char ) );
	if( ! bChannels )
	{
	  esyslog( "LoadEPG: Error, failed to allocate memory for bChannels" );
	  goto endrunning;
	}
        bTitles = ( unsigned char * ) calloc( MAX_BUFFER_SIZE_TITLES, sizeof( unsigned char ) );
        if( ! bTitles )
        {
          esyslog( "LoadEPG: Error, failed to allocate memory for bTitles" );
          goto endrunning;
        }
        bSummaries = ( unsigned char * ) calloc( MAX_BUFFER_SIZE_SUMMARIES, sizeof( unsigned char ) );
        if( ! bSummaries )
        {
          esyslog( "LoadEPG: Error, failed to allocate memory for bSummaries" );
          goto endrunning;
        }
	if( sscanf( ( lProviders + CurrentProvider )->Parm1, "%i :%c :%[^:]:%i", &Frequency, &Polarization, SourceName, &SymbolRate ) == 4 )
	{
	  int DeviceID = cDevice::NumDevices() - 1;
	  DeviceID = 0;
	  while( DeviceID >= 0 )
	  {
	    DvbAdapterNumber = -1;
	    HasSwitched = false;
	    //EpgDevice = cDevice::GetDevice( DeviceID );
	    //if( EpgDevice )
	    {
#if 0
	      if( ! EpgDevice->ProvidesSource( cSource::FromString( SourceName ) ) )
	      {
	        // virtual device (xine,drx3,...)
	        if( EpgDevice->IsPrimaryDevice() )
	        {
	          HasSwitched = true;
		  for( int i = 0; i < DeviceID; i ++ )
		  {
		    if( cDevice::GetDevice( i )->ProvidesSource( cSource::FromString( SourceName ) ) )
		    {
		      if( ! cDevice::GetDevice( i )->Receiving( true ) )
		      {
		        DvbAdapterNumber = cDevice::GetDevice( i )->CardIndex();
		      }
		    }
		  }
		  if( DvbAdapterNumber == -1 )
		  {
	            for( int i = 0; i < DeviceID; i ++ )
		    {
		      if( cDevice::GetDevice( i )->ProvidesSource( cSource::FromString( SourceName ) ) )
		      {
		        if( ! cDevice::GetDevice( i )->Receiving() )
		        {
		          DvbAdapterNumber = cDevice::GetDevice( i )->CardIndex();
		        }
		      }
		    }
		  }
	        }
	      }
	      else
	      {
	        // card ss1 or ss2
		if( EpgDevice->IsPrimaryDevice() || EpgDevice->ActualDevice() )
		{
		  HasSwitched = true;
		}
		if( ! cDevice::GetDevice( DeviceID )->Receiving( true ) )
		{
		  DvbAdapterNumber = cDevice::GetDevice( DeviceID )->CardIndex();
		}
		if( DvbAdapterNumber == -1 )
		{
		  if( ! cDevice::GetDevice( DeviceID )->Receiving() )
		  {
		    DvbAdapterNumber = cDevice::GetDevice( DeviceID )->CardIndex();
		  }
		}
	      }
#endif
	      DvbAdapterNumber = adapter;
	      if( DvbAdapterNumber != -1 )
	      {
#if 1
	        if( Config->DvbAdapterNumber == 0 || ( Config->DvbAdapterNumber - 1 ) == DvbAdapterNumber )
		{
	          VdrChannel = Channels.GetByNumber( EpgDevice->CurrentChannel() );
	          EpgChannel = new cChannel();
#ifndef STANDALONE
	          *EpgChannel = *VdrChannel;
#endif
#if APIVERSNUM >= 10700
                  EpgChannel->cChannel::SetSatTransponderData( cSource::FromString( SourceName ), Frequency, Polarization, SymbolRate, DVBFE_FEC_AUTO, DVBFE_MOD_AUTO, DVBFE_DELSYS_DVBS, DVBFE_ROLLOFF_UNKNOWN );
#elif APIVERSNUM == 10514	      
                  EpgChannel->cChannel::SetSatTransponderData( cSource::FromString( SourceName ), Frequency, Polarization, SymbolRate, DVBFE_FEC_AUTO, DVBFE_MOD_AUTO, DVBFE_DELSYS_DVBS, DVBFE_ROLLOFF_UNKNOWN );
#else
                  EpgChannel->cChannel::SetSatTransponderData( cSource::FromString( SourceName ), Frequency, Polarization, SymbolRate, FEC_AUTO );
#endif
                  EpgChannel->SetId( 4096, 4095, 4094, 0 );
		  int CaIds[MAXCAIDS + 1] = { 0 };
		  EpgChannel->SetCaIds( CaIds );
                  
                  EpgDevice->SwitchChannel( EpgChannel, HasSwitched );
		  TimeoutRotor = time( NULL ) + TIMEOUT_ROTOR;
		  TunningTransponder:;
	          cCondWait::SleepMs( 2000 );
	          //if( EpgDevice->HasLock() )
	          if( 1 )
	          {
		    esyslog( "LoadEPG: tuned transponder with adapter number=%i\n", DvbAdapterNumber );
		    LoadFromSatellite();
#if 0
		    if( HasSwitched )
		    {
		      EpgDevice->SwitchChannel( VdrChannel, true );
		      HasSwitched = false;
		    }
#endif
		    if( IsError )
		    {
		      if( ( Config->DvbAdapterNumber - 1 ) == DvbAdapterNumber )
		      {
		        break;
		      }
		    }
		    else
		    {
		      break;
		    }
		  }
		  else
		  {
		    if( ( Config->DvbAdapterHasRotor - 1 ) == DvbAdapterNumber )
		    {
		      if( time( NULL ) < TimeoutRotor )
		      {
		        goto TunningTransponder;
		      }
		    }
		    if( HasSwitched )
		    {
		      EpgDevice->SwitchChannel( VdrChannel, true );
		      HasSwitched = false;
		    }
		    IsError = true;
		    if( ( Config->DvbAdapterNumber - 1 ) == DvbAdapterNumber )
		    {
		      break;
		    }
		  }
		  EpgChannel = NULL;
		}
#endif
	      }
	    }
	    DeviceID --;
	  }
	  if( DvbAdapterNumber == -1 )
	  {
	    esyslog( "LoadEPG: Error, none of the devices provides this source %s", SourceName );
	    IsError = true;
	    break;
	  }
	}
	if( nChannels == 0 || nTitles == 0 )
	{
	  IsError = true;
	}
	Setup.UpdateChannels = SetupUpdateChannels;
        break;
#ifndef STANDALONE
      case DATA_FORMAT_FILE:
        LoadFromFile( ( lProviders + CurrentProvider )->Parm1 );
	cCondWait::SleepMs( 5000 );
        break;
      case DATA_FORMAT_SCRIPT:
        LoadFromScript( ( lProviders + CurrentProvider )->Parm1, ( lProviders + CurrentProvider )->Parm2 );
	cCondWait::SleepMs( 5000 );
        break;
#endif
      default:
        break;
    }
    endrunning:;
    if( lThemes )
    {
      free( lThemes );
      lThemes = NULL;
    }
    if( lChannels )
    {
      free( lChannels );
      lChannels = NULL;
    }
    if( lBouquets )
    {
      free( lBouquets );
      lBouquets = NULL;
    }
    if( lTitles )
    {
      free( lTitles );
      lTitles = NULL;
    }
    if( lSummaries )
    {
      free( lSummaries );
      lSummaries = NULL;
    }
    if( bChannels )
    {
      free( bChannels );
      bChannels = NULL;
    }
    if( bTitles )
    {
      free( bTitles );
      bTitles = NULL;
    }
    if( bSummaries )
    {
      free( bSummaries );
      bSummaries = NULL;
    }
    esyslog( "LoadEPG: End task" );
    if( DEBUG )
    {
      fprintf( stderr, "LoadEPG: End task\n" );
    }
    if( EpgDevice )
    {
      if( HasSwitched )
      {
        EpgDevice->SwitchChannel( VdrChannel, true );
        HasSwitched = false;
      }
    }
  }
#ifndef STANDALONE
  Control->Error = IsError;
#endif
  Setup.UpdateChannels = SetupUpdateChannels;
}
// }}}

// cTaskLoadepg Huffman decode {{{
int cTaskLoadepg::DecodeHuffmanCode( unsigned char *Data, int Length )
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
  for( i = 0; i < Length; i ++ )
  {
    Byte = Data[i];
    Mask = 0x80;
    if( i == 0 )
    {
      Mask = 0x20;
      lastByte = i;
      lastMask = Mask;
    }
    loop1:;
    if( IsFound )
    {
      lastByte = i;
      lastMask = Mask;
      IsFound = false;
    }
    if( ( Byte & Mask ) == 0 )
    {
      if( CodeError )
      {
        DecodeErrorText[q] = 0x30;
	q ++;
	goto nextloop1;
      }
      if( nH->P0 != NULL )
      {
        nH = nH->P0;
	if( nH->Value != NULL )
	{
	  memcpy( &DecodeText[p], nH->Value, strlen( nH->Value ) );
	  p += strlen( nH->Value );
	  nH = &H;
	  IsFound = true;
	}
      }
      else
      {
	memcpy( &DecodeText[p], "<...?...>", 9 );
	p += 9;
	i = lastByte;
	Byte = Data[lastByte];
	Mask = lastMask;
	CodeError = true;
        goto loop1;
      }
    }
    else
    {
      if( CodeError )
      {
        DecodeErrorText[q] = 0x31;
	q ++;
	goto nextloop1;
      }
      if( nH->P1 != NULL )
      {
        nH = nH->P1;
	if( nH->Value != NULL )
	{
	  memcpy( &DecodeText[p], nH->Value, strlen( nH->Value ) );
	  p += strlen( nH->Value );
	  nH = &H;
	  IsFound = true;
	}
      }
      else
      {
	memcpy( &DecodeText[p], "<...?...>", 9 );
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
    if( Mask > 0 )
    {
      goto loop1;
    }
  }
  DecodeText[p] = '\0';
  DecodeErrorText[q] = '\0';
  return p;
}
// }}}

// cTaskLoadepg::ReadFileDictionary {{{
bool cTaskLoadepg::ReadFileDictionary( void )
{
  char *FileName;
  FILE *FileDict;
  char *Line;
  char Buffer[256];
  asprintf( &FileName, "%s/%s", Config->Directory, ( lProviders + CurrentProvider )->Parm2 );
  FileDict = fopen( FileName, "r" );
  if( FileDict == NULL )
  {
    esyslog( "LoadEPG: Error opening file '%s'. %s", FileName, strerror( errno ) );
    free( FileName );
    return false;
  }
  else
  {
    int i;
    int LenPrefix;
    char string1[256];
    char string2[256];
    H.Value = NULL;
    H.P0 = NULL;
    H.P1 = NULL;
    while( ( Line = fgets( Buffer, sizeof( Buffer ), FileDict ) ) != NULL )
    {
      if( ! isempty( Line ) )
      {
        memset( string1, 0, sizeof( string1 ) );
	memset( string2, 0, sizeof( string2 ) );
	if( sscanf( Line, "%c=%[^\n]\n", string1, string2 ) == 2 )
	{
	  goto codingstart;
	}
        else if( sscanf( Line, "%[^=]=%[^\n]\n", string1, string2 ) == 2 )
	{
	  codingstart:;
	  nH = &H;
	  LenPrefix = strlen( string2 );
	  for( i = 0; i < LenPrefix; i ++ )
	  {
	    switch( string2[i] )
	    {
	      case '0':
	        if( nH->P0 == NULL )
		{
		  nH->P0 = new sNodeH();
		  nH = nH->P0;
		  nH->Value = NULL;
		  nH->P0 = NULL;
		  nH->P1 = NULL;
		  if( ( LenPrefix - 1 ) == i )
		  {
		    asprintf( &nH->Value, "%s", string1 );
		  }
		}
		else
		{
		  nH = nH->P0;
		  if( nH->Value != NULL || ( LenPrefix - 1 ) == i )
		  {
		    esyslog( "LoadEPG: Error, huffman prefix code already exists for \"%s\"=%s with '%s'", string1, string2, nH->Value );
		  }
		}
	        break;
	      case '1':
	        if( nH->P1 == NULL )
		{
		  nH->P1 = new sNodeH();
		  nH = nH->P1;
		  nH->Value = NULL;
		  nH->P0 = NULL;
		  nH->P1 = NULL;
		  if( ( LenPrefix - 1 ) == i )
		  {
		    asprintf( &nH->Value, "%s", string1 );
		  }
		}
		else
		{
		  nH = nH->P1;
		  if( nH->Value != NULL || ( LenPrefix - 1 ) == i )
		  {
		    esyslog( "LoadEPG: Error, huffman prefix code already exists for \"%s\"=%s with '%s'", string1, string2, nH->Value );
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
    fclose( FileDict );
  }
  
  // check tree huffman nodes
  FileDict = fopen( FileName, "r" );
  if( FileDict )
  {
    int i;
    int LenPrefix;
    char string1[256];
    char string2[256];
    while( ( Line = fgets( Buffer, sizeof( Buffer ), FileDict ) ) != NULL )
    {
      if( ! isempty( Line ) )
      {
        memset( string1, 0, sizeof( string1 ) );
	memset( string2, 0, sizeof( string2 ) );
	if( sscanf( Line, "%c=%[^\n]\n", string1, string2 ) == 2 )
	{
	  goto verifystart;
	}
        else if( sscanf( Line, "%[^=]=%[^\n]\n", string1, string2 ) == 2 )
	{
	  verifystart:;
	  nH = &H;
	  LenPrefix = strlen( string2 );
	  for( i = 0; i < LenPrefix; i ++ )
	  {
	    switch( string2[i] )
	    {
	      case '0':
	        if( nH->P0 != NULL )
		{
		  nH = nH->P0;
		}
	        break;
	      case '1':
	        if( nH->P1 != NULL )
		{
		  nH = nH->P1;
		}
	        break;
	      default:
	        break;
	    }
	  }
	  if( nH->Value != NULL )
	  {
	    if( memcmp( nH->Value, string1, strlen( nH->Value ) ) != 0 )
	    {
	      esyslog( "LoadEPG: Error, huffman prefix value '%s' not equal to '%s'", nH->Value, string1 );
	    }
	  }
	  else
	  {
	    esyslog( "LoadEPG: Error, huffman prefix value is not exists for \"%s\"=%s", string1, string2 );
	  }
        }
      }
    }
    fclose( FileDict );
  }
  free( FileName );
  return true;
}
// }}}

// cTaskLoadepg::ReadFileThemes {{{
bool cTaskLoadepg::ReadFileThemes( void )
{
  char *FileName;
  FILE *FileThemes;
  char *Line;
  char Buffer[256];
  asprintf( &FileName, "%s/%s", Config->Directory, ( lProviders + CurrentProvider )->Parm3 );
  FileThemes = fopen( FileName, "r" );
  if( FileThemes == NULL )
  {
    esyslog( "LoadEPG: Error opening file '%s'. %s", FileName, strerror( errno ) );
    free( FileName );
    return false;
  }
  else
  {
    int id = 0;
    char string1[256];
    char string2[256];
    while( ( Line = fgets( Buffer, sizeof( Buffer ), FileThemes ) ) != NULL )
    {
      memset( string1, 0, sizeof( string1 ) );
      memset( string2, 0, sizeof( string2 ) );
      if( ! isempty( Line ) )
      {
	sTheme *T = ( lThemes + id );
        if( sscanf( Line, "%[^=] =%[^\n] ", string1, string2 ) == 2 )
	{
	  snprintf( ( char * ) T->Name, 255, "%s", string2 );
	}
	else
	{
	  T->Name[0] = '\0';
	}
	id ++;
      }
    }
    fclose( FileThemes );
  }
  free( FileName );
  return true;
}
// }}}

// cTaskLoadepg::LoadFromSatellite {{{
void cTaskLoadepg::LoadFromSatellite( void )
{
  nFilters = 0;
  GetLocalTimeOffset();
  AddFilter( 0x14, 0x70, 0xfc ); // TOT && TDT
  switch( ( lProviders + CurrentProvider )->DataFormat )
  {
    case DATA_FORMAT_SKYBOX:
      AddFilter( 0x11, 0x4a );
      AddFilter( 0x11, 0x46 );
      AddFilter( 0x30, 0xa0, 0xfc );
      AddFilter( 0x31, 0xa0, 0xfc );
      AddFilter( 0x32, 0xa0, 0xfc );
      AddFilter( 0x33, 0xa0, 0xfc );
      AddFilter( 0x34, 0xa0, 0xfc );
      AddFilter( 0x35, 0xa0, 0xfc );
      AddFilter( 0x36, 0xa0, 0xfc );
      AddFilter( 0x37, 0xa0, 0xfc );
      AddFilter( 0x40, 0xa8, 0xfc );
      AddFilter( 0x41, 0xa8, 0xfc );
      AddFilter( 0x42, 0xa8, 0xfc );
      AddFilter( 0x43, 0xa8, 0xfc );
      AddFilter( 0x44, 0xa8, 0xfc );
      AddFilter( 0x45, 0xa8, 0xfc );
      AddFilter( 0x46, 0xa8, 0xfc );
      AddFilter( 0x47, 0xa8, 0xfc );
      PollingFilters( 2000 );
      break;
    case DATA_FORMAT_MHW_1:
      AddFilter( 0xd2, 0x90 );
      AddFilter( 0xd3, 0x90 );
      AddFilter( 0xd3, 0x91 );
      AddFilter( 0xd3, 0x92 );
      PollingFilters( 3000 );
      break;
    case DATA_FORMAT_MHW_2:
      AddFilter( 0x231, 0xc8 );
      AddFilter( 0x234, 0xe6 );
      AddFilter( 0x236, 0x96 );
      PollingFilters( 3000 );
      break;
    default:
      IsError = true;
      break;
  }
  if( ! IsError )
  {
    if (vdrmode)
      CreateEpgDataFile();
    else
      CreateEpgXml();
  }
}
// }}}


char * get_channelident( sChannel *C)
{
  char *s;
  char *t;

  if (C == NULL)
    asprintf( &s, "undefined");
  else if (C->shortname != NULL && useshortxmlids)
    asprintf( &s, "%d.%s.%s.dvb.guide", C->Sid, C->shortname, C->providername);
  else {
    asprintf( &t, "%d.%d", C->Sid, C->SkyNumber);
    asprintf( &s, "%s", skyxmltvid(t, C->providername));
  }
  return s;
}

// cTaskLoadepg::CreateXmlChannels {{{
void cTaskLoadepg::CreateXmlChannels( )
{
  char *ServiceName;
  qsort( lChannels, nChannels, sizeof( sChannel ), &qsortChannelsBySkyNumber );
  for( int i = 0; i < nChannels; i ++ )
  {
    sChannel *C = ( lChannels + i );
    if( C->Nid > 0 && C->Tid > 0 && C->Sid > 0 )
    {
      tChannelID ChVID = tChannelID( ( lProviders + CurrentProvider )->SourceId, C->Nid, C->Tid, C->Sid );
      if (C->providername == NULL)
	continue;
      if (strcmp(C->providername,"(null)")==0)
	continue;
      if (!C->IsEpg)
	continue;
      if (!C->IsFound)
	continue;
      if (C->name)
      {
	//asprintf( &ServiceName, "%s - %s", C->name, C->shortname );
	asprintf( &ServiceName, "%s", C->name );
      }
      else
      {
	//asprintf( &ServiceName, " " );
	continue;
      }
      char * channelid = get_channelident(C);
      if (channelid != NULL)
      {
	printf("<channel id=\"%s\">\n", channelid);
	printf("\t<display-name>%s</display-name>\n", ServiceName); //xmlify(buf));
	printf("</channel>\n");
	free(channelid);
      }
      if( ServiceName )
      {
	free( ServiceName );
	ServiceName = NULL;
      }
    }
  }
}
// }}}

// cTaskLoadepg::CreateFileChannels {{{
void cTaskLoadepg::CreateFileChannels( const char *FileChannels )
{
  FILE *File;
  char *ChID;
  char *ServiceName;
  File = fopen( FileChannels, "w" );
  if( File )
  {
    fprintf( File, "|  ID  | %-26.26s | %-22.22s | EPG | %-8.8s |\n", "Channel ID", "Channel Name", "Sky Num." );
    fprintf( File, "|------|-%-26.26s-|-%-22.22s-|-----|-%-8.8s-|\n", "------------------------------", "-----------------------------", "--------------------" );
    for( int i = 0; i < nChannels; i ++ )
    {
      sChannel *C = ( lChannels + i );
      if( C->Nid > 0 && C->Tid > 0 && C->Sid > 0 )
      {
	tChannelID ChVID = tChannelID( ( lProviders + CurrentProvider )->SourceId, C->Nid, C->Tid, C->Sid );
#ifdef STANDALONE
	cChannel *VC = NULL;
#else
	cChannel *VC = Channels.GetByChannelID( ChVID, true );
#endif
	if( VC )
	{
	  asprintf( &ServiceName, "%s", VC->Name() );
	}
	else if (C->name)
	{
	  asprintf( &ServiceName, "%s - %s", C->name, C->shortname );
	}
	else
	{
	  asprintf( &ServiceName, " " );
	}
        asprintf( &ChID, "%s-%i-%i-%i-0", *cSource::ToString( ( lProviders + CurrentProvider )->SourceId ), C->Nid, C->Tid, C->Sid );
        fprintf( File, "|% 5d | %-26.26s | %-22.22s |", C->ChannelId, ChID, ServiceName );
	if( C->IsEpg )
	{
	  if( C->IsFound )
	  {
	    fprintf( File, " %-3.3s |", "YES" );
	  }
	  else
	  {
	    fprintf( File, " %-3.3s |", "NO" );
	  }
	}
	else
	{
	  fprintf( File, " %-3.3s |", "..." );
	}
        fprintf( File, "  % 6d  |\n", C->SkyNumber );
	if( ChID )
	{
	  free( ChID );
	  ChID = NULL;
	}
	if( ServiceName )
	{
	  free( ServiceName );
	  ServiceName = NULL;
	}
      }
    }
    fprintf( File, "|------|-%-26.26s-|-%-22.22s-|-----|-%-8.8s-|\n", "------------------------------", "-----------------------------", "--------------------" );
    fclose( File );
  }
}
// }}}

#ifndef STANDALONE
// cTaskLoadepg::LoadFromFile and cTaskLoadepg::LoadFromScript {{{
void cTaskLoadepg::LoadFromFile( const char *FileEpg )
{
  char *FileTmp;
  FILE *File;
  asprintf( &FileTmp, "%s/%s", Config->Directory, FileEpg );
  File = fopen( FileTmp, "r" );
  if( File == NULL )
  {
    free( FileTmp );
    asprintf( &FileTmp, "%s", FileEpg );
    File = fopen( FileTmp, "r" );
    if( File == NULL )
    {
      esyslog( "LoadEPG: Error opening epg data file '%s' from function LoadFromFile(), %s", FileTmp, strerror( errno ) );
      free( FileTmp );
      IsError = true;
      return;
    }
  }
  fclose( File );
#if APIVERSNUM >= 10500
  const char *SysChar = cCharSetConv::SystemCharacterTable();
  if( SysChar == NULL || ! strcasestr( SysChar, "ISO-8859-15" ) )
  {
    char *Cmd;
    asprintf( &Cmd, "iconv -f ISO-8859-15 -t %s -o \"%s.converted\" \"%s\"", SysChar ? SysChar : "UTF-8", FileTmp, FileTmp );
    SystemExec( Cmd );
    free( Cmd );
    asprintf( &Cmd, "mv \"%s.converted\" \"%s\"", FileTmp, FileTmp );
    SystemExec( Cmd );
    free( Cmd );
  }
#endif
  File = fopen( FileTmp, "r" );
  if( cSchedules::Read( File ) )
  {
    cSchedules::Cleanup( true );
  }
  fclose( File );
}

void cTaskLoadepg::LoadFromScript( const char *FileScript, const char *FileEpg )
{
  char *FileTmp;
  FILE *File;
  asprintf( &FileTmp, "%s/%s", Config->Directory, FileScript );
  File = fopen( FileTmp, "r" );
  if( File == NULL )
  {
    free( FileTmp );
    asprintf( &FileTmp, "%s", FileScript );
    File = fopen( FileTmp, "r" );
    if( File == NULL )
    {
      esyslog( "LoadEPG: Error opening script file '%s' from function LoadFromScript(), %s", FileTmp, strerror( errno ) );
      free( FileTmp );
      IsError = true;
      return;
    }
  }
  fclose( File );
  if( system( FileTmp ) == -1 )
  {
    esyslog( "LoadEPG: Error execute script file '%s' from function LoadFromScript()", FileTmp );
    free( FileTmp );
    IsError = true;
    return;
  }
  else
  {
    LoadFromFile( FileEpg );
  }
  free( FileTmp );
}
// }}}
#endif

// cTaskLoadepg Filter Control {{{
void cTaskLoadepg::AddFilter( unsigned short int Pid, unsigned char Tid, unsigned char Mask )
{
  if( nFilters >= MAX_FILTERS )
  {
    esyslog( "LoadEPG: Error, numbers of filters is greater than %i, can't add filter pid=0x%04x tid=0x%02x", MAX_FILTERS, Pid, Tid );
    return;
  }
  Filters[nFilters].Step = 0;
  Filters[nFilters].Pid = Pid;
  Filters[nFilters].Tid = Tid;
  Filters[nFilters].Mask = Mask;
  nFilters ++;
}

void cTaskLoadepg::StartFilter( int FilterId )
{
  dmx_sct_filter_params sctFilterParams;
  memset( &sctFilterParams, 0, sizeof( sctFilterParams ) );
  sctFilterParams.pid = Filters[FilterId].Pid;
  sctFilterParams.timeout = TIMEOUT_FILTER;
  sctFilterParams.flags = DMX_IMMEDIATE_START;
  sctFilterParams.filter.filter[0] = Filters[FilterId].Tid;
  sctFilterParams.filter.mask[0] = Filters[FilterId].Mask;
  if( ioctl( Filters[FilterId].Fd, DMX_SET_FILTER, &sctFilterParams ) >= 0 )
  {
    Filters[FilterId].Step = 1;
  }
  else
  {
    esyslog( "LoadEPG: Error, can't starting filter pid=0x%04x tid=0x%02x", Filters[FilterId].Pid, Filters[FilterId].Tid );
    Filters[FilterId].Step = 3;
  }
}

void cTaskLoadepg::StopFilter( int ActiveFilterId )
{
  if( ActiveFilters[ActiveFilterId].Fd >= 0 )
  {
    if( ioctl( ActiveFilters[ActiveFilterId].Fd, DMX_STOP ) < 0 )
    {
      esyslog( "LoadEPG: Error, ioctl DMX_STOP failed" );
      perror( strerror( errno ) );
    }
    if( close( ActiveFilters[ActiveFilterId].Fd ) == 0 )
    {
      ActiveFilters[ActiveFilterId].Fd = -1;
    }
  }
}
// }}}

// cTaskLoadepg::PollingFilters {{{
void cTaskLoadepg::PollingFilters( int Timeout )
{
  char *FileName;
  int File;
  int Status;
  
  IsError = false;
  IsRunning = true;
  asprintf( &FileName, DVB_DEVICE_DEMUX, DvbAdapterNumber );
  nActiveFilters = 0;
  for( int i = 0; i < nFilters; i ++ )
  {
    if( nActiveFilters < MAX_ACTIVE_FILTERS )
    {
      File = open( FileName, O_RDWR | O_NONBLOCK );
      if( File )
      {
        ActiveFilters[nActiveFilters].Fd = File;
	ActiveFilters[nActiveFilters].FilterId = -1;
	ActiveFilters[nActiveFilters].IsBusy = false;
	nActiveFilters ++;
      }
      else
      {
        esyslog( "LoadEPG: Error, can't open filter handle on '%s'", FileName );
	IsError = true;
      }
    }
    else
    {
      break;
    }
  }
  if( FileName )
  {
    free( FileName );
  }
  for( int i = 0; i < MAX_FILTERS; i ++ )
  {
    memset( &InitialBuffer[i], 0, 32 );
  }
  
  if( ! IsError )
  {
    if( nActiveFilters > 0 )
    {
      while( IsRunning )
      {
        // checking end reading filter
        for( int i = 0; i < nFilters; i ++ )
	{
	  if( Filters[i].Step == 2 )
	  {
	    Filters[i].Step = 3;
	    for( int ii = 0; ii < nActiveFilters; ii ++ )
	    {
	      if( ActiveFilters[ii].FilterId == i )
	      {
	        ActiveFilters[ii].FilterId = -1;
		ActiveFilters[ii].IsBusy = false;
		break;
	      }
	    }
	  }
	}
	
	// preparing active filters
	IsRunning = false;
	for( int i = 0; i < nActiveFilters; i ++ )
	{
	  if( ! ActiveFilters[i].IsBusy )
	  {
	    for( int ii = 0; ii < nFilters; ii ++ )
	    {
	      if( Filters[ii].Step == 0 )
	      {
	        Filters[ii].Fd = ActiveFilters[i].Fd;
	        StartFilter( ii );
		if( Filters[ii].Step == 1 )
		{
		  ActiveFilters[i].FilterId = ii;
		  ActiveFilters[i].IsBusy = true;
		  break;
		}
	      }
	    }
	  }
	  if( ActiveFilters[i].IsBusy )
	  {
	    IsRunning = true;
	  }
	}
	for( int i = 0; i < nActiveFilters; i ++ )
	{
	  PFD[i].fd = ActiveFilters[i].Fd;
	  PFD[i].events = POLLIN;
	  PFD[i].revents = 0;
	}
	
	// exit from polling
	if( ! IsRunning )
	{
	  break;
	}
	
	// running poll filters
	Status = poll( PFD, nActiveFilters, Timeout );
        if( Status > 0 )
        {
	  for( int i = 0; i < nActiveFilters; i ++ )
	  {
	    if( PFD[i].revents & POLLIN )
	    {
	      if( ActiveFilters[i].IsBusy )
	      {
	        ReadBuffer( ActiveFilters[i].FilterId, PFD[i].fd );
	      }
	    }
	    if( IsError )
	    {
	      esyslog( "LoadEPG: Error, unknown" );
	      IsRunning = false;
	    }
	  }
        }
        else if( Status == 0 )
        {
          esyslog( "LoadEPG: Error, timeout polling filter" );
	  IsError = true;
	  IsRunning = false;
        }
        else
        {
	  esyslog( "LoadEPG: Error polling filter" );
	  IsError = true;
	  IsRunning = false;
        }
      }
    }
  }
  
  // clean and stop filters
  for( int i = 0; i < nActiveFilters; i ++ )
  {
    StopFilter( i );
  }
}
// }}}

// cTaskLoadepg::Stop {{{
void cTaskLoadepg::Stop()
{
  esyslog( "LoadEPG: Stop" );
  IsRunning = false;
  Cancel(2);
}
// }}}

// cTaskLoadepg::ReadBuffer {{{
void cTaskLoadepg::ReadBuffer( int FilterId, int Fd )
{
  unsigned char Buffer[2*4096];
  int Bytes;
  Bytes = read( Fd, Buffer, sizeof( Buffer ) );
  if( Bytes < 0 )
  {
    if( errno != EOVERFLOW )
    {
      esyslog( "LoadEPG: Error, failed to read filter for pid=0x%04x tid=0x%02x", Filters[FilterId].Pid, Filters[FilterId].Tid );
      Filters[FilterId].Step = 2;
    }
    else
    {
      esyslog( "LoadEPG: Error, buffer overflow to read filter for pid=0x%04x tid=0x%02x", Filters[FilterId].Pid, Filters[FilterId].Tid );
    }
  }
  else
  {
    if( Bytes > 3 )
    {
      switch( ( lProviders + CurrentProvider )->DataFormat )
      {
        case DATA_FORMAT_SKYBOX:
	  if( ! SI::CRC32::isValid( ( const char * ) Buffer, Bytes ) )
	  {
	    return;
	  }
	  switch( Buffer[0] )
          {
	    case 0x73:
	      GetSatelliteTimeOffset( FilterId, Buffer, Bytes );
	      break;
	    case 0x46:
	      SupplementChannelsSKYBOX( FilterId, Buffer, Bytes - 4 );
	      break;
	    case 0x4a:
	      GetChannelsSKYBOX( FilterId, Buffer, Bytes - 4 );
	      break;
            case 0xa0:
	    case 0xa1:
	    case 0xa2:
	    case 0xa3:
	      GetTitlesSKYBOX( FilterId, Buffer, Bytes - 4 );
	      break;
            case 0xa8:
	    case 0xa9:
	    case 0xaa:
	    case 0xab:
	      GetSummariesSKYBOX( FilterId, Buffer, Bytes - 4 );
	      break;
            default:
	      break;
          }
	  break;
	case DATA_FORMAT_MHW_1:
	  if( Buffer[0] ==  0x73 )
	  {
	    GetSatelliteTimeOffset( FilterId, Buffer, Bytes );
	  }
	  if( Filters[FilterId].Pid == 0xd2 )
	  {
	    switch( Buffer[0] )
	    {
	      case 0x90:
	        GetTitlesMHW1( FilterId, Buffer, Bytes );
	        break;
	      default:
	        break;
	    }
	  }
	  if( Filters[FilterId].Pid == 0xd3 )
	  {
	    switch( Buffer[0] )
	    {
	      case 0x90:
	        GetSummariesMHW1( FilterId, Buffer, Bytes );
	        break;
	      case 0x91:
	        GetChannelsMHW1( FilterId, Buffer, Bytes );
	        break;
	      case 0x92:
	        GetThemesMHW1( FilterId, Buffer, Bytes );
	        break;
	      default:
		break;
	    }
	  }
	  break;
	case DATA_FORMAT_MHW_2:
	  if( Buffer[0] ==  0x73 )
	  {
	    GetSatelliteTimeOffset( FilterId, Buffer, Bytes );
	  }
	  switch( Buffer[0] )
	  {
	    case 0xc8:
	      if( Buffer[3] == 0x00 )
	      {
	        GetChannelsMHW2( FilterId, Buffer, Bytes );
	      }
	      if( Buffer[3] == 0x01 )
	      {
	        GetThemesMHW2( FilterId, Buffer, Bytes );
	      }
	      break;
	    case 0xe6:
	      GetTitlesMHW2( FilterId, Buffer, Bytes );
	      break;
	    case 0x96:
	      GetSummariesMHW2( FilterId, Buffer, Bytes );
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
void cTaskLoadepg::GetLocalTimeOffset( void )
{
  time_t timeLocal;
  time_t timeUtc;
  struct tm *tmCurrent;
  
  timeLocal = time( NULL );
  tmCurrent = gmtime( &timeLocal );
  tmCurrent->tm_isdst = -1;
  timeUtc = mktime( tmCurrent );
  LocalTimeOffset = ( int ) difftime( timeLocal, timeUtc );
  timeLocal -= 86400;
  tmCurrent = gmtime( &timeLocal );
  Yesterday = tmCurrent->tm_wday;
  tmCurrent->tm_hour = 0;
  tmCurrent->tm_min = 0;
  tmCurrent->tm_sec = 0;
  tmCurrent->tm_isdst = -1;
  YesterdayEpoch = mktime( tmCurrent );
  esyslog( "LoadEPG: Local Time Offset=[UTC]%+i", LocalTimeOffset / 3600 );
}

void cTaskLoadepg::GetSatelliteTimeOffset( int FilterId, unsigned char *Data, int Length )
{
  if( Data[0] == 0x73 )
  {
    int satMJD = ( Data[3] << 8 ) | Data[4];
    int satH = BcdToInt( Data[5] );
    int satM = BcdToInt( Data[6] );
    int satS = BcdToInt( Data[7] );
    int DescriptorsLoopLength = ( ( Data[8] & 0x0f ) << 8 ) | Data[9];
    int p1 = 10;
    while( DescriptorsLoopLength > 0 )
    {
      int DescriptorTag = Data[p1];
      int DescriptorLength = Data[p1+1];
      int SatelliteCountryRegionId;
      int SatelliteTimeOffsetPolarity;
      int SatelliteTimeOffsetH;
      int SatelliteTimeOffsetM;
      switch( DescriptorTag )
      {
        case 0x58:
	  unsigned char SatelliteCountryCode[4];
	  for( int i = 0; i < 3; i ++ )
	  {
	    SatelliteCountryCode[i] = Data[p1+2+i];
	  }
	  SatelliteCountryCode[3] = '\0';
	  CleanString( SatelliteCountryCode );
	  SatelliteCountryRegionId = ( Data[p1+5] & 0xfc ) >> 6;
	  SatelliteTimeOffsetPolarity = ( Data[p1+5] & 0x01 );
	  SatelliteTimeOffsetH = BcdToInt( Data[p1+6] );
	  SatelliteTimeOffsetM = BcdToInt( Data[p1+7] );
	  if( SatelliteTimeOffsetPolarity == 1 )
	  {
	    SatelliteTimeOffset = 0 - ( SatelliteTimeOffsetH * 3600 );
	  }
	  else
	  {
	    SatelliteTimeOffset = SatelliteTimeOffsetH * 3600;
	  }
	  EpgTimeOffset = ( LocalTimeOffset - SatelliteTimeOffset );
	  esyslog( "LoadEPG: Satellite Time Offset=[UTC]%+i", SatelliteTimeOffset / 3600 );
	  esyslog( "LoadEPG: Epg Time Offset=%+i seconds", EpgTimeOffset );
	  if( DEBUG )
	  {
	    esyslog( "LoadEPG: Satellite Time UTC: %s %02i:%02i:%02i", GetStringMJD( satMJD ), satH, satM, satS );
	    esyslog( "LoadEPG: Satellite CountryCode=%s", SatelliteCountryCode );
	    esyslog( "LoadEPG: Satellite CountryRegionId=%i", SatelliteCountryRegionId );
	    esyslog( "LoadEPG: Satellite LocalTimeOffsetPolarity=%i", SatelliteTimeOffsetPolarity );
	    esyslog( "LoadEPG: Satellite LocalTimeOffset=%02i:%02i", SatelliteTimeOffsetH, SatelliteTimeOffsetM );
	  }
	  break;
	default:
	  //fprintf( stderr, "0x%02x\n", DescriptorTag );
	  break;
      }
      p1 += ( DescriptorLength + 2 );
      DescriptorsLoopLength -= ( DescriptorLength + 2 );
    }
  }
  Filters[FilterId].Step = 2;
}
// }}}

// SKYBOX Stuff {{{
// cTaskLoadepg::SupplementChannelsSKYBOX {{{
void cTaskLoadepg::SupplementChannelsSKYBOX( int FilterId, unsigned char *Data, int Length )
{
  if (!EndBAT)
  {
    return;
  }

  if ( EndSDT )
  {
    Filters[FilterId].Step = 2;
    //Dprintf("endsdt\n");
    return;
  }

  SI::SDT sdt(Data, false);
  if (!sdt.CheckCRCAndParse())
    return;

  SI::SDT::Service SiSdtService;
  for (SI::Loop::Iterator it; sdt.serviceLoop.getNext(SiSdtService, it); ) {

    sChannel Key, *C;
    Key.ChannelId = 
    Key.Sid = SiSdtService.getServiceId();
    Key.Nid = sdt.getOriginalNetworkId();
    Key.Tid = sdt.getTransportStreamId();
    C = ( sChannel * ) bsearch( &Key, lChannels, nChannels, sizeof( sChannel ), &bsearchChannelBySid );

    if (firstSDTChannel == NULL)
    {
      firstSDTChannel = C;
    }
    else if (firstSDTChannel == C)
    {
      if (nChannelUpdates == 0) {
	EndSDT = true;
      } else
	nChannelUpdates = 0;
    }

    SI::Descriptor *d;
    for (SI::Loop::Iterator it2; (d = SiSdtService.serviceDescriptors.getNext(it2)); ) {
      switch (d->getDescriptorTag()) {
	case SI::ServiceDescriptorTag: 
	  {
	    SI::ServiceDescriptor *sd = (SI::ServiceDescriptor *)d;
	    switch (sd->getServiceType()) {
	      case 0x01: // digital television service
	      case 0x02: // digital radio sound service
	      case 0x04: // NVOD reference service
	      case 0x05: // NVOD time-shifted service
		{
		  char NameBuf[1024];
		  char ShortNameBuf[1024];
		  char ProviderNameBuf[1024];
		  //Dprintf("B %02x %x-%x %x-%x %x-%x\n", sd->getServiceType(), Key.Nid, lChannels[10].Nid, Key.Tid, lChannels[10].Tid, Key.Sid, lChannels[10].Sid );
		  sd->serviceName.getText(NameBuf, ShortNameBuf, sizeof(NameBuf), sizeof(ShortNameBuf));
		  char *pn = compactspace(NameBuf);
		  char *ps = compactspace(ShortNameBuf);
		  sd->providerName.getText(ProviderNameBuf, sizeof(ProviderNameBuf));
		  char *provname = compactspace(ProviderNameBuf);
		  if (C) {
		    if (C->name == NULL)
		    {
		      asprintf( &C->name, "%s", pn);
		      asprintf( &C->providername, "%s", provname);
		      //asprintf( &C->shortname, "%s", ps);
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
	    SI::MultilingualServiceNameDescriptor *md = (SI::MultilingualServiceNameDescriptor *)d;
	    SI::MultilingualServiceNameDescriptor::Name n;
	    for (SI::Loop::Iterator it2; (md->nameLoop.getNext(n, it2)); ) {
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
		  if (C->name)
		  {
		    free(C->name);
		    C->name = NULL;
		  }
		  //C->name = n.name.getText();
		  char b[100];
		  n.name.getText(b, sizeof(b));
		  C->name = strdup(b);
		  C->IsNameUpdated = true;
		  //nChannelUpdates++;
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
void cTaskLoadepg::GetChannelsSKYBOX( int FilterId, unsigned char *Data, int Length )
{
  unsigned char SectionNumber = Data[6];
  unsigned char LastSectionNumber = Data[7];
  
  if( SectionNumber == 0x00 && nBouquets == 0 )
  {
    return;
  }
  
  // Table BAT
  if( Data[0] == 0x4a )
  {
    if( EndBAT )
    {
      Filters[FilterId].Step = 2;
      //Dprintf("endbat\n");
      return;
    }
    unsigned short int BouquetId = ( Data[3] << 8 ) | Data[4];
    int BouquetDescriptorsLength = ( ( Data[8] & 0x0f ) << 8 ) | Data[9];
    int TransportStreamLoopLength = ( ( Data[BouquetDescriptorsLength+10] & 0x0f ) << 8 ) | Data[BouquetDescriptorsLength+11];
    int p1 = ( BouquetDescriptorsLength + 12 );
    while( TransportStreamLoopLength > 0 )
    {
      unsigned short int Tid = ( Data[p1] << 8 ) | Data[p1+1];
      unsigned short int Nid = ( Data[p1+2] << 8 ) | Data[p1+3];
      int TransportDescriptorsLength = ( ( Data[p1+4] & 0x0f ) << 8 ) | Data[p1+5];
      int p2 = ( p1 + 6 );
      p1 += ( TransportDescriptorsLength + 6 );
      TransportStreamLoopLength -= ( TransportDescriptorsLength + 6 );
      while( TransportDescriptorsLength > 0 )
      {
        unsigned char DescriptorTag = Data[p2];
	int DescriptorLength = Data[p2+1];
	int p3 = ( p2 + 2 );
        p2 += ( DescriptorLength + 2 );
	TransportDescriptorsLength -= ( DescriptorLength + 2 );
	switch( DescriptorTag )
	{
	  case 0x41:	// service_list
	    break;
	  case 0x5f:	// private data specifier indicates BSkyB
	    break;
	  case 0x93:	// unknown private
	    break;
	  case 0xb1:
	    p3 += 2;
	    DescriptorLength -= 2;
	    while( DescriptorLength > 0 )
	    {
	      // 0x01 = Video Channel
	      // 0x02 = Audio Channel
	      // 0x05 = Other Channel
	      //if( Data[p3+2] == 0x01 || Data[p3+2] == 0x02 || Data[p3+2] == 0x05 )
	      //{
	        unsigned short Sid = ( Data[p3] << 8 ) | Data[p3+1];
		unsigned char unk1 = Data[p3+2];
	        unsigned short ChannelId = ( Data[p3+3] << 8 ) | Data[p3+4];
	        unsigned short SkyNumber = ( Data[p3+5] << 8 ) | Data[p3+6];
	        unsigned short Flags = ( Data[p3+7] << 8 ) | Data[p3+8];
		unsigned short unkval = Flags >> 4;
		int unkflag1 = (Flags & 8) >> 3;
		int unkflag2 = (Flags & 4) >> 2;
		int unkflag3 = (Flags & 2) >> 1;
		int unkflag4 = (Flags & 1);
		/*
		 * 
		 */
		if( SkyNumber > 100 && SkyNumber < 1000 )
		{
		  if( ChannelId > 0 )
		  {
		    sChannel Key, *C;
		    Key.ChannelId = ChannelId;
		    Key.Nid = Nid;
		    Key.Tid = Tid;
		    Key.Sid = Sid;
		    C = ( sChannel * ) bsearch( &Key, lChannels, nChannels, sizeof( sChannel ), &bsearchChannelByChID );
		    if( C == NULL )
		    {
		      C = ( lChannels + nChannels );
		      C->ChannelId = ChannelId;
		      C->Nid = Nid;
		      C->Tid = Tid;
		      C->Sid = Sid;
		      C->SkyNumber = SkyNumber;
		      C->pData = 0;
		      C->lenData = 0;
		      C->IsFound = false;
		      C->IsEpg = false;
		      nChannels ++;
		      if( nChannels >= MAX_CHANNELS )
		      {
                        esyslog( "LoadEPG: Error, channels found more than %i", MAX_CHANNELS );
			IsError = true;
			return;
		      }
		      qsort( lChannels, nChannels, sizeof( sChannel ), &qsortChannelsByChID );
		    }
		  }
		}
	      //}
	      p3 += 9;
	      DescriptorLength -= 9;
	    }
	    break;
	  default:
	    Dprintf( "unprocessed descriptor 0x%02x\n", DescriptorTag );
	    break;
	}
      }
    }
    sBouquet *B;
    for( int i = 0; i < nBouquets; i ++ )
    {
      B = ( lBouquets + i );
      if( B->BouquetId == BouquetId )
      {
        goto CheckBouquetSections;
      }
    }
    B = ( lBouquets + nBouquets );
    B->BouquetId = BouquetId;
    for( int i = 0; i <= LastSectionNumber; i ++ )
    {
      B->SectionNumber[i] = -1;
    }
    B->LastSectionNumber = LastSectionNumber;
    nBouquets ++;
    CheckBouquetSections:;
    B->SectionNumber[SectionNumber] = SectionNumber;
    EndBAT = true;
    for( int i = 0; i < nBouquets; i ++ )
    {
      B = ( lBouquets + i );
      for( int ii = 0; ii <= B->LastSectionNumber; ii ++ )
      {
        if( B->SectionNumber[ii] == -1 )
        {
          EndBAT = false;
          break;
        }
      }
    }
  }
}
// }}}

// cTaskLoadepg::GetTitlesSKYBOX {{{
void cTaskLoadepg::GetTitlesSKYBOX( int FilterId, unsigned char *Data, int Length )
{
  int p;
  unsigned short int ChannelId;
  unsigned short int MjdTime;
  int Len1;
  int Len2;
  
  if( Length < 20 )
  {
    return;
  }
  if( memcmp( &InitialBuffer[FilterId][0], Data, 20 ) == 0 )
  {
    Filters[FilterId].Step = 2;
    return;
  }
  else
  {
    if( InitialBuffer[FilterId][0] == 0 )
    {
      memcpy( &InitialBuffer[FilterId][0], Data, 20 );
    }
    ChannelId = ( Data[3] << 8 ) | Data[4];
    MjdTime = ( ( Data[8] << 8 ) | Data[9] );
    if( ChannelId > 0 )
    {
      if( MjdTime > 0 )
      {
        p = 10;
	loop1:;
	sTitle *T = ( lTitles + nTitles );
	T->ChannelId = ChannelId;
	T->MjdTime = MjdTime;
	T->EventId = ( Data[p] << 8 ) | Data[p+1];
	Len1 = ( ( Data[p+2] & 0x0f ) << 8 ) | Data[p+3];
	if( Data[p+4] != 0xb5 )
	{
	  if( DEBUG )
	  {
	    esyslog( "LoadEPG: Data error signature for title" );
	  }
	  goto endloop1;
	}
        if( Len1 > Length )
	{
	  if( DEBUG )
	  {
	    esyslog( "LoadEPG: Data error length for title" );
	  }
	  goto endloop1;
	}
	p += 4;
	Len2 = Data[p+1] - 7;
	T->StartTime = ( ( MjdTime - 40587 ) * 86400 ) + ( ( Data[p+2] << 9 ) | ( Data[p+3] << 1 ) );
	T->Duration = ( ( Data[p+4] << 9 ) | ( Data[p+5] << 1 ) );
	T->ThemeId = Data[p+6];
	T->pData = pT;
	T->lenData = Len2;
	if( ( pT + Len2 + 2 ) > MAX_BUFFER_SIZE_TITLES )
	{
	  esyslog( "LoadEPG: Error, buffer overflow, titles size more than %i bytes", MAX_BUFFER_SIZE_TITLES );
	  IsError = true;
	  return;
	}
	memcpy( &bTitles[pT], &Data[p+9], Len2 );
	pT += ( Len2 + 1 );
	p += Len1;
	nTitles ++;
	if( nTitles >= MAX_TITLES )
	{
	  esyslog( "LoadEPG: Error, titles found more than %i", MAX_TITLES );
	  IsError = true;
	  return;
	}
	if( p < Length )
	{
	  goto loop1;
	}
	endloop1:;
      }
    }
  }
}
// }}}

// cTaskLoadepg::GetSummariesSKYBOX {{{
void cTaskLoadepg::GetSummariesSKYBOX( int FilterId, unsigned char *Data, int Length )
{
  int p;
  unsigned short int ChannelId;
  unsigned short int MjdTime;
  int Len1;
  int Len2;
  
  if( Length < 20 )
  {
    return;
  }
  if( memcmp( &InitialBuffer[FilterId][0], Data, 20 ) == 0 )
  {
    Filters[FilterId].Step = 2;
    return;
  }
  else
  {
    if( InitialBuffer[FilterId][0] == 0 )
    {
      memcpy( &InitialBuffer[FilterId][0], Data, 20 );
    }
    ChannelId = ( Data[3] << 8 ) | Data[4];
    MjdTime = ( ( Data[8] << 8 ) | Data[9] );
    if( ChannelId > 0 )
    {
      if( MjdTime > 0 )
      {
        p = 10;
        loop1:;
	sSummary *S = ( lSummaries + nSummaries );
	S->ChannelId = ChannelId;
	S->MjdTime = MjdTime;
	S->EventId = ( Data[p] << 8 ) | Data[p+1];
	Len1 = ( ( Data[p+2] & 0x0f ) << 8 ) | Data[p+3];
	if( Data[p+4] != 0xb9 )
	{
	  if( DEBUG )
	  {
	    esyslog( "LoadEPG: Data error signature for summary" );
	  }
	  goto endloop1;
        }
        if( Len1 > Length )
        {
          if( DEBUG )
	  {
	    esyslog( "LoadEPG: Data error length for summary" );
	  }
          goto endloop1;
        }
        p += 4;
        Len2 = Data[p+1];
        S->pData = pS;
        S->lenData = Len2;
        if( ( pS + Len2 + 2 ) > MAX_BUFFER_SIZE_SUMMARIES )
        {
          esyslog( "LoadEPG: Error, buffer overflow, summaries size more than %i bytes", MAX_BUFFER_SIZE_SUMMARIES );
          IsError = true;
          return;
        }
        memcpy( &bSummaries[pS], &Data[p+2], Len2 );
        pS += ( Len2 + 1 );
        p += Len1;
        nSummaries ++;
        if( nSummaries >= MAX_SUMMARIES )
        {
          esyslog( "LoadEPG: Error, summaries found more than %i", MAX_SUMMARIES );
          IsError = true;
	  return;
        }
        if( p < Length )
        {
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
void cTaskLoadepg::CleanString( unsigned char *String )
{
  unsigned char *Src;
  unsigned char *Dst;
  int Spaces;
  int pC;
  Src = String;
  Dst = String;
  Spaces = 0;
  pC = 0;
  while( *Src )
  {
    // corrections
    if( *Src == 0x8c ) // iso-8859-2 LATIN CAPITAL LETTER S WITH ACUTE
    {
      *Src = 0xa6;
    }
    if( *Src == 0x8f ) // iso-8859-2 LATIN CAPITAL LETTER Z WITH ACUTE
    {
      *Src = 0xac;
    }
    
    if( *Src < 0x20 )
    {
      *Src = 0x20;
    }
    if( *Src == 0x20 )
    {
      Spaces ++;
      if( pC == 0 )
      {
        Spaces ++;
      }
    }
    else
    {
      Spaces = 0;
    }
    if( Spaces < 2 )
    {
      *Dst = *Src;
      *Dst ++;
      pC ++;
    }
    *Src ++;
  }
  if( Spaces > 0 )
  {
    Dst --;
    *Dst = 0;
  }
  else
  {
    *Dst = 0;
  }
}
// }}}

// MHW1 Stuff {{{
// cTaskLoadepg::GetThemesMHW1 {{{
void cTaskLoadepg::GetThemesMHW1( int FilterId, unsigned char *Data, int Length )
{
  if( Length > 19 )
  {
    sThemeMHW1 *Theme = ( sThemeMHW1 * ) ( Data + 19 );
    nThemes = ( Length - 19 ) / 15;
    if( nThemes > MAX_THEMES )
    {
      esyslog( "LoadEPG: Error, themes found more than %i", MAX_THEMES );
      IsError = true;
      return;
    }
    else
    {
      int ThemeId = 0;
      int Offset = 0;
      unsigned char *ThemesIndex = ( Data + 3 );
      for( int i = 0; i < nThemes; i ++ )
      {
        if( ThemesIndex[ThemeId] == i )
        {
          Offset = ( Offset + 15 ) & 0xf0;
	  ThemeId ++;
        }
        sTheme *T = ( lThemes + Offset );
        memcpy( &T->Name, &Theme->Name, 15 );
        CleanString( T->Name );
        Offset ++;
        Theme ++;
      }
      Filters[FilterId].Step = 2;
    }
  }
}
// }}}

// cTaskLoadepg::GetChannelsMHW1 {{{
void cTaskLoadepg::GetChannelsMHW1( int FilterId, unsigned char *Data, int Length )
{
  sChannelMHW1 *Channel = ( sChannelMHW1 * ) ( Data + 4 );
  nChannels = ( Length - 4 ) / sizeof( sChannelMHW1 );
  if( nChannels > MAX_CHANNELS )
  {
    esyslog( "LoadEPG: Error, channels found more than %i", MAX_CHANNELS );
    IsError = true;
    return;
  }
  else
  {
    for( int i = 0; i < nChannels; i ++ )
    {
      sChannel *C = ( lChannels + i );
      C->ChannelId = i;
      C->Nid = HILO16( Channel->NetworkId );
      C->Tid = HILO16( Channel->TransportId );
      C->Sid = HILO16( Channel->ServiceId );
      C->SkyNumber = 0;
      C->pData = pC;
      C->lenData = 16;
      C->IsFound = false;
      C->IsEpg = true;
      if( ( pC + 18 ) > MAX_BUFFER_SIZE_CHANNELS )
      {
        esyslog( "LoadEPG: Error, buffer overflow, channels size more than %i bytes", MAX_BUFFER_SIZE_CHANNELS );
	IsError = true;
	return;
      }
      memcpy( &bChannels[pC], &Channel->Name, 16 );
      CleanString( &bChannels[pC] );
      pC += 17;
      Channel ++;
    }
    Filters[FilterId].Step = 2;
  }
}
// }}}

// cTaskLoadepg::GetTitlesMHW1 {{{
void cTaskLoadepg::GetTitlesMHW1( int FilterId, unsigned char *Data, int Length )
{
  sTitleMHW1 *Title = ( sTitleMHW1 * ) Data;
  if( Length == 46 )
  {
    if( Title->ChannelId != 0xff )
    {
      if( nTitles < MAX_TITLES )
      {
        if( memcmp( &InitialBuffer[FilterId][0], Data, 46 ) == 0 )
        {
	  Filters[FilterId].Step = 2;
        }
        else
        {
          if( InitialBuffer[FilterId][0] == 0 )
	  {
	    memcpy( &InitialBuffer[FilterId][0], Data, 46 );
	  }
	  sTitle *T = ( lTitles + nTitles );
	  int Day = Title->Day;
	  int Hours = Title->Hours;
	  int Minutes = Title->Minutes;
	  if( Hours > 15 )
	  {
	    Hours -= 4;
	  }
	  else if( Hours > 7 )
	  {
	    Hours -= 2;
	  }
	  else
	  {
	    Day ++;
	  }
	  if( Day > 6 )
	  {
	    Day = Day - 7;
	  }
	  Day -= Yesterday;
	  if( Day < 1 )
	  {
	    Day = 7 + Day;
	  }
	  if( Day == 1 && Hours < 6 )
	  {
	    Day = 8;
	  }
	  int StartTime = ( Day * 86400 ) + ( Hours * 3600 ) + ( Minutes * 60 );
	  StartTime += YesterdayEpoch;
	  T->ChannelId = Title->ChannelId - 1;
	  T->ThemeId = Title->ThemeId;
	  T->MjdTime = 0;
	  T->EventId = HILO32( Title->ProgramId );
	  T->StartTime = StartTime;
	  T->Duration = HILO16( Title->Duration ) * 60;
	  T->SummaryAvailable = Title->SummaryAvailable;
	  T->pData = pT;
	  T->lenData = 23;
	  if( ( pT + 25 ) > MAX_BUFFER_SIZE_TITLES )
	  {
	    esyslog( "LoadEPG: Error, buffer overflow, titles size more than %i bytes", MAX_BUFFER_SIZE_TITLES );
	    IsError = true;
	    return;
	  }
	  memcpy( &bTitles[pT], &Title->Title, 23 );
	  CleanString( &bTitles[pT] );
	  pT += 24;
	  nTitles ++;
        }
      }
      else
      {
        esyslog( "LoadEPG: Error, titles found more than %i", MAX_TITLES );
        IsError = true;
	return;
      }
    }
  }
}
// }}}

// cTaskLoadepg::GetSummariesMHW1 {{{
void cTaskLoadepg::GetSummariesMHW1( int FilterId, unsigned char *Data, int Length )
{
  sSummaryMHW1 *Summary = ( sSummaryMHW1 * ) Data;
  if( Length > 11 )
  {
    if( Summary->NumReplays < 10 )
    {
      if( Length > ( 11 + ( Summary->NumReplays * 7 ) ) )
      {
        if( Summary->Byte7 == 0xff && Summary->Byte8 && Summary->Byte9 == 0xff )
	{
          if( nSummaries < MAX_SUMMARIES )
          {
            if( memcmp( &InitialBuffer[FilterId][0], Data, 20 ) == 0 )
            {
	      Filters[FilterId].Step = 2;
            }
            else
            {
              if( InitialBuffer[FilterId][0] == 0 )
	      {
	        memcpy( &InitialBuffer[FilterId][0], Data, 20 );
	      }
	      int SummaryOffset = 11 + ( Summary->NumReplays * 7 );
	      int SummaryLength = Length - SummaryOffset;
	      sSummary *S = ( lSummaries + nSummaries );
	      S->ChannelId = 0;
	      S->MjdTime = 0;
	      S->EventId = HILO32( Summary->ProgramId );
	      S->pData = pS;
	      S->lenData = SummaryLength;
	      if( ( pS + SummaryLength + 2 ) > MAX_BUFFER_SIZE_SUMMARIES )
	      {
	        esyslog( "LoadEPG: Error, buffer overflow, summaries size more than %i bytes", MAX_BUFFER_SIZE_SUMMARIES );
		IsError = true;
		return;
	      }
	      memcpy( &bSummaries[pS], &Data[SummaryOffset], SummaryLength );
	      CleanString( &bSummaries[pS] );
	      pS += ( SummaryLength + 1 );
	      nSummaries ++;
            }
          }
          else
          {
            esyslog( "LoadEPG: Error, summaries found more than %i", MAX_SUMMARIES );
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
void cTaskLoadepg::GetThemesMHW2( int FilterId, unsigned char *Data, int Length )
{
  int p1;
  int p2;
  int pThemeName = 0;
  int pSubThemeName = 0;
  int lenThemeName = 0;
  int lenSubThemeName = 0;
  int pThemeId = 0;
  if( EndThemes && EndChannels )
  {
    Filters[FilterId].Step = 2;
    return;
  }
  if( Length > 4 )
  {
    for( int i = 0; i < Data[4]; i ++ )
    {
      p1 = ( ( Data[5+i*2] << 8 ) | Data[6+i*2] ) + 3;
      if( Length > p1 )
      {
        for( int ii = 0; ii <= ( Data[p1] & 0x3f ); ii ++ )
        {
          p2 = ( ( Data[p1+1+ii*2] << 8 ) | Data[p1+2+ii*2] ) + 3;
          if( Length > p2 )
	  {
            if( ii == 0 )
            {
              pThemeName = p2 + 1;
	      lenThemeName = Data[p2] & 0x1f;
	      lenSubThemeName = 0;
            }
            else
            {
              pSubThemeName = p2 + 1;
	      lenSubThemeName = Data[p2] & 0x1f;
            }
	    if( Length >= ( pThemeName + lenThemeName ) )
	    {
              pThemeId = ( ( i & 0x3f ) << 6 ) | ( ii & 0x3f );
	      if( ( lenThemeName + 2 ) < 256 )
              {
                memcpy( ( lThemes + pThemeId )->Name, &Data[pThemeName], lenThemeName );
                if( Length >= ( pSubThemeName + lenSubThemeName ) )
		{
		  if( lenSubThemeName > 0 )
                  {
                    if( ( lenThemeName + lenSubThemeName + 2 ) < 256 )
	            {
	              ( lThemes + pThemeId )->Name[lenThemeName] = ' ';
	              memcpy( &( lThemes + pThemeId )->Name[lenThemeName+1], &Data[pSubThemeName], lenSubThemeName );
	            }
                  }
		}
	        CleanString( ( lThemes + pThemeId )->Name );
	        nThemes ++;
                if( nThemes > MAX_THEMES )
                {
                  esyslog( "LoadEPG: Error, themes found more than %i", MAX_THEMES );
                  IsError = true;
		  return;
                }
              }
	    }
	    else
	    {
	      return;
	    }
	  }
	  else
	  {
	    return;
	  }
        }
      }
      else
      {
        return;
      }
    }
    EndThemes = true;
  }
}
// }}}

// cTaskLoadepg::GetChannelsMHW2 {{{
void cTaskLoadepg::GetChannelsMHW2( int FilterId, unsigned char *Data, int Length )
{
  if( EndThemes && EndChannels )
  {
    Filters[FilterId].Step = 2;
    return;
  }
  if( Length > 119 )
  {
    nChannels = Data[119];
    if( nChannels > MAX_CHANNELS )
    {
      esyslog( "LoadEPG: Error, channels found more than %i", MAX_CHANNELS );
      IsError = true;
      return;
    }
    else
    {
      int pName = ( ( nChannels * 8 ) + 120 );
      if( Length > pName )
      {
        sChannelMHW2 *Channel = ( sChannelMHW2 * ) ( Data + 120 );
        for( int i = 0; i < nChannels; i ++ )
        {
          sChannel *C = ( lChannels + i );
	  C->ChannelId = i;
	  C->Nid = HILO16( Channel->NetworkId );
	  C->Tid = HILO16( Channel->TransportId );
	  C->Sid = HILO16( Channel->ServiceId );
	  C->SkyNumber = 0;
	  int lenName = Data[pName] & 0x0f;
	  if( ( pC + lenName + 2 ) > MAX_BUFFER_SIZE_CHANNELS )
	  {
	    esyslog( "LoadEPG: Error, buffer overflow, channels size more than %i bytes", MAX_BUFFER_SIZE_CHANNELS );
	    IsError = true;
	    return;
	  }
	  memcpy( &bChannels[pC], &Data[pName+1], lenName );
	  pName += ( lenName + 1 );
	  C->pData = pC;
	  C->lenData = lenName;
	  C->IsFound = false;
	  C->IsEpg = true;
	  pC += ( lenName + 1 );
          Channel ++;
        }
	EndChannels = true;
      }
    }
  }
}
// }}}

// cTaskLoadepg::GetTitlesMHW2 {{{
void cTaskLoadepg::GetTitlesMHW2( int FilterId, unsigned char *Data, int Length )
{
  if( Length > 18 )
  {
    int Pos = 18;
    int Len = 0;
    bool Check = false;
    while( Pos < Length )
    {
      Check = false;
      Pos += 7;
      if( Pos < Length )
      {
        Pos += 3;
	if( Pos < Length )
	{
	  if( Data[Pos] > 0xc0 )
	  {
	    Pos += ( Data[Pos] - 0xc0 );
	    Pos += 4;
	    if( Pos < Length )
	    {
	      if( Data[Pos] == 0xff )
	      {
	        Pos += 1;
		Check = true;
	      }
	    }
	  }
	}
      }
      if( Check == false )
      {
	return;
      }
    }
    if( memcmp( &InitialBuffer[FilterId][0], Data, 16 ) == 0 )
    {
      Filters[FilterId].Step = 2;
    }
    else
    {
      if( InitialBuffer[FilterId][0] == 0 )
      {
	memcpy( &InitialBuffer[FilterId][0], Data, 16 );
      }
      Pos = 18;
      while( Pos < Length )
      {
	sTitle *T = ( lTitles + nTitles );
	T->ChannelId = Data[Pos];
	T->MjdTime = ( Data[Pos+3] << 8 ) | Data[Pos+4];
	T->StartTime = ( ( T->MjdTime - 40587 ) * 86400 )
	             + ( ( ( ( ( Data[Pos+5] & 0xf0 ) >> 4 ) * 10 ) + ( Data[Pos+5] & 0x0f ) ) * 3600 )
		     + ( ( ( ( ( Data[Pos+6] & 0xf0 ) >> 4 ) * 10 ) + ( Data[Pos+6] & 0x0f ) ) * 60 );
	T->Duration = ( ( ( Data[Pos+8] << 8 ) | Data[Pos+9] ) >> 4 ) * 60;
	Len = Data[Pos+10] & 0x3f;
	T->pData = pT;
	T->lenData = Len;
	if( ( pT + Len + 2 ) > MAX_BUFFER_SIZE_TITLES )
	{
	  esyslog( "LoadEPG: Error, buffer overflow, titles size more than %i bytes", MAX_BUFFER_SIZE_TITLES );
	  IsError = true;
	  return;
	}
	memcpy( &bTitles[pT], &Data[Pos+11], Len );
	CleanString( &bTitles[pT] );
	pT += ( Len + 1 );
	Pos += Len + 11;
	T->ThemeId = ( ( Data[7] & 0x3f ) << 6 ) | ( Data[Pos] & 0x3f );
	T->EventId = ( Data[Pos+1] << 8 ) | Data[Pos+2];
	T->SummaryAvailable = 1;
	Pos += 4;
	nTitles ++;
	if( nTitles > MAX_TITLES )
	{
	  esyslog( "LoadEPG: Error, titles found more than %i", MAX_TITLES );
	  IsError = true;
	  return;
	}
      }
    }
  }
}
// }}}

// cTaskLoadepg::GetSummariesMHW2 {{{
void cTaskLoadepg::GetSummariesMHW2( int FilterId, unsigned char *Data, int Length )
{
  if( Length > ( Data[14] + 17 ) )
  {
    if( memcmp( &InitialBuffer[FilterId][0], Data, 16 ) == 0 )
    {
      Filters[FilterId].Step = 2;
    }
    else
    {
      if( InitialBuffer[FilterId][0] == 0 )
      {
	memcpy( &InitialBuffer[FilterId][0], Data, 16 );
      }
      int lenText = Data[14];
      int lenSummary = lenText;
      int Pos = 15;
      int Loop = Data[Pos+lenSummary] & 0x0f;
      if( ( pS + lenText + 2 ) < MAX_BUFFER_SIZE_SUMMARIES )
      {
        sSummary *S = ( lSummaries + nSummaries );
	S->ChannelId = 0;
	S->MjdTime = 0;
	S->EventId = ( Data[3] << 8 ) | Data[4];
	S->pData = pS;
        memcpy( &bSummaries[S->pData], &Data[Pos], lenText );
	bSummaries[S->pData+lenSummary] = '|';
	lenSummary += 1;
	Pos += ( lenText + 1 );
	if( Loop > 0 )
	{
	  while( Loop > 0 )
	  {
	    lenText = Data[Pos];
	    Pos += 1;
	    if( ( Pos + lenText ) < Length )
	    {
	      if( ( pS + lenSummary + lenText + 2 ) > MAX_BUFFER_SIZE_SUMMARIES )
	      {
	        esyslog( "LoadEPG: Error, buffer overflow, summaries size more than %i bytes", MAX_BUFFER_SIZE_SUMMARIES );
		IsError = true;
		return;
	      }
	      memcpy( &bSummaries[S->pData+lenSummary], &Data[Pos], lenText );
	      lenSummary += lenText;
	      if( Loop > 1 )
	      {
		bSummaries[S->pData+lenSummary] = '|';
		lenSummary += 1;
	      }
	    }
	    else
	    {
	      break;
	    }
	    Pos += lenText;
	    Loop --;
	  }
	}
	CleanString( &bSummaries[S->pData] );
	S->lenData = lenSummary;
	pS += ( lenSummary + 1 );
	nSummaries ++;
      }
      else
      {
        esyslog( "LoadEPG: Error, buffer overflow, summaries size more than %i", MAX_BUFFER_SIZE_SUMMARIES );
	IsError = true;
        return;
      }
    }
  }
}
// }}}
// }}}

// cTaskLoadepg::CreateEpgXml {{{
void cTaskLoadepg::CreateEpgXml( void )
{
  char *ChannelName;
  esyslog( "LoadEPG: found %i equivalents channels", nEquivChannels );
  esyslog( "LoadEPG: found %i themes", nThemes );
  esyslog( "LoadEPG: found %i channels", nChannels );
  esyslog( "LoadEPG: found %i titles", nTitles );
  esyslog( "LoadEPG: found %i summaries", nSummaries );
  qsort( lEquivChannels, nEquivChannels, sizeof( sEquivChannel ), &qsortEquivChannels );
  qsort( lChannels, nChannels, sizeof( sChannel ), &qsortChannels );
  qsort( lTitles, nTitles, sizeof( sTitle ), &qsortTitles );
  qsort( lSummaries, nSummaries, sizeof( sSummary ), &qsortSummaries );

  if( ( lProviders + CurrentProvider )->DataFormat == DATA_FORMAT_SKYBOX )
  {
    // SKYBOX
    if( ReadFileDictionary() )
    {
      ReadFileThemes();
      if( nChannels > 0 )
      {
	if( nTitles > 0 )
	{
	  if( nSummaries > 0 )
	  {
	    int i;
	    int prev_i;
	    int EventId;
	    unsigned short int ChannelId;
	    bool IsChannel;
	    bool IsEquivChannel;
	    sChannel KeyC, *C;
	    sEquivChannel KeyEC, *EC;
	    i = 0;
	    prev_i = 0;
	    EventId = 1;
	    ChannelId = 0;
	    IsChannel = false;
	    IsEquivChannel = false;

	    while( i < nTitles )
	    {
	      char       date_strbuf[256];
	      time_t StartTime;

	      sTitle *T = ( lTitles + i );

	      KeyC.ChannelId = T->ChannelId;
	      C = ( sChannel * ) bsearch( &KeyC, lChannels, nChannels, sizeof( sChannel ), &bsearchChannelByChannelId );
	      if( C )
	      {
		C->IsEpg = true;
		C->IsFound = true;
	      }

	      char * channelIdent = get_channelident(C);
	      printf("<programme channel=\"%s\" ", channelIdent);
	      StartTime = ( T->StartTime + EpgTimeOffset );
	      strftime(date_strbuf, sizeof(date_strbuf), "start=\"%Y%m%d%H%M%S %z\"", localtime(&StartTime) );
	      printf("%s ", date_strbuf);
	      StartTime = ( T->StartTime + T->Duration + EpgTimeOffset );
	      strftime(date_strbuf, sizeof(date_strbuf), "stop=\"%Y%m%d%H%M%S %z\"", localtime(&StartTime));
	      printf("%s>\n ", date_strbuf);

	      //printf("\t<EventID>%i</EventID>\n", HILO(evt->event_id));
	      //printf("\t<RunningStatus>%i</RunningStatus>\n", evt->running_status);
	      //1 Airing, 2 Starts in a few seconds, 3 Pausing, 4 About to air

	      if( DecodeHuffmanCode( &bTitles[T->pData], T->lenData ) )
	      {
		CleanString( DecodeText );
	        //printf("\t<title lang=\"%s\">%s</title>\n", xmllang(&evtdesc->lang_code1), xmlify(evt));
		printf("\t<title lang=\"%s\">%s</title>\n", 
		    /* xmllang(&evtdesc->lang_code1) */ "en", xmlify((const char *)DecodeText));
	      }
	      sSummary KeyS, *S;
	      KeyS.ChannelId = T->ChannelId;
	      KeyS.MjdTime = T->MjdTime;
	      KeyS.EventId = T->EventId;
	      S = ( sSummary * ) bsearch( &KeyS, lSummaries, nSummaries, sizeof( sSummary ), &bsearchSummarie );
	      if( S )
	      {
		if( DecodeHuffmanCode( &bSummaries[S->pData], S->lenData ) )
		{
		  CleanString( DecodeText );
		  char *d = xmlify((const char *)DecodeText);
		  if (d && *d)
		  {
		    //printf("\t<sub-title lang=\"%s\">%s</sub-title>\n", 
		    //   /*xmllang(&evtdesc->lang_code1)*/ "en", d);
		    printf("\t<desc lang=\"%s\">", /*xmllang(&levt->lang_code1)*/ "en");
		    printf("%s", d);
		    printf("</desc>\n");
		  }
		}
	      }


#if 0
	      if( ChannelId != T->ChannelId )
	      {
		if( IsChannel )
		{
		  if( ChannelId > 0 )
		  {
		    //fprintf( File, "c\n" );
		  }
		}
		if( IsEquivChannel )
		{
		  if( ChannelId > 0 )
		  {
		    //fprintf( File, "c\n" );
		  }
		  i = prev_i;
		  T = ( lTitles + i );
		}
		IsChannel = false;
		IsEquivChannel = false;
		KeyC.ChannelId = T->ChannelId;
		C = ( sChannel * ) bsearch( &KeyC, lChannels, nChannels, sizeof( sChannel ), &bsearchChannelByChannelId );
		if( C )
		{
		  C->IsEpg = true;
		  tChannelID ChVID = tChannelID( ( lProviders + CurrentProvider )->SourceId, C->Nid, C->Tid, C->Sid );
#ifdef STANDALONE
		  cChannel *VC = NULL;
#else
		  cChannel *VC = Channels.GetByChannelID( ChVID, true );
		  if( VC )
#endif
		  {
#ifdef STANDALONE
		    KeyEC.OriginalSourceId = C->ChannelId;
		    KeyEC.OriginalNid = C->Nid;
		    KeyEC.OriginalTid = C->Tid;
		    KeyEC.OriginalSid = C->Sid;
#else
		    KeyEC.OriginalSourceId = VC->Source();
		    KeyEC.OriginalNid = VC->Nid();
		    KeyEC.OriginalTid = VC->Tid();
		    KeyEC.OriginalSid = VC->Sid();
#endif
		    EC = ( sEquivChannel * ) bsearch( &KeyEC, lEquivChannels, nEquivChannels, sizeof( sEquivChannel ), &bsearchEquivChannel );
		    if( EC && Config->UseFileEquivalents )
		    {
		      tChannelID ChEID = tChannelID( EC->EquivSourceId, EC->EquivNid, EC->EquivTid, EC->EquivSid, EC->EquivRid );
#ifdef STANDALONE
		      cChannel *VEC = NULL;
#else
		      cChannel *VEC = Channels.GetByChannelID( ChEID, false );
#endif
		      if( VEC )
		      {
			//fprintf( File, "<se>C %s-%i-%i-%i-%i name=\"%s\"\n", *cSource::ToString( VEC->Source() ), VEC->Nid(), VEC->Tid(), VEC->Sid(), VEC->Rid(), VEC->Name() );
			asprintf( &ChannelName, "%s", VEC->Name() );
			IsEquivChannel = true;
			prev_i = i;
		      }
		      EC->OriginalSourceId = 0;
		      EC->OriginalNid = 0;
		      EC->OriginalTid = 0;
		      EC->OriginalSid = 0;
		      qsort( lEquivChannels, nEquivChannels, sizeof( sEquivChannel ), &qsortEquivChannels );
		    }
		    else
		    {
		      //fprintf( File, "C %s-%i-%i-%i-%i %s\n", *cSource::ToString( VC->Source() ), VC->Nid(), VC->Tid(), VC->Sid(), VC->Rid(), VC->Name() );
		      //fprintf( File, "<sky>C %i-%i-%i-%i sky=%i\n", C->ChannelId, C->Nid, C->Tid, C->Sid, C->SkyNumber );
		      //asprintf( &ChannelName, "%s", VC->Name() );
		      asprintf( &ChannelName, "%i", C->SkyNumber );
		      IsChannel = true;
		      C->IsFound = true;
		    }
		  }
		}
		ChannelId = T->ChannelId;
	      }
	      if( IsChannel || IsEquivChannel )
	      {
		//fprintf( File, "E %u %u %u\n", EventId, ( T->StartTime + EpgTimeOffset ), T->Duration );
		if( DecodeHuffmanCode( &bTitles[T->pData], T->lenData ) )
		{
		  CleanString( DecodeText );
		  //fprintf( File, "T %s\n", DecodeText );
		  if( DecodeErrorText[0] != '\0' )
		  {
		    if( IsChannel )
		    {
		      time_t StartTime;
		      StartTime = ( T->StartTime + EpgTimeOffset );
		      //fprintf( Err, "Channel: %s - %s\n", ChannelName, ctime( &StartTime ) );
		      //fprintf( Err, "T %s%s\n\n", DecodeText, DecodeErrorText );
		    }
		  }
		  if( DEBUG && DEBUG_STARTTIME )
		  {
		    time_t StartTime;
		    char *DateTime;
		    StartTime = ( T->StartTime + EpgTimeOffset );
		    asprintf( &DateTime, "%s", ctime( &StartTime ) );
		    //fprintf( File, "S - %s", DateTime );
		  }
		  sTheme *ST = ( lThemes + T->ThemeId );
		  if( ST->Name[0] != '\0' )
		  {
		    //fprintf( File, "S %s\n", ST->Name );
		  }
		  else
		  {
		    if( DEBUG )
		    {
		      time_t StartTime;
		      StartTime = ( T->StartTime + EpgTimeOffset );
		      //fprintf( Err, "Channel: %s - %s - ThemeId=0x%02x\n", ChannelName, ctime( &StartTime ), T->ThemeId );
		    }
		  }
		  sSummary KeyS, *S;
		  KeyS.ChannelId = T->ChannelId;
		  KeyS.MjdTime = T->MjdTime;
		  KeyS.EventId = T->EventId;
		  S = ( sSummary * ) bsearch( &KeyS, lSummaries, nSummaries, sizeof( sSummary ), &bsearchSummarie );
		  if( S )
		  {
		    if( DecodeHuffmanCode( &bSummaries[S->pData], S->lenData ) )
		    {
		      CleanString( DecodeText );
		      //fprintf( File, "D %s\n", DecodeText );
		    }
		    if( DecodeErrorText[0] != '\0' )
		    {
		      if( IsChannel )
		      {
			time_t StartTime;
			StartTime = ( T->StartTime + EpgTimeOffset );
			//fprintf( Err, "Channel: %s - %s\n", ChannelName, ctime( &StartTime ) );
			//fprintf( Err, "D %s%s\n\n", DecodeText, DecodeErrorText );
		      }
		    }
		  }
		}
		//fprintf( File, "e\n" );
	      }
#endif
	      delete channelIdent;
	      printf("</programme>\n");
	      i ++;
	      EventId ++;
	    }


	    if( IsChannel || IsEquivChannel )
	    {
	      //fprintf( File, "c\n" );
	    }
	  }
	}
      }
    }
  }
  else
  {
    // MHW_1 && MHW_2
    int i;
    int prev_i;
    unsigned char ChannelId;
    sChannel KeyC, *C;
    sEquivChannel KeyEC, *EC;
    bool IsChannel;
    bool IsEquivChannel;
    i = 0;
    prev_i = 0;
    ChannelId = 0xff;
    IsChannel = false;
    IsEquivChannel = false;
    while( i < nTitles )
    {
      sTitle *T = ( lTitles + i );
      if( ChannelId != T->ChannelId )
      {
	if( IsChannel )
	{
	  if( ChannelId < 0xff )
	  {
	    //fprintf( File, "c\n" );
	  }
	}
	if( IsEquivChannel )
	{
	  if( ChannelId > 0 )
	  {
	    //fprintf( File, "c\n" );
	  }
	  i = prev_i;
	  T = ( lTitles + i );
	}
	IsChannel = false;
	IsEquivChannel = false;
	KeyC.ChannelId = T->ChannelId;
	C = ( sChannel * ) bsearch( &KeyC, lChannels, nChannels, sizeof( sChannel ), &bsearchChannelByChannelId );
	if( C )
	{
	  tChannelID ChVID = tChannelID( ( lProviders + CurrentProvider )->SourceId, C->Nid, C->Tid, C->Sid );
#ifdef STANDALONE
	  cChannel *VC = NULL;
#else
	  cChannel *VC = Channels.GetByChannelID( ChVID, false );
#endif
	  if( VC )
	  {
	    KeyEC.OriginalSourceId = VC->Source();
	    KeyEC.OriginalNid = VC->Nid();
	    KeyEC.OriginalTid = VC->Tid();
	    KeyEC.OriginalSid = VC->Sid();
	    EC = ( sEquivChannel * ) bsearch( &KeyEC, lEquivChannels, nEquivChannels, sizeof( sEquivChannel ), &bsearchEquivChannel );
	    if( EC && Config->UseFileEquivalents )
	    {
	      tChannelID ChEID = tChannelID( EC->EquivSourceId, EC->EquivNid, EC->EquivTid, EC->EquivSid, EC->EquivRid );
#ifdef STANDALONE
	      cChannel *VEC = NULL;
#else
	      cChannel *VEC = Channels.GetByChannelID( ChEID, false );
#endif
	      if( VEC )
	      {
		//fprintf( File, "<mhw>C %s-%i-%i-%i-%i %s\n", *cSource::ToString( VEC->Source() ), VEC->Nid(), VEC->Tid(), VEC->Sid(), VEC->Rid(), VEC->Name() );
		asprintf( &ChannelName, "%s", VEC->Name() );
		IsEquivChannel = true;
		prev_i = i;
	      }
	      EC->OriginalSourceId = 0;
	      EC->OriginalNid = 0;
	      EC->OriginalTid = 0;
	      EC->OriginalSid = 0;
	      qsort( lEquivChannels, nEquivChannels, sizeof( sEquivChannel ), &qsortEquivChannels );
	    }
	    else
	    {
	      //fprintf( File, "<mhw>C %s-%i-%i-%i-%i %s\n", *cSource::ToString( VC->Source() ), VC->Nid(), VC->Tid(), VC->Sid(), VC->Rid(), VC->Name() );
	      asprintf( &ChannelName, "%s", VC->Name() );
	      IsChannel = true;
	      C->IsFound = true;
	    }
	  }
	}
	ChannelId = T->ChannelId;
      }
      if( IsChannel || IsEquivChannel )
      {
	//fprintf( File, "E %u %u %u 01 FF\n", T->EventId, ( T->StartTime + EpgTimeOffset ), T->Duration );
	//fprintf( File, "T %s\n", &bTitles[T->pData] );
	if( ( lThemes + T->ThemeId )->Name[0] != '\0' )
	{
	  if( DEBUG && DEBUG_STARTTIME )
	  {
	    time_t StartTime;
	    char *DateTime;
	    StartTime = ( T->StartTime + EpgTimeOffset );
	    asprintf( &DateTime, "%s", ctime( &StartTime ) );
	    //fprintf( File, "S %s - %d\' - %s", ( lThemes + T->ThemeId )->Name, T->Duration / 60, DateTime );
	  }
	  else
	  {
	    //fprintf( File, "S %s - %d\'\n", ( lThemes + T->ThemeId )->Name, T->Duration / 60 );
	  }
	}
	sSummary KeyS, *S;
	KeyS.ChannelId = 0;
	KeyS.MjdTime = 0;
	KeyS.EventId = T->EventId;
	S = ( sSummary * ) bsearch( &KeyS, lSummaries, nSummaries, sizeof( sSummary ), &bsearchSummarie );
	if( S )
	{
	  //fprintf( File, "D %s\n", &bSummaries[S->pData] );
	}
	//fprintf( File, "e\n" );
      }
      i ++;
    }
    if(  IsChannel || IsEquivChannel )
    {
      //fprintf( File, "c\n" );
    }
  }
  CreateXmlChannels( );
}
// }}}

// cTaskLoadepg::CreateEpgDataFile {{{
void cTaskLoadepg::CreateEpgDataFile( void )
{
  FILE *File;
  FILE *Err;
  char *ChannelName;
  esyslog( "LoadEPG: found %i equivalents channels", nEquivChannels );
  esyslog( "LoadEPG: found %i themes", nThemes );
  esyslog( "LoadEPG: found %i channels", nChannels );
  esyslog( "LoadEPG: found %i titles", nTitles );
  esyslog( "LoadEPG: found %i summaries", nSummaries );
  qsort( lEquivChannels, nEquivChannels, sizeof( sEquivChannel ), &qsortEquivChannels );
  qsort( lChannels, nChannels, sizeof( sChannel ), &qsortChannels );
  qsort( lTitles, nTitles, sizeof( sTitle ), &qsortTitles );
  qsort( lSummaries, nSummaries, sizeof( sSummary ), &qsortSummaries );
  File = fopen( FILE_EPG_TMP, "w" );
  Err = fopen( FILE_EPG_ERR, "w" );
  if( File )
  {
    if( Err )
    {
      if( ( lProviders + CurrentProvider )->DataFormat == DATA_FORMAT_SKYBOX )
      {
        // SKYBOX
        if( ReadFileDictionary() )
        {
          ReadFileThemes();
	  if( nChannels > 0 )
          {
            if( nTitles > 0 )
      	    {
  	      if( nSummaries > 0 )
    	      {
  	        int i;
		int prev_i;
		int EventId;
	        unsigned short int ChannelId;
	        bool IsChannel;
		bool IsEquivChannel;
	        sChannel KeyC, *C;
		sEquivChannel KeyEC, *EC;
	        i = 0;
		prev_i = 0;
		EventId = 1;
	        ChannelId = 0;
	        IsChannel = false;
		IsEquivChannel = false;
	        while( i < nTitles )
	        {
	          sTitle *T = ( lTitles + i );
	          if( ChannelId != T->ChannelId )
		  {
		    if( IsChannel )
		    {
		      if( ChannelId > 0 )
		      {
		        fprintf( File, "c\n" );
		      }
		    }
		    if( IsEquivChannel )
		    {
		      if( ChannelId > 0 )
		      {
		        fprintf( File, "c\n" );
		      }
		      i = prev_i;
		      T = ( lTitles + i );
		    }
		    IsChannel = false;
		    IsEquivChannel = false;
		    KeyC.ChannelId = T->ChannelId;
		    C = ( sChannel * ) bsearch( &KeyC, lChannels, nChannels, sizeof( sChannel ), &bsearchChannelByChannelId );
		    if( C )
		    {
		      C->IsEpg = true;
		      tChannelID ChVID = tChannelID( ( lProviders + CurrentProvider )->SourceId, C->Nid, C->Tid, C->Sid );
#ifdef STANDALONE
		      cChannel *VC = NULL;
#else
		      cChannel *VC = Channels.GetByChannelID( ChVID, true );
		      if( VC )
#endif
		      {
#ifdef STANDALONE
		        KeyEC.OriginalSourceId = C->ChannelId;
			KeyEC.OriginalNid = C->Nid;
			KeyEC.OriginalTid = C->Tid;
			KeyEC.OriginalSid = C->Sid;
#else
		        KeyEC.OriginalSourceId = VC->Source();
			KeyEC.OriginalNid = VC->Nid();
			KeyEC.OriginalTid = VC->Tid();
			KeyEC.OriginalSid = VC->Sid();
#endif
			EC = ( sEquivChannel * ) bsearch( &KeyEC, lEquivChannels, nEquivChannels, sizeof( sEquivChannel ), &bsearchEquivChannel );
			if( EC && Config->UseFileEquivalents )
			{
			  tChannelID ChEID = tChannelID( EC->EquivSourceId, EC->EquivNid, EC->EquivTid, EC->EquivSid, EC->EquivRid );
#ifdef STANDALONE
			  cChannel *VEC = NULL;
#else
			  cChannel *VEC = Channels.GetByChannelID( ChEID, false );
#endif
			  if( VEC )
			  {
			    fprintf( File, "<se>C %s-%i-%i-%i-%i name=\"%s\"\n", *cSource::ToString( VEC->Source() ), VEC->Nid(), VEC->Tid(), VEC->Sid(), VEC->Rid(), VEC->Name() );
			    asprintf( &ChannelName, "%s", VEC->Name() );
		            IsEquivChannel = true;
			    prev_i = i;
			  }
			  EC->OriginalSourceId = 0;
			  EC->OriginalNid = 0;
			  EC->OriginalTid = 0;
			  EC->OriginalSid = 0;
			  qsort( lEquivChannels, nEquivChannels, sizeof( sEquivChannel ), &qsortEquivChannels );
			}
			else
			{
			  //fprintf( File, "C %s-%i-%i-%i-%i %s\n", *cSource::ToString( VC->Source() ), VC->Nid(), VC->Tid(), VC->Sid(), VC->Rid(), VC->Name() );
			  fprintf( File, "<sky>C %i-%i-%i-%i sky=%i\n", C->ChannelId, C->Nid, C->Tid, C->Sid, C->SkyNumber );
			  //asprintf( &ChannelName, "%s", VC->Name() );
			  asprintf( &ChannelName, "%i", C->SkyNumber );
		          IsChannel = true;
			  C->IsFound = true;
			}
		      }
		    }
		    ChannelId = T->ChannelId;
	          }
		  if( IsChannel || IsEquivChannel )
		  {
		    fprintf( File, "E %u %u %u\n", EventId, ( T->StartTime + EpgTimeOffset ), T->Duration );
		    if( DecodeHuffmanCode( &bTitles[T->pData], T->lenData ) )
		    {
		      CleanString( DecodeText );
		      fprintf( File, "T %s\n", DecodeText );
		      if( DecodeErrorText[0] != '\0' )
		      {
		        if( IsChannel )
			{
		          time_t StartTime;
			  StartTime = ( T->StartTime + EpgTimeOffset );
		          fprintf( Err, "Channel: %s - %s\n", ChannelName, ctime( &StartTime ) );
		          fprintf( Err, "T %s%s\n\n", DecodeText, DecodeErrorText );
			}
		      }
		      if( DEBUG && DEBUG_STARTTIME )
		      {
	                time_t StartTime;
	                char *DateTime;
	                StartTime = ( T->StartTime + EpgTimeOffset );
	                asprintf( &DateTime, "%s", ctime( &StartTime ) );
			fprintf( File, "S - %s", DateTime );
		      }
		      sTheme *ST = ( lThemes + T->ThemeId );
		      if( ST->Name[0] != '\0' )
		      {
		        fprintf( File, "S %s\n", ST->Name );
		      }
		      else
		      {
		        if( DEBUG )
			{
			  time_t StartTime;
			  StartTime = ( T->StartTime + EpgTimeOffset );
			  fprintf( Err, "Channel: %s - %s - ThemeId=0x%02x\n", ChannelName, ctime( &StartTime ), T->ThemeId );
			}
		      }
		      sSummary KeyS, *S;
		      KeyS.ChannelId = T->ChannelId;
		      KeyS.MjdTime = T->MjdTime;
		      KeyS.EventId = T->EventId;
		      S = ( sSummary * ) bsearch( &KeyS, lSummaries, nSummaries, sizeof( sSummary ), &bsearchSummarie );
		      if( S )
		      {
		        if( DecodeHuffmanCode( &bSummaries[S->pData], S->lenData ) )
		        {
		          CleanString( DecodeText );
			  fprintf( File, "D %s\n", DecodeText );
		        }
			if( DecodeErrorText[0] != '\0' )
			{
			  if( IsChannel )
			  {
			    time_t StartTime;
			    StartTime = ( T->StartTime + EpgTimeOffset );
			    fprintf( Err, "Channel: %s - %s\n", ChannelName, ctime( &StartTime ) );
			    fprintf( Err, "D %s%s\n\n", DecodeText, DecodeErrorText );
			  }
			}
		      }
		    }
		    fprintf( File, "e\n" );
		  }
	          i ++;
		  EventId ++;
	        }
	        if( IsChannel || IsEquivChannel )
	        {
	          fprintf( File, "c\n" );
	        }
	      }
	    }
          }
        }
      }
      else
      {
        // MHW_1 && MHW_2
	int i;
	int prev_i;
	unsigned char ChannelId;
	sChannel KeyC, *C;
	sEquivChannel KeyEC, *EC;
	bool IsChannel;
	bool IsEquivChannel;
	i = 0;
	prev_i = 0;
	ChannelId = 0xff;
	IsChannel = false;
	IsEquivChannel = false;
	while( i < nTitles )
	{
	  sTitle *T = ( lTitles + i );
	  if( ChannelId != T->ChannelId )
	  {
	    if( IsChannel )
	    {
	      if( ChannelId < 0xff )
	      {
	        fprintf( File, "c\n" );
	      }
	    }
	    if( IsEquivChannel )
	    {
	      if( ChannelId > 0 )
	      {
	        fprintf( File, "c\n" );
	      }
	      i = prev_i;
	      T = ( lTitles + i );
	    }
	    IsChannel = false;
	    IsEquivChannel = false;
	    KeyC.ChannelId = T->ChannelId;
	    C = ( sChannel * ) bsearch( &KeyC, lChannels, nChannels, sizeof( sChannel ), &bsearchChannelByChannelId );
	    if( C )
	    {
	      tChannelID ChVID = tChannelID( ( lProviders + CurrentProvider )->SourceId, C->Nid, C->Tid, C->Sid );
#ifdef STANDALONE
	      cChannel *VC = NULL;
#else
	      cChannel *VC = Channels.GetByChannelID( ChVID, false );
#endif
	      if( VC )
	      {
                KeyEC.OriginalSourceId = VC->Source();
		KeyEC.OriginalNid = VC->Nid();
		KeyEC.OriginalTid = VC->Tid();
		KeyEC.OriginalSid = VC->Sid();
		EC = ( sEquivChannel * ) bsearch( &KeyEC, lEquivChannels, nEquivChannels, sizeof( sEquivChannel ), &bsearchEquivChannel );
		if( EC && Config->UseFileEquivalents )
		{
		  tChannelID ChEID = tChannelID( EC->EquivSourceId, EC->EquivNid, EC->EquivTid, EC->EquivSid, EC->EquivRid );
#ifdef STANDALONE
		  cChannel *VEC = NULL;
#else
		  cChannel *VEC = Channels.GetByChannelID( ChEID, false );
#endif
		  if( VEC )
		  {
		    fprintf( File, "<mhw>C %s-%i-%i-%i-%i %s\n", *cSource::ToString( VEC->Source() ), VEC->Nid(), VEC->Tid(), VEC->Sid(), VEC->Rid(), VEC->Name() );
		    asprintf( &ChannelName, "%s", VEC->Name() );
		    IsEquivChannel = true;
		    prev_i = i;
		  }
		  EC->OriginalSourceId = 0;
		  EC->OriginalNid = 0;
		  EC->OriginalTid = 0;
		  EC->OriginalSid = 0;
		  qsort( lEquivChannels, nEquivChannels, sizeof( sEquivChannel ), &qsortEquivChannels );
		}
		else
		{
		  fprintf( File, "<mhw>C %s-%i-%i-%i-%i %s\n", *cSource::ToString( VC->Source() ), VC->Nid(), VC->Tid(), VC->Sid(), VC->Rid(), VC->Name() );
		  asprintf( &ChannelName, "%s", VC->Name() );
		  IsChannel = true;
		  C->IsFound = true;
		}
	      }
	    }
	    ChannelId = T->ChannelId;
	  }
	  if( IsChannel || IsEquivChannel )
	  {
	    fprintf( File, "E %u %u %u 01 FF\n", T->EventId, ( T->StartTime + EpgTimeOffset ), T->Duration );
	    fprintf( File, "T %s\n", &bTitles[T->pData] );
	    if( ( lThemes + T->ThemeId )->Name[0] != '\0' )
	    {
	      if( DEBUG && DEBUG_STARTTIME )
	      {
	        time_t StartTime;
	        char *DateTime;
	        StartTime = ( T->StartTime + EpgTimeOffset );
	        asprintf( &DateTime, "%s", ctime( &StartTime ) );
	        fprintf( File, "S %s - %d\' - %s", ( lThemes + T->ThemeId )->Name, T->Duration / 60, DateTime );
	      }
	      else
	      {
	        fprintf( File, "S %s - %d\'\n", ( lThemes + T->ThemeId )->Name, T->Duration / 60 );
	      }
	    }
	    sSummary KeyS, *S;
	    KeyS.ChannelId = 0;
	    KeyS.MjdTime = 0;
	    KeyS.EventId = T->EventId;
	    S = ( sSummary * ) bsearch( &KeyS, lSummaries, nSummaries, sizeof( sSummary ), &bsearchSummarie );
	    if( S )
	    {
	      fprintf( File, "D %s\n", &bSummaries[S->pData] );
	    }
	    fprintf( File, "e\n" );
	  }
	  i ++;
        }
	if(  IsChannel || IsEquivChannel )
	{
	  fprintf( File, "c\n" );
	}
      }
      fclose( File );
      fclose( Err );
#ifndef STANDALONE
      LoadFromFile( FILE_EPG_TMP );
#endif
      CreateFileChannels( FILE_EPG_CHANNELS );
    }
    else
    {
      esyslog( "LoadEPG: Error, can't open file '%s', %s", FILE_EPG_ERR, strerror( errno ) );
    }
  }
  else
  {
    esyslog( "LoadEPG: Error, can't open file '%s', %s", FILE_EPG_TMP, strerror( errno ) );
  }
};
/// }}}
// }}}

// EPGGrabber {{{
EPGGrabber::EPGGrabber()
{
  Config = new sConfig();
  asprintf( &Config->Directory, "%s", conf );
  Config->DvbAdapterNumber = 0;
  Config->DvbAdapterHasRotor = 0;
  Config->UseFileEquivalents = false;
  Config->EnableOsdMessages = false;
  CurrentProvider = 0;
  nProviders = 0;
  nEquivChannels = 0;
  lProviders = NULL;
  lEquivChannels = NULL;
  //Control = NULL;
  Task = NULL;
}

EPGGrabber::~EPGGrabber()
{
  // Clean up after yourself!
  if( Config )
  {
    if( Config->Directory )
    {
      free( Config->Directory );
    }
    free( Config );
  }
  if( lProviders )
  {
    for( int i = 0; i < nProviders; i ++ )
    {
      if( ( lProviders + i )->Title )
      {
        free( ( lProviders + i )->Title );
      }
      if( ( lProviders + i )->Parm1 )
      {
        free( ( lProviders + i )->Parm1 );
      }
      if( ( lProviders + i )->Parm2 )
      {
        free( ( lProviders + i )->Parm2 );
      }
    }
    free( lProviders );
  }
  if( lEquivChannels )
  {
    free( lEquivChannels );
  }
#if 0
  if( Control )
  {
    delete( Control );
  }
#endif
  if( Task )
  {
    delete( Task );
  }
}
// }}}

// GrabEPG {{{
void EPGGrabber::Grab()
{
  ReadConfigLoadepg();
  Task = new cTaskLoadepg();
  if( Task )
  {
    Task->Start();
    time_t starttime = time(NULL);
    while ( Task->Active() )
    {
      usleep(1000000);
      time_t now = time(NULL);
      time_t diff = now - starttime;
      if (Task->IsLoopRunning())
      {
#ifdef DEBUG
	Dprintf("%d found %d channels %d titles %d summaries\n", diff, nChannels, nTitles, nSummaries);
#else
	Dprintf("\r%d found %d channels %d titles %d summaries\r", diff, nChannels, nTitles, nSummaries);
#endif
      }
      if (diff >= 60)
      {
	Dprintf("timed out\n");
	break;
      }
    }

    Task->Stop();
    // stop load epg
    delete( Task );
    Task = NULL;
  }
}

// }}}

// vim: foldmethod=marker ts=8 sw=2
