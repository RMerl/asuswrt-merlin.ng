BlueZ test coverage
*******************


Automated unit testing
======================

Application		Count	Description
-------------------------------------------
test-crc		   9	Link Layer CRC-24 checksum
test-eir		  14	EIR and AD parsing
test-lib		  14	SDP library functions
test-sdp		 133	SDP qualification test cases
test-uuid		  30	UUID conversion handling
test-mgmt		   9	Management interface handling
test-crypto		   5	Cryptographic toolbox helpers
test-textfile		   4	Old textfile storage format
test-ringbuf		   3	Ring buffer functionality
test-queue		   6	Queue handling functionality
test-uhid		   6	Userspace HID functionality
test-hfp		  29	HFP Audio Gateway functionality
test-avdtp		  60	AVDTP qualification test cases
test-avctp		   9	AVCTP qualification test cases
test-avrcp		 110	AVRCP qualification test cases
test-gobex		  31	Generic OBEX functionality
test-gobex-packet	   9	OBEX packet handling
test-gobex-header	  28	OBEX header handling
test-gobex-apparam	  18	OBEX apparam handling
test-gobex-transfer	  36	OBEX transfer handling
test-gdbus-client	  13	D-Bus client handling
test-gatt		 180	GATT qualification test cases
test-hog		   6	HID Over GATT qualification test cases
			-----
			 761


Automated end-to-end testing
============================

Application		Count	Description
-------------------------------------------
mgmt-tester		 331	Kernel management interface testing
l2cap-tester		  33	Kernel L2CAP implementation testing
rfcomm-tester		   9	Kernel RFCOMM implementation testing
bnep-tester		   1	Kernel BNEP implementation testing
smp-tester		   8	Kernel SMP implementation testing
sco-tester		   8	Kernel SCO implementation testing
gap-tester		   1	Daemon D-Bus API testing
hci-tester		  14	Controller hardware testing
userchan-tester		   3	Kernel HCI User Channel testting
			-----
			 408


Android end-to-end testing
==========================

Application		Count	Description
-------------------------------------------
android-tester		 194	Android HAL interface testing
ipc-tester		 132	Android IPC resistance testing
			-----
			 326


Android automated unit testing
==============================

Application		Count	Description
-------------------------------------------
test-ipc		  14	Android IPC library functions
			-----
			  14
