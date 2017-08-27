#!/usr/bin/perl -w

# ==============================================================================
#
# Filename: bounce.pl
# Description:
# 	Perl/Tk based GUI front-end to tbox scripts.
#   All Bounce traces can be parsed and processed without the use of this GUI.
#
#	GUI IS NOT FOR COMMERCIAL DEPLOYMENT
#   bounce.pl GUI is ONLY for internal development/debugging.
#
#	GUI icons are only for non-commercial use.
#   Icon Theme by Everaldo Coelho everaldo@veraldo.com www.everaldo.com
#
#
# ======================== INSTALLATION SPECIFIC INFO ==========================
#
# You may place this in your .Xdefaults config 
# 	bounce*font: -adobe-helvetica-medium-r-*-*-14-*-*-*-*-*-*-*
#
#	LightBlue1, SlateGray1, LightCyan1
#	ivory1, ivory3
#	gray95, gray80
#	honeydew1, honeydew3
#

@INC = ( @INC, './images/' );				# pixmap images
use strict;
use English;

# Tk classes used
use Tk;
use Tk qw(exit);
use Tk::widgets qw( Dialog Photo Balloon Button Toplevel TextUndo LabEntry
					BrowseEntry LabFrame NoteBook DirTree Optionmenu );
use Cwd;
use File::Basename;

#--- Globals ---
my $bdbg			= 0;
my $cwd				= getcwd();
my $tfilter			= "$cwd/tbox/tfilter.pl";
my $tindent			= "$cwd/tbox/tindent.pl";
my $symfind			= "$cwd/tbox/symfind.pl";
my $symfix			= "$cwd/tbox/symfix";
my $dos2unix		= "$cwd/tbox/d2u.sh";

my $tracefile		= '';			# input bounce dump trace filename
my $dir_path		= shift;		# argv[1] is the directory path

# Window Cosmetics
my $guiFont         = 'Arial 14';
my $displayFont		= 'Courier 12 normal';
my $backgroundClr	= 'gray80';
my $errorClr		= 'indianred3';
my $funcBgnClr      = 'red';
my $funcEndClr      = 'SeaGreen4';

#--- Windows
use vars qw/$MainWindow $ToolboxWindow/;				# Windows
use vars qw/$openFileImg $tbImg $tbRunImg/;				# Images

#--- Frames
use vars qw/$mwTopFm $mwBotFm/;							# MainWindow frames
use vars qw/$mwFileFm $mwButnFm $mwTBoxFm/;				# MainWindow frames
use vars qw/$tbTopFm $tbLeftFm $tbRightFm $tbBotFm/;	# Toolbox frames

#--- MainWindow widgets and variables
use vars qw/$mwDisplayWd $mwFileEntryWd $mwLoadBtnWd $mwToolboxBtnWd/;
use vars qw/%fileList/;

#--- Toolbox widgets and variables
use vars qw/$tbCaptionWd $tbRunWd $tbBookWd/;
use vars qw/$tbHomeTabWd $tbIndentTabWd $tbFilterTabWd
			$tbResolveTabWd $tbSymBuildTabWd/;			# Notebook tabs

use vars qw/$tbCaption $tbSelect/;

#--- Filter/Collapse widgets and variables
use vars qw/$tbPrunesize $tbPruneWd $tbPruneValue
			$tbFilterBtn $tbCollapseBtn $tbPruneNoneBtn
			$tbFilterTaskValue $tbFilterTaskWd
			$tbFilterApplyCheckValue $tbFilterApplyCheckBtn
			$tbFilterSaveCheckValue $tbFilterSaveCheckBtn
			$tbFilterOpenFile $tbFilterOpenFileWd $tbFilterOpenFileBtn/;

#--- Indent Trace widgets and variables
use vars qw/$tbIndentTabsize $tbIndentTabWd
			$tbIndentApplyCheckValue $tbIndentApplyCheckBtn 
			$tbIndentSaveCheckValue $tbIndentSaveCheckBtn
			$tbIndentOpenFile $tbIndentOpenFileWd $tbIndentOpenFileBtn/;

#--- Resolve Symbol Tool (symfix) widgets and variables
use vars qw/$tbResolveTrace  $tbResolveTraceWd  $tbResolveTraceBtn
			$tbResolveSymfile $tbResolveSymfileWd $tbResolveSymfileBtn/;

#--- Build Symbol Table Tool (symfind) widget and variables
use vars qw/$tbBuildsymViewDir $tbBuildsymViewDirWd $tbBuildsymViewDirBtn
			$tbBuildsymTarget $tbBuildsymTargetWd 
			$tbBuildsymModule $tbBuildsymModuleWd $tbBuildsymModuleBtn
			$tbBuildsymFile $tbBuildsymFileWd/;


#--- MainWindow X configuration
$MainWindow = MainWindow->new(-name => 'bounce', -title => 'Bounce Viewer');
$MainWindow->iconname("Bounce Viewer");

#--- Images used by various widgets
#
# These icons are only for non-commercial use.
# Icon Theme by Everaldo Coelho everaldo@veraldo.com www.everaldo.com
# 
$openFileImg = $MainWindow->Photo(-file=>mwFindImg('hardware_penguinL.gif'));
$tbImg  = $MainWindow->Photo(-file=>mwFindImg('toolboxL.gif'));
$tbRunImg = $MainWindow->Photo(-file=>mwFindImg('penguinL.gif'));

