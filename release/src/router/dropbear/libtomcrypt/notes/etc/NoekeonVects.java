/*
    NoekeonVects.java - Generate Noekeon test vectors using BouncyCastle.

    Written in 2011 by Patrick Pelletier <code@funwithsoftware.org>

    To the extent possible under law, the author(s) have dedicated all
    copyright and related and neighboring rights to this software to
    the public domain worldwide.  This software is distributed without
    any warranty.

    This file is dedicated to the public domain with the CC0 Public Domain
    Dedication: http://creativecommons.org/publicdomain/zero/1.0/legalcode.txt

    You may also consider this file to be covered by the WTFPL, as contained
    in the LibTomCrypt LICENSE file, if that makes you happier for some reason.

    ----------------------------------------------------------------------

    This program was inspired by the comment in Botan 1.10.1's
    doc/examples/eax_test.cpp:

    // Noekeon: unknown cause, though LTC's lone test vector does not
    // match Botan

    So, I investigated the discrepancy by comparing them with a third
    implementation, BouncyCastle: http://www.bouncycastle.org/java.html

    I determined that there are two reasons why LibTomCrypt's Noekeon does
    not match Botan:

    1) Botan uses "indirect Noekeon" (with a key schedule), while
       LibTomCrypt and BouncyCastle both use "direct Noekeon" (without
       a key schedule).  See slide 14 of
       http://gro.noekeon.org/Noekeon-slides.pdf

    2) However, LibTomCrypt's direct Noekeon still does not match
       BouncyCastle's direct Noekeon.  This is because of a bug in
       LibTomCrypt's PI1 and PI2 functions:
       https://github.com/libtom/libtomcrypt/issues/5

    This program uses BouncyCastle to produce test vectors which are
    suitable for Botan (by explicitly scheduling the key, thus
    building indirect Noekeon out of BouncyCastle's direct Noekeon),
    and also produces test vectors which would be suitable for
    LibTomCrypt (direct Noekeon) once its PI1 and PI2 functions are
    fixed to match the Noekeon specification.

    Although this program uses a PRNG from BouncyCastle to generate
    data for the test vectors, it uses a fixed seed and thus will
    produce the same output every time it is run.
*/

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Locale;
import org.bouncycastle.crypto.digests.RIPEMD128Digest;
import org.bouncycastle.crypto.engines.NoekeonEngine;
import org.bouncycastle.crypto.modes.EAXBlockCipher;
import org.bouncycastle.crypto.params.AEADParameters;
import org.bouncycastle.crypto.params.KeyParameter;
import org.bouncycastle.crypto.prng.DigestRandomGenerator;
import org.bouncycastle.util.encoders.HexEncoder;

public class NoekeonVects
{
    private final DigestRandomGenerator r =
        new DigestRandomGenerator(new RIPEMD128Digest());

    private final HexEncoder h = new HexEncoder();

    private final NoekeonEngine noekeon = new NoekeonEngine();

    private final KeyParameter null_key = new KeyParameter(new byte[16]);

    private final boolean schedule_key;

    private final boolean botan_format;

    private byte[] randomBytes(int n)
    {
        byte[] b = new byte[n];
        r.nextBytes(b);
        return b;
    }

    private void hexOut(byte[] b) throws IOException
    {
        // HexEncoder uses lowercase, and Botan's test vectors must
        // be in uppercase, so...
        ByteArrayOutputStream os = new ByteArrayOutputStream();
        h.encode(b, 0, b.length, os);
        String s = os.toString("US-ASCII");
        System.out.print(s.toUpperCase(Locale.US));
    }

    private void printCArray(byte[] a) throws IOException
    {
        byte[] b = new byte[1];
        for (int i = 0; i < a.length; i++)
            {
                if (i > 0)
                    System.out.print(", ");
                System.out.print("0x");
                b[0] = a[i];
                hexOut(b);
            }
    }

    private void printVector(byte[] key, byte[] plaintext, byte[] ciphertext)
        throws IOException
    {
        if (botan_format)
            {
                hexOut(plaintext);
                System.out.print(":");
                hexOut(ciphertext);
                System.out.println(":\\");
                hexOut(key);
                System.out.println();
            }
        else
            {
                System.out.println("   {");
                System.out.println("      16,");
                System.out.print("      { ");
                printCArray (key);
                System.out.println(" },");
                System.out.print("      { ");
                printCArray (plaintext);
                System.out.println(" },");
                System.out.print("      { ");
                printCArray (ciphertext);
                System.out.println(" }");
                System.out.println("   },");
            }
    }

