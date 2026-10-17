.. SPDX-License-Identifier: GPL-2.0+

Renesas
=======

About this
----------

This document describes the information about Renesas supported boards
and their usage steps.

Renesas SoC based boards
------------------------

Renesas is a SoC solutions provider for automotive and industrial applications.

.. list-table:: Supported Renesas SoC based boards
   :widths: 10, 25, 15, 10, 25
   :header-rows: 1

   * - Family
     - Board
     - SoC
     - Arch
     - defconfig

   * - R2D
     - R2D-PLUS
     - SH7751
     - sh
     - r2dplus_defconfig

   * - RZ/A1
     - GR-PEACH
     - R7S72100 (RZ/A1H)
     - arm
     - grpeach_defconfig

   * - R-Car Gen2
     - Lager
     - R8A7790 (H2)
     - arm
     - lager_defconfig

   * -
     - Stout
     - R8A7790 (H2)
     - arm
     - stout_defconfig

   * -
     - Koelsch
     - R8A7791 (M2-W)
     - arm
     - koelsch_defconfig

   * -
     - Porter
     - R8A7791 (M2-W)
     - arm
     - porter_defconfig

   * -
     - Blanche
     - R8A7792 (V2H)
     - arm
     - blanche_defconfig

   * -
     - Gose
     - R8A7793 (M2-N)
     - arm
     - gose_defconfig

   * -
     - Alt
     - R8A7794 (E2)
     - arm
     - alt_defconfig

   * -
     - Silk
     - R8A7794 (E2)
     - arm
     - silk_defconfig

   * - R-Car Gen3
     - Salvator-X(S)
     - R8A77951 (H3)
     - arm64
     - rcar3_salvator-x_defconfig

   * -
     - ULCB
     - R8A77951 (H3)
     - arm64
     - rcar3_ulcb_defconfig

   * -
     - Salvator-X(S)
     - R8A77960 (M3-W)
     - arm64
     - rcar3_salvator-x_defconfig

   * -
     - ULCB
     - R8A77960 (M3-W)
     - arm64
     - rcar3_ulcb_defconfig

   * -
     - Salvator-X(S)
     - R8A77965 (M3-N)
     - arm64
     - rcar3_salvator-x_defconfig

   * -
     - ULCB
     - R8A77965 (M3-N)
     - arm64
     - rcar3_ulcb_defconfig

   * -
     - Eagle
     - R8A77970 (V3M)
     - arm64
     - r8a77970_eagle_defconfig

   * -
     - V3MSK
     - R8A77970 (V3M)
     - arm64
     - r8a77970_v3msk_defconfig

   * -
     - Condor
     - R8A77980 (V3H)
     - arm64
     - r8a77980_condor_defconfig

   * -
     - V3HSK
     - R8A77980 (V3H)
     - arm64
     - r8a77980_v3hsk_defconfig

   * -
     - Ebisu
     - R8A77990 (E3)
     - arm64
     - r8a77990_ebisu_defconfig

   * -
     - Draak
     - R8A77995 (D3)
     - arm64
     - r8a77995_draak_defconfig

   * - R-Car Gen4
     - Falcon
     - R8A779A0 (V3U)
     - arm64
     - r8a779a0_falcon_defconfig

   * -
     - Spider
     - R8A779F0 (S4)
     - arm64
     - r8a779f0_spider_defconfig

   * -
     - White Hawk
     - R8A779G0 (V4H)
     - arm64
     - r8a779g0_whitehawk_defconfig

   * - RZ/G2 Family
     - Beacon EmbeddedWorks RZ/G2M SoM
     - R8A774A1 (RZ/G2M)
     - arm64
     - rzg2_beacon_defconfig

   * -
     - HopeRun HiHope RZ/G2M
     - R8A774A1 (RZ/G2M)
     - arm64
     - hihope_rzg2_defconfig

   * -
     - Beacon EmbeddedWorks RZ/G2N SoM
     - R8A774B1 (RZ/G2N)
     - arm64
     - rzg2_beacon_defconfig

   * -
     - HopeRun HiHope RZ/G2N
     - R8A774B1 (RZ/G2N)
     - arm64
     - hihope_rzg2_defconfig

   * -
     - Silicon Linux RZ/G2E evaluation kit (EK874)
     - R8A774C0 (RZ/G2E)
     - arm64
     - silinux_ek874_defconfig

   * -
     - Beacon EmbeddedWorks RZ/G2H SoM
     - R8A774E1 (RZ/G2H)
     - arm64
     - rzg2_beacon_defconfig

   * -
     - HopeRun HiHope RZ/G2H
     - R8A774E1 (RZ/G2H)
     - arm64
     - hihope_rzg2_defconfig

   * - :doc:`RZ/N1 Family <rzn1>`
     - Schneider RZ/N1D board
     - R9A06G032 (RZ/N1D)
     - arm64
     - rzn1_snarc_defconfig

   * -
     - Schneider RZ/N1S board
     - R9A06G033 (RZ/N1S)
     - arm64
     - rzn1_snarc_defconfig

Build
-----

Locate the appropriate defconfig in the table above. Then apply standard build
procedure::

    make <board_defconfig>
    make