#--- MainWindow layout frames
$mwTopFm  = $MainWindow->Frame()->pack(qw/-fill x -side top/);
$mwBotFm  = $MainWindow->Frame()->pack(qw/-fill both -expand 1 -side top/);

#--- MainWindow Top frame is broken into three frames, arranged left to right
$mwFileFm = $mwTopFm->Frame()->pack(qw/-fill x -expand 1 -side left/);
$mwButnFm = $mwTopFm->Frame()->pack(qw/-side left/);
$mwTBoxFm = $mwTopFm->Frame()->pack(qw/-side left/);


#
#--- MainWindow bounce display text undo widget --------------------------------
#
$mwDisplayWd= $mwBotFm->Scrolled(
		qw/ TextUndo -width 60 -height 40 -wrap none -borderwidth 2
			-setgrid true -background gray95 -selectbackground black
			-selectforeground white -scrollbars se/
	)->pack(qw/-fill both -expand 1 -padx 5 -pady 5/);
$mwDisplayWd->configure(-font => $displayFont);
$mwDisplayWd->tagConfigure('ERROR',
		-background => $errorClr, -foreground => 'white');
$mwDisplayWd->tagConfigure('BGN', -foreground => $funcBgnClr );
$mwDisplayWd->tagConfigure('END', -foreground => $funcEndClr );

tie (*DISPLAY, 'Tk::Text', $mwDisplayWd);
print DISPLAY "\n\tDisplays the input trace or parsed/filtered output\n";


#
#--- MainWindow bounce trace file path entry widget ----------------------------
#
$mwFileEntryWd = $mwFileFm->BrowseEntry(
		-width => 60, -font => $guiFont, -textvariable => \$tracefile,
		-state => 'normal', -style => 'MSWin32',
		-label => 'Bounce Filename: ', -labelPack => [-side => 'left'],
		-browsecmd => \&mwOpenTrace)
	->pack(qw/-fill x -expand 1 -padx 5/);
$mwFileEntryWd->Subwidget('entry')->Subwidget('entry')
	->configure(-background => $backgroundClr);
$mwFileEntryWd->focus;
$mwFileEntryWd->bind('<KeyPress-Return>', \&mwOpenTrace);
$MainWindow->Balloon()->attach($mwFileEntryWd,
	-msg => "Path of bounce trace file to display.");


#
#--- MainWindow bounce trace file load button widget ---------------------------
#
$mwLoadBtnWd = $mwButnFm->Button( -text => 'Load', -image => $openFileImg,
	-command => sub
	{
		my @pre_suffix = (["Bounce trace files",
				[qw/.bnc .log .txt bounce trace/]], ["All files",'*']);
		$tracefile = $MainWindow->getOpenFile(
						-initialdir=>$dir_path, -filetypes=>\@pre_suffix);
		&mwOpenTrace;
	})->pack(qw/-padx 5 -ipadx 4 -ipady 4/);
$MainWindow->Balloon()->attach($mwLoadBtnWd,
	-msg => "Find and load the bounce trace file to be displayed.");


#
#--- MainWindow Toobox window creation button widget ---------------------------
#
$mwToolboxBtnWd = $mwTBoxFm->Button( -text => 'Tool', -image => $tbImg,
	-command => sub
	{
		if (! Exists($ToolboxWindow)) {
			&mwCreateToolboxWindow;
		} else {
			$ToolboxWindow->deiconify();
			$ToolboxWindow->raise();
		}
	})->pack(qw/-padx 5 -ipadx 4 -ipady 4/);
$MainWindow->Balloon()->attach($mwToolboxBtnWd,
	-msg => "Open parsing tool box window.");


#--- Subroutines ---------------------------------------------------------------

#
#--- Tk->findINC() does not look into my @INC, darn...
#
sub mwFindImg
{
	my $incdir;
	my $imagefile = join('/',@_);
	$imagefile =~ s,::,/,g;
	foreach $incdir (@INC)
	{
		my $found = "$incdir/$imagefile";
		return $found if (-e $found);
	}
	return undef;
}

#
#--- Check whether the trace file is readable text file and display it
#
sub mwOpenTrace
{
	my $lines = 0;
	return 0 if ( !defined($tracefile) || ($tracefile eq ""));

	my $logfile = $tracefile;
	$tbIndentOpenFile = $tracefile;
	$tbFilterOpenFile = $tracefile;
	if (! ($logfile =~ /\.bnc$/) && ( $Tk::platform eq 'unix' ) ) {
		system "$dos2unix $logfile";
	}

	$mwDisplayWd->delete(qw/1.0 end/);

	if ( (-T $tracefile) ) {
		if ( !open(FileHandle,$tracefile) ) {
			$mwDisplayWd->insert("end",
				"ERROR: Could not open trace file\n $tracefile\n", 'ERROR');
			$tracefile = "";
			return -1;
		}

		while (<FileHandle>) {
			$lines++;
			   if ( /=>/ ) {	$mwDisplayWd->insert('end', $_, 'BGN'); }
			elsif ( /<=/ ) {	$mwDisplayWd->insert('end', $_, 'END'); }
			else            { $mwDisplayWd->insert('end', $_); }
		}

		close(FileHandle);

		$mwFileEntryWd->configure(-textvariable=>\$tracefile);

		if ( ! defined($fileList{$tracefile}) ) {
			$fileList{$tracefile} = $tracefile;
			$mwFileEntryWd->insert('end', $tracefile);
		}
		return $lines;
	} else {
		$mwDisplayWd->insert("end",
			"ERROR: Invalid Bounce trace file:\n $tracefile\n", 'ERROR');
		$tracefile = "";
		return -1;
	}
}


