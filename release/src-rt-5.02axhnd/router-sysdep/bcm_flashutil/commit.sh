#!/bin/sh
# commit image if booted but not committed and /data/commit_image_after_reboot file exists 

if [ -e /data/commit_image_after_reboot ]; then
	rm /data/commit_image_after_reboot
	BOOTSTATE="$(/bin/bcm_bootstate)"

	boot_part="$(/bin/bcm_bootstate | grep 'Booted Partition')"
	first_seq="$(/bin/bcm_bootstate | grep 'First  partition sequence number:')"
	second_seq="$(/bin/bcm_bootstate | grep 'Second partition sequence number:')"

	# cut all before ":"
	boot_part=${boot_part##*":"};
	first_seq=${first_seq##*":"};
	second_seq=${second_seq##*":"};

	if [ "$first_seq" == " 0" ] && [ "$second_seq" == " 999" ]; then
		first_seq="1000"
	fi

	if [ "$second_seq" == " 0" ] && [ "$first_seq" == " 999" ]; then
		second_seq="1000"
	fi

	if [ "$boot_part" == " First" ] && [ "$first_seq" -gt "$second_seq" ]; then
		echo "committing first partition image"
		/bin/bcm_bootstate +1
	fi

	if [ "$boot_part" == " Second" ] && [ "$second_seq" -gt "$first_seq" ]; then
		echo "committing second partition image"
		/bin/bcm_bootstate +2
	fi
fi

