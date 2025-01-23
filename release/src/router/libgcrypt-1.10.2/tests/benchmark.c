/* benchmark.c - for libgcrypt
 * Copyright (C) 2002, 2004, 2005, 2006, 2008 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef _GCRYPT_IN_LIBGCRYPT
# include "../src/gcrypt-int.h"
# include "../compat/libcompat.h"
#else
# include <gcrypt.h>
#endif

#include "stopwatch.h"


#define PGM "benchmark"
#include "t-common.h"

/* Do encryption tests with large buffers (100 KiB).  */
static int large_buffers;

/* Do encryption tests with huge buffers (256 MiB). */
static int huge_buffers;

/* Number of cipher repetitions.  */
static int cipher_repetitions;

/* Number of hash repetitions.  */
static int hash_repetitions;

/* Number of hash repetitions.  */
static int mac_repetitions;

/* Alignment of the buffers.  */
static int buffer_alignment;

/* Whether to include the keysetup in the cipher timings.  */
static int cipher_with_keysetup;

/* Whether fips mode was active at startup.  */
static int in_fips_mode;

/* Whether we are running as part of the regression test suite.  */
static int in_regression_test;

/* Whether --progress is in use.  */
static int with_progress;

/* Runtime flag to switch to a different progress output.  */
static int single_char_progress;


static const char sample_private_dsa_key_1024[] =
"(private-key\n"
"  (dsa\n"
"   (p #00A126202D592214C5A8F6016E2C3F4256052ACB1CB17D88E64B1293FAF08F5E4685"
       "03E6F68366B326A56284370EB2103E92D8346A163E44A08FDC422AC8E9E44268557A"
       "853539A6AF39353A59CE5E78FD98B57D0F3E3A7EBC8A256AC9A775BA59689F3004BF"
       "C3035730C4C0C51626C5D7F5852637EC589BB29DAB46C161572E4B#)\n"
"   (q #00DEB5A296421887179ECA1762884DE2AF8185AFC5#)\n"
"   (g #3958B34AE7747194ECBD312F8FEE8CBE3918E94DF9FD11E2912E56318F33BDC38622"
       "B18DDFF393074BCA8BAACF50DF27AEE529F3E8AEECE55C398DAB3A5E04C2EA142312"
       "FACA2FE7F0A88884F8DAC3979EE67598F9A383B2A2325F035C796F352A5C3CDF2CB3"
       "85AD24EC52A6E55247E1BB37D260F79E617D2A4446415B6AD79A#)\n"
"   (y #519E9FE9AB0545A6724E74603B7B04E48DC1437E0284A11EA605A7BA8AB1CF354FD4"
       "ECC93880AC293391C69B558AD84E7AAFA88F11D028CF3A378F241D6B056A90C588F6"
       "66F68D27262B4DA84657D15057D371BCEC1F6504032507D5B881E45FC93A1B973155"
       "D91C57219D090C3ACD75E7C2B9F1176A208AC03D6C12AC28A271#)\n"
"   (x #4186F8A58C5DF46C5BCFC7006BEEBF05E93C0CA7#)\n"
"))\n";

static const char sample_public_dsa_key_1024[] =
"(public-key\n"
"  (dsa\n"
"   (p #00A126202D592214C5A8F6016E2C3F4256052ACB1CB17D88E64B1293FAF08F5E4685"
       "03E6F68366B326A56284370EB2103E92D8346A163E44A08FDC422AC8E9E44268557A"
       "853539A6AF39353A59CE5E78FD98B57D0F3E3A7EBC8A256AC9A775BA59689F3004BF"
       "C3035730C4C0C51626C5D7F5852637EC589BB29DAB46C161572E4B#)\n"
"   (q #00DEB5A296421887179ECA1762884DE2AF8185AFC5#)\n"
"   (g #3958B34AE7747194ECBD312F8FEE8CBE3918E94DF9FD11E2912E56318F33BDC38622"
       "B18DDFF393074BCA8BAACF50DF27AEE529F3E8AEECE55C398DAB3A5E04C2EA142312"
       "FACA2FE7F0A88884F8DAC3979EE67598F9A383B2A2325F035C796F352A5C3CDF2CB3"
       "85AD24EC52A6E55247E1BB37D260F79E617D2A4446415B6AD79A#)\n"
"   (y #519E9FE9AB0545A6724E74603B7B04E48DC1437E0284A11EA605A7BA8AB1CF354FD4"
       "ECC93880AC293391C69B558AD84E7AAFA88F11D028CF3A378F241D6B056A90C588F6"
       "66F68D27262B4DA84657D15057D371BCEC1F6504032507D5B881E45FC93A1B973155"
       "D91C57219D090C3ACD75E7C2B9F1176A208AC03D6C12AC28A271#)\n"
"))\n";


static const char sample_private_dsa_key_2048[] =
"(private-key\n"
"  (dsa\n"
"   (p #00B54636673962B64F7DC23C71ACEF6E7331796F607560B194DFCC0CA370E858A365"
       "A413152FB6EB8C664BD171AC316FE5B381CD084D07377571599880A068EF1382D85C"
       "308B4E9DEAC12D66DE5C4A826EBEB5ED94A62E7301E18927E890589A2F230272A150"
       "C118BC3DC2965AE0D05BE4F65C6137B2BA7EDABB192C3070D202C10AA3F534574970"
       "71454DB8A73DDB6511A5BA98EF1450FD90DE5BAAFC9FD3AC22EBEA612DD075BB7405"
       "D56866D125E33982C046808F7CEBA8E5C0B9F19A6FE451461660A1CBA9EF68891179"
       "0256A573D3B8F35A5C7A0C6C31F2DB90E25A26845252AD9E485EF2D339E7B5890CD4"
       "2F9C9F315ED409171EC35CA04CC06B275577B3#)\n"
"   (q #00DA67989167FDAC4AE3DF9247A716859A30C0CF9C5A6DBA01EABA3481#)\n"
"   (g #48E35DA584A089D05142AA63603FDB00D131B07A0781E2D5A8F9614D2B33D3E40A78"
       "98A9E10CDBB612CF093F95A3E10D09566726F2C12823836B2D9CD974BB695665F3B3"
       "5D219A9724B87F380BD5207EDA0AE38C79E8F18122C3F76E4CEB0ABED3250914987F"
       "B30D4B9E19C04C28A5D4F45560AF586F6A1B41751EAD90AE7F044F4E2A4A50C1F508"
       "4FC202463F478F678B9A19392F0D2961C5391C546EF365368BB46410C9C1CEE96E9F"
       "0C953570C2ED06328B11C90E86E57CAA7FA5ABAA278E22A4C8C08E16EE59F484EC44"
       "2CF55535BAA2C6BEA8833A555372BEFE1E665D3C7DAEF58061D5136331EF4EB61BC3"
       "6EE4425A553AF8885FEA15A88135BE133520#)\n"
"   (y #66E0D1A69D663466F8FEF2B7C0878DAC93C36A2FB2C05E0306A53B926021D4B92A1C"
       "2FA6860061E88E78CBBBA49B0E12700F07DBF86F72CEB2927EDAC0C7E3969C3A47BB"
       "4E0AE93D8BB3313E93CC7A72DFEEE442EFBC81B3B2AEC9D8DCBE21220FB760201D79"
       "328C41C773866587A44B6954767D022A88072900E964089D9B17133603056C985C4F"
       "8A0B648F297F8D2C3CB43E4371DC6002B5B12CCC085BDB2CFC5074A0587566187EE3"
       "E11A2A459BD94726248BB8D6CC62938E11E284C2C183576FBB51749EB238C4360923"
       "79C08CE1C8CD77EB57404CE9B4744395ACF721487450BADE3220576F2F816248B0A7"
       "14A264330AECCB24DE2A1107847B23490897#)\n"
"   (x #477BD14676E22563C5ABA68025CEBA2A48D485F5B2D4AD4C0EBBD6D0#)\n"
"))\n";


static const char sample_public_dsa_key_2048[] =
"(public-key\n"
"  (dsa\n"
"   (p #00B54636673962B64F7DC23C71ACEF6E7331796F607560B194DFCC0CA370E858A365"
       "A413152FB6EB8C664BD171AC316FE5B381CD084D07377571599880A068EF1382D85C"
       "308B4E9DEAC12D66DE5C4A826EBEB5ED94A62E7301E18927E890589A2F230272A150"
       "C118BC3DC2965AE0D05BE4F65C6137B2BA7EDABB192C3070D202C10AA3F534574970"
       "71454DB8A73DDB6511A5BA98EF1450FD90DE5BAAFC9FD3AC22EBEA612DD075BB7405"
       "D56866D125E33982C046808F7CEBA8E5C0B9F19A6FE451461660A1CBA9EF68891179"
       "0256A573D3B8F35A5C7A0C6C31F2DB90E25A26845252AD9E485EF2D339E7B5890CD4"
       "2F9C9F315ED409171EC35CA04CC06B275577B3#)\n"
"   (q #00DA67989167FDAC4AE3DF9247A716859A30C0CF9C5A6DBA01EABA3481#)\n"
"   (g #48E35DA584A089D05142AA63603FDB00D131B07A0781E2D5A8F9614D2B33D3E40A78"
       "98A9E10CDBB612CF093F95A3E10D09566726F2C12823836B2D9CD974BB695665F3B3"
       "5D219A9724B87F380BD5207EDA0AE38C79E8F18122C3F76E4CEB0ABED3250914987F"
       "B30D4B9E19C04C28A5D4F45560AF586F6A1B41751EAD90AE7F044F4E2A4A50C1F508"
       "4FC202463F478F678B9A19392F0D2961C5391C546EF365368BB46410C9C1CEE96E9F"
       "0C953570C2ED06328B11C90E86E57CAA7FA5ABAA278E22A4C8C08E16EE59F484EC44"
       "2CF55535BAA2C6BEA8833A555372BEFE1E665D3C7DAEF58061D5136331EF4EB61BC3"
       "6EE4425A553AF8885FEA15A88135BE133520#)\n"
"   (y #66E0D1A69D663466F8FEF2B7C0878DAC93C36A2FB2C05E0306A53B926021D4B92A1C"
       "2FA6860061E88E78CBBBA49B0E12700F07DBF86F72CEB2927EDAC0C7E3969C3A47BB"
       "4E0AE93D8BB3313E93CC7A72DFEEE442EFBC81B3B2AEC9D8DCBE21220FB760201D79"
       "328C41C773866587A44B6954767D022A88072900E964089D9B17133603056C985C4F"
       "8A0B648F297F8D2C3CB43E4371DC6002B5B12CCC085BDB2CFC5074A0587566187EE3"
       "E11A2A459BD94726248BB8D6CC62938E11E284C2C183576FBB51749EB238C4360923"
       "79C08CE1C8CD77EB57404CE9B4744395ACF721487450BADE3220576F2F816248B0A7"
       "14A264330AECCB24DE2A1107847B23490897#)\n"