#
#--- Create and populate the toolbox window
#
sub mwCreateToolboxWindow
{
	$tbSelect  = "";
	$tbCaption = "Collection of Bounce parsing tools";

	$ToolboxWindow = $MainWindow->Toplevel();
	$ToolboxWindow->title("Bounce Parser");
	$ToolboxWindow->geometry('664x400');

	# Top Frame to carry a left LabFrame and a right frame for Quit button
	$tbTopFm  = $ToolboxWindow->Frame()
		->pack(qw/-fill x -side top -padx 10 -pady 10/);

	# Bottom Frame to carry the NoteBook
	$tbBotFm = $ToolboxWindow->Frame(-borderwidth=>2)
		->pack(qw/-fill both -expand 1 -side bottom/);

	# LabFrame to carry the tool description
	$tbLeftFm = $tbTopFm->LabFrame(
			-font => $guiFont, -borderwidth => 2, -relief => 'groove',
			-label => "Parsing Tools", -labelside => "acrosstop")
		->pack(qw/-fill both -expand 1 -side left/);
	$tbCaptionWd = $tbLeftFm->Label(
			-font => $guiFont, -textvariable => \$tbCaption)
		->pack(qw/-side top/);

	# Place a Quit button in the right top frame
	$tbRightFm = $tbTopFm->Frame()->pack(qw/-side right/);
	$tbRunWd = $tbRightFm->Button(-state => 'disabled',
			-text => 'Run', -image => $tbRunImg, -command=> \&tbRunCmd)
		->pack(qw/-padx 6 -pady 6 -ipadx 4 -ipady 4/);

	$ToolboxWindow->Balloon()->attach($tbRunWd,
		-msg => "Execute the tool box command.");

	$tbBookWd = $tbBotFm->NoteBook(-font => $guiFont )
		->pack(qw/-fill both -expand 1 -ipadx 6 -ipady 6/);

	$tbHomeTabWd = $tbBookWd->add('Sheet 1', -label => 'ToolBox',
									-createcmd => \&tbCreateHomeTab,
									-raisecmd => \&tbRaiseHomeTab);

	$tbFilterTabWd = $tbBookWd->add('Sheet 2', -label => 'Collapse|Filter',
									-createcmd => \&tbCreateFilterTab,
									-raisecmd => \&tbRaiseFilterTab);

	$tbIndentTabWd = $tbBookWd->add('Sheet 3', -label => 'Indent Trace',
									-createcmd => \&tbCreateIndentTab,
									-raisecmd => \&tbRaiseIndentTab);

	$tbResolveTabWd = $tbBookWd->add('Sheet 4', -label => 'Resolve Symbols',
									-createcmd => \&tbCreateResolveTab,
									-raisecmd => \&tbRaiseResolveTab);

	$tbSymBuildTabWd = $tbBookWd->add('Sheet 5', -label => 'Build Symbols',
									-createcmd => \&tbCreateBuildTab,
									-raisecmd => \&tbRaiseBuildTab);

	# MSWin32: Script symfind.pl and Linux executable symfix disabled
	if ( $Tk::platform ne 'unix' ) {
		$tbBookWd->pageconfigure( 'Sheet 4', -state=> 'disabled' );
		$tbBookWd->pageconfigure( 'Sheet 5', -state=> 'disabled' );
	}

	$tbBookWd->pack(
					-expand => "yes", -fill => "both",
					-padx => 5, -pady => 5,
					-side => "top");
}

#
#--- Toolbox notebook create and raise home tab --------------------------------
#
sub tbCreateHomeTab
{
	$tbSelect  = "";
	$tbCaption = "Collection of Bounce parsing tools";
	$tbCaptionWd->configure(-textvariable => \$tbCaption);
}
sub tbRaiseHomeTab
{
	$tbSelect  = "";
	$tbCaption = "Collection of Bounce parsing tools";
	$tbCaptionWd->configure(-textvariable => \$tbCaption);
	$tbRunWd->configure(-state => 'disabled');
}

#
#--- Toolbox notebook create and raise filter tool tab -------------------------
#

