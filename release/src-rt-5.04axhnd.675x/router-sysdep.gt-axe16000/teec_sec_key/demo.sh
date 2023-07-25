#!/bin/bash

# Run tee-supplicant

generate_key(){
    echo "01234567891bcdef01234567891bcdef" > /data/linux/aes_key
}


generate_plain_text(){
echo "/*" > /data/plaintext
echo " " >> /data/plaintext
echo "Unless you and Broadcom execute a separate written software license" >> /data/plaintext
echo "agreement governing use of this software, this software is licensed" >> /data/plaintext
echo "to you under the terms of the GNU General Public License version 2" >> /data/plaintext
echo "(the \"GPL\"), available at http://www.broadcom.com/licenses/GPLv2.php," >> /data/plaintext
echo "with the following added to such license:" >> /data/plaintext
echo "" >> /data/plaintext
echo "   As a special exception, the copyright holders of this software give" >> /data/plaintext
echo "   you permission to link this software with independent modules, and" >> /data/plaintext
echo "   to copy and distribute the resulting executable under terms of your" >> /data/plaintext
echo "   choice, provided that you also meet, for each linked independent" >> /data/plaintext
echo "   module, the terms and conditions of the license of that module." >> /data/plaintext
echo "   An independent module is a module which is not derived from this" >> /data/plaintext
echo "   software.  The special exception does not apply to any modifications" >> /data/plaintext
echo "   of the software." >> /data/plaintext
echo "" >> /data/plaintext
echo "Not withstanding the above, under no circumstances may you combine" >> /data/plaintext
echo "this software in any way with any other Broadcom software provided" >> /data/plaintext
echo "under a license other than the GPL, without Broadcom's express prior" >> /data/plaintext
echo "written consent." >> /data/plaintext
echo ":>" >> /data/plaintext
echo "*/" >> /data/plaintext
}

clear_screen()
{
    line_num=1
    while [ $line_num -le $1 ]
    do
        echo ""
        line_num=$(( $line_num + 1 ))
    done
}