"))\n";


static const char sample_private_dsa_key_3072[] =
"(private-key\n"
"  (dsa\n"
"   (p #00BA73E148AEA5E8B64878AF5BE712B8302B9671C5F3EEB7722A9D0D9868D048C938"
       "877C91C335C7819292E69C7D34264F1578E32EC2DA8408DF75D0EB76E0D3030B84B5"
       "62D8EF93AB53BAB6B8A5DE464F5CA87AEA43BDCF0FB0B7815AA3114CFC84FD916A83"
       "B3D5FD78390189332232E9D037D215313FD002FF46C048B66703F87FAE092AAA0988"
       "AC745336EBE672A01DEDBD52395783579B67CF3AE1D6F1602CCCB12154FA0E00AE46"
       "0D9B289CF709194625BCB919B11038DEFC50ADBBA20C3F320078E4E9529B4F6848E2"
       "AB5E6278DB961FE226F2EEBD201E071C48C5BEF98B4D9BEE42C1C7102D893EBF8902"
       "D7A91266340AFD6CE1D09E52282FFF5B97EAFA3886A3FCF84FF76D1E06538D0D8E60"
       "B3332145785E07D29A5965382DE3470D1D888447FA9C00A2373378FC3FA7B9F7D17E"
       "95A6A5AE1397BE46D976EF2C96E89913AC4A09351CA661BF6F67E30407DA846946C7"
       "62D9BAA6B77825097D3E7B886456BB32E3E74516BF3FD93D71B257AA8F723E01CE33"
       "8015353D3778B02B892AF7#)\n"
"   (q #00BFF3F3CC18FA018A5B8155A8695E1E4939660D5E4759322C39D50F3B93E5F68B#)\n"
"   (g #6CCFD8219F5FCE8EF2BEF3262929787140847E38674B1EF8DB20255E212CB6330EC4"
       "DFE8A26AB7ECC5760DEB9BBF59A2B2821D510F1868172222867558B8D204E889C474"
       "7CA30FBF9D8CF41AE5D5BD845174641101593849FF333E6C93A6550931B2B9D56B98"
       "9CAB01729D9D736FA6D24A74D2DDE1E9E648D141473E443DD6BBF0B3CAB64F9FE4FC"
       "134B2EB57437789F75C744DF1FA67FA8A64603E5441BC7ECE29E00BDF262BDC81E8C"
       "7330A18A412DE38E7546D342B89A0AF675A89E6BEF00540EB107A2FE74EA402B0D89"
       "F5C02918DEEEAF8B8737AC866B09B50810AB8D8668834A1B9E1E53866E2B0A926FAB"
       "120A0CDE5B3715FFFE6ACD1AB73588DCC1EC4CE9392FE57F8D1D35811200CB07A0E6"
       "374E2C4B0AEB7E3D077B8545C0E438DCC0F1AE81E186930E99EBC5B91B77E92803E0"
       "21602887851A4FFDB3A7896AC655A0901218C121C5CBB0931E7D5EAC243F37711B5F"
       "D5A62B1B38A83F03D8F6703D8B98DF367FC8A76990335F62173A5391836F0F2413EC"
       "4997AF9EB55C6660B01A#)\n"
"   (y #2320B22434C5DB832B4EC267CC52E78DD5CCFA911E8F0804E7E7F32B186B2D4167AE"
       "4AA6869822E76400492D6A193B0535322C72B0B7AA4A87E33044FDC84BE24C64A053"
       "A37655EE9EABDCDC1FDF63F3F1C677CEB41595DF7DEFE9178D85A3D621B4E4775492"
       "8C0A58D2458D06F9562E4DE2FE6129A64063A99E88E54485B97484A28188C4D33F15"
       "DDC903B6CEA0135E3E3D27B4EA39319696305CE93D7BA7BE00367DBE3AAF43491E71"
       "CBF254744A5567F5D70090D6139E0C990239627B3A1C5B20B6F9F6374B8D8D8A8997"
       "437265BE1E3B4810D4B09254400DE287A0DFFBAEF339E48D422B1D41A37E642BC026"
       "73314701C8FA9792845C129351A87A945A03E6C895860E51D6FB8B7340A94D1A8A7B"
       "FA85AC83B4B14E73AB86CB96C236C8BFB0978B61B2367A7FE4F7891070F56C78D5DD"
       "F5576BFE5BE4F333A4E2664E79528B3294907AADD63F4F2E7AA8147B928D8CD69765"
       "3DB98C4297CB678046ED55C0DBE60BF7142C594603E4D705DC3D17270F9F086EC561"
       "2703D518D8D49FF0EBE6#)\n"
"   (x #00A9FFFC88E67D6F7B810E291C050BAFEA7FC4A75E8D2F16CFED3416FD77607232#)\n"
"))\n";

static const char sample_public_dsa_key_3072[] =
"(public-key\n"
"  (dsa\n"
"   (p #00BA73E148AEA5E8B64878AF5BE712B8302B9671C5F3EEB7722A9D0D9868D048C938"
       "877C91C335C7819292E69C7D34264F1578E32EC2DA8408DF75D0EB76E0D3030B84B5"
       "62D8EF93AB53BAB6B8A5DE464F5CA87AEA43BDCF0FB0B7815AA3114CFC84FD916A83"
       "B3D5FD78390189332232E9D037D215313FD002FF46C048B66703F87FAE092AAA0988"
       "AC745336EBE672A01DEDBD52395783579B67CF3AE1D6F1602CCCB12154FA0E00AE46"
       "0D9B289CF709194625BCB919B11038DEFC50ADBBA20C3F320078E4E9529B4F6848E2"
       "AB5E6278DB961FE226F2EEBD201E071C48C5BEF98B4D9BEE42C1C7102D893EBF8902"
       "D7A91266340AFD6CE1D09E52282FFF5B97EAFA3886A3FCF84FF76D1E06538D0D8E60"
       "B3332145785E07D29A5965382DE3470D1D888447FA9C00A2373378FC3FA7B9F7D17E"
       "95A6A5AE1397BE46D976EF2C96E89913AC4A09351CA661BF6F67E30407DA846946C7"
       "62D9BAA6B77825097D3E7B886456BB32E3E74516BF3FD93D71B257AA8F723E01CE33"
       "8015353D3778B02B892AF7#)\n"
"   (q #00BFF3F3CC18FA018A5B8155A8695E1E4939660D5E4759322C39D50F3B93E5F68B#)\n"
"   (g #6CCFD8219F5FCE8EF2BEF3262929787140847E38674B1EF8DB20255E212CB6330EC4"
       "DFE8A26AB7ECC5760DEB9BBF59A2B2821D510F1868172222867558B8D204E889C474"
       "7CA30FBF9D8CF41AE5D5BD845174641101593849FF333E6C93A6550931B2B9D56B98"
       "9CAB01729D9D736FA6D24A74D2DDE1E9E648D141473E443DD6BBF0B3CAB64F9FE4FC"
       "134B2EB57437789F75C744DF1FA67FA8A64603E5441BC7ECE29E00BDF262BDC81E8C"
       "7330A18A412DE38E7546D342B89A0AF675A89E6BEF00540EB107A2FE74EA402B0D89"
       "F5C02918DEEEAF8B8737AC866B09B50810AB8D8668834A1B9E1E53866E2B0A926FAB"
       "120A0CDE5B3715FFFE6ACD1AB73588DCC1EC4CE9392FE57F8D1D35811200CB07A0E6"
       "374E2C4B0AEB7E3D077B8545C0E438DCC0F1AE81E186930E99EBC5B91B77E92803E0"
       "21602887851A4FFDB3A7896AC655A0901218C121C5CBB0931E7D5EAC243F37711B5F"
       "D5A62B1B38A83F03D8F6703D8B98DF367FC8A76990335F62173A5391836F0F2413EC"
       "4997AF9EB55C6660B01A#)\n"
"   (y #2320B22434C5DB832B4EC267CC52E78DD5CCFA911E8F0804E7E7F32B186B2D4167AE"
       "4AA6869822E76400492D6A193B0535322C72B0B7AA4A87E33044FDC84BE24C64A053"
       "A37655EE9EABDCDC1FDF63F3F1C677CEB41595DF7DEFE9178D85A3D621B4E4775492"
       "8C0A58D2458D06F9562E4DE2FE6129A64063A99E88E54485B97484A28188C4D33F15"
       "DDC903B6CEA0135E3E3D27B4EA39319696305CE93D7BA7BE00367DBE3AAF43491E71"
       "CBF254744A5567F5D70090D6139E0C990239627B3A1C5B20B6F9F6374B8D8D8A8997"
       "437265BE1E3B4810D4B09254400DE287A0DFFBAEF339E48D422B1D41A37E642BC026"
       "73314701C8FA9792845C129351A87A945A03E6C895860E51D6FB8B7340A94D1A8A7B"
       "FA85AC83B4B14E73AB86CB96C236C8BFB0978B61B2367A7FE4F7891070F56C78D5DD"
       "F5576BFE5BE4F333A4E2664E79528B3294907AADD63F4F2E7AA8147B928D8CD69765"
       "3DB98C4297CB678046ED55C0DBE60BF7142C594603E4D705DC3D17270F9F086EC561"
       "2703D518D8D49FF0EBE6#)\n"
"))\n";


static const char sample_public_elg_key_1024[] =
"(public-key"
"  (elg"
"   (p #00F7CC7C08AF096B620C545C9353B1140D698FF8BE2D97A3515C17C7F8DABCDB8FB6"
       "64A46416C90C530C18DF5ABB6C1DDE3AE2FA9DDC9CE40DF644CDE2E759F6DE43F31A"
       "EEEBC136A460B3E4B0A8F99326A335145B19F4C81B13804894B7D2A30F78A8A7D7F4"
       "52B83836FDB0DE90BE327FB5E5318757BEF5FE0FC3A5461CBEA0D3#)"
"   (g #06#)"
"   (y #36B38FB63E3340A0DD8A0468E9FAA512A32DA010BF7110201D0A3DF1B8FEA0E16F3C"
       "80374584E554804B96EAA8C270FE531F75D0DBD81BA65640EDB1F76D46C27D2925B7"
       "3EC3B295CDAEEF242904A84D74FB2879425F82D4C5B59BB49A992F85D574168DED85"
       "D227600BBEF7AF0B8F0DEB785528370E4C4B3E4D65C536122A5A#)"