sub tbCreateFilterTab
{
	$tbPrunesize = 2;
	$tbPruneValue = 'None';
	$tbFilterTaskValue = '';
	$tbFilterOpenFile = $tracefile;

	$tbCaption= "Collapse|Filter functions in trace file";
	$tbCaptionWd->configure(-textvariable => \$tbCaption);

	my $topFm = $tbFilterTabWd->Frame()
		->pack(qw/-side top -expand 1 -fill x/);
	my $midFm = $tbFilterTabWd->Frame()
		->pack(qw/-side top -expand 1 -fill both/);
	my $botFm = $tbFilterTabWd->Frame()
        ->pack(qw/-side top -expand 1 -fill both/);

	$tbFilterOpenFileWd = $topFm->LabEntry(
			-width => 40, -font => $guiFont,
			-textvariable => \$tbFilterOpenFile,
			-background => $backgroundClr,
			-label => 'Input TraceFile: ',
			-labelPack => [-side => 'left']
		)->pack(qw/-expand 1 -fill x -padx 5 -side left/);
	$tbFilterOpenFileWd->Subwidget('entry')->focus;
	$tbFilterOpenFileWd->bind('<KeyPress-Return>', \&tbFilterCheck);
	$ToolboxWindow->Balloon()->attach($tbFilterOpenFileWd,
		-msg => "Path of input trace file to be filtered.");
	$tbFilterOpenFileBtn = $topFm->Button( -text => 'Open',
		-command => sub
		{
			my @pre_suffix = (["Bounce trace files",
					[qw/.bnc .log .txt bounce trace/]], ["All files",'*']);
			$tbFilterOpenFile = $ToolboxWindow->getOpenFile(
					-initialdir=>$dir_path,-filetypes=>\@pre_suffix);
			&tbFilterCheck;
		})->pack(qw/-padx 4 -ipadx 4 -ipady 4 -side left/);
	$ToolboxWindow->Balloon()->attach($tbFilterOpenFileBtn,
		-msg => "Find and load trace file to be filtered.");

	my $display_var = "Prune depth = 2";
	my $stored_var  = 2;
	$tbPrunesize = $stored_var;
	$tbPruneWd = $midFm->Optionmenu(
			-textvariable => \$display_var,
			-variable => \$stored_var,
			-options =>[["Prune depth = 1", 1],
						["Prune depth = 2", 2],
						["Prune depth = 3", 3],
						["Prune depth = 4", 4],
						["Prune depth = 5", 5],
						["Prune depth = 6", 6],
						["Prune depth = 7", 7],
						["Prune depth = 8", 8] ],
			-command => sub
			{
				my $prunesize = shift @_;
				if ( int($prunesize) eq int(0) ) {
					$ToolboxWindow->Dialog(
						-title => "Invalid prune depth size selected",
						-text  => "Invalid prune depth size selected",
						-bitmap => 'error'
					)->Show;
				} else {
					$tbPrunesize = int($prunesize);
				}
			}
		)->pack(qw/-side left -padx 20 -pady 10 -expand 1 -fill x/);
	$ToolboxWindow->Balloon()->attach($tbPruneWd,
		-msg => "Select prune depth size for filtering.");

	$tbFilterBtn = $midFm->Radiobutton(-text => 'Filter',
			-value => 'Filter', -variable => \$tbPruneValue,
			-command => \&tbFilterCheck)
		->pack(qw/-side left -padx 20 -pady 10/);
	$ToolboxWindow->Balloon()->attach($tbFilterBtn,
		-msg => "Filter entry and exit upto prune depth in input trace file.");

	$tbCollapseBtn = $midFm->Radiobutton(-text => 'Collapse',
			-value => 'Collapse', -variable => \$tbPruneValue,
			-command => \&tbFilterCheck)
		->pack(qw/-side left -padx 20 -pady 10/);
	$ToolboxWindow->Balloon()->attach($tbCollapseBtn,
		-msg => "Collapse exit upto prune depth in input trace file.");

	$tbPruneNoneBtn = $midFm->Radiobutton(-text => 'None',
			-value => 'None', -variable => \$tbPruneValue,
			-command => \&tbFilterCheck)
		->pack(qw/-side left -padx 20 -pady 10/);
	$ToolboxWindow->Balloon()->attach($tbFilterBtn,
		-msg => "Filter entry and exit upto prune depth in input trace file.");

	$tbFilterTaskWd = $botFm->LabEntry(
			-width => 16, -font => $guiFont,
			-textvariable => \$tbFilterTaskValue,
			-label => 'Task Name: ',
			-labelPack => [ -side => 'left' ]
		)->pack(qw/-side left -padx 20 -pady 10 -expand 1 -fill x/);
	$tbFilterTaskWd->bind('<KeyPress-Return>', \&tbFilterCheck);
	$ToolboxWindow->Balloon()->attach($tbFilterTaskWd,
		-msg => "Specify task name for filtering in input trace file.");

	$tbFilterApplyCheckBtn = $botFm->Checkbutton(
			-text => 'Apply', -variable => \$tbFilterApplyCheckValue,
			-command => \&tbFilterCheck)
		->pack(qw/-side left -padx 20 -pady 10/);
	$ToolboxWindow->Balloon()->attach($tbFilterApplyCheckBtn,
		-msg => "Filter the displayed file in the browser window.");

	$tbFilterSaveCheckBtn = $botFm->Checkbutton(
			-text => 'Save', -variable => \$tbFilterSaveCheckValue,
			-command => \&tbFilterCheck)
		->pack(qw/-side left -padx 20 -pady 10/);
	$ToolboxWindow->Balloon()->attach($tbFilterSaveCheckBtn,
		-msg => "Save the filtered file using the same filename.");
}

