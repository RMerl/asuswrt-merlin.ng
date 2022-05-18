/**
 ** Simple entropy harvester based upon the havege RNG
 **
 ** Copyright 2018-2022 Jirka Hladky hladky DOT jiri AT gmail DOT com
 ** Copyright 2009-2013 Gary Wuertz gary@issiweb.com
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 ** This source is an adaptation of work released as
 **
 ** Copyright (C) 2006 - André Seznec - Olivier Rochecouste
 **
 ** under version 2.1 of the GNU Lesser General Public License
 **
 ** The original form is retained with minor variable renames for
 ** more consistent macro itilization. See havegecollect.c for
 ** details.
 */

/* ------------------------------------------------------------------------
 * On average, one iteration accesses two 8-word blocks in the PWALK
 * table, and generates 16 words in the RESULT array.
 *
 * The data read in the Walk table are updated and permuted after each use.
 * The result of the hardware clock counter read is used for this update.
 *
 * 21 conditional tests are present. The conditional tests are grouped in
 * two nested  groups of 10 conditional tests and 1 test that controls the
 * permutation.
 *
 * In average, there should be 4 tests executed and, in average, 2 of them
 * should be mispredicted.
 * ------------------------------------------------------------------------
 */

  PTTEST = PT >> 20;

  if (PTTEST & 1) {
    PTTEST ^= 3; PTTEST = PTTEST >> 1;
    if (PTTEST & 1) {
      PTTEST ^= 3; PTTEST = PTTEST >> 1;
      if (PTTEST & 1) {
        PTTEST ^= 3; PTTEST = PTTEST >> 1;
        if (PTTEST & 1) {
          PTTEST ^= 3; PTTEST = PTTEST >> 1;
          if (PTTEST & 1) {
            PTTEST ^= 3; PTTEST = PTTEST >> 1;
            if (PTTEST & 1) {
              PTTEST ^= 3; PTTEST = PTTEST >> 1;
              if (PTTEST & 1) {
                PTTEST ^= 3; PTTEST = PTTEST >> 1;
                if (PTTEST & 1) {
                  PTTEST ^= 3; PTTEST = PTTEST >> 1;
                  if (PTTEST & 1) {
                    PTTEST ^= 3; PTTEST = PTTEST >> 1;
                    if (PTTEST & 1) {
                      PTTEST ^= 3; PTTEST = PTTEST >> 1;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  };

  PTTEST = PTTEST >> 1;
  pt = (PT >> 18) & 7;

  PT = PT & ANDPT;

  HARDCLOCKR(HTICK1);

  Pt0 = &PWALK[PT];
  Pt1 = &PWALK[PT2];
  Pt2 = &PWALK[PT  ^ 1];
  Pt3 = &PWALK[PT2 ^ 4];

  RESULT[i++] ^= *Pt0;
  RESULT[i++] ^= *Pt1;
  RESULT[i++] ^= *Pt2;
  RESULT[i++] ^= *Pt3;

  inter = ROR32(*Pt0,1) ^ HTICK1;
  *Pt0  = ROR32(*Pt1,2) ^ HTICK1;
  *Pt1  = inter;
  *Pt2  = ROR32(*Pt2, 3) ^ HTICK1;
  *Pt3  = ROR32(*Pt3, 4) ^ HTICK1;

  Pt0 = &PWALK[PT  ^ 2];
  Pt1 = &PWALK[PT2 ^ 2];
  Pt2 = &PWALK[PT  ^ 3];
  Pt3 = &PWALK[PT2 ^ 6];

  RESULT[i++] ^= *Pt0;
  RESULT[i++] ^= *Pt1;
  RESULT[i++] ^= *Pt2;
  RESULT[i++] ^= *Pt3;

  if (PTTEST & 1) {
    Ptinter = Pt0;
    Pt2 = Pt0;
    Pt0 = Ptinter;
  }

  PTTEST = (PT2 >> 18);
  inter  = ROR32(*Pt0, 5) ^ HTICK1;
  *Pt0   = ROR32(*Pt1, 6) ^ HTICK1;
  *Pt1   = inter;

  HARDCLOCKR(HTICK2);

  *Pt2 = ROR32(*Pt2, 7) ^ HTICK2;
  *Pt3 = ROR32(*Pt3, 8) ^ HTICK2;

  Pt0 = &PWALK[PT  ^ 4];
  Pt1 = &PWALK[PT2 ^ 1];

  PT2 = (RESULT[(i - 8) ^ PT1] ^ PWALK[PT2 ^ PT1 ^ 7]);
  PT2 = ((PT2 & ANDPT) & (0xfffffff7)) ^ ((PT ^ 8) & 0x8);

  /* avoid PT and PT2 to point on the same cache block */
  PT1 = ((PT2 >> 28) & 7);

  if (PTTEST & 1) {
    PTTEST ^= 3; PTTEST = PTTEST >> 1;
    if (PTTEST & 1) {
      PTTEST ^= 3; PTTEST = PTTEST >> 1;
      if (PTTEST & 1) {
        PTTEST ^= 3; PTTEST = PTTEST >> 1;
        if (PTTEST & 1) {
          PTTEST ^= 3; PTTEST = PTTEST >> 1;
          if (PTTEST & 1) {
            PTTEST ^= 3; PTTEST = PTTEST >> 1;
            if (PTTEST & 1) {
              PTTEST ^= 3; PTTEST = PTTEST >> 1;
              if (PTTEST & 1) {
                PTTEST ^= 3; PTTEST = PTTEST >> 1;
                if (PTTEST & 1) {
                  PTTEST ^= 3; PTTEST = PTTEST >> 1;
                  if (PTTEST & 1) {
                    PTTEST ^= 3; PTTEST = PTTEST >> 1;
                    if (PTTEST & 1) {
                      PTTEST ^= 3; PTTEST = PTTEST >> 1;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  };

  Pt2 = &PWALK[PT  ^ 5];
  Pt3 = &PWALK[PT2 ^ 5];

  RESULT[i++] ^= *Pt0;
  RESULT[i++] ^= *Pt1;
  RESULT[i++] ^= *Pt2;
  RESULT[i++] ^= *Pt3;

  inter = ROR32(*Pt0 , 9) ^ HTICK2;
  *Pt0  = ROR32(*Pt1 , 10) ^ HTICK2;
  *Pt1  = inter;
  *Pt2  = ROR32(*Pt2, 11) ^ HTICK2;
  *Pt3  = ROR32(*Pt3, 12) ^ HTICK2;

  Pt0 = &PWALK[PT  ^ 6];
  Pt1 = &PWALK[PT2 ^ 3];
  Pt2 = &PWALK[PT  ^ 7];
  Pt3 = &PWALK[PT2 ^ 7];

  RESULT[i++] ^= *Pt0;
  RESULT[i++] ^= *Pt1;
  RESULT[i++] ^= *Pt2;
  RESULT[i++] ^= *Pt3;

  inter = ROR32(*Pt0, 13) ^ HTICK2;
  *Pt0  = ROR32(*Pt1, 14) ^ HTICK2;
  *Pt1  = inter;
  *Pt2  = ROR32(*Pt2, 15) ^ HTICK2;
  *Pt3  = ROR32(*Pt3, 16) ^ HTICK2;

  /* avoid PT and PT2 to point on the same cache block */
  PT = (((RESULT[(i - 8) ^ pt] ^ PWALK[PT ^ pt ^ 7])) &
        (0xffffffef)) ^ ((PT2 ^ 0x10) & 0x10);