"   ))";
static const char sample_private_elg_key_1024[] =
"(private-key"
"  (elg"
"   (p #00F7CC7C08AF096B620C545C9353B1140D698FF8BE2D97A3515C17C7F8DABCDB8FB6"
       "64A46416C90C530C18DF5ABB6C1DDE3AE2FA9DDC9CE40DF644CDE2E759F6DE43F31A"
       "EEEBC136A460B3E4B0A8F99326A335145B19F4C81B13804894B7D2A30F78A8A7D7F4"
       "52B83836FDB0DE90BE327FB5E5318757BEF5FE0FC3A5461CBEA0D3#)"
"   (g #06#)"
"   (y #36B38FB63E3340A0DD8A0468E9FAA512A32DA010BF7110201D0A3DF1B8FEA0E16F3C"
       "80374584E554804B96EAA8C270FE531F75D0DBD81BA65640EDB1F76D46C27D2925B7"
       "3EC3B295CDAEEF242904A84D74FB2879425F82D4C5B59BB49A992F85D574168DED85"
       "D227600BBEF7AF0B8F0DEB785528370E4C4B3E4D65C536122A5A#)"
"   (x #03656C6186FCD27D4A4B1F5010DC0D2AE7833B501E423FCD51DE5EB6D80DACFE#)"
"   ))";


static const char sample_public_elg_key_2048[] =
"(public-key"
"  (elg"
"   (p #00BE5A2BB4E562D7B644E3D01321CB818DBA27295C339FC2C47EAE9823225EE1E7B6"
       "38C5DE300E931080E09CC89A18C9D180C16559FEF0D89D6A09534BB86489CCCEE30D"
       "C18E007A8726BB99F2B2D90D2694597757B120CD2435C0098AD1B74C20004C25BA97"
       "73EAA4FBEC594EE17F8B25867EEB0F9F857C751116ADED68ADA2A1E9F9F4F40D18F0"
       "EC1221CA6A746FC5F4CDA2B8B5D0AB83834564ACF6FDBB1AB01D4BFBD1E2C0108FF5"
       "5FB3190C6D6DA4D95EA683EFA44935CFBC0BF5C6118ACC3768AEA9A98D06024841B8"
       "D07C234289D22A5E3948F199C397AA991C59A55BEA0C01E91902E039116946FEA135"
       "768011AF6B622C5AF366EF0196FC4EAEAA8127#)"
"   (g #07#)"
"   (y #5AFF87BC23D8B97AA62897A5C1CDFFA86C59F39EDBD6012B6F333CE23D872009B8C8"
       "D1E220E18CFCADFE0AA16346BA2EA132472FFEC746D11C6E758896052313BB501210"
       "2389C683A25A3464E9B35A192BAE0A3BB99C973126F7560D968C4A754901DC967354"
       "D61A90ACD56D90DCC4337AFB71FAE3FD18C60EB0D6DD173877DF5DB5199C4931FE4E"
       "5046F814422580E1162798406FC6554781142DBB7922D4B5B37A111F23761636090F"
       "6212681E133365191CF15753AE737F17943ED4B7506DE0A85C3B6D63227F9D65ADF8"
       "2C3DF0676C8F43B5B1C07D9AD4E6D0C812401D7DA7B9484DBA8CD3B73B19A95EB237"
       "D493E092AEA2371AA904009C8960B0969D12#)"
"   ))";
static const char sample_private_elg_key_2048[] =
"(private-key"
"  (elg"
"   (p #00BE5A2BB4E562D7B644E3D01321CB818DBA27295C339FC2C47EAE9823225EE1E7B6"
       "38C5DE300E931080E09CC89A18C9D180C16559FEF0D89D6A09534BB86489CCCEE30D"
       "C18E007A8726BB99F2B2D90D2694597757B120CD2435C0098AD1B74C20004C25BA97"
       "73EAA4FBEC594EE17F8B25867EEB0F9F857C751116ADED68ADA2A1E9F9F4F40D18F0"
       "EC1221CA6A746FC5F4CDA2B8B5D0AB83834564ACF6FDBB1AB01D4BFBD1E2C0108FF5"
       "5FB3190C6D6DA4D95EA683EFA44935CFBC0BF5C6118ACC3768AEA9A98D06024841B8"
       "D07C234289D22A5E3948F199C397AA991C59A55BEA0C01E91902E039116946FEA135"
       "768011AF6B622C5AF366EF0196FC4EAEAA8127#)"
"   (g #07#)"
"   (y #5AFF87BC23D8B97AA62897A5C1CDFFA86C59F39EDBD6012B6F333CE23D872009B8C8"
       "D1E220E18CFCADFE0AA16346BA2EA132472FFEC746D11C6E758896052313BB501210"
       "2389C683A25A3464E9B35A192BAE0A3BB99C973126F7560D968C4A754901DC967354"
       "D61A90ACD56D90DCC4337AFB71FAE3FD18C60EB0D6DD173877DF5DB5199C4931FE4E"
       "5046F814422580E1162798406FC6554781142DBB7922D4B5B37A111F23761636090F"
       "6212681E133365191CF15753AE737F17943ED4B7506DE0A85C3B6D63227F9D65ADF8"
       "2C3DF0676C8F43B5B1C07D9AD4E6D0C812401D7DA7B9484DBA8CD3B73B19A95EB237"
       "D493E092AEA2371AA904009C8960B0969D12#)"
"   (x #0628C3903972C55BDC1BC4223075616D3F3BA57D55532DDB40CB14CF72070E0D28BF"
       "D0402B9088D25ED8FC#)"
"  ))";

static const char sample_public_elg_key_3072[] =
"(public-key"
"  (elg"
"   (p #008EAA3497AFE3706E1A57FFA52E68C64C500731B58EBAFEB51C4A20AB15BA57FA72"
       "BA1510A4703D5AA6F05DB67E4A776F92AD08800577DC686D00B793167A5D79C997E0"
       "5B9A9E5974B4B68B4D71ED8EC37F2F45235D901997D72915643F058E712AA18275A2"
       "C6F9F7C2B9B7CD1E814D215F12A840800B546AEF2A2E6C077CDD1A322738FFD36DB2"
       "FA5420B5848EED870BC1A6CF55040AE8D2A5945F11AE2BCBE107B41A59EFDBD3B05C"
       "F4C876C02C9AEAE22CD4C86806A415302936E4C1E5AA59DBBCCD2F83C20941A29888"
       "A70ADB94D3B8A6489C46BF2C5219CD9FD2341EA21D4E68A4ECC468FD09D215FE96D4"
       "7AEA12FD22B2456D2CC13672FC7E9772A365C68668157C51E46966B6A1831C429BA0"
       "D513519713C49C13C5FC7C14BE0A117627B204C4478D0A93C6B57929E448C9B65BF2"
       "390E04BC5940320C0262FC1A221E7C796493432239A6F12BC62C5CF32E8ADBC1730C"
       "84C6E6E6BD95AF62835941F3F344AF46BFE5A8F629D5FA699FE37EF8B8C6A2484E42"
       "D226206FDF7D1FB93A5457#)"
"   (g #0B#)"
"   (y #18E734FF645AE169079AEAFC78772371089AD3088627ECF77034AFBDF33ADF594AAF"
       "3288F6979E0DB59CE3D2F0FEE031DFF187F1E4549D3C79668794CB19C14481ECDE2D"
       "D50861AB674F87A011D50D35F28E424D0D2353850899C2CDD0CC8FDBFC5A0CA395F0"
       "E605D46CBDD140DBEF426EBD638C9ADD83C195C45CE84ED2D2B21B87800C783A4F79"
       "12226FEFBDA01C66B254534A51765AF09687275AA80C5DFBA143A6262E47C547D7E2"
       "289413F8C5C56AED3FA7E5DF5526958E2294FE318AF590C0E720029C202563E6E686"
       "9EC810F39A859262FB6047C1D418CAA9047A00BDB127B44B69CF6BC8E6B3709B4C23"
       "79783C5F8457EFE23EDA6FF00D1DDCC29268FC4A6C18577BE2B7004089CBB824027A"
       "A53C86B51DB054CC83B4F50C8923E2E9431F0A77D741237226CC68591083A2E40171"
       "5C7B74100BB74003E2264F8B44A0B0BC5404C44218ABE65C04AA573877506CE4F48C"
       "9E3F8AD1CD8DD9F285DD015C2FC5DEBCFA5779AD87F0BBC62E9EC6246021AB450DB9"
       "4DDDEFAFD2C7C66E235D#)"
"   ))";
static const char sample_private_elg_key_3072[] =
"(private-key"
"  (elg"
"   (p #008EAA3497AFE3706E1A57FFA52E68C64C500731B58EBAFEB51C4A20AB15BA57FA72"
       "BA1510A4703D5AA6F05DB67E4A776F92AD08800577DC686D00B793167A5D79C997E0"
       "5B9A9E5974B4B68B4D71ED8EC37F2F45235D901997D72915643F058E712AA18275A2"
       "C6F9F7C2B9B7CD1E814D215F12A840800B546AEF2A2E6C077CDD1A322738FFD36DB2"
       "FA5420B5848EED870BC1A6CF55040AE8D2A5945F11AE2BCBE107B41A59EFDBD3B05C"
       "F4C876C02C9AEAE22CD4C86806A415302936E4C1E5AA59DBBCCD2F83C20941A29888"
       "A70ADB94D3B8A6489C46BF2C5219CD9FD2341EA21D4E68A4ECC468FD09D215FE96D4"
       "7AEA12FD22B2456D2CC13672FC7E9772A365C68668157C51E46966B6A1831C429BA0"
       "D513519713C49C13C5FC7C14BE0A117627B204C4478D0A93C6B57929E448C9B65BF2"
       "390E04BC5940320C0262FC1A221E7C796493432239A6F12BC62C5CF32E8ADBC1730C"
       "84C6E6E6BD95AF62835941F3F344AF46BFE5A8F629D5FA699FE37EF8B8C6A2484E42"
       "D226206FDF7D1FB93A5457#)"
"   (g #0B#)"
"   (y #18E734FF645AE169079AEAFC78772371089AD3088627ECF77034AFBDF33ADF594AAF"
       "3288F6979E0DB59CE3D2F0FEE031DFF187F1E4549D3C79668794CB19C14481ECDE2D"
       "D50861AB674F87A011D50D35F28E424D0D2353850899C2CDD0CC8FDBFC5A0CA395F0"
       "E605D46CBDD140DBEF426EBD638C9ADD83C195C45CE84ED2D2B21B87800C783A4F79"
       "12226FEFBDA01C66B254534A51765AF09687275AA80C5DFBA143A6262E47C547D7E2"
       "289413F8C5C56AED3FA7E5DF5526958E2294FE318AF590C0E720029C202563E6E686"
       "9EC810F39A859262FB6047C1D418CAA9047A00BDB127B44B69CF6BC8E6B3709B4C23"
       "79783C5F8457EFE23EDA6FF00D1DDCC29268FC4A6C18577BE2B7004089CBB824027A"
       "A53C86B51DB054CC83B4F50C8923E2E9431F0A77D741237226CC68591083A2E40171"
       "5C7B74100BB74003E2264F8B44A0B0BC5404C44218ABE65C04AA573877506CE4F48C"
       "9E3F8AD1CD8DD9F285DD015C2FC5DEBCFA5779AD87F0BBC62E9EC6246021AB450DB9"
       "4DDDEFAFD2C7C66E235D#)"