sub tbRaiseFilterTab
{
	$tbPrunesize = 2;
	$tbPruneValue = 'None';
	$tbFilterTaskValue = '';
	$tbFilterOpenFile = $tracefile;

	$tbSelect  = "Filter";
	$tbCaption= "Collapse|Filter functions in trace file";
	$tbCaptionWd->configure(-textvariable => \$tbCaption);
	$tbRunWd->configure(-state => 'disabled');
}

sub tbFilterCheck
{
	$tbRunWd->configure(-state => 'disabled');

	if (!defined($tbFilterOpenFile)) {
		$tbFilterOpenFile = '';
		return;
	}

	if ( ($tbFilterOpenFile eq '') &&
		 (($tbFilterApplyCheckValue) || ($tbFilterSaveCheckValue)) ) {
		$ToolboxWindow->Dialog(
			-title => "No tracefile specified.",
			-text  => "No tracefile specified",
			-bitmap => 'error'
		)->Show;
		return;
	}

	return if ($tbFilterTaskValue eq '') and ($tbPruneValue eq 'None');

	# Apply to browser window
	if ($tbFilterApplyCheckValue) {
		$tbRunWd->configure(-state => 'normal');
	}

	# Open, indent and save filtered file
	if ($tbFilterSaveCheckValue) {
		if (! -w $tbFilterOpenFile) {
			$ToolboxWindow->Dialog(
				-title => "Tracefile is not writable.",
				-text  => "Tracefile is not writable",
				-bitmap => 'error'
			)->Show;
			$tbRunWd->configure(-state => 'disabled');
		} else {
			$tbRunWd->configure(-state => 'normal');
		}
	}
}

#
#--- Toolbox notebook create and raise indent tool tab -------------------------
#

sub tbCreateIndentTab
{
	$tbIndentTabsize = 2;
	$tbIndentApplyCheckValue = 0;
	$tbIndentSaveCheckValue = 0;
	$tbIndentOpenFile = $tracefile;

	$tbCaption = "Indent nested functions in trace file";
	$tbCaptionWd->configure(-textvariable => \$tbCaption);

	my $topFm = $tbIndentTabWd->Frame()
		->pack(qw/-side top -expand 1 -fill x/);
	my $botFm = $tbIndentTabWd->Frame()
		->pack(qw/-side top -expand 1 -fill x/);

	$tbIndentOpenFileWd = $topFm->LabEntry(
			-width => 40, -font => $guiFont,
			-textvariable => \$tbIndentOpenFile,
			-background => $backgroundClr,
			-label => 'Input TraceFile: ',
			-labelPack => [-side => 'left']
		)->pack(qw/-expand 1 -fill x -padx 5 -side left/);
	$tbIndentOpenFileWd->Subwidget('entry')->focus;
	$tbIndentOpenFileWd->bind('<KeyPress-Return>', \&tbIndentCheck);
	$ToolboxWindow->Balloon()->attach($tbIndentOpenFileWd,
		-msg => "Path of input trace file to be indented.");
	$tbIndentOpenFileBtn = $topFm->Button( -text => 'Open',
		-command => sub
		{
			my @pre_suffix = (["Bounce trace files",
					[qw/.bnc .log .txt bounce trace/]], ["All files",'*']);
			$tbIndentOpenFile = $ToolboxWindow->getOpenFile(
					-initialdir=>$dir_path,-filetypes=>\@pre_suffix);
			&tbIndentCheck;
		})->pack(qw/-padx 4 -ipadx 4 -ipady 4 -side left/);
	$ToolboxWindow->Balloon()->attach($tbIndentOpenFileBtn,
		-msg => "Find and load trace file to be indented.");

	my $display_var = "TabSpace = 2";
	my $stored_var  = 2;
	$tbIndentTabsize = $stored_var;
	$tbIndentTabWd = $botFm->Optionmenu(
			-textvariable => \$display_var,
			-variable => \$stored_var,
			-options =>[["TabSpace = 1",  1], 
						["TabSpace = 2",  2],
						["TabSpace = 4",  4],
						["TabSpace = 6",  6],
						["TabSpace = 8",  8] ],
			-command => sub
			{
				my $tabsize = shift @_;
				if ( int($tabsize) eq int(0) ) {
					$ToolboxWindow->Dialog(
						-title => "Invalid indent tab selected", 
						-text  => "Invalid indent tab selected", 
						-bitmap => 'error'
					)->Show;
				} else {
					$tbIndentTabsize = int($tabsize);
				}
			}
		)->pack(qw/-side left -padx 20 -pady 10 -expand 1 -fill x/);
	$ToolboxWindow->Balloon()->attach($tbIndentTabWd,
		-msg => "Select tab size for indentation.");

	$tbIndentApplyCheckBtn = $botFm->Checkbutton(
			-text => 'Apply', -variable => \$tbIndentApplyCheckValue,
			-command => \&tbIndentCheck)
		->pack(qw/-side left -padx 20 -pady 10/);
	$ToolboxWindow->Balloon()->attach($tbIndentApplyCheckBtn,
		-msg => "Indent the displayed file in the browser window.");

	$tbIndentSaveCheckBtn = $botFm->Checkbutton(
			-text => 'Save', -variable => \$tbIndentSaveCheckValue,
			-command => \&tbIndentCheck)
		->pack(qw/-side left -padx 20 -pady 10/);
	$ToolboxWindow->Balloon()->attach($tbIndentSaveCheckBtn,
		-msg => "Save the indented file using the same filename.");
	
}