print_directory () {
ls -g /data/tee > /data/teefile.txt
tee_lines=$(/bin/demo_helper /data/teefile.txt 0)
ls -g /data/linux > /data/linfile.txt
lin_lines=$(/bin/demo_helper /data/linfile.txt 0)

print_line=0
line_num=1
while [ $line_num -le 10 ]
do
   tee_len=0
   if [ $line_num -le $tee_lines ]
   then
       tee_line=$(/bin/demo_helper /data/teefile.txt $line_num)
       tee_len=$(/bin/demo_helper $tee_line -1)
   else
       tee_line=""
       tee_len=0
   fi

   space=$((57 - $tee_len))

   if [ $line_num -le $lin_lines ]
   then
       lin_line=$(/bin/demo_helper /data/linfile.txt $line_num )
   else
       lin_line=""
   fi

   if [ ${#tee_line} -gt 0 ] || [ ${#lin_line} -gt 0 ]
   then
       echo -n $tee_line

       counter=1
       while [ $counter -le $space ]
       do
           echo -n " "
           counter=$(( $counter + 1 ))
       done

       echo "| $lin_line"
       print_line=$(( $print_line + 1 ))
   fi

line_num=$(( $line_num + 1 ))

done

echo " "
if [ $print_line -le 0 ]
then
echo "                                            < Empty Directories > "
fi
echo " "

}

# Find all the process that needs to be killed
ps | grep tee-supplicant > /data/process
ps | grep teec_sec_key >> /data/process
/bin/demo_helper /data/process -9
sleep 1
# Clean up the mess
rm -f /data/sec_fifo
rm -f /data/sec_fifo1
rm -fr /data/tee/
mkdir /data/tee
rm -fr /data/linux
mkdir /data/linux
sleep 1

# Run the necessary apps
/bin/tee-supplicant&
/bin/teec_sec_key&
sleep 1

generate_plain_text

echo "+--------------------------------------------------------------------------------------------------------------+"
echo "| INTRODUCTION                                                                                                 |"
echo "+-------------------------------------------------------+------------------------------------------------------+"
echo "|  On the left side                                     |  On the right side                                   |"
echo "|  - Crypto using Trusted Environment (Optee)           |  - Crypto using Linux (Openssl)                      |"
echo "|  - Keys are encrypted using secure device key         |  - Keys are visible in Linux                         |"
echo "|  - OPTEE performs encryption & decryption             |  - OpenSSL performs encryption & decryption          |"
echo "+-------------------------------------------------------+------------------------------------------------------+"
read -p "Press enter to continue ..."
clear_screen 50
echo "+-------------------------------------------------------+------------------------------------------------------+"
echo "| OPTEE generated files/keys will be inside /data/tee   | Linux generated files/keys willbe inside /data/linux |"
echo "+-------------------------------------------------------+------------------------------------------------------+"
print_directory
read -p "Press enter to continue ..."
clear_screen 50
echo "+--------------------------------------------------------------------------------------------------------------+"
echo "| Key generation                                                                                               |"
echo "+-------------------------------------------------------+------------------------------------------------------+"
echo "| In OPTEE                                              | In Linux                                             |"
echo "|  - teec_sec_key -k keyref1 -gen-aes -l 128            |  - openssl enc -nosalt -aes-128-cbc -k hello -P      |"
echo "| Result                                                | Result                                               |"
echo "|  - Key and IV are NOT visible                         |  - Key and IV are visible                            |"
echo "|  - Key is given a reference: keyref1                  |  - key=5D41402ABC4B2A76B9719D911017C59228B...        |"
echo "|                                                       |  - iv =DE37085F7EDA3711FA2D9FE7E0FC29BE              |"
echo "+-------------------------------------------------------+------------------------------------------------------+"
sleep 1
echo "1 aes_key1" > /data/sec_fifo
sleep 1
echo "6 aes_key1 128" > /data/sec_fifo
generate_key
sleep 2
echo " "
echo "+-------------------------------------------------------+------------------------------------------------------+"
echo "|  Key and IV are accessible ONLY in OPTEE              |  Key and IV are accessible in Linux                  |"
echo "|  - Files inside /data/tee are encrypted by OPTEE      |  - Files inside /data/linux are NOT encrypted        |"
echo "+-------------------------------------------------------+------------------------------------------------------+"
print_directory
read -p "Press enter to continue ..."
clear_screen 50
echo "+--------------------------------------------------------------------------------------------------------------+"
echo "| Plaintext encryption                                                                                         |"
echo "|  - plaintext.txt is a file containing text that we want to encrypt                                           |"
echo "+-------------------------------------------------------+------------------------------------------------------+"
echo "|  In OPTEE                                             |  In Linux                                            |"
echo "|  - teec_sec_key -enc -in plaintext.txt -k keyref1     |  - openssl enc -in plaintext.txt -K 5D4140..-i=DE.   |"
echo "|  - Ciphertext is visible                              |  - Ciphertext is visible                             |"
echo "|  Result                                               |  Result                                              |"
echo "|  - OPTEE side, we refer to the key using keyref1      |  - Linux side, we pass the key & IV explicitly       |"
echo "|  - Ciphertext saved in /data/tee/ciphertext.bin.      |  - Ciphertext saved in /data/linux/ciphertext.bin.   |"
echo "+-------------------------------------------------------+------------------------------------------------------+"
sleep 1
echo "7 aes_key1 /data/plaintext /data/tee/ciphertext.bin" > /data/sec_fifo
aes_key=$(/bin/demo_helper /data/linux/aes_key 1)
result=$(openssl enc -aes-128-ctr -in /data/plaintext -out /data/linux/ciphertext.bin -K $aes_key -iv 00000000000000000000000000000000)
sleep 2
# Find all the process that needs to be killed
ps | grep tee-supplicant > /data/process
ps | grep teec_sec_key >> /data/process
/bin/demo_helper /data/process -9
clear_screen 50