    private KeyParameter maybe_schedule_key(byte[] key)
    {
        if (schedule_key)
            {
                noekeon.init(true, null_key);
                byte[] scheduled = new byte[16];
                noekeon.processBlock(key, 0, scheduled, 0);
                return new KeyParameter(scheduled);
            }
        else
            return new KeyParameter(key);
    }

    private byte[] encrypt(byte[] plaintext, byte[] key)
    {
        KeyParameter kp = maybe_schedule_key(key);
        noekeon.init(true, kp);
        byte[] ciphertext = new byte[16];
        noekeon.processBlock(plaintext, 0, ciphertext, 0);
        return ciphertext;
    }

    public NoekeonVects(long seed, boolean schedule_key, boolean botan_format)
    {
        this.schedule_key = schedule_key;
        this.botan_format = botan_format;
        r.addSeedMaterial(seed);
    }

    public void ecb_vectors() throws IOException
    {
        for (int i = 0; i < 8; i++)
            {
                byte[] key = randomBytes(16);
                byte[] plaintext = randomBytes(16);
                byte[] ciphertext = encrypt(plaintext, key);
                printVector(key, plaintext, ciphertext);
            }
    }

    public void eax_vectors() throws Exception
    {
        System.out.println("EAX-noekeon (16 byte key)");
        EAXBlockCipher eax = new EAXBlockCipher(new NoekeonEngine());
        byte[] output = new byte[48];
        byte[] tag = new byte[16];

        for (int j = 0; j < 16; j++)
            tag[j] = (byte) j;

        for (int i = 0; i <= 32; i++)
            {
                byte[] header_nonce_plaintext = new byte[i];
                for (int j = 0; j < i; j++)
                    header_nonce_plaintext[j] = (byte) j;
                AEADParameters params =
                    new AEADParameters(maybe_schedule_key(tag),
                                       128,
                                       header_nonce_plaintext,
                                       header_nonce_plaintext);
                eax.init(true, params);
                int off = eax.processBytes(header_nonce_plaintext, 0, i,
                                           output, 0);
                off += eax.doFinal(output, off);
                if (off != i + 16)
                    throw new RuntimeException("didn't expect that");
                byte[] ciphertext = new byte[i];
                for (int j = 0; j < i; j++)
                    ciphertext[j] = output[j];
                for (int j = 0; j < 16; j++)
                    tag[j] = output[i + j];
                System.out.print(i < 10 ? "  " : " ");
                System.out.print(i);
                System.out.print(": ");
                hexOut(ciphertext);
                System.out.print(", ");
                hexOut(tag);
                System.out.println();
            }
    }

    public static void main(String[] argv) throws Exception
    {
        NoekeonVects bot = new NoekeonVects(0xdefacedbadfacadeL, true, true);
        NoekeonVects tom = new NoekeonVects(0xdefacedbadfacadeL, false, false);
        System.out.println("# ECB vectors for indirect Noekeon, in Botan's");
        System.out.println("# test vector format, suitable for insertion");
        System.out.println("# into Botan's file checks/validate.dat");
        System.out.println("# Block cipher format is plaintext:ciphertext:key");
        bot.ecb_vectors();
        System.out.println();
        System.out.println("/* ECB vectors for direct Noekeon, as C arrays");
        System.out.println(" * suitable for insertion into LibTomCrypt's");
        System.out.println(" * noekeon_test() in src/ciphers/noekeon.c,");
        System.out.println(" * once LTC's PI1/PI2 bug is fixed. */");
        tom.ecb_vectors();
        System.out.println();
        System.out.println("# EAX vectors for indirect Noekeon, in the format");
        System.out.println("# generated by LTC's demos/tv_gen.c and consumed");
        System.out.println("# by Botan's doc/examples/eax_test.cpp, suitable");
        System.out.println("# for insertion in Botan's doc/examples/eax.vec");
        bot.eax_vectors();
        System.out.println();
        System.out.println("# EAX vectors for direct Noekeon, in the format");
        System.out.println("# generated by LTC's demos/tv_gen.c and consumed");
        System.out.println("# by Botan's doc/examples/eax_test.cpp, which");
        System.out.println("# should match LTC's notes/eax_tv.txt, once");
        System.out.println("# LTC's PI1/PI2 bug is fixed.");
        tom.eax_vectors();
        System.out.flush();
    }
}