sub tbRaiseIndentTab
{
	$tbIndentTabsize = 2;
	$tbIndentApplyCheckValue = 0;
	$tbIndentSaveCheckValue = 0;
	$tbIndentOpenFile = $tracefile;

	$tbSelect  = "Indent";
	$tbCaption = "Indent nested functions in trace file";
	$tbCaptionWd->configure(-textvariable => \$tbCaption);
	$tbRunWd->configure(-state => 'disabled');
}

sub tbIndentCheck {

	$tbRunWd->configure(-state => 'disabled');

	if (!defined($tbIndentOpenFile)) {
		$tbIndentOpenFile = '';
		return;
	}

	if ( ($tbIndentOpenFile eq '') &&
		 (($tbIndentApplyCheckValue) || ($tbIndentSaveCheckValue)) ) {
		$ToolboxWindow->Dialog(
			-title => "No tracefile specified.",
			-text  => "No tracefile specified",
			-bitmap => 'error'
		)->Show;
		return;
	}

	# Apply to browser window
	if ($tbIndentApplyCheckValue) {
		$tbRunWd->configure(-state => 'normal');
	}

	# Open, indent and save indented file
	if ($tbIndentSaveCheckValue) {
		if (! -w $tbIndentOpenFile) {
			$ToolboxWindow->Dialog(
				-title => "Tracefile is not writable.",
				-text  => "Tracefile is not writable",
				-bitmap => 'error'
			)->Show;
			$tbRunWd->configure(-state => 'disabled');
		} else {
			$tbRunWd->configure(-state => 'normal');
		}
	}
}

#
#--- Toolbox notebook create and raise symbol resolve tool tab -----------------
#
sub tbCreateResolveTab
{
	my $filename = '';
	my @trace_suffix = (["Bounce trace files",
			[qw/.log .txt trace/]], ["All files",'*']);
	my @sym_suffix = (["Symbol table file",
			[qw/.sym .map symbols/]], ["All files",'*']);

	$tbResolveTrace = "";
	$tbResolveSymfile = "";

	$tbCaption = "Resolve addresses to symbols in trace file";
	$tbCaptionWd->configure(-textvariable => \$tbCaption);
	my $topFm = $tbResolveTabWd->Frame()
		->pack(qw/-side top -expand 1 -fill x/);
	my $botFm = $tbResolveTabWd->Frame()
		->pack(qw/-side top -expand 1 -fill x/);

	$tbResolveTraceWd = $topFm->LabEntry(
			-width => 40, -font => $guiFont,
			-textvariable => \$tbResolveTrace,
			-background => $backgroundClr,
			-label => 'Input TraceFile: ',
			-labelPack => [-side => 'left']
		)->pack(qw/-expand 1 -fill x -padx 5 -side left/);
	$tbResolveTraceWd->Subwidget('entry')->focus;
	$tbResolveTraceWd->bind('<KeyPress-Return>', \&tbResolveCheckTrace);
	$ToolboxWindow->Balloon()->attach($tbResolveTraceWd,
		-msg => "Path of input file containing unresolved symbols.");
	$tbResolveTraceBtn = $topFm->Button( -text => 'Open',
		-command => sub
		{
			$tbResolveTrace = $ToolboxWindow->getOpenFile(
							-initialdir=>$dir_path,-filetypes=>\@trace_suffix);
			&tbResolveCheckTrace;
		})->pack(qw/-padx 4 -ipadx 4 -ipady 4 -side left/);
	$ToolboxWindow->Balloon()->attach($tbResolveTraceBtn,
		-msg => "Find and load file containing unresolved symbols.");

	$tbResolveSymfileWd = $botFm->LabEntry(
			-width => 40, -font => $guiFont,
			-textvariable => \$tbResolveSymfile,
			-background => $backgroundClr,
			-label => 'Symbols Table: ',
			-labelPack => [-side => 'left']
		)->pack(qw/-expand 1 -fill x -padx 5 -side left/);
	$tbResolveSymfileWd->bind('<KeyPress-Return>', \&tbResolveCheckSymfile);
	$ToolboxWindow->Balloon()->attach($tbResolveSymfileWd,
		-msg => "Path of file containing built symbol table.");
	$tbResolveSymfileBtn = $botFm->Button( -text => 'Open',
		-command => sub
		{
			$tbResolveSymfile = $ToolboxWindow->getOpenFile(
							-initialdir=>$dir_path,-filetypes=>\@sym_suffix);
			&tbResolveCheckSymfile;
		})->pack(qw/-padx 4 -ipadx 4 -ipady 4 -side left/);
	$ToolboxWindow->Balloon()->attach($tbResolveSymfileBtn,
		-msg => "Find and load file containing built symbol table.");	
}

sub tbRaiseResolveTab
{
	$tbResolveTrace = "";
	$tbResolveSymfile = "";

	$tbSelect  = "Resolve";
	$tbCaption = "Resolve addresses to symbols in trace file";
	$tbCaptionWd->configure(-textvariable => \$tbCaption);
	$tbRunWd->configure(-state => 'disabled');
}