"   (x #03A73F0389E470AAC831B039F8AA0C4EBD3A47DD083E32EEA08E4911236CD597C272"
       "9823D47A51C8535DA52FE6DAB3E8D1C20D#)"
"  ))";


#define BUG() do {fprintf ( stderr, "Ooops at %s:%d\n", __FILE__ , __LINE__ );\
		  exit(2);} while(0)


static void
show_sexp (const char *prefix, gcry_sexp_t a)
{
  char *buf;
  size_t size;

  fputs (prefix, stderr);
  size = gcry_sexp_sprint (a, GCRYSEXP_FMT_ADVANCED, NULL, 0);
  buf = malloc (size);
  if (!buf)
    die ("out of core\n");

  gcry_sexp_sprint (a, GCRYSEXP_FMT_ADVANCED, buf, size);
  fprintf (stderr, "%.*s", (int)size, buf);
}


static void
progress_cb (void *cb_data, const char *what, int printchar,
             int current, int total)
{
  (void)cb_data;

  if (single_char_progress)
    {
      fputc (printchar, stdout);
      fflush (stderr);
    }
  else
    {
      fprintf (stderr, PGM ": progress (%s %c %d %d)\n",
               what, printchar, current, total);
      fflush (stderr);
    }
}


static void
random_bench (int very_strong)
{
  char buf[128];
  int i;

  printf ("%-10s", "random");

  if (!very_strong)
    {
      start_timer ();
      for (i=0; i < 100; i++)
        gcry_randomize (buf, sizeof buf, GCRY_STRONG_RANDOM);
      stop_timer ();
      printf (" %s", elapsed_time (1));
    }

  start_timer ();
  for (i=0; i < 100; i++)
    gcry_randomize (buf, 8,
                    very_strong? GCRY_VERY_STRONG_RANDOM:GCRY_STRONG_RANDOM);
  stop_timer ();
  printf (" %s", elapsed_time (1));

  putchar ('\n');
  if (verbose)
    xgcry_control ((GCRYCTL_DUMP_RANDOM_STATS));
}



static void
md_bench ( const char *algoname )
{
  int algo;
  gcry_md_hd_t hd;
  int i, j, repcount;
  char buf_base[1000+15];
  size_t bufsize = 1000;
  char *buf;
  char *largebuf_base;
  char *largebuf;
  char digest[512/8];
  gcry_error_t err = GPG_ERR_NO_ERROR;

  if (!algoname)
    {
      for (i=1; i < 400; i++)
        if (in_fips_mode && i == GCRY_MD_MD5)
          ; /* Don't use MD5 in fips mode.  */
        else if ( !gcry_md_test_algo (i) )
          md_bench (gcry_md_algo_name (i));
      return;
    }

  buf = buf_base + ((16 - ((size_t)buf_base & 0x0f)) % buffer_alignment);

  algo = gcry_md_map_name (algoname);
  if (!algo)
    {
      fprintf (stderr, PGM ": invalid hash algorithm `%s'\n", algoname);
      exit (1);
    }

  err = gcry_md_open (&hd, algo, 0);
  if (err)
    {
      fprintf (stderr, PGM ": error opening hash algorithm `%s'\n", algoname);
      exit (1);
    }

  for (i=0; i < bufsize; i++)
    buf[i] = i;

  printf ("%-12s", gcry_md_algo_name (algo));

  start_timer ();
  for (repcount=0; repcount < hash_repetitions; repcount++)
    for (i=0; i < 1000; i++)
      gcry_md_write (hd, buf, bufsize);
  gcry_md_final (hd);
  stop_timer ();
  printf (" %s", elapsed_time (1));
  fflush (stdout);

  gcry_md_reset (hd);
  start_timer ();
  for (repcount=0; repcount < hash_repetitions; repcount++)
    for (i=0; i < 10000; i++)
      gcry_md_write (hd, buf, bufsize/10);
  gcry_md_final (hd);
  stop_timer ();
  printf (" %s", elapsed_time (1));
  fflush (stdout);

  gcry_md_reset (hd);
  start_timer ();
  for (repcount=0; repcount < hash_repetitions; repcount++)
    for (i=0; i < 1000000; i++)
      gcry_md_write (hd, buf, 1);
  gcry_md_final (hd);
  stop_timer ();
  printf (" %s", elapsed_time (1));
  fflush (stdout);

  start_timer ();
  for (repcount=0; repcount < hash_repetitions; repcount++)
    for (i=0; i < 1000; i++)
      for (j=0; j < bufsize; j++)
        gcry_md_putc (hd, buf[j]);
  gcry_md_final (hd);
  stop_timer ();
  printf (" %s", elapsed_time (1));
  fflush (stdout);

  gcry_md_close (hd);

  /* Now 100 hash operations on 10000 bytes using the fast function.
     We initialize the buffer so that all memory pages are committed
     and we have repeatable values.  */
  if (gcry_md_get_algo_dlen (algo) > sizeof digest)
    die ("digest buffer too short\n");

  if (gcry_md_get_algo_dlen (algo))
    {
      largebuf_base = malloc (10000+15);
      if (!largebuf_base)
        die ("out of core\n");
      largebuf = (largebuf_base
                  + ((16 - ((size_t)largebuf_base & 0x0f)) % buffer_alignment));

      for (i=0; i < 10000; i++)
        largebuf[i] = i;
      start_timer ();
      for (repcount=0; repcount < hash_repetitions; repcount++)
        for (i=0; i < 100; i++)
          gcry_md_hash_buffer (algo, digest, largebuf, 10000);
      stop_timer ();
      printf (" %s", elapsed_time (1));
      free (largebuf_base);
    }

  putchar ('\n');
  fflush (stdout);
}



static void
mac_bench ( const char *algoname )
{
  int algo;
  gcry_mac_hd_t hd;
  int step, pos, j, i, repcount;
  char buf_base[1000+15];
  size_t bufsize = 1000;
  char *buf;
  char mac[3][512];
  char key[512];
  unsigned int maclen, keylen;
  size_t macoutlen;
  gcry_error_t err = GPG_ERR_NO_ERROR;

  if (!algoname)
    {
      for (i=1; i < 600; i++)
        if (in_fips_mode && i == GCRY_MAC_HMAC_MD5)
          ; /* Don't use MD5 in fips mode.  */
        else if ( !gcry_mac_test_algo (i) )
          mac_bench (gcry_mac_algo_name (i));
      return;
    }

  buf = buf_base + ((16 - ((size_t)buf_base & 0x0f)) % buffer_alignment);

  algo = gcry_mac_map_name (algoname);
  if (!algo)
    {
      fprintf (stderr, PGM ": invalid MAC algorithm `%s'\n", algoname);
      exit (1);
    }

  maclen = gcry_mac_get_algo_maclen (algo);
  if (maclen > sizeof(mac))
    maclen = sizeof(mac);

  keylen = gcry_mac_get_algo_keylen (algo);
  if (keylen == 0)
    keylen = 32;
  if (keylen > sizeof(key))
    keylen = sizeof(key);
  for (i=0; i < keylen; i++)
    key[i] = (keylen - i) ^ 0x54;

  err = gcry_mac_open (&hd, algo, 0, NULL);
  if (err)
    {
      fprintf (stderr, PGM ": error opening mac algorithm `%s': %s\n", algoname,
               gpg_strerror (err));
      exit (1);
    }

  err = gcry_mac_setkey (hd, key, keylen);
  if (err)
    {
      fprintf (stderr, PGM ": error setting key for mac algorithm `%s': %s\n",
               algoname, gpg_strerror (err));
      exit (1);
    }

  for (i=0; i < bufsize; i++)
    buf[i] = i;

  if (algo >= GCRY_MAC_POLY1305_AES && algo <= GCRY_MAC_POLY1305_SEED)
    {
      static const char iv[16] = { 1, 2, 3, 4, };
      err = gcry_mac_setiv(hd, iv, sizeof(iv));
      if (err)
        {
          fprintf (stderr, PGM ": error setting nonce for mac algorithm `%s': %s\n",
                   algoname, gpg_strerror (err));
          exit (1);
        }
    }

  printf ("%-20s", gcry_mac_algo_name (algo));

  start_timer ();
  for (repcount=0; repcount < mac_repetitions; repcount++)
    for (i=0; i < 1000; i++)
      gcry_mac_write (hd, buf, bufsize);
  macoutlen = maclen;
  gcry_mac_read (hd, mac[0], &macoutlen);
  stop_timer ();
  printf (" %s", elapsed_time (1));
  fflush (stdout);

  gcry_mac_reset (hd);
  start_timer ();
  for (repcount=0; repcount < mac_repetitions; repcount++)
    for (i=0; i < 1000; i++)
      for (step=bufsize/10, pos=0, j=0; j < 10; j++, pos+=step)
        gcry_mac_write (hd, &buf[pos], step);
  macoutlen = maclen;
  gcry_mac_read (hd, mac[1], &macoutlen);
  stop_timer ();
  printf (" %s", elapsed_time (1));
  fflush (stdout);

  gcry_mac_reset (hd);
  start_timer ();
  for (repcount=0; repcount < mac_repetitions; repcount++)
    for (i=0; i < 1000; i++)
      for (step=bufsize/100, pos=0, j=0; j < 100; j++, pos+=step)
        gcry_mac_write (hd, &buf[pos], step);
  macoutlen = maclen;
  gcry_mac_read (hd, mac[2], &macoutlen);
  stop_timer ();
  printf (" %s", elapsed_time (1));
  fflush (stdout);

  gcry_mac_close (hd);

  for (i=1; i < 3; i++)
    {
      if (memcmp(mac[i-1], mac[i], maclen))
        {
          fprintf (stderr, PGM ": mac mismatch with algorithm `%s'\n",
                   algoname);
          exit(1);
        }
    }

  putchar ('\n');
  fflush (stdout);
}


