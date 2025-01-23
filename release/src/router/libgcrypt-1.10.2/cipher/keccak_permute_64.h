/* keccak_permute_64.h - Keccak permute function (simple 64bit)
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

/* The code is based on public-domain/CC0 "keccakc1024/simple/Keccak-simple.c"
 * implementation by Ronny Van Keer from SUPERCOP toolkit package.
 */

/* Function that computes the Keccak-f[1600] permutation on the given state. */
static unsigned int
KECCAK_F1600_PERMUTE_FUNC_NAME(KECCAK_STATE *hd)
{
  const u64 *round_consts = _gcry_keccak_round_consts_64bit;
  const u64 *round_consts_end = _gcry_keccak_round_consts_64bit + 24;
  u64 Aba, Abe, Abi, Abo, Abu;
  u64 Aga, Age, Agi, Ago, Agu;
  u64 Aka, Ake, Aki, Ako, Aku;
  u64 Ama, Ame, Ami, Amo, Amu;
  u64 Asa, Ase, Asi, Aso, Asu;
  u64 BCa, BCe, BCi, BCo, BCu;
  u64 Da, De, Di, Do, Du;
  u64 Eba, Ebe, Ebi, Ebo, Ebu;
  u64 Ega, Ege, Egi, Ego, Egu;
  u64 Eka, Eke, Eki, Eko, Eku;
  u64 Ema, Eme, Emi, Emo, Emu;
  u64 Esa, Ese, Esi, Eso, Esu;
  u64 *state = hd->u.state64;

  Aba = state[0];
  Abe = state[1];
  Abi = state[2];
  Abo = state[3];
  Abu = state[4];
  Aga = state[5];
  Age = state[6];
  Agi = state[7];
  Ago = state[8];
  Agu = state[9];
  Aka = state[10];
  Ake = state[11];
  Aki = state[12];
  Ako = state[13];
  Aku = state[14];
  Ama = state[15];
  Ame = state[16];
  Ami = state[17];
  Amo = state[18];
  Amu = state[19];
  Asa = state[20];
  Ase = state[21];
  Asi = state[22];
  Aso = state[23];
  Asu = state[24];

  do
    {
      /* prepareTheta */
      BCa = Aba ^ Aga ^ Aka ^ Ama ^ Asa;
      BCe = Abe ^ Age ^ Ake ^ Ame ^ Ase;
      BCi = Abi ^ Agi ^ Aki ^ Ami ^ Asi;
      BCo = Abo ^ Ago ^ Ako ^ Amo ^ Aso;
      BCu = Abu ^ Agu ^ Aku ^ Amu ^ Asu;

      /* thetaRhoPiChiIotaPrepareTheta(round  , A, E) */
      Da = BCu ^ ROL64(BCe, 1);
      De = BCa ^ ROL64(BCi, 1);
      Di = BCe ^ ROL64(BCo, 1);
      Do = BCi ^ ROL64(BCu, 1);
      Du = BCo ^ ROL64(BCa, 1);

      Aba ^= Da;
      BCa = Aba;
      Age ^= De;
      BCe = ROL64(Age, 44);
      Aki ^= Di;
      BCi = ROL64(Aki, 43);
      Amo ^= Do;
      BCo = ROL64(Amo, 21);
      Asu ^= Du;
      BCu = ROL64(Asu, 14);
      Eba = BCa ^ ANDN64(BCe, BCi);
      Eba ^= *(round_consts++);
      Ebe = BCe ^ ANDN64(BCi, BCo);
      Ebi = BCi ^ ANDN64(BCo, BCu);
      Ebo = BCo ^ ANDN64(BCu, BCa);
      Ebu = BCu ^ ANDN64(BCa, BCe);

      Abo ^= Do;
      BCa = ROL64(Abo, 28);
      Agu ^= Du;
      BCe = ROL64(Agu, 20);
      Aka ^= Da;
      BCi = ROL64(Aka, 3);
      Ame ^= De;
      BCo = ROL64(Ame, 45);
      Asi ^= Di;
      BCu = ROL64(Asi, 61);
      Ega = BCa ^ ANDN64(BCe, BCi);
      Ege = BCe ^ ANDN64(BCi, BCo);
      Egi = BCi ^ ANDN64(BCo, BCu);
      Ego = BCo ^ ANDN64(BCu, BCa);
      Egu = BCu ^ ANDN64(BCa, BCe);

      Abe ^= De;
      BCa = ROL64(Abe, 1);
      Agi ^= Di;
      BCe = ROL64(Agi, 6);
      Ako ^= Do;
      BCi = ROL64(Ako, 25);
      Amu ^= Du;
      BCo = ROL64(Amu, 8);
      Asa ^= Da;
      BCu = ROL64(Asa, 18);
      Eka = BCa ^ ANDN64(BCe, BCi);
      Eke = BCe ^ ANDN64(BCi, BCo);
      Eki = BCi ^ ANDN64(BCo, BCu);
      Eko = BCo ^ ANDN64(BCu, BCa);
      Eku = BCu ^ ANDN64(BCa, BCe);

      Abu ^= Du;
      BCa = ROL64(Abu, 27);
      Aga ^= Da;
      BCe = ROL64(Aga, 36);
      Ake ^= De;
      BCi = ROL64(Ake, 10);
      Ami ^= Di;
      BCo = ROL64(Ami, 15);
      Aso ^= Do;
      BCu = ROL64(Aso, 56);
      Ema = BCa ^ ANDN64(BCe, BCi);
      Eme = BCe ^ ANDN64(BCi, BCo);
      Emi = BCi ^ ANDN64(BCo, BCu);
      Emo = BCo ^ ANDN64(BCu, BCa);
      Emu = BCu ^ ANDN64(BCa, BCe);

      Abi ^= Di;
      BCa = ROL64(Abi, 62);
      Ago ^= Do;
      BCe = ROL64(Ago, 55);
      Aku ^= Du;
      BCi = ROL64(Aku, 39);
      Ama ^= Da;
      BCo = ROL64(Ama, 41);
      Ase ^= De;
      BCu = ROL64(Ase, 2);
      Esa = BCa ^ ANDN64(BCe, BCi);
      Ese = BCe ^ ANDN64(BCi, BCo);
      Esi = BCi ^ ANDN64(BCo, BCu);
      Eso = BCo ^ ANDN64(BCu, BCa);
      Esu = BCu ^ ANDN64(BCa, BCe);

      /* prepareTheta */
      BCa = Eba ^ Ega ^ Eka ^ Ema ^ Esa;
      BCe = Ebe ^ Ege ^ Eke ^ Eme ^ Ese;
      BCi = Ebi ^ Egi ^ Eki ^ Emi ^ Esi;
      BCo = Ebo ^ Ego ^ Eko ^ Emo ^ Eso;
      BCu = Ebu ^ Egu ^ Eku ^ Emu ^ Esu;

      /* thetaRhoPiChiIotaPrepareTheta(round+1, E, A) */
      Da = BCu ^ ROL64(BCe, 1);
      De = BCa ^ ROL64(BCi, 1);
      Di = BCe ^ ROL64(BCo, 1);
      Do = BCi ^ ROL64(BCu, 1);
      Du = BCo ^ ROL64(BCa, 1);

      Eba ^= Da;
      BCa = Eba;
      Ege ^= De;
      BCe = ROL64(Ege, 44);
      Eki ^= Di;
      BCi = ROL64(Eki, 43);
      Emo ^= Do;
      BCo = ROL64(Emo, 21);
      Esu ^= Du;
      BCu = ROL64(Esu, 14);
      Aba = BCa ^ ANDN64(BCe, BCi);
      Aba ^= *(round_consts++);
      Abe = BCe ^ ANDN64(BCi, BCo);
      Abi = BCi ^ ANDN64(BCo, BCu);
      Abo = BCo ^ ANDN64(BCu, BCa);
      Abu = BCu ^ ANDN64(BCa, BCe);

      Ebo ^= Do;
      BCa = ROL64(Ebo, 28);
      Egu ^= Du;
      BCe = ROL64(Egu, 20);
      Eka ^= Da;
      BCi = ROL64(Eka, 3);
      Eme ^= De;
      BCo = ROL64(Eme, 45);
      Esi ^= Di;
      BCu = ROL64(Esi, 61);
      Aga = BCa ^ ANDN64(BCe, BCi);
      Age = BCe ^ ANDN64(BCi, BCo);
      Agi = BCi ^ ANDN64(BCo, BCu);
      Ago = BCo ^ ANDN64(BCu, BCa);
      Agu = BCu ^ ANDN64(BCa, BCe);

      Ebe ^= De;
      BCa = ROL64(Ebe, 1);
      Egi ^= Di;
      BCe = ROL64(Egi, 6);
      Eko ^= Do;
      BCi = ROL64(Eko, 25);
      Emu ^= Du;
      BCo = ROL64(Emu, 8);
      Esa ^= Da;
      BCu = ROL64(Esa, 18);
      Aka = BCa ^ ANDN64(BCe, BCi);
      Ake = BCe ^ ANDN64(BCi, BCo);
      Aki = BCi ^ ANDN64(BCo, BCu);
      Ako = BCo ^ ANDN64(BCu, BCa);
      Aku = BCu ^ ANDN64(BCa, BCe);

      Ebu ^= Du;
      BCa = ROL64(Ebu, 27);
      Ega ^= Da;
      BCe = ROL64(Ega, 36);
      Eke ^= De;
      BCi = ROL64(Eke, 10);
      Emi ^= Di;
      BCo = ROL64(Emi, 15);
      Eso ^= Do;
      BCu = ROL64(Eso, 56);
      Ama = BCa ^ ANDN64(BCe, BCi);
      Ame = BCe ^ ANDN64(BCi, BCo);
      Ami = BCi ^ ANDN64(BCo, BCu);
      Amo = BCo ^ ANDN64(BCu, BCa);
      Amu = BCu ^ ANDN64(BCa, BCe);

      Ebi ^= Di;
      BCa = ROL64(Ebi, 62);
      Ego ^= Do;
      BCe = ROL64(Ego, 55);
      Eku ^= Du;
      BCi = ROL64(Eku, 39);
      Ema ^= Da;
      BCo = ROL64(Ema, 41);
      Ese ^= De;
      BCu = ROL64(Ese, 2);
      Asa = BCa ^ ANDN64(BCe, BCi);
      Ase = BCe ^ ANDN64(BCi, BCo);
      Asi = BCi ^ ANDN64(BCo, BCu);
      Aso = BCo ^ ANDN64(BCu, BCa);
      Asu = BCu ^ ANDN64(BCa, BCe);
    }
  while (round_consts < round_consts_end);

  state[0] = Aba;
  state[1] = Abe;
  state[2] = Abi;
  state[3] = Abo;
  state[4] = Abu;
  state[5] = Aga;
  state[6] = Age;
  state[7] = Agi;
  state[8] = Ago;
  state[9] = Agu;
  state[10] = Aka;
  state[11] = Ake;
  state[12] = Aki;
  state[13] = Ako;
  state[14] = Aku;
  state[15] = Ama;
  state[16] = Ame;
  state[17] = Ami;
  state[18] = Amo;
  state[19] = Amu;
  state[20] = Asa;
  state[21] = Ase;
  state[22] = Asi;
  state[23] = Aso;
  state[24] = Asu;

  return sizeof(void *) * 4 + sizeof(u64) * 12 * 5;
}