sub tbResolveCheckTrace
{
	return 0 if (!defined($tbResolveTrace) || ($tbResolveTrace eq ""));
	if (! -T $tbResolveTrace ) {
		$MainWindow->Dialog (
				-title  => "Invalid Bounce trace filename: $tbResolveTrace",
				-text   => "Invalid Bounce trace filename: $tbResolveTrace",
				-bitmap => 'error')->Show;
		$tbResolveTrace = "";
	}
	if (($tbResolveTrace ne "") && ($tbResolveSymfile ne "")) {
		$tbRunWd->configure(-state => 'normal');
	}
}

sub tbResolveCheckSymfile
{
	return 0 if ( !defined($tbResolveSymfile) || ($tbResolveSymfile eq "") );
	if (! -T $tbResolveSymfile ) {
		$MainWindow->Dialog (
				-title  => "Invalid Symbols file: $tbResolveSymfile",
				-text   => "Invalid Symbol file: $tbResolveSymfile",
				-bitmap => 'error')->Show;
		$tbResolveSymfile = "";
	}
	if (($tbResolveTrace ne "") && ($tbResolveSymfile ne "")) {
		$tbRunWd->configure(-state => 'normal');
	}
}


#
# --- symFind ------------------------------------------------------------------
#

#use vars qw/$tbBuildsymViewDir $tbBuildsymViewDirWd tbBuildsymViewDirBtn
#            $tbBuildsymTarget $tbBuildsymTargetWd
#			 $tbBuildsymModule $tbBuildsymModuleWd $tbBuildsymModuleBtn
#            $tbBuildsymFile $tbBuildsymFileWd/;

sub tbCreateBuildTab
{
	my $filename = '';
	my @pre_suffix = (["Bounce /proc/modules",[qw/.log .txt mod modules/]],
					  ["All files",'*']);

	$tbBuildsymViewDir = "";
	$tbBuildsymTarget = "";
	$tbBuildsymModule = "";
	$tbBuildsymFile = "";

	$tbCaption = "Build symbol table from view and module info.";
	$tbCaptionWd->configure(-textvariable => \$tbCaption);

	my $topFm = $tbSymBuildTabWd->Frame()
		->pack(qw/-side top -expand 1 -fill both/);
	my $botFm = $tbSymBuildTabWd->Frame()
		->pack(qw/-side top -expand 1 -fill both/);
	my $symFm = $tbSymBuildTabWd->Frame()
		->pack(qw/-side top -expand 1 -fill both/);
	my $leftFm = $topFm->Frame()
		->pack(qw/-side left -expand 1 -fill both/);
	my $rightFm = $topFm->Frame()
		->pack(qw/-side right -expand 1 -fill both/);

	my $sel_dir = Cwd::cwd();
	$sel_dir = dirname( $sel_dir );
	$sel_dir = dirname( $sel_dir );

	# Clearcase View path of CommEngine
	$tbBuildsymViewDirWd = $leftFm->Scrolled('DirTree',
			-scrollbars => "osoe", -width => 40, -height => 10,
			-exportselection => 1, -browsecmd => sub { $sel_dir = shift },
			-command => sub
			{
				 $tbBuildsymViewDir = $sel_dir;
				 &tbBuildsymCheck;
			})
		->pack(qw/-padx 5 -pady 5 -side top -expand 1 -fill both/);
	$tbBuildsymViewDirWd->focus();
	$ToolboxWindow->Balloon()->attach($tbBuildsymViewDirWd,
	-msg =>"Double-Click to select toplevel clearcase (CommEngine) directory.");


	# Targets ListBox
	$tbBuildsymTargetWd = $rightFm->Scrolled("Listbox",
			-selectmode => "single", -scrollbars => "oooe")
		->pack(qw/-padx 5 -pady 5 -side top -expand 1 -fill both/);
	$tbBuildsymTargetWd->insert('end',
		qw/	96816R 96368GW 96368GWV/);
	$tbBuildsymTargetWd->bind('<Button-1>',
			sub {
				$tbBuildsymTarget = $tbBuildsymTargetWd->get(
									$tbBuildsymTargetWd->curselection());
				&tbBuildsymCheck;
			});
	$ToolboxWindow->Balloon()->attach($tbBuildsymTargetWd,
		-msg => "Select the build target PROFILE");

	# Select View + Target Button
	#$tbBuildsymViewDirBtn = $rightFm->Button(
	#		-text => "Select View + Target",
	#		-command => \&tbBuildsymCheck)
	#	->pack(qw/-padx 10 -pady 5 -side top/); 

	# Module info File Entry
	$tbBuildsymModuleWd = $botFm->LabEntry(
			-width => 20, -textvariable => \$tbBuildsymModule,
			-label => 'ModInfo: ',
			-labelPack => [-side => 'left']
		)->pack(qw/-padx 5 -pady 5 -side left -expand 1 -fill x/);
	$tbBuildsymModuleWd->bind('<KeyPress-Return>', \&tbBuildsymCheckModule);
	$ToolboxWindow->Balloon()->attach($tbBuildsymModuleWd,
		-msg => "Path of input file containing cat /proc/modules output.");
	$tbBuildsymModuleBtn = $botFm->Button( -text => 'Open',
		-command => sub
		{
			$tbBuildsymModule = $ToolboxWindow->getOpenFile(
				-initialdir=>$dir_path,-filetypes=>\@pre_suffix);
			&tbBuildsymCheckModule;
		})->pack(qw/-padx 5 -pady 5 -side left/);
	$ToolboxWindow->Balloon()->attach($tbBuildsymModuleBtn,
		-msg => "Find and load file containing /proc/modules.");

	$tbBuildsymFileWd = $symFm->LabEntry(
			-width => 20, -textvariable => \$tbBuildsymFile,
			-label => 'Sym-File: ', -labelPack => [-side => 'left']
		)->pack(qw/-padx 5 -pady 4 -side top -fill x/);
	$tbBuildsymFileWd->bind('<KeyPress-Return>', \&tbBuildsymCheck);
	$ToolboxWindow->Balloon()->attach($tbBuildsymFileWd,
		-msg => "Path of output file containing built symbol table.");
}

