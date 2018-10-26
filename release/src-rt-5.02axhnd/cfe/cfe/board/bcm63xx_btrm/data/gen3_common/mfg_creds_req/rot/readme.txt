The Avago buyout of brcm put the control and access of the IT security-controlled HSM into uncertainty.
For gen3 SoCs, the RSA private/public key within the HSM is denoted as Krot-mfg. Since it is used only 
during the manufacturing secure mode, and is out of the picture for field secure mode, all new gen3 SoCs
have Krot-mfg stored in the directory in which this readme is located and no longer uses the HSM. The
file Krot-mfg-encrypted.pem is aes-128-cbc encrypted with the same pass-phrase that encrypts the files
bcm63xx_encr*.c located in the cfe/cfe/board/bcm63xx_btrm/src direcotry. After the file is decrypted,
the pem file contains both the private and public portion of the RSA key Krot-mfg
