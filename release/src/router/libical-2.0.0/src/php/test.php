<?php

// '../../../../' is a dirty hack to be able to store LibicalWrap.so in my
// homedir instead of in the machine-global directory

if (dl('../../../../home/arnouten/dev/libical-0.23/src/php/LibicalWrap.so')) {
  print "Success\n";
} else {
  print "Problem\n";
  exit();
}

$calstr = `cat /home/qharmony/public_html/kalender/oldcode/cal/calendars/US32Holidays.ics`;

$calendar = icalcomponent_new_from_string($calstr);

$comp = icalcomponent_get_first_component($calendar, 1);

print icalcomponent_as_ical_string($comp);

?>
