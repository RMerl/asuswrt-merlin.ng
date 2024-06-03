/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Intel Corporation
 *
 * Partially based on southcluster.asl for other x86 platforms
 */

Device (PCI0)
{
    Name (_HID, EISAID("PNP0A08"))    /* PCIe */
    Name (_CID, EISAID("PNP0A03"))    /* PCI */

    Name (_ADR, 0)
    Name (_BBN, 0)

    Name (MCRS, ResourceTemplate()
    {
        /* Bus Numbers */
        WordBusNumber(ResourceProducer, MinFixed, MaxFixed, PosDecode,
                0x0000, 0x0000, 0x00ff, 0x0000, 0x0100, , , PB00)

        /* IO Region 0 */
        WordIO(ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
                0x0000, 0x0000, 0x0cf7, 0x0000, 0x0cf8, , , PI00)

        /* PCI Config Space */
        IO(Decode16, 0x0cf8, 0x0cf8, 0x0001, 0x0008)

        /* IO Region 1 */
        WordIO(ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
                0x0000, 0x0d00, 0xffff, 0x0000, 0xf300, , , PI01)

        /* GPIO Low Memory Region */
        DWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed,
                Cacheable, ReadWrite,
                0x00000000, 0x000ddcc0, 0x000ddccf, 0x00000000,
                0x00000010, , , GP00)

        /* PSH Memory Region 0 */
        DWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed,
                Cacheable, ReadWrite,
                0x00000000, 0x04819000, 0x04898fff, 0x00000000,
                0x00080000, , , PSH0)

        /* PSH Memory Region 1 */
        DWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed,
                Cacheable, ReadWrite,
                0x00000000, 0x04919000, 0x04920fff, 0x00000000,
                0x00008000, , , PSH1)

        /* SST Memory Region */
        DWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed,
                Cacheable, ReadWrite,
                0x00000000, 0x05e00000, 0x05ffffff, 0x00000000,
                0x00200000, , , SST0)

        /* PCI Memory Region */
        DWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed,
                Cacheable, ReadWrite,
                0x00000000, 0x80000000, 0xffffffff, 0x00000000,
                0x80000000, , , PMEM)
    })

    Method (_CRS, 0, Serialized)
    {
        Return (MCRS)
    }

    Method (_OSC, 4)
    {
        /* Check for proper GUID */
        If (LEqual(Arg0, ToUUID("33db4d5b-1ff7-401c-9657-7441c03dd766"))) {
            /* Let OS control everything */
            Return (Arg3)
        } Else {
            /* Unrecognized UUID */
            CreateDWordField(Arg3, 0, CDW1)
            Or(CDW1, 4, CDW1)
            Return (Arg3)
        }
    }

    Device (SDHC)
    {
        Name (_ADR, 0x00010003)
        Name (_DEP, Package (0x01)
        {
            GPIO
        })
        Name (PSTS, Zero)

        Method (_STA)
        {
            Return (STA_VISIBLE)
        }

        Method (_PS3, 0, NotSerialized)
        {
        }

        Method (_PS0, 0, NotSerialized)
        {
            If (PSTS == Zero)
            {
                If (^^GPIO.AVBL == One)
                {
                    ^^GPIO.WFD3 = One
                    PSTS = One
                }
            }
        }

        /* BCM43340 */
        Device (BRC1)
        {
            Name (_ADR, 0x01)
            Name (_DEP, Package (0x01)
            {
                GPIO
            })

            Method (_STA)
            {
                Return (STA_VISIBLE)
            }

            Method (_RMV, 0, NotSerialized)
            {
                Return (Zero)
            }

            Method (_PS3, 0, NotSerialized)
            {
                If (^^^GPIO.AVBL == One)
                {
                    ^^^GPIO.WFD3 = Zero
                    PSTS = Zero
                }
            }

            Method (_PS0, 0, NotSerialized)
            {
                If (PSTS == Zero)
                {
                    If (^^^GPIO.AVBL == One)
                    {
                        ^^^GPIO.WFD3 = One
                        PSTS = One
                    }
                }
            }
        }

        Device (BRC2)
        {
            Name (_ADR, 0x02)
            Method (_STA, 0, NotSerialized)
            {
                Return (STA_VISIBLE)
            }

            Method (_RMV, 0, NotSerialized)
            {
                Return (Zero)
            }
        }
    }

    Device (SPI5)
    {
        Name (_ADR, 0x00070001)
        Name (RBUF, ResourceTemplate()
        {
            GpioIo(Exclusive, PullUp, 0, 0, IoRestrictionOutputOnly,
                "\\_SB.PCI0.GPIO", 0, ResourceConsumer, , ) { 110 }
            GpioIo(Exclusive, PullUp, 0, 0, IoRestrictionOutputOnly,
                "\\_SB.PCI0.GPIO", 0, ResourceConsumer, , ) { 111 }
            GpioIo(Exclusive, PullUp, 0, 0, IoRestrictionOutputOnly,
                "\\_SB.PCI0.GPIO", 0, ResourceConsumer, , ) { 112 }
            GpioIo(Exclusive, PullUp, 0, 0, IoRestrictionOutputOnly,
                "\\_SB.PCI0.GPIO", 0, ResourceConsumer, , ) { 113 }

            FixedDMA(0x000d, 0x0002, Width32bit, )
            FixedDMA(0x000c, 0x0003, Width32bit, )
        })

        Method (_CRS, 0, NotSerialized)
        {
            Return (RBUF)
        }

        /*
         * See
         * http://www.kernel.org/doc/Documentation/acpi/gpio-properties.txt
         * for more information about GPIO bindings.
         */
        Name (_DSD, Package () {
            ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
            Package () {
                Package () {
                    "cs-gpios", Package () {
                        ^SPI5, 0, 0, 0,
                        ^SPI5, 1, 0, 0,
                        ^SPI5, 2, 0, 0,
                        ^SPI5, 3, 0, 0,
                    },
                },
            }
        })

        Method (_STA, 0, NotSerialized)
        {
            Return (STA_VISIBLE)
        }
    }

    Device (I2C1)
    {
        Name (_ADR, 0x00080000)

        Method (_STA, 0, NotSerialized)
        {
            Return (STA_VISIBLE)
        }

        Name (RBUF, ResourceTemplate()
        {
            FixedDMA(0x0009, 0x0000, Width32bit, )
            FixedDMA(0x0008, 0x0001, Width32bit, )
        })

        Method (_CRS, 0, NotSerialized)
        {
            Return (RBUF)
        }
    }

    Device (I2C6)
    {
        Name (_ADR, 0x00090001)

        Method (_STA, 0, NotSerialized)
        {
            Return (STA_VISIBLE)
        }
    }

    Device (GPIO)
    {
        Name (_ADR, 0x000c0000)

        Method (_STA)
        {
            Return (STA_VISIBLE)
        }

        Name (AVBL, Zero)
        Method (_REG, 2, NotSerialized)
        {
            If (Arg0 == 0x08)
            {
                AVBL = Arg1
            }
        }

        OperationRegion (GPOP, GeneralPurposeIo, 0, 1)
        Field (GPOP, ByteAcc, NoLock, Preserve)
        {
            Connection (
                GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly,
                    "\\_SB.PCI0.GPIO", 0, ResourceConsumer, , ) { 96 }
            ),
            WFD3, 1,
        }
    }

    Device (PWM0)
    {
        Name (_ADR, 0x00170000)

        Method (_STA, 0, NotSerialized)
        {
            Return (STA_VISIBLE)
        }
    }

    Device (HSU0)
    {
        Name (_ADR, 0x00040001)

        Method (_STA, 0, NotSerialized)
        {
            Return (STA_VISIBLE)
        }

        Device (BTH0)
        {
            Name (_HID, "BCM2E95")
            Name (_DEP, Package ()
            {
                GPIO,
                HSU0
            })

            Method (_STA, 0, NotSerialized)
            {
                Return (STA_VISIBLE)
            }

            Method (_CRS, 0, Serialized)
            {
                Name (RBUF, ResourceTemplate()
                {
                    UartSerialBus(0x0001C200, DataBitsEight, StopBitsOne,
                        0xFC, LittleEndian, ParityTypeNone, FlowControlHardware,
                        0x20, 0x20, "\\_SB.PCI0.HSU0", 0, ResourceConsumer, , )
                    GpioInt(Level, ActiveHigh, Exclusive, PullNone, 0,
                        "\\_SB.PCI0.GPIO", 0, ResourceConsumer, , ) { 185 }
                    GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly,
                        "\\_SB.PCI0.GPIO", 0, ResourceConsumer, , ) { 184 }
                    GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly,
                        "\\_SB.PCI0.GPIO", 0, ResourceConsumer, , ) { 71 }
                })
                Return (RBUF)
            }

            Name (_DSD, Package () {
                ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
                Package () {
                    Package () { "host-wakeup-gpios", Package () { ^BTH0, 0, 0, 0 } },
                    Package () { "device-wakeup-gpios", Package () { ^BTH0, 1, 0, 0 } },
                    Package () { "shutdown-gpios", Package () { ^BTH0, 2, 0, 0 } },
                }
            })
        }
    }

    Device (IPC1)
    {
        Name (_ADR, 0x00130000)

        Method (_STA, 0, NotSerialized)
        {
            Return (STA_VISIBLE)
        }

        Device (PMIC)
        {
            Name (_ADR, Zero)
            Name (_HID, "INTC100E")
            Name (_CID, "INTC100E")
            Name (_DDN, "Basin Cove PMIC")
            Name (_DEP, Package ()
            {
                IPC1
            })

            Method (_STA, 0, NotSerialized)
            {
                Return (STA_VISIBLE)
            }

            Method (_CRS, 0, Serialized)
            {
                Name (RBUF, ResourceTemplate()
                {
                    /*
		     * Shadow registers in SRAM for PMIC:
		     *   SRAM	PMIC register
		     *   --------------------
		     *   0x00-	Unknown
		     *   0x03	THRMIRQ (0x04)
		     *   0x04	BCUIRQ (0x05)
		     *   0x05	ADCIRQ (0x06)
		     *   0x06	CHGRIRQ0 (0x07)
		     *   0x07	CHGRIRQ1 (0x08)
		     *   0x08-	Unknown
		     *   0x0a	PBSTATUS (0x27)
		     *   0x0b-	Unknown
		     */
                    Memory32Fixed(ReadWrite, 0xFFFFF610, 0x00000010)
                    Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, ,, ) { 30 }
                    Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, ,, ) { 23 }
                    Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, ,, ) { 52 }
                    Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, ,, ) { 51 }
                    Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, ,, ) { 50 }
                    Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, ,, ) { 27 }
                    Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, ,, ) { 49 }
                })
                Return (RBUF)
            }

            OperationRegion (PMOP, 0x8D, Zero, 0x0100)
            Field (PMOP, DWordAcc, NoLock, Preserve)
            {
                SEL1,   32,
                SEL2,   32,
                VCCL,   32,
                VNNL,   32,
                AONL,   32,
                CNTC,   32,
                CNTN,   32,
                AONN,   32,
                CNT1,   32,
                CNT2,   32,
                CNT3,   32,
                FLEX,   32,
                PRG1,   32,
                PRG2,   32,
                PRG3,   32,
                VLDO,   32,
            }

            Name (AVBL, Zero)
            Method (_REG, 2, NotSerialized)
            {
                If ((Arg0 == 0x8D))
                {
                    AVBL = Arg1
                }
            }
        }
    }
}

Device (FLIS)
{
    Name (_HID, "INTC1002")
    Name (_DDN, "Intel Merrifield Family-Level Interface Shim")
    Name (RBUF, ResourceTemplate()
    {
        Memory32Fixed(ReadWrite, 0xFF0C0000, 0x00008000)
        PinGroup("spi5", ResourceProducer, ) { 90, 91, 92, 93, 94, 95, 96 }
        PinGroup("uart0", ResourceProducer, ) { 115, 116, 117, 118 }
        PinGroup("uart1", ResourceProducer, ) { 119, 120, 121, 122 }
        PinGroup("uart2", ResourceProducer, ) { 123, 124, 125, 126 }
        PinGroup("pwm0", ResourceProducer, ) { 144 }
        PinGroup("pwm1", ResourceProducer, ) { 145 }
        PinGroup("pwm2", ResourceProducer, ) { 132 }
        PinGroup("pwm3", ResourceProducer, ) { 133 }
    })

    Method (_CRS, 0, NotSerialized)
    {
        Return (RBUF)
    }

    Method (_STA, 0, NotSerialized)
    {
        Return (STA_VISIBLE)
    }
}
