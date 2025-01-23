/* keccak_permute_32.h - Keccak permute function (simple 32bit bit-interleaved)
 * Copyright (C) 2015 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/* The code is based on public-domain/CC0 "keccakc1024/simple32bi/
 * Keccak-simple32BI.c" implementation by Ronny Van Keer from SUPERCOP toolkit
 * package.
 */

/* Function that computes the Keccak-f[1600] permutation on the given state. */
static unsigned int
KECCAK_F1600_PERMUTE_FUNC_NAME(KECCAK_STATE *hd)
{
  const u32 *round_consts = round_consts_32bit;
  const u32 *round_consts_end = round_consts_32bit + 2 * 24;
  u32 Aba0, Abe0, Abi0, Abo0, Abu0;
  u32 Aba1, Abe1, Abi1, Abo1, Abu1;
  u32 Aga0, Age0, Agi0, Ago0, Agu0;
  u32 Aga1, Age1, Agi1, Ago1, Agu1;
  u32 Aka0, Ake0, Aki0, Ako0, Aku0;
  u32 Aka1, Ake1, Aki1, Ako1, Aku1;
  u32 Ama0, Ame0, Ami0, Amo0, Amu0;
  u32 Ama1, Ame1, Ami1, Amo1, Amu1;
  u32 Asa0, Ase0, Asi0, Aso0, Asu0;
  u32 Asa1, Ase1, Asi1, Aso1, Asu1;
  u32 BCa0, BCe0, BCi0, BCo0, BCu0;
  u32 BCa1, BCe1, BCi1, BCo1, BCu1;
  u32 Da0, De0, Di0, Do0, Du0;
  u32 Da1, De1, Di1, Do1, Du1;
  u32 Eba0, Ebe0, Ebi0, Ebo0, Ebu0;
  u32 Eba1, Ebe1, Ebi1, Ebo1, Ebu1;
  u32 Ega0, Ege0, Egi0, Ego0, Egu0;
  u32 Ega1, Ege1, Egi1, Ego1, Egu1;
  u32 Eka0, Eke0, Eki0, Eko0, Eku0;
  u32 Eka1, Eke1, Eki1, Eko1, Eku1;
  u32 Ema0, Eme0, Emi0, Emo0, Emu0;
  u32 Ema1, Eme1, Emi1, Emo1, Emu1;
  u32 Esa0, Ese0, Esi0, Eso0, Esu0;
  u32 Esa1, Ese1, Esi1, Eso1, Esu1;
  u32 *state = hd->u.state32bi;

  Aba0 = state[0];
  Aba1 = state[1];
  Abe0 = state[2];
  Abe1 = state[3];
  Abi0 = state[4];
  Abi1 = state[5];
  Abo0 = state[6];
  Abo1 = state[7];
  Abu0 = state[8];
  Abu1 = state[9];
  Aga0 = state[10];
  Aga1 = state[11];
  Age0 = state[12];
  Age1 = state[13];
  Agi0 = state[14];
  Agi1 = state[15];
  Ago0 = state[16];
  Ago1 = state[17];
  Agu0 = state[18];
  Agu1 = state[19];
  Aka0 = state[20];
  Aka1 = state[21];
  Ake0 = state[22];
  Ake1 = state[23];
  Aki0 = state[24];
  Aki1 = state[25];
  Ako0 = state[26];
  Ako1 = state[27];
  Aku0 = state[28];
  Aku1 = state[29];
  Ama0 = state[30];
  Ama1 = state[31];
  Ame0 = state[32];
  Ame1 = state[33];
  Ami0 = state[34];
  Ami1 = state[35];
  Amo0 = state[36];
  Amo1 = state[37];
  Amu0 = state[38];
  Amu1 = state[39];
  Asa0 = state[40];
  Asa1 = state[41];
  Ase0 = state[42];
  Ase1 = state[43];
  Asi0 = state[44];
  Asi1 = state[45];
  Aso0 = state[46];
  Aso1 = state[47];
  Asu0 = state[48];
  Asu1 = state[49];

  do
    {
      /* prepareTheta */
      BCa0 = Aba0 ^ Aga0 ^ Aka0 ^ Ama0 ^ Asa0;
      BCa1 = Aba1 ^ Aga1 ^ Aka1 ^ Ama1 ^ Asa1;
      BCe0 = Abe0 ^ Age0 ^ Ake0 ^ Ame0 ^ Ase0;
      BCe1 = Abe1 ^ Age1 ^ Ake1 ^ Ame1 ^ Ase1;
      BCi0 = Abi0 ^ Agi0 ^ Aki0 ^ Ami0 ^ Asi0;
      BCi1 = Abi1 ^ Agi1 ^ Aki1 ^ Ami1 ^ Asi1;
      BCo0 = Abo0 ^ Ago0 ^ Ako0 ^ Amo0 ^ Aso0;
      BCo1 = Abo1 ^ Ago1 ^ Ako1 ^ Amo1 ^ Aso1;
      BCu0 = Abu0 ^ Agu0 ^ Aku0 ^ Amu0 ^ Asu0;
      BCu1 = Abu1 ^ Agu1 ^ Aku1 ^ Amu1 ^ Asu1;

      /* thetaRhoPiChiIota(round  , A, E) */
      Da0 = BCu0 ^ ROL32(BCe1, 1);
      Da1 = BCu1 ^ BCe0;
      De0 = BCa0 ^ ROL32(BCi1, 1);
      De1 = BCa1 ^ BCi0;
      Di0 = BCe0 ^ ROL32(BCo1, 1);
      Di1 = BCe1 ^ BCo0;
      Do0 = BCi0 ^ ROL32(BCu1, 1);
      Do1 = BCi1 ^ BCu0;
      Du0 = BCo0 ^ ROL32(BCa1, 1);
      Du1 = BCo1 ^ BCa0;

      Aba0 ^= Da0;
      BCa0 = Aba0;
      Age0 ^= De0;
      BCe0 = ROL32(Age0, 22);
      Aki1 ^= Di1;
      BCi0 = ROL32(Aki1, 22);
      Amo1 ^= Do1;
      BCo0 = ROL32(Amo1, 11);
      Asu0 ^= Du0;
      BCu0 = ROL32(Asu0, 7);
      Eba0 = BCa0 ^ ANDN32(BCe0, BCi0);
      Eba0 ^= *(round_consts++);
      Ebe0 = BCe0 ^ ANDN32(BCi0, BCo0);
      Ebi0 = BCi0 ^ ANDN32(BCo0, BCu0);
      Ebo0 = BCo0 ^ ANDN32(BCu0, BCa0);
      Ebu0 = BCu0 ^ ANDN32(BCa0, BCe0);

      Aba1 ^= Da1;
      BCa1 = Aba1;
      Age1 ^= De1;
      BCe1 = ROL32(Age1, 22);
      Aki0 ^= Di0;
      BCi1 = ROL32(Aki0, 21);
      Amo0 ^= Do0;
      BCo1 = ROL32(Amo0, 10);
      Asu1 ^= Du1;
      BCu1 = ROL32(Asu1, 7);
      Eba1 = BCa1 ^ ANDN32(BCe1, BCi1);
      Eba1 ^= *(round_consts++);
      Ebe1 = BCe1 ^ ANDN32(BCi1, BCo1);
      Ebi1 = BCi1 ^ ANDN32(BCo1, BCu1);
      Ebo1 = BCo1 ^ ANDN32(BCu1, BCa1);
      Ebu1 = BCu1 ^ ANDN32(BCa1, BCe1);

      Abo0 ^= Do0;
      BCa0 = ROL32(Abo0, 14);
      Agu0 ^= Du0;
      BCe0 = ROL32(Agu0, 10);
      Aka1 ^= Da1;
      BCi0 = ROL32(Aka1, 2);
      Ame1 ^= De1;
      BCo0 = ROL32(Ame1, 23);
      Asi1 ^= Di1;
      BCu0 = ROL32(Asi1, 31);
      Ega0 = BCa0 ^ ANDN32(BCe0, BCi0);
      Ege0 = BCe0 ^ ANDN32(BCi0, BCo0);
      Egi0 = BCi0 ^ ANDN32(BCo0, BCu0);
      Ego0 = BCo0 ^ ANDN32(BCu0, BCa0);
      Egu0 = BCu0 ^ ANDN32(BCa0, BCe0);

      Abo1 ^= Do1;
      BCa1 = ROL32(Abo1, 14);
      Agu1 ^= Du1;
      BCe1 = ROL32(Agu1, 10);
      Aka0 ^= Da0;
      BCi1 = ROL32(Aka0, 1);
      Ame0 ^= De0;
      BCo1 = ROL32(Ame0, 22);
      Asi0 ^= Di0;
      BCu1 = ROL32(Asi0, 30);
      Ega1 = BCa1 ^ ANDN32(BCe1, BCi1);
      Ege1 = BCe1 ^ ANDN32(BCi1, BCo1);
      Egi1 = BCi1 ^ ANDN32(BCo1, BCu1);
      Ego1 = BCo1 ^ ANDN32(BCu1, BCa1);
      Egu1 = BCu1 ^ ANDN32(BCa1, BCe1);

      Abe1 ^= De1;
      BCa0 = ROL32(Abe1, 1);
      Agi0 ^= Di0;
      BCe0 = ROL32(Agi0, 3);
      Ako1 ^= Do1;
      BCi0 = ROL32(Ako1, 13);
      Amu0 ^= Du0;
      BCo0 = ROL32(Amu0, 4);
      Asa0 ^= Da0;
      BCu0 = ROL32(Asa0, 9);
      Eka0 = BCa0 ^ ANDN32(BCe0, BCi0);
      Eke0 = BCe0 ^ ANDN32(BCi0, BCo0);
      Eki0 = BCi0 ^ ANDN32(BCo0, BCu0);
      Eko0 = BCo0 ^ ANDN32(BCu0, BCa0);
      Eku0 = BCu0 ^ ANDN32(BCa0, BCe0);

      Abe0 ^= De0;
      BCa1 = Abe0;
      Agi1 ^= Di1;
      BCe1 = ROL32(Agi1, 3);
      Ako0 ^= Do0;
      BCi1 = ROL32(Ako0, 12);
      Amu1 ^= Du1;
      BCo1 = ROL32(Amu1, 4);
      Asa1 ^= Da1;
      BCu1 = ROL32(Asa1, 9);
      Eka1 = BCa1 ^ ANDN32(BCe1, BCi1);
      Eke1 = BCe1 ^ ANDN32(BCi1, BCo1);
      Eki1 = BCi1 ^ ANDN32(BCo1, BCu1);
      Eko1 = BCo1 ^ ANDN32(BCu1, BCa1);
      Eku1 = BCu1 ^ ANDN32(BCa1, BCe1);

      Abu1 ^= Du1;
      BCa0 = ROL32(Abu1, 14);
      Aga0 ^= Da0;
      BCe0 = ROL32(Aga0, 18);
      Ake0 ^= De0;
      BCi0 = ROL32(Ake0, 5);
      Ami1 ^= Di1;
      BCo0 = ROL32(Ami1, 8);
      Aso0 ^= Do0;
      BCu0 = ROL32(Aso0, 28);
      Ema0 = BCa0 ^ ANDN32(BCe0, BCi0);
      Eme0 = BCe0 ^ ANDN32(BCi0, BCo0);
      Emi0 = BCi0 ^ ANDN32(BCo0, BCu0);
      Emo0 = BCo0 ^ ANDN32(BCu0, BCa0);
      Emu0 = BCu0 ^ ANDN32(BCa0, BCe0);

      Abu0 ^= Du0;
      BCa1 = ROL32(Abu0, 13);
      Aga1 ^= Da1;
      BCe1 = ROL32(Aga1, 18);
      Ake1 ^= De1;
      BCi1 = ROL32(Ake1, 5);
      Ami0 ^= Di0;
      BCo1 = ROL32(Ami0, 7);
      Aso1 ^= Do1;
      BCu1 = ROL32(Aso1, 28);
      Ema1 = BCa1 ^ ANDN32(BCe1, BCi1);
      Eme1 = BCe1 ^ ANDN32(BCi1, BCo1);
      Emi1 = BCi1 ^ ANDN32(BCo1, BCu1);
      Emo1 = BCo1 ^ ANDN32(BCu1, BCa1);
      Emu1 = BCu1 ^ ANDN32(BCa1, BCe1);

      Abi0 ^= Di0;
      BCa0 = ROL32(Abi0, 31);
      Ago1 ^= Do1;
      BCe0 = ROL32(Ago1, 28);
      Aku1 ^= Du1;
      BCi0 = ROL32(Aku1, 20);
      Ama1 ^= Da1;
      BCo0 = ROL32(Ama1, 21);
      Ase0 ^= De0;
      BCu0 = ROL32(Ase0, 1);
      Esa0 = BCa0 ^ ANDN32(BCe0, BCi0);
      Ese0 = BCe0 ^ ANDN32(BCi0, BCo0);
      Esi0 = BCi0 ^ ANDN32(BCo0, BCu0);
      Eso0 = BCo0 ^ ANDN32(BCu0, BCa0);
      Esu0 = BCu0 ^ ANDN32(BCa0, BCe0);

      Abi1 ^= Di1;
      BCa1 = ROL32(Abi1, 31);
      Ago0 ^= Do0;
      BCe1 = ROL32(Ago0, 27);
      Aku0 ^= Du0;
      BCi1 = ROL32(Aku0, 19);
      Ama0 ^= Da0;
      BCo1 = ROL32(Ama0, 20);
      Ase1 ^= De1;
      BCu1 = ROL32(Ase1, 1);
      Esa1 = BCa1 ^ ANDN32(BCe1, BCi1);
      Ese1 = BCe1 ^ ANDN32(BCi1, BCo1);
      Esi1 = BCi1 ^ ANDN32(BCo1, BCu1);
      Eso1 = BCo1 ^ ANDN32(BCu1, BCa1);
      Esu1 = BCu1 ^ ANDN32(BCa1, BCe1);

      /* prepareTheta */
      BCa0 = Eba0 ^ Ega0 ^ Eka0 ^ Ema0 ^ Esa0;
      BCa1 = Eba1 ^ Ega1 ^ Eka1 ^ Ema1 ^ Esa1;
      BCe0 = Ebe0 ^ Ege0 ^ Eke0 ^ Eme0 ^ Ese0;
      BCe1 = Ebe1 ^ Ege1 ^ Eke1 ^ Eme1 ^ Ese1;
      BCi0 = Ebi0 ^ Egi0 ^ Eki0 ^ Emi0 ^ Esi0;
      BCi1 = Ebi1 ^ Egi1 ^ Eki1 ^ Emi1 ^ Esi1;
      BCo0 = Ebo0 ^ Ego0 ^ Eko0 ^ Emo0 ^ Eso0;
      BCo1 = Ebo1 ^ Ego1 ^ Eko1 ^ Emo1 ^ Eso1;
      BCu0 = Ebu0 ^ Egu0 ^ Eku0 ^ Emu0 ^ Esu0;
      BCu1 = Ebu1 ^ Egu1 ^ Eku1 ^ Emu1 ^ Esu1;

      /* thetaRhoPiChiIota(round+1, E, A) */
      Da0 = BCu0 ^ ROL32(BCe1, 1);
      Da1 = BCu1 ^ BCe0;
      De0 = BCa0 ^ ROL32(BCi1, 1);
      De1 = BCa1 ^ BCi0;
      Di0 = BCe0 ^ ROL32(BCo1, 1);
      Di1 = BCe1 ^ BCo0;
      Do0 = BCi0 ^ ROL32(BCu1, 1);
      Do1 = BCi1 ^ BCu0;
      Du0 = BCo0 ^ ROL32(BCa1, 1);
      Du1 = BCo1 ^ BCa0;

      Eba0 ^= Da0;
      BCa0 = Eba0;
      Ege0 ^= De0;
      BCe0 = ROL32(Ege0, 22);
      Eki1 ^= Di1;
      BCi0 = ROL32(Eki1, 22);
      Emo1 ^= Do1;
      BCo0 = ROL32(Emo1, 11);
      Esu0 ^= Du0;
      BCu0 = ROL32(Esu0, 7);
      Aba0 = BCa0 ^ ANDN32(BCe0, BCi0);
      Aba0 ^= *(round_consts++);
      Abe0 = BCe0 ^ ANDN32(BCi0, BCo0);
      Abi0 = BCi0 ^ ANDN32(BCo0, BCu0);
      Abo0 = BCo0 ^ ANDN32(BCu0, BCa0);
      Abu0 = BCu0 ^ ANDN32(BCa0, BCe0);

      Eba1 ^= Da1;
      BCa1 = Eba1;
      Ege1 ^= De1;
      BCe1 = ROL32(Ege1, 22);
      Eki0 ^= Di0;
      BCi1 = ROL32(Eki0, 21);
      Emo0 ^= Do0;
      BCo1 = ROL32(Emo0, 10);
      Esu1 ^= Du1;
      BCu1 = ROL32(Esu1, 7);
      Aba1 = BCa1 ^ ANDN32(BCe1, BCi1);
      Aba1 ^= *(round_consts++);
      Abe1 = BCe1 ^ ANDN32(BCi1, BCo1);
      Abi1 = BCi1 ^ ANDN32(BCo1, BCu1);
      Abo1 = BCo1 ^ ANDN32(BCu1, BCa1);
      Abu1 = BCu1 ^ ANDN32(BCa1, BCe1);

      Ebo0 ^= Do0;
      BCa0 = ROL32(Ebo0, 14);
      Egu0 ^= Du0;
      BCe0 = ROL32(Egu0, 10);
      Eka1 ^= Da1;
      BCi0 = ROL32(Eka1, 2);
      Eme1 ^= De1;
      BCo0 = ROL32(Eme1, 23);
      Esi1 ^= Di1;
      BCu0 = ROL32(Esi1, 31);
      Aga0 = BCa0 ^ ANDN32(BCe0, BCi0);
      Age0 = BCe0 ^ ANDN32(BCi0, BCo0);
      Agi0 = BCi0 ^ ANDN32(BCo0, BCu0);
      Ago0 = BCo0 ^ ANDN32(BCu0, BCa0);
      Agu0 = BCu0 ^ ANDN32(BCa0, BCe0);

      Ebo1 ^= Do1;
      BCa1 = ROL32(Ebo1, 14);
      Egu1 ^= Du1;
      BCe1 = ROL32(Egu1, 10);
      Eka0 ^= Da0;
      BCi1 = ROL32(Eka0, 1);
      Eme0 ^= De0;
      BCo1 = ROL32(Eme0, 22);
      Esi0 ^= Di0;
      BCu1 = ROL32(Esi0, 30);
      Aga1 = BCa1 ^ ANDN32(BCe1, BCi1);
      Age1 = BCe1 ^ ANDN32(BCi1, BCo1);
      Agi1 = BCi1 ^ ANDN32(BCo1, BCu1);
      Ago1 = BCo1 ^ ANDN32(BCu1, BCa1);
      Agu1 = BCu1 ^ ANDN32(BCa1, BCe1);

      Ebe1 ^= De1;
      BCa0 = ROL32(Ebe1, 1);
      Egi0 ^= Di0;
      BCe0 = ROL32(Egi0, 3);
      Eko1 ^= Do1;
      BCi0 = ROL32(Eko1, 13);
      Emu0 ^= Du0;
      BCo0 = ROL32(Emu0, 4);
      Esa0 ^= Da0;
      BCu0 = ROL32(Esa0, 9);
      Aka0 = BCa0 ^ ANDN32(BCe0, BCi0);
      Ake0 = BCe0 ^ ANDN32(BCi0, BCo0);
      Aki0 = BCi0 ^ ANDN32(BCo0, BCu0);
      Ako0 = BCo0 ^ ANDN32(BCu0, BCa0);
      Aku0 = BCu0 ^ ANDN32(BCa0, BCe0);

      Ebe0 ^= De0;
      BCa1 = Ebe0;
      Egi1 ^= Di1;
      BCe1 = ROL32(Egi1, 3);
      Eko0 ^= Do0;
      BCi1 = ROL32(Eko0, 12);
      Emu1 ^= Du1;
      BCo1 = ROL32(Emu1, 4);
      Esa1 ^= Da1;
      BCu1 = ROL32(Esa1, 9);
      Aka1 = BCa1 ^ ANDN32(BCe1, BCi1);
      Ake1 = BCe1 ^ ANDN32(BCi1, BCo1);
      Aki1 = BCi1 ^ ANDN32(BCo1, BCu1);
      Ako1 = BCo1 ^ ANDN32(BCu1, BCa1);
      Aku1 = BCu1 ^ ANDN32(BCa1, BCe1);

      Ebu1 ^= Du1;
      BCa0 = ROL32(Ebu1, 14);
      Ega0 ^= Da0;
      BCe0 = ROL32(Ega0, 18);
      Eke0 ^= De0;
      BCi0 = ROL32(Eke0, 5);
      Emi1 ^= Di1;
      BCo0 = ROL32(Emi1, 8);
      Eso0 ^= Do0;
      BCu0 = ROL32(Eso0, 28);
      Ama0 = BCa0 ^ ANDN32(BCe0, BCi0);
      Ame0 = BCe0 ^ ANDN32(BCi0, BCo0);
      Ami0 = BCi0 ^ ANDN32(BCo0, BCu0);
      Amo0 = BCo0 ^ ANDN32(BCu0, BCa0);
      Amu0 = BCu0 ^ ANDN32(BCa0, BCe0);

      Ebu0 ^= Du0;
      BCa1 = ROL32(Ebu0, 13);
      Ega1 ^= Da1;
      BCe1 = ROL32(Ega1, 18);
      Eke1 ^= De1;
      BCi1 = ROL32(Eke1, 5);
      Emi0 ^= Di0;
      BCo1 = ROL32(Emi0, 7);
      Eso1 ^= Do1;
      BCu1 = ROL32(Eso1, 28);
      Ama1 = BCa1 ^ ANDN32(BCe1, BCi1);
      Ame1 = BCe1 ^ ANDN32(BCi1, BCo1);
      Ami1 = BCi1 ^ ANDN32(BCo1, BCu1);
      Amo1 = BCo1 ^ ANDN32(BCu1, BCa1);
      Amu1 = BCu1 ^ ANDN32(BCa1, BCe1);

      Ebi0 ^= Di0;
      BCa0 = ROL32(Ebi0, 31);
      Ego1 ^= Do1;
      BCe0 = ROL32(Ego1, 28);
      Eku1 ^= Du1;
      BCi0 = ROL32(Eku1, 20);
      Ema1 ^= Da1;
      BCo0 = ROL32(Ema1, 21);
      Ese0 ^= De0;
      BCu0 = ROL32(Ese0, 1);
      Asa0 = BCa0 ^ ANDN32(BCe0, BCi0);
      Ase0 = BCe0 ^ ANDN32(BCi0, BCo0);
      Asi0 = BCi0 ^ ANDN32(BCo0, BCu0);
      Aso0 = BCo0 ^ ANDN32(BCu0, BCa0);
      Asu0 = BCu0 ^ ANDN32(BCa0, BCe0);

      Ebi1 ^= Di1;
      BCa1 = ROL32(Ebi1, 31);
      Ego0 ^= Do0;
      BCe1 = ROL32(Ego0, 27);
      Eku0 ^= Du0;
      BCi1 = ROL32(Eku0, 19);
      Ema0 ^= Da0;
      BCo1 = ROL32(Ema0, 20);
      Ese1 ^= De1;
      BCu1 = ROL32(Ese1, 1);
      Asa1 = BCa1 ^ ANDN32(BCe1, BCi1);
      Ase1 = BCe1 ^ ANDN32(BCi1, BCo1);
      Asi1 = BCi1 ^ ANDN32(BCo1, BCu1);
      Aso1 = BCo1 ^ ANDN32(BCu1, BCa1);
      Asu1 = BCu1 ^ ANDN32(BCa1, BCe1);
    }
  while (round_consts < round_consts_end);

  state[0] = Aba0;
  state[1] = Aba1;
  state[2] = Abe0;
  state[3] = Abe1;
  state[4] = Abi0;
  state[5] = Abi1;
  state[6] = Abo0;
  state[7] = Abo1;
  state[8] = Abu0;
  state[9] = Abu1;
  state[10] = Aga0;
  state[11] = Aga1;
  state[12] = Age0;
  state[13] = Age1;
  state[14] = Agi0;
  state[15] = Agi1;
  state[16] = Ago0;
  state[17] = Ago1;
  state[18] = Agu0;
  state[19] = Agu1;
  state[20] = Aka0;
  state[21] = Aka1;
  state[22] = Ake0;
  state[23] = Ake1;
  state[24] = Aki0;
  state[25] = Aki1;
  state[26] = Ako0;
  state[27] = Ako1;
  state[28] = Aku0;
  state[29] = Aku1;
  state[30] = Ama0;
  state[31] = Ama1;
  state[32] = Ame0;
  state[33] = Ame1;
  state[34] = Ami0;
  state[35] = Ami1;
  state[36] = Amo0;
  state[37] = Amo1;
  state[38] = Amu0;
  state[39] = Amu1;
  state[40] = Asa0;
  state[41] = Asa1;
  state[42] = Ase0;
  state[43] = Ase1;
  state[44] = Asi0;
  state[45] = Asi1;
  state[46] = Aso0;
  state[47] = Aso1;
  state[48] = Asu0;
  state[49] = Asu1;

  return sizeof(void *) * 4 + sizeof(u32) * 12 * 5 * 2;
}
