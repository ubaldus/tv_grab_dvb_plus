#ifndef __FORMATS_H__
#define __FORMATS_H__

/*
 * formats.h
 *
 * MediaHighway data structures.
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

/*
 * MediaHighWay Ver. 1
 */
typedef struct
{
  u_char Name                   [15];
} sThemeMHW1;

typedef struct
{
  u_char NetworkIdHigh            :8;
  u_char NetworkIdLow             :8;
  u_char TransportIdHigh          :8;
  u_char TransportIdLow           :8;
  u_char ServiceIdHigh            :8;
  u_char ServiceIdLow             :8;
  u_char Name                   [16];
} sChannelMHW1;

typedef struct
{
  u_char TableId                  :8;
#if BYTE_ORDER == BIG_ENDIAN
  u_char SectionSyntaxIndicator   :1;
  u_char                          :1;
  u_char                          :2;
  u_char SectionLengthHigh        :4;
#else
  u_char SectionLengthHigh        :4;
  u_char                          :2;
  u_char                          :1;
  u_char SectionSyntaxIndicator   :1;
#endif
  u_char SectionLengthLow         :8;
  u_char ChannelId                :8;
  u_char ThemeId                  :8;
#if BYTE_ORDER == BIG_ENDIAN
  u_char Day                      :3;
  u_char Hours                    :5;
#else
  u_char Hours                    :5;
  u_char Day                      :3;
#endif
#if BYTE_ORDER == BIG_ENDIAN
  u_char Minutes                  :6;
  u_char                          :1;
  u_char SummaryAvailable         :1;
#else
  u_char SummaryAvailable         :1;
  u_char                          :1;
  u_char Minutes                  :6;
#endif
  u_char                          :8;
  u_char                          :8;
  u_char DurationHigh             :8;
  u_char DurationLow              :8;
  u_char Title                  [23];
  u_char PpvIdHigh                :8;
  u_char PpvIdMediumHigh          :8;
  u_char PpvIdMediumLow           :8;
  u_char PpvIdLow                 :8;
  u_char ProgramIdHigh            :8;
  u_char ProgramIdMediumHigh      :8;
  u_char ProgramIdMediumLow       :8;
  u_char ProgramIdLow             :8;
  u_char                          :8;
  u_char                          :8;
  u_char                          :8;
  u_char                          :8;
} sTitleMHW1;

typedef struct {
  u_char TableId                  :8;
#if BYTE_ORDER == BIG_ENDIAN
  u_char SectionSyntaxIndicator   :1;
  u_char                          :1;
  u_char                          :2;
  u_char SectionLengthHigh        :4;
#else
  u_char SectionLengthHigh        :4;
  u_char                          :2;
  u_char                          :1;
  u_char SectionSyntaxIndicator   :1;
#endif
  u_char SectionLengthLow         :8;
  u_char ProgramIdHigh            :8;
  u_char ProgramIdMediumHigh      :8;
  u_char ProgramIdMediumLow       :8;
  u_char ProgramIdLow             :8;
  u_char Byte7                    :8;
  u_char Byte8                    :8;
  u_char Byte9                    :8;
  u_char NumReplays               :8;
} sSummaryMHW1;


/*
 * MediaHighWay Ver. 2
 */
typedef struct
{
  u_char NetworkIdHigh            :8;
  u_char NetworkIdLow             :8;
  u_char TransportIdHigh          :8;
  u_char TransportIdLow           :8;
  u_char ServiceIdHigh            :8;
  u_char ServiceIdLow             :8;
  u_char                          :8;
  u_char                          :8;
} sChannelMHW2;

#define HILO16( x ) ( ( ( x##High << 8 ) | x##Low ) & 0xffff )
#define HILO32( x ) ( ( ( ( ( x##High << 24 ) | ( x##MediumHigh << 16 ) ) | ( x##MediumLow << 8 ) ) | x##Low ) & 0xffffffff )

#endif