static unsigned int
KECCAK_F1600_ABSORB_FUNC_NAME(KECCAK_STATE *hd, int pos, const byte *lanes,
			      size_t nlanes, int blocklanes)
{
  unsigned int burn = 0;

  while (nlanes)
    {
      switch (blocklanes)
	{
	case 21:
	  /* SHAKE128 */
	  while (pos == 0 && nlanes >= 21)
	    {
	      nlanes -= 21;
	      absorb_lanes64_8(&hd->u.state64[0], lanes); lanes += 8 * 8;
	      absorb_lanes64_8(&hd->u.state64[8], lanes); lanes += 8 * 8;
	      absorb_lanes64_4(&hd->u.state64[16], lanes); lanes += 8 * 4;
	      absorb_lanes64_1(&hd->u.state64[20], lanes); lanes += 8 * 1;

	      burn = KECCAK_F1600_PERMUTE_FUNC_NAME(hd);
	    }
	  break;

	case 18:
	  /* SHA3-224 */
	  while (pos == 0 && nlanes >= 18)
	    {
	      nlanes -= 18;
	      absorb_lanes64_8(&hd->u.state64[0], lanes); lanes += 8 * 8;
	      absorb_lanes64_8(&hd->u.state64[8], lanes); lanes += 8 * 8;
	      absorb_lanes64_2(&hd->u.state64[16], lanes); lanes += 8 * 2;

	      burn = KECCAK_F1600_PERMUTE_FUNC_NAME(hd);
	    }
	  break;

	case 17:
	  /* SHA3-256 & SHAKE256 */
	  while (pos == 0 && nlanes >= 17)
	    {
	      nlanes -= 17;
	      absorb_lanes64_8(&hd->u.state64[0], lanes); lanes += 8 * 8;
	      absorb_lanes64_8(&hd->u.state64[8], lanes); lanes += 8 * 8;
	      absorb_lanes64_1(&hd->u.state64[16], lanes); lanes += 8 * 1;

	      burn = KECCAK_F1600_PERMUTE_FUNC_NAME(hd);
	    }
	  break;

	case 13:
	  /* SHA3-384 */
	  while (pos == 0 && nlanes >= 13)
	    {
	      nlanes -= 13;
	      absorb_lanes64_8(&hd->u.state64[0], lanes); lanes += 8 * 8;
	      absorb_lanes64_4(&hd->u.state64[8], lanes); lanes += 8 * 4;
	      absorb_lanes64_1(&hd->u.state64[12], lanes); lanes += 8 * 1;

	      burn = KECCAK_F1600_PERMUTE_FUNC_NAME(hd);
	    }
	  break;

	case 9:
	  /* SHA3-512 */
	  while (pos == 0 && nlanes >= 9)
	    {
	      nlanes -= 9;
	      absorb_lanes64_8(&hd->u.state64[0], lanes); lanes += 8 * 8;
	      absorb_lanes64_1(&hd->u.state64[8], lanes); lanes += 8 * 1;

	      burn = KECCAK_F1600_PERMUTE_FUNC_NAME(hd);
	    }
	  break;
	}

      while (nlanes)
	{
	  hd->u.state64[pos] ^= buf_get_le64(lanes);
	  lanes += 8;
	  nlanes--;

	  if (++pos == blocklanes)
	    {
	      burn = KECCAK_F1600_PERMUTE_FUNC_NAME(hd);
	      pos = 0;
	      break;
	    }
	}
    }

  return burn;
}
