#
my (@diff,$filelist1);
my $LOG_FILE = "C:\\diffs.txt";
main();

sub main () {
	my($error,$output);
	my @filelist = `cleartool lsco -recurse -short -me -cview`;
         
        open(LG,">$LOG_FILE");
        print LG " -----List of files Modified --- \n\n";
	foreach $filelist1(@filelist) {
        print LG "$filelist1\n\n";
        }
        close(LG);
	
	foreach $filelist1(@filelist) {
		open(LG,">>$LOG_FILE");
		print LG "\n\nChanges for file $filelist1\n\n\n";
#use this for serial output
		my @out = `cleartool diff -ser -pred $filelist1`;
#use this for side-by-side diff out put-adjust col width if output is truncated
#		my @out = `cleartool diff -col 250 -pred $filelist1`;
		foreach my $qw (@out) { print LG  $qw;}
		close(LG);
		
	}
 
}