static void ccm_aead_init(gcry_cipher_hd_t hd, size_t buflen, int authlen)
{
  const int _L = 4;
  const int noncelen = 15 - _L;
  char nonce[noncelen];
  u64 params[3];
  gcry_error_t err = GPG_ERR_NO_ERROR;

  memset (nonce, 0x33, noncelen);

  err = gcry_cipher_setiv (hd, nonce, noncelen);
  if (err)
    {
      fprintf (stderr, "gcry_cipher_setiv failed: %s\n",
               gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }

  params[0] = buflen; /* encryptedlen */
  params[1] = 0; /* aadlen */
  params[2] = authlen; /* authtaglen */
  err = gcry_cipher_ctl (hd, GCRYCTL_SET_CCM_LENGTHS, params, sizeof(params));
  if (err)
    {
      fprintf (stderr, "gcry_cipher_setiv failed: %s\n",
               gpg_strerror (err));
      gcry_cipher_close (hd);
      exit (1);
    }
}


static gcry_error_t
cipher_encrypt (gcry_cipher_hd_t h, char *out, size_t outsize,
		const char *in, size_t inlen, size_t max_inlen)
{
  gcry_error_t ret;

  while (inlen)
    {
      size_t currlen = inlen;

      if (currlen > max_inlen)
	currlen = max_inlen;

      ret = gcry_cipher_encrypt(h, out, outsize, in, currlen);
      if (ret)
	return ret;

      out += currlen;
      in += currlen;
      outsize -= currlen;
      inlen -= currlen;
    }

  return 0;
}


static gcry_error_t
cipher_decrypt (gcry_cipher_hd_t h, char *out, size_t outsize,
		const char *in, size_t inlen, size_t max_inlen)
{
  gcry_error_t ret;

  while (inlen)
    {
      size_t currlen = inlen;

      if (currlen > max_inlen)
	currlen = max_inlen;

      ret = gcry_cipher_decrypt(h, out, outsize, in, currlen);
      if (ret)
	return ret;

      out += currlen;
      in += currlen;
      outsize -= currlen;
      inlen -= currlen;
    }

  return 0;
}


static void
cipher_bench ( const char *algoname )
{
  static int header_printed;
  int algo;
  gcry_cipher_hd_t hd;
  int i;
  int keylen, blklen;
  char key[128];
  char *outbuf, *buf;
  char *raw_outbuf, *raw_buf;
  size_t allocated_buflen, buflen;
  int repetitions;
  static const struct {
    int mode;
    const char *name;
    int blocked;
    unsigned int max_inlen;
    void (* const aead_init)(gcry_cipher_hd_t hd, size_t buflen, int authlen);
    int req_blocksize;
    int authlen;
    int noncelen;
    int doublekey;
  } modes[] = {
    { GCRY_CIPHER_MODE_ECB, "   ECB/Stream", 1, 0xffffffffU },
    { GCRY_CIPHER_MODE_CBC, " CBC/Poly1305", 1, 0xffffffffU },
    { GCRY_CIPHER_MODE_CFB, "      CFB", 0, 0xffffffffU },
    { GCRY_CIPHER_MODE_OFB, "      OFB", 0, 0xffffffffU },
    { GCRY_CIPHER_MODE_CTR, "      CTR", 0, 0xffffffffU },
    { GCRY_CIPHER_MODE_XTS, "      XTS", 0, 16 << 20,
      NULL, GCRY_XTS_BLOCK_LEN, 0, 0, 1 },
    { GCRY_CIPHER_MODE_CCM, "      CCM", 0, 0xffffffffU,
      ccm_aead_init, GCRY_CCM_BLOCK_LEN, 8, },
    { GCRY_CIPHER_MODE_GCM, "      GCM", 0, 0xffffffffU,
      NULL, GCRY_GCM_BLOCK_LEN, GCRY_GCM_BLOCK_LEN },
    { GCRY_CIPHER_MODE_OCB, "      OCB", 1, 0xffffffffU,
      NULL, 16, 16, 15 },
    { GCRY_CIPHER_MODE_EAX, "      EAX", 0, 0xffffffffU,
      NULL, 0, 8, 8 },
    { GCRY_CIPHER_MODE_STREAM, "", 0, 0xffffffffU },
    { GCRY_CIPHER_MODE_POLY1305, "", 0, 0xffffffffU,
      NULL, 1, 16, 12 },
    {0}
  };
  int modeidx;
  gcry_error_t err = GPG_ERR_NO_ERROR;

  if (!algoname)
    {
      for (i=1; i < 400; i++)
        if ( !gcry_cipher_test_algo (i) )
          cipher_bench (gcry_cipher_algo_name (i));
      return;
    }

  if (huge_buffers)
    {
      allocated_buflen = 256 * 1024 * 1024;
      repetitions = 4;
    }
  else if (large_buffers)
    {
      allocated_buflen = 1024 * 100;
      repetitions = 10;
    }
  else
    {
      allocated_buflen = 1024;
      repetitions = 1000;
    }
  repetitions *= cipher_repetitions;

  raw_buf = gcry_xcalloc (allocated_buflen+15, 1);
  buf = (raw_buf
         + ((16 - ((size_t)raw_buf & 0x0f)) % buffer_alignment));
  outbuf = raw_outbuf = gcry_xmalloc (allocated_buflen+15);
  outbuf = (raw_outbuf
            + ((16 - ((size_t)raw_outbuf & 0x0f)) % buffer_alignment));

  if (!header_printed)
    {
      if (cipher_repetitions != 1)
        printf ("Running each test %d times.\n", cipher_repetitions);
      printf ("%-12s", "");
      for (modeidx=0; modes[modeidx].mode; modeidx++)
        if (*modes[modeidx].name)
          printf (" %-15s", modes[modeidx].name );
      putchar ('\n');
      printf ("%-12s", "");
      for (modeidx=0; modes[modeidx].mode; modeidx++)
        if (*modes[modeidx].name)
          printf (" ---------------" );
      putchar ('\n');
      header_printed = 1;
    }

  algo = gcry_cipher_map_name (algoname);
  if (!algo)
    {
      fprintf (stderr, PGM ": invalid cipher algorithm `%s'\n", algoname);
      exit (1);
    }

  keylen = gcry_cipher_get_algo_keylen (algo);
  if (!keylen)
    {
      fprintf (stderr, PGM ": failed to get key length for algorithm `%s'\n",
	       algoname);
      exit (1);
    }
  if ( keylen * 2 > sizeof key )
    {
        fprintf (stderr, PGM ": algo %d, keylength problem (%d)\n",
                 algo, keylen );
        exit (1);
    }
  for (i=0; i < keylen * 2; i++)
    key[i] = i + (clock () & 0xff);

  blklen = gcry_cipher_get_algo_blklen (algo);
  if (!blklen)
    {
      fprintf (stderr, PGM ": failed to get block length for algorithm `%s'\n",
	       algoname);
      exit (1);
    }

  printf ("%-12s", gcry_cipher_algo_name (algo));
  fflush (stdout);

  for (modeidx=0; modes[modeidx].mode; modeidx++)
    {
      size_t modekeylen = keylen * (!!modes[modeidx].doublekey + 1);
      int is_stream = modes[modeidx].mode == GCRY_CIPHER_MODE_STREAM
                      || modes[modeidx].mode == GCRY_CIPHER_MODE_POLY1305;

      if ((blklen > 1 && is_stream) || (blklen == 1 && !is_stream))
        continue;

      if (modes[modeidx].mode == GCRY_CIPHER_MODE_POLY1305
          && algo != GCRY_CIPHER_CHACHA20)
        continue;

      /* GCM is not available in FIPS mode */
      if (in_fips_mode && modes[modeidx].mode == GCRY_CIPHER_MODE_GCM)
        continue;

      if (modes[modeidx].req_blocksize > 0
          && blklen != modes[modeidx].req_blocksize)
        {
          printf (" %7s %7s", "-", "-" );
          continue;
        }

      for (i=0; i < sizeof buf; i++)
        buf[i] = i;

      err = gcry_cipher_open (&hd, algo, modes[modeidx].mode, 0);
      if (err)
        {
          fprintf (stderr, PGM ": error opening cipher `%s'\n", algoname);
          exit (1);
        }

      if (!cipher_with_keysetup)
        {
          err = gcry_cipher_setkey (hd, key, modekeylen);
          if (err)
            {
              fprintf (stderr, "gcry_cipher_setkey failed: %s\n",
                       gpg_strerror (err));
              gcry_cipher_close (hd);
              exit (1);
            }
        }

      buflen = allocated_buflen;
      if (modes[modeidx].blocked)
        buflen = (buflen / blklen) * blklen;

      start_timer ();
      for (i=err=0; !err && i < repetitions; i++)
        {
          if (cipher_with_keysetup)
            {
              err = gcry_cipher_setkey (hd, key, modekeylen);
              if (err)
                {
                  fprintf (stderr, "gcry_cipher_setkey failed: %s\n",
                           gpg_strerror (err));
                  gcry_cipher_close (hd);
                  exit (1);
                }
            }

          if (modes[modeidx].noncelen)
            {
              char nonce[100];
              size_t noncelen;

              noncelen = modes[modeidx].noncelen;
              if (noncelen > sizeof nonce)
                noncelen = sizeof nonce;
              memset (nonce, 42, noncelen);
              err = gcry_cipher_setiv (hd, nonce, noncelen);
              if (err)
                {
                  fprintf (stderr, "gcry_cipher_setiv failed: %s\n",
                           gpg_strerror (err));
                  gcry_cipher_close (hd);
                  exit (1);
                }
            }

          if (modes[modeidx].aead_init)
            {
              (*modes[modeidx].aead_init) (hd, buflen, modes[modeidx].authlen);
              gcry_cipher_final (hd);
              err = cipher_encrypt (hd, outbuf, buflen, buf, buflen,
				    modes[modeidx].max_inlen);
              if (err)
                break;
              err = gcry_cipher_gettag (hd, outbuf, modes[modeidx].authlen);
            }
          else
            {
              err = cipher_encrypt (hd, outbuf, buflen, buf, buflen,
				    modes[modeidx].max_inlen);
            }
        }
      stop_timer ();

      printf (" %s", elapsed_time (1));
      fflush (stdout);
      gcry_cipher_close (hd);
      if (err)
        {
          fprintf (stderr, "gcry_cipher_encrypt failed: %s\n",
                   gpg_strerror (err) );
          exit (1);
        }

      err = gcry_cipher_open (&hd, algo, modes[modeidx].mode, 0);
      if (err)
        {
          fprintf (stderr, PGM ": error opening cipher `%s'/n", algoname);
          exit (1);
        }

      if (!cipher_with_keysetup)
        {
          err = gcry_cipher_setkey (hd, key, modekeylen);
          if (err)
            {
              fprintf (stderr, "gcry_cipher_setkey failed: %s\n",
                       gpg_strerror (err));
              gcry_cipher_close (hd);
              exit (1);
            }
        }

      start_timer ();
      for (i=err=0; !err && i < repetitions; i++)
        {
          if (cipher_with_keysetup)
            {
              err = gcry_cipher_setkey (hd, key, modekeylen);
              if (err)
                {
                  fprintf (stderr, "gcry_cipher_setkey failed: %s\n",
                           gpg_strerror (err));
                  gcry_cipher_close (hd);
                  exit (1);
                }
            }

          if (modes[modeidx].noncelen)
            {
              char nonce[100];
              size_t noncelen;

              noncelen = modes[modeidx].noncelen;
              if (noncelen > sizeof nonce)
                noncelen = sizeof nonce;
              memset (nonce, 42, noncelen);
              err = gcry_cipher_setiv (hd, nonce, noncelen);
              if (err)
                {
                  fprintf (stderr, "gcry_cipher_setiv failed: %s\n",
                           gpg_strerror (err));
                  gcry_cipher_close (hd);
                  exit (1);
                }
            }

          if (modes[modeidx].aead_init)
            {
              (*modes[modeidx].aead_init) (hd, buflen, modes[modeidx].authlen);
              gcry_cipher_final (hd);
              err = cipher_decrypt (hd, outbuf, buflen, buf, buflen,
				    modes[modeidx].max_inlen);
              if (err)
                break;
              err = gcry_cipher_checktag (hd, outbuf, modes[modeidx].authlen);
              if (gpg_err_code (err) == GPG_ERR_CHECKSUM)
                err = 0;
            }
          else
            {
              gcry_cipher_final (hd);
              err = cipher_decrypt (hd, outbuf, buflen, buf, buflen,
				    modes[modeidx].max_inlen);
            }
        }
      stop_timer ();
      printf (" %s", elapsed_time (1));
      fflush (stdout);
      gcry_cipher_close (hd);
      if (err)
        {
          fprintf (stderr, "gcry_cipher_decrypt failed: %s\n",
                   gpg_strerror (err) );
          exit (1);
        }
    }

  putchar ('\n');
  gcry_free (raw_buf);
  gcry_free (raw_outbuf);
}



static void
rsa_bench (int iterations, int print_header, int no_blinding)
{
  gpg_error_t err;
  int p_sizes[] = { 1024, 2048, 3072, 4096 };
  int testno;

  if (print_header)
    printf ("Algorithm         generate %4d*priv %4d*public\n"
            "------------------------------------------------\n",
            iterations, iterations );
  for (testno=0; testno < DIM (p_sizes); testno++)
    {
      gcry_sexp_t key_spec, key_pair, pub_key, sec_key;
      gcry_mpi_t x;
      gcry_sexp_t data;
      gcry_sexp_t sig = NULL;
      int count;
      unsigned nbits = p_sizes[testno];

      printf ("RSA %3d bit    ", nbits);
      fflush (stdout);

      if (in_fips_mode && nbits < 2048)
        {
          puts ("[skipped in fips mode]");
          continue;
        }

      err = gcry_sexp_build (&key_spec, NULL,
                             gcry_fips_mode_active ()
                             ? "(genkey (RSA (nbits %d)))"
                             : "(genkey (RSA (nbits %d)(transient-key)))",
                             nbits);
      if (err)
        die ("creating S-expression failed: %s\n", gcry_strerror (err));

      start_timer ();
      err = gcry_pk_genkey (&key_pair, key_spec);
      if (err)
        die ("creating %d bit RSA key failed: %s\n",
             nbits, gcry_strerror (err));

      pub_key = gcry_sexp_find_token (key_pair, "public-key", 0);
      if (! pub_key)
        die ("public part missing in key\n");
      sec_key = gcry_sexp_find_token (key_pair, "private-key", 0);
      if (! sec_key)
        die ("private part missing in key\n");
      gcry_sexp_release (key_pair);
      gcry_sexp_release (key_spec);

      stop_timer ();
      printf ("   %s", elapsed_time (1));
      fflush (stdout);

      x = gcry_mpi_new (nbits);
      gcry_mpi_randomize (x, nbits-8, GCRY_WEAK_RANDOM);
      err = gcry_sexp_build (&data, NULL,
                             "(data (flags raw) (value %m))", x);
      gcry_mpi_release (x);
      if (err)
        die ("converting data failed: %s\n", gcry_strerror (err));

      start_timer ();
      for (count=0; count < iterations; count++)
        {
          gcry_sexp_release (sig);
          err = gcry_pk_sign (&sig, data, sec_key);
          if (err)
            die ("signing failed (%d): %s\n", count, gpg_strerror (err));
        }
      stop_timer ();
      printf ("   %s", elapsed_time (1));
      fflush (stdout);

      start_timer ();
      for (count=0; count < iterations; count++)
        {
          err = gcry_pk_verify (sig, data, pub_key);
          if (err)
            {
              putchar ('\n');
              show_sexp ("seckey:\n", sec_key);
              show_sexp ("data:\n", data);
              show_sexp ("sig:\n", sig);
              die ("verify failed (%d): %s\n", count, gpg_strerror (err));
            }
        }
      stop_timer ();
      printf ("     %s", elapsed_time (1));

      if (no_blinding)
        {
          fflush (stdout);
          x = gcry_mpi_new (nbits);
          gcry_mpi_randomize (x, nbits-8, GCRY_WEAK_RANDOM);
          err = gcry_sexp_build (&data, NULL,
                                 "(data (flags no-blinding) (value %m))", x);
          gcry_mpi_release (x);
          if (err)
            die ("converting data failed: %s\n", gcry_strerror (err));

          start_timer ();
          for (count=0; count < iterations; count++)
            {
              gcry_sexp_release (sig);
              err = gcry_pk_sign (&sig, data, sec_key);
              if (err)
                die ("signing failed (%d): %s\n", count, gpg_strerror (err));
            }
          stop_timer ();
          printf ("   %s", elapsed_time (1));
          fflush (stdout);
        }

      putchar ('\n');
      fflush (stdout);

      gcry_sexp_release (sig);
      gcry_sexp_release (data);
      gcry_sexp_release (sec_key);
      gcry_sexp_release (pub_key);
    }
}


static void
elg_bench (int iterations, int print_header)
{
  gpg_error_t err;
  gcry_sexp_t pub_key[3], sec_key[3];
  int p_sizes[3] = { 1024, 2048, 3072 };
  gcry_sexp_t data = NULL;
  gcry_sexp_t enc = NULL;
  gcry_sexp_t plain = NULL;
  int i, j;

  err = gcry_sexp_sscan (pub_key+0, NULL, sample_public_elg_key_1024,
                         strlen (sample_public_elg_key_1024));
  if (!err)
    err = gcry_sexp_sscan (sec_key+0, NULL, sample_private_elg_key_1024,
                           strlen (sample_private_elg_key_1024));
  if (!err)
    err = gcry_sexp_sscan (pub_key+1, NULL, sample_public_elg_key_2048,
                           strlen (sample_public_elg_key_2048));
  if (!err)
    err = gcry_sexp_sscan (sec_key+1, NULL, sample_private_elg_key_2048,
                           strlen (sample_private_elg_key_2048));
  if (!err)
    err = gcry_sexp_sscan (pub_key+2, NULL, sample_public_elg_key_3072,
                           strlen (sample_public_elg_key_3072));
  if (!err)
    err = gcry_sexp_sscan (sec_key+2, NULL, sample_private_elg_key_3072,
                           strlen (sample_private_elg_key_3072));
  if (err)
    {
      fprintf (stderr, PGM ": converting sample keys failed: %s\n",
               gcry_strerror (err));
      exit (1);
    }

  if (print_header)
    printf ("Algorithm         generate %4d*priv %4d*public\n"
            "------------------------------------------------\n",
            iterations, iterations );
  for (i=0; i < DIM (p_sizes); i++)
    {
      char timerbuf1[100];

      {
        gcry_mpi_t x = gcry_mpi_new (p_sizes[i]);
        gcry_mpi_randomize (x, p_sizes[i] - 16, GCRY_WEAK_RANDOM);
        err = gcry_sexp_build (&data, NULL, "(data (flags raw) (value %m))", x);
        gcry_mpi_release (x);
      }
      if (err)
        {
          fprintf (stderr, PGM ": converting data failed: %s\n",
                   gcry_strerror (err));
          exit (1);
        }

      printf ("ELG %d bit             -", p_sizes[i]);
      fflush (stdout);

      if (in_fips_mode)
        {
          puts ("[skipped in fips mode]");
          goto next;
        }

      start_timer ();
      for (j=0; j < iterations; j++)
        {
          gcry_sexp_release (enc);
          err = gcry_pk_encrypt (&enc, data, pub_key[i]);
          if (err)
            {
              putchar ('\n');
              fprintf (stderr, PGM ": encrypt failed: %s\n",
                       gpg_strerror (err));
              exit (1);
            }
        }
      stop_timer ();
      snprintf (timerbuf1, sizeof timerbuf1, "   %s", elapsed_time (1));
      fflush (stdout);

      start_timer ();
      for (j=0; j < iterations; j++)
        {
          gcry_sexp_release (plain);
          err = gcry_pk_decrypt (&plain, enc, sec_key[i]);
          if (err)
            {
              putchar ('\n');
              fprintf (stderr, PGM ": decrypt failed: %s\n",
                       gpg_strerror (err));
              exit (1);
            }
        }
      stop_timer ();

      printf ("   %s  %s\n", elapsed_time (1), timerbuf1);
      fflush (stdout);

    next:
      gcry_sexp_release (plain);
      plain = NULL;
      gcry_sexp_release (enc);
      enc = NULL;
      gcry_sexp_release (data);
      data = NULL;
    }

  for (i=0; i < DIM (p_sizes); i++)
    {
      gcry_sexp_release (sec_key[i]);
      gcry_sexp_release (pub_key[i]);
    }
}


static void
dsa_bench (int iterations, int print_header)
{
  gpg_error_t err;
  gcry_sexp_t pub_key[3], sec_key[3];
  int p_sizes[3] = { 1024, 2048, 3072 };
  int q_sizes[3] = { 160, 224, 256 };
  gcry_sexp_t data;
  gcry_sexp_t sig = NULL;
  int i, j;

  err = gcry_sexp_sscan (pub_key+0, NULL, sample_public_dsa_key_1024,
                         strlen (sample_public_dsa_key_1024));
  if (!err)
    err = gcry_sexp_sscan (sec_key+0, NULL, sample_private_dsa_key_1024,
                           strlen (sample_private_dsa_key_1024));
  if (!err)
    err = gcry_sexp_sscan (pub_key+1, NULL, sample_public_dsa_key_2048,
                           strlen (sample_public_dsa_key_2048));
  if (!err)
    err = gcry_sexp_sscan (sec_key+1, NULL, sample_private_dsa_key_2048,
                           strlen (sample_private_dsa_key_2048));
  if (!err)
    err = gcry_sexp_sscan (pub_key+2, NULL, sample_public_dsa_key_3072,
                           strlen (sample_public_dsa_key_3072));
  if (!err)
    err = gcry_sexp_sscan (sec_key+2, NULL, sample_private_dsa_key_3072,
                           strlen (sample_private_dsa_key_3072));
  if (err)
    {
      fprintf (stderr, PGM ": converting sample keys failed: %s\n",
               gcry_strerror (err));
      exit (1);
    }

  if (print_header)
    printf ("Algorithm         generate %4d*priv %4d*public\n"
            "------------------------------------------------\n",
            iterations, iterations );
  for (i=0; i < DIM (q_sizes); i++)
    {
      gcry_mpi_t x;

      x = gcry_mpi_new (q_sizes[i]);
      gcry_mpi_randomize (x, q_sizes[i], GCRY_WEAK_RANDOM);
      err = gcry_sexp_build (&data, NULL, "(data (flags raw) (value %m))", x);
      gcry_mpi_release (x);
      if (err)
        {
          fprintf (stderr, PGM ": converting data failed: %s\n",
                   gcry_strerror (err));
          exit (1);
        }

      printf ("DSA %d/%d             -", p_sizes[i], q_sizes[i]);
      fflush (stdout);

      if (in_fips_mode)
        {
          puts ("[skipped in fips mode]");
          goto next;
        }

      start_timer ();
      for (j=0; j < iterations; j++)
        {
          gcry_sexp_release (sig);
          err = gcry_pk_sign (&sig, data, sec_key[i]);
          if (err)
            {
              putchar ('\n');
              fprintf (stderr, PGM ": signing failed: %s\n",
                       gpg_strerror (err));
              exit (1);
            }
        }
      stop_timer ();
      printf ("   %s", elapsed_time (1));
      fflush (stdout);

      start_timer ();
      for (j=0; j < iterations; j++)
        {
          err = gcry_pk_verify (sig, data, pub_key[i]);
          if (err)
            {
              putchar ('\n');
              fprintf (stderr, PGM ": verify failed: %s\n",
                       gpg_strerror (err));
              exit (1);
            }
        }
      stop_timer ();
      printf ("     %s\n", elapsed_time (1));
      fflush (stdout);

    next:
      gcry_sexp_release (sig);
      gcry_sexp_release (data);
      sig = NULL;
    }


  for (i=0; i < DIM (q_sizes); i++)
    {
      gcry_sexp_release (sec_key[i]);
      gcry_sexp_release (pub_key[i]);
    }
}


static void
ecc_bench (int iterations, int print_header)
{
#if USE_ECC
  gpg_error_t err;
  const char *p_sizes[] = { "192", "224", "256", "384", "521", "Ed25519", "Ed448",
              "gost256", "gost512" };
  int testno;

  if (print_header)
    printf ("Algorithm         generate %4d*priv %4d*public\n"
            "------------------------------------------------\n",
            iterations, iterations );
  for (testno=0; testno < DIM (p_sizes); testno++)
    {
      gcry_sexp_t key_spec, key_pair, pub_key, sec_key;
      gcry_mpi_t x;
      gcry_sexp_t data;
      gcry_sexp_t sig = NULL;
      int count;
      int p_size;
      int is_ed25519;
      int is_ed448;
      int is_gost;

      is_ed25519 = !strcmp (p_sizes[testno], "Ed25519");
      is_ed448 = !strcmp (p_sizes[testno], "Ed448");
      is_gost = !strncmp (p_sizes[testno], "gost", 4);

      /* Only P-{224,256,384,521} are allowed in fips mode */
      if (gcry_fips_mode_active()
          && (is_ed25519 || is_ed448 || is_gost
              || !strcmp (p_sizes[testno], "192")))
         continue;

      if (is_ed25519)
        {
          p_size = 256;
          printf ("EdDSA Ed25519 ");
          fflush (stdout);
        }
      else if (is_ed448)
        {
          p_size = 448;
          printf ("EdDSA Ed448   ");
          fflush (stdout);
        }
      else if (is_gost)
        {
          p_size = atoi (p_sizes[testno] + 4);
          printf ("GOST  %3d bit ", p_size);
          fflush (stdout);
        }
      else
        {
          p_size = atoi (p_sizes[testno]);
          printf ("ECDSA %3d bit ", p_size);
        }
      fflush (stdout);

      if (is_ed25519)
        err = gcry_sexp_build (&key_spec, NULL,
                               "(genkey (ecdsa (curve \"Ed25519\")"
                               "(flags eddsa)))");
      else if (is_ed448)
        err = gcry_sexp_build (&key_spec, NULL,
                               "(genkey (ecdsa (curve \"Ed448\")"
                               "(flags eddsa)))");
      else if (is_gost)
        err = gcry_sexp_build (&key_spec, NULL,
                               "(genkey (ecdsa (curve %s)))",
                               p_size == 256 ? "GOST2001-test" : "GOST2012-512-test");
      else
        err = gcry_sexp_build (&key_spec, NULL,
                               "(genkey (ECDSA (nbits %d)))", p_size);
      if (err)
        die ("creating S-expression failed: %s\n", gcry_strerror (err));

      start_timer ();
      err = gcry_pk_genkey (&key_pair, key_spec);
      if (err)
        die ("creating %d bit ECC key failed: %s\n",
             p_size, gcry_strerror (err));
      if (verbose > 2)
        show_sexp ("ECC key:\n", key_pair);

      pub_key = gcry_sexp_find_token (key_pair, "public-key", 0);
      if (! pub_key)
        die ("public part missing in key\n");
      sec_key = gcry_sexp_find_token (key_pair, "private-key", 0);
      if (! sec_key)
        die ("private part missing in key\n");
      gcry_sexp_release (key_pair);
      gcry_sexp_release (key_spec);

      stop_timer ();
      printf ("     %s", elapsed_time (1));
      fflush (stdout);

      x = gcry_mpi_new (p_size);
      gcry_mpi_randomize (x, p_size, GCRY_WEAK_RANDOM);
      if (is_ed25519)
        err = gcry_sexp_build (&data, NULL,
                               "(data (flags eddsa)(hash-algo sha512)"
                               " (value %m))", x);
      else if (is_ed448)
        err = gcry_sexp_build (&data, NULL,
                               "(data (flags eddsa)(hash-algo shake256)"
                               " (value %m))", x);
      else if (is_gost)
        err = gcry_sexp_build (&data, NULL, "(data (flags gost) (value %m))", x);
      else
        err = gcry_sexp_build (&data, NULL, "(data (flags raw) (value %m))", x);
      gcry_mpi_release (x);

      if (err)
        die ("converting data failed: %s\n", gcry_strerror (err));

      start_timer ();
      for (count=0; count < iterations; count++)
        {
          gcry_sexp_release (sig);
          err = gcry_pk_sign (&sig, data, sec_key);
          if (err)
            {
              if (verbose)
                {
                  putc ('\n', stderr);
                  show_sexp ("signing key:\n", sec_key);
                  show_sexp ("signed data:\n", data);
                }
              die ("signing failed: %s\n", gpg_strerror (err));
            }
        }
      stop_timer ();
      printf ("   %s", elapsed_time (1));
      fflush (stdout);

      start_timer ();
      for (count=0; count < iterations; count++)
        {
          err = gcry_pk_verify (sig, data, pub_key);
          if (err)
            {
              putchar ('\n');
              show_sexp ("seckey:\n", sec_key);
              show_sexp ("data:\n", data);
              show_sexp ("sig:\n", sig);
              die ("verify failed: %s\n", gpg_strerror (err));
            }
        }
      stop_timer ();
      printf ("     %s\n", elapsed_time (1));
      fflush (stdout);

      gcry_sexp_release (sig);
      gcry_sexp_release (data);
      gcry_sexp_release (sec_key);
      gcry_sexp_release (pub_key);
    }
#endif /*USE_ECC*/
}



static void
do_powm ( const char *n_str, const char *e_str, const char *m_str)
{
  gcry_mpi_t e, n, msg, cip;
  gcry_error_t err;
  int i;

  err = gcry_mpi_scan (&n, GCRYMPI_FMT_HEX, n_str, 0, 0);
  if (err) BUG ();
  err = gcry_mpi_scan (&e, GCRYMPI_FMT_HEX, e_str, 0, 0);
  if (err) BUG ();
  err = gcry_mpi_scan (&msg, GCRYMPI_FMT_HEX, m_str, 0, 0);
  if (err) BUG ();

  cip = gcry_mpi_new (0);

  start_timer ();
  for (i=0; i < 1000; i++)
    gcry_mpi_powm (cip, msg, e, n);
  stop_timer ();
  printf (" %s", elapsed_time (1)); fflush (stdout);
/*    { */
/*      char *buf; */

/*      if (gcry_mpi_aprint (GCRYMPI_FMT_HEX, (void**)&buf, NULL, cip)) */
/*        BUG (); */
/*      printf ("result: %s\n", buf); */
/*      gcry_free (buf); */
/*    } */
  gcry_mpi_release (cip);
  gcry_mpi_release (msg);
  gcry_mpi_release (n);
  gcry_mpi_release (e);
}


static void
mpi_bench (void)
{
  printf ("%-10s", "powm"); fflush (stdout);

  do_powm (
"20A94417D4D5EF2B2DA99165C7DC87DADB3979B72961AF90D09D59BA24CB9A10166FDCCC9C659F2B9626EC23F3FA425F564A072BA941B03FA81767CC289E4",
           "29",
"B870187A323F1ECD5B8A0B4249507335A1C4CE8394F38FD76B08C78A42C58F6EA136ACF90DFE8603697B1694A3D81114D6117AC1811979C51C4DD013D52F8"
           );
  do_powm (
           "20A94417D4D5EF2B2DA99165C7DC87DADB3979B72961AF90D09D59BA24CB9A10166FDCCC9C659F2B9626EC23F3FA425F564A072BA941B03FA81767CC289E41071F0246879A442658FBD18C1771571E7073EEEB2160BA0CBFB3404D627069A6CFBD53867AD2D9D40231648000787B5C84176B4336144644AE71A403CA40716",
           "29",
           "B870187A323F1ECD5B8A0B4249507335A1C4CE8394F38FD76B08C78A42C58F6EA136ACF90DFE8603697B1694A3D81114D6117AC1811979C51C4DD013D52F8FC4EE4BB446B83E48ABED7DB81CBF5E81DE4759E8D68AC985846D999F96B0D8A80E5C69D272C766AB8A23B40D50A4FA889FBC2BD2624222D8EB297F4BAEF8593847"
           );
  do_powm (
           "20A94417D4D5EF2B2DA99165C7DC87DADB3979B72961AF90D09D59BA24CB9A10166FDCCC9C659F2B9626EC23F3FA425F564A072BA941B03FA81767CC289E41071F0246879A442658FBD18C1771571E7073EEEB2160BA0CBFB3404D627069A6CFBD53867AD2D9D40231648000787B5C84176B4336144644AE71A403CA4071620A94417D4D5EF2B2DA99165C7DC87DADB3979B72961AF90D09D59BA24CB9A10166FDCCC9C659F2B9626EC23F3FA425F564A072BA941B03FA81767CC289E41071F0246879A442658FBD18C1771571E7073EEEB2160BA0CBFB3404D627069A6CFBD53867AD2D9D40231648000787B5C84176B4336144644AE71A403CA40716",
           "29",
           "B870187A323F1ECD5B8A0B4249507335A1C4CE8394F38FD76B08C78A42C58F6EA136ACF90DFE8603697B1694A3D81114D6117AC1811979C51C4DD013D52F8FC4EE4BB446B83E48ABED7DB81CBF5E81DE4759E8D68AC985846D999F96B0D8A80E5C69D272C766AB8A23B40D50A4FA889FBC2BD2624222D8EB297F4BAEF8593847B870187A323F1ECD5B8A0B4249507335A1C4CE8394F38FD76B08C78A42C58F6EA136ACF90DFE8603697B1694A3D81114D6117AC1811979C51C4DD013D52F8FC4EE4BB446B83E48ABED7DB81CBF5E81DE4759E8D68AC985846D999F96B0D8A80E5C69D272C766AB8A23B40D50A4FA889FBC2BD2624222D8EB297F4BAEF8593847"
           );

  putchar ('\n');


}


static void
prime_bench (void)
{
  gpg_error_t err;
  int i;
  gcry_mpi_t prime;
  int old_prog = single_char_progress;

  single_char_progress = 1;
  if (!with_progress)
    printf ("%-10s", "prime");
  fflush (stdout);
  start_timer ();
  for (i=0; i < 10; i++)
    {
      if (with_progress)
        fputs ("primegen ", stdout);
      err = gcry_prime_generate (&prime,
                                 1024, 0,
                                 NULL,
                                 NULL, NULL,
                                 GCRY_WEAK_RANDOM,
                                 GCRY_PRIME_FLAG_SECRET);
      if (with_progress)
        {
          fputc ('\n', stdout);
          fflush (stdout);
        }
      if (err)
        {
          fprintf (stderr, PGM ": error creating prime: %s\n",
                   gpg_strerror (err));
          exit (1);
        }
      gcry_mpi_release (prime);
    }
  stop_timer ();
  if (with_progress)
    printf ("%-10s", "prime");
  printf (" %s\n", elapsed_time (1)); fflush (stdout);

  single_char_progress = old_prog;
}


int
main( int argc, char **argv )
{
  int last_argc = -1;
  int no_blinding = 0;
  int use_secmem = 0;
  int pk_count = 100;

  buffer_alignment = 1;

  if (argc)
    { argc--; argv++; }

  /* We skip this test if we are running under the test suite (no args
     and srcdir defined) and GCRYPT_NO_BENCHMARKS is set.  */
  if (!argc && getenv ("srcdir") && getenv ("GCRYPT_NO_BENCHMARKS"))
    exit (77);

  if (getenv ("GCRYPT_IN_REGRESSION_TEST"))
    {
      in_regression_test = 1;
      pk_count = 10;
    }

  while (argc && last_argc != argc )
    {
      last_argc = argc;
      if (!strcmp (*argv, "--"))
        {
          argc--; argv++;
          break;
        }
      else if (!strcmp (*argv, "--help"))
        {
          fputs ("usage: benchmark "
                 "[md|mac|cipher|random|mpi|rsa|dsa|ecc|prime [algonames]]\n",
                 stdout);
          exit (0);
        }
      else if (!strcmp (*argv, "--verbose"))
        {
          verbose++;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--debug"))
        {
          verbose += 2;
          debug++;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--use-secmem"))
        {
          use_secmem = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--prefer-standard-rng"))
        {
          /* This is anyway the default, but we may want to use it for
             debugging. */
          xgcry_control ((GCRYCTL_SET_PREFERRED_RNG_TYPE, GCRY_RNG_TYPE_STANDARD));
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--prefer-fips-rng"))
        {
          xgcry_control ((GCRYCTL_SET_PREFERRED_RNG_TYPE, GCRY_RNG_TYPE_FIPS));
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--prefer-system-rng"))
        {
          xgcry_control ((GCRYCTL_SET_PREFERRED_RNG_TYPE, GCRY_RNG_TYPE_SYSTEM));
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--no-blinding"))
        {
          no_blinding = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--large-buffers"))
        {
          large_buffers = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--huge-buffers"))
        {
          huge_buffers = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--cipher-repetitions"))
        {
          argc--; argv++;
          if (argc)
            {
              cipher_repetitions = atoi(*argv);
              argc--; argv++;
            }
        }
      else if (!strcmp (*argv, "--cipher-with-keysetup"))
        {
          cipher_with_keysetup = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--hash-repetitions"))
        {
          argc--; argv++;
          if (argc)
            {
              hash_repetitions = atoi(*argv);
              argc--; argv++;
            }
        }
      else if (!strcmp (*argv, "--mac-repetitions"))
        {
          argc--; argv++;
          if (argc)
            {
              mac_repetitions = atoi(*argv);
              argc--; argv++;
            }
        }
      else if (!strcmp (*argv, "--pk-count"))
        {
          argc--; argv++;
          if (argc)
            {
              pk_count = atoi(*argv);
              argc--; argv++;
            }
        }
      else if (!strcmp (*argv, "--alignment"))
        {
          argc--; argv++;
          if (argc)
            {
              buffer_alignment = atoi(*argv);
              argc--; argv++;
            }
        }
      else if (!strcmp (*argv, "--disable-hwf"))
        {
          argc--; argv++;
          if (argc)
            {
              if (gcry_control (GCRYCTL_DISABLE_HWF, *argv, NULL))
                fprintf (stderr, PGM ": unknown hardware feature `%s'"
                         " - option ignored\n", *argv);
              argc--; argv++;
            }
        }
      else if (!strcmp (*argv, "--fips"))
        {
          argc--; argv++;
          /* This command needs to be called before gcry_check_version.  */
          xgcry_control ((GCRYCTL_FORCE_FIPS_MODE, 0));
        }
      else if (!strcmp (*argv, "--progress"))
        {
          argc--; argv++;
          with_progress = 1;
        }
    }

  if (buffer_alignment < 1 || buffer_alignment > 16)
    die ("value for --alignment must be in the range 1 to 16\n");

  xgcry_control ((GCRYCTL_SET_VERBOSITY, (int)verbose));

  if (!gcry_check_version (GCRYPT_VERSION))
    {
      fprintf (stderr, PGM ": version mismatch; pgm=%s, library=%s\n",
               GCRYPT_VERSION, gcry_check_version (NULL));
      exit (1);
    }

  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u , 0));

  if (gcry_fips_mode_active ())
    in_fips_mode = 1;
  else if (!use_secmem)
    xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));

  if (with_progress)
    gcry_set_progress_handler (progress_cb, NULL);

  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));

  if (cipher_repetitions < 1)
    cipher_repetitions = 1;
  if (hash_repetitions < 1)
    hash_repetitions = 1;
  if (mac_repetitions < 1)
    mac_repetitions = 1;

  if (in_regression_test)
    fputs ("Note: " PGM " running in quick regression test mode.\n", stdout);

  if ( !argc )
    {
      xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));
      md_bench (NULL);
      putchar ('\n');
      mac_bench (NULL);
      putchar ('\n');
      cipher_bench (NULL);
      putchar ('\n');
      rsa_bench (pk_count, 1, no_blinding);
      elg_bench (pk_count, 0);
      dsa_bench (pk_count, 0);
      ecc_bench (pk_count, 0);
      putchar ('\n');
      mpi_bench ();
      putchar ('\n');
      random_bench (0);
    }
  else if ( !strcmp (*argv, "random") || !strcmp (*argv, "strongrandom"))
    {
      if (argc == 1)
        random_bench ((**argv == 's'));
      else if (argc == 2)
        {
          xgcry_control ((GCRYCTL_SET_RANDOM_SEED_FILE, argv[1]));
          random_bench ((**argv == 's'));
          xgcry_control ((GCRYCTL_UPDATE_RANDOM_SEED_FILE));
        }
      else
        fputs ("usage: benchmark [strong]random [seedfile]\n", stdout);
    }
  else if ( !strcmp (*argv, "md"))
    {
      if (argc == 1)
        md_bench (NULL);
      else
        for (argc--, argv++; argc; argc--, argv++)
          md_bench ( *argv );
    }
  else if ( !strcmp (*argv, "mac"))
    {
      if (argc == 1)
        mac_bench (NULL);
      else
        for (argc--, argv++; argc; argc--, argv++)
          mac_bench ( *argv );
    }
  else if ( !strcmp (*argv, "cipher"))
    {
      if (argc == 1)
        cipher_bench (NULL);
      else
        for (argc--, argv++; argc; argc--, argv++)
          cipher_bench ( *argv );
    }
  else if ( !strcmp (*argv, "mpi"))
    {
        mpi_bench ();
    }
  else if ( !strcmp (*argv, "pubkey"))
    {
        xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));
        rsa_bench (pk_count, 1, no_blinding);
        elg_bench (pk_count, 0);
        dsa_bench (pk_count, 0);
        ecc_bench (pk_count, 0);
    }
  else if ( !strcmp (*argv, "rsa"))
    {
        xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));
        rsa_bench (pk_count, 1, no_blinding);
    }
  else if ( !strcmp (*argv, "elg"))
    {
        xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));
        elg_bench (pk_count, 1);
    }
  else if ( !strcmp (*argv, "dsa"))
    {
        xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));
        dsa_bench (pk_count, 1);
    }
  else if ( !strcmp (*argv, "ecc"))
    {
        xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));
        ecc_bench (pk_count, 1);
    }
  else if ( !strcmp (*argv, "prime"))
    {
        xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));
        prime_bench ();
    }
  else
    {
      fprintf (stderr, PGM ": bad arguments\n");
      return 1;
    }


  if (in_fips_mode && !gcry_fips_mode_active ())
    fprintf (stderr, PGM ": FIPS mode is not anymore active\n");

  return 0;
}