sub tbRaiseBuildTab
{
	$tbBuildsymViewDir = "";
	$tbBuildsymTarget = "";
	$tbBuildsymModule = "";
	$tbBuildsymFile = "";

	$tbSelect  = "SymBuild";
	$tbCaption = "Build symbol table for view and module info.";
	$tbCaptionWd->configure(-textvariable => \$tbCaption);
	$tbRunWd->configure(-state => 'disabled');
	$tbBuildsymViewDirWd->focus();
}

sub tbBuildsymCheck
{
	if (($tbBuildsymViewDir ne "") && ($tbBuildsymTarget ne "")
		&& ($tbBuildsymModule ne "") && ($tbBuildsymFile ne "")) {
		$tbRunWd->configure(-state => 'normal');
	}
}

sub tbBuildsymCheckModule
{
	return 0 if ( !defined($tbBuildsymModule) || ($tbBuildsymModule eq "") );
	if (! -T $tbBuildsymModule ) {
		$MainWindow->Dialog (
				-title  => "Invalid ModInfo File $tbBuildsymModule",
				-text   => "Invalid ModInfo File $tbBuildsymModule",
				-bitmap => 'error')->Show;
		$tbBuildsymModule = "";
	}
	$tbBuildsymFile = dirname($tbBuildsymModule);
	$tbBuildsymFile = $tbBuildsymFile . "/" . $tbBuildsymTarget . ".sym";
	&tbBuildsymCheck;
}


#
#--- tbRunCmd ------------------------------------------------------------
#

sub tbRunCmd
{
	my ($command, $genFile, $runState, $filename, $suffix);
	$genFile = '';
	$runState = $tbRunWd->configure(-state);

	if ($tbSelect eq "Filter") {
		$genFile = $tbFilterOpenFile;
		$command = $tfilter . " -i " . $tbFilterOpenFile;
		if ($tbFilterTaskValue ne '') {
			$command .= " -t " . $tbFilterTaskValue;
			$genFile .= ".$tbFilterTaskValue";
		}
		if ($tbPruneValue ne 'None') {
			if ($tbPruneValue eq 'Filter') {
				$command .= " -f ";
				$genFile .= ".f";
			} elsif ($tbPruneValue eq 'Collapse') {
				$command .= " -c ";
				$genFile .= ".c";
			}
			$command .= " -p " . $tbPrunesize;
			$genFile .= "$tbPrunesize";
		}
		if ($tbFilterSaveCheckValue) {
			$command .= " -s ";
			$genFile = $tbFilterOpenFile;
		}
	} elsif ($tbSelect eq "Indent") {
		$genFile = $tbIndentOpenFile . "." . $tbIndentTabsize;
		$command = $tindent . " -i " . $tbIndentOpenFile
							. " -t " . $tbIndentTabsize;
		if ($tbIndentSaveCheckValue) {
			$command .= " -s ";
			$genFile = $tbIndentOpenFile;
		}
	} elsif ($tbSelect eq "Resolve") {
		$genFile = $tbResolveTrace . ".bnc";
		if ( $Tk::platform eq 'unix' ) {
	        system "$dos2unix $tbResolveTrace";
			$command = $symfix  . " -s " . $tbResolveSymfile
								. " -i " . $tbResolveTrace
								. " > "  . $genFile;
		} else {
			$command = $symfind . " -t " . $tbResolveSymfile
								. " -i " . $tbResolveTrace
								. " -o " . $genFile;
		}
	} elsif ($tbSelect eq "SymBuild") {
		$genFile = $tbBuildsymFile;
		$command = $symfind . " -c " . $tbBuildsymViewDir
							. " -p " . $tbBuildsymTarget
							. " -m " . $tbBuildsymModule
							. " -g " . $genFile;
	} else {
		print STDERR "Unknown tbox command\n";
		return;
	}

	print "\nRunning command $command\n\n...please wait...\n" if $bdbg;
	system "$command";
	print "Generated: $genFile\n" if $bdbg;
	($filename, $suffix ) = basename( $genFile, "" );
	$ToolboxWindow->Dialog(
			-title => "Done $tbSelect $filename",
			-text  => "Done $tbSelect $filename"
		)->Show;

	if (($tbSelect eq "Filter") && ($tbFilterApplyCheckValue)) {
		$tracefile = $genFile;
		&mwOpenTrace;
	} elsif (($tbSelect eq "Indent") && ($tbIndentApplyCheckValue)) {
		$tracefile = $genFile;
		&mwOpenTrace;
	}
}

MainLoop;
