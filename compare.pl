#!/usr/bin/perl -w

$#ARGV==3 || die "usage: compare.pl cmdin refout yourout maxerrs\n";

open(CMD,$ARGV[0]);
open(REF,$ARGV[1]);
open(TEST,$ARGV[2]);
$maxerrs=$ARGV[3];

$numerr=0;
$i=0;

while ($numerr<$maxerrs && !eof(CMD) && !eof(REF) && !eof(TEST)) { 
  $cmd=<CMD>; chomp($cmd);
  $ref=<REF>; chomp($ref);
  $test=<TEST>; chomp($test);
  
  if ($cmd =~ /DISPLAY/) { 
    # DISPLAY is a special case since it
    # spans multiple output lines, each of which needs to be checked.
    # it must be the case that both implementations found this was OK.

    undef %refcontent if defined(%refcontent);
    while (1) {
      $disp=<REF>; chomp($disp);
      last if $disp=~/END DISPLAY/;
      $disp=~/\((\S+)\s*,\s*(\S+)\)/;
      $refcontent{$1}=$2;
    }
      
    undef %testcontent if defined(%testcontent);
    while (1) {
      $disp=<TEST>; chomp($disp);
      last if $disp=~/END DISPLAY/;
      $disp=~/\((\S+)\s*,\s*(\S+)\)/;
      $testcontent{$1}=$2;
    }
    
    @refkeys = sort keys %refcontent;
    @testkeys = sort keys %testcontent;

    $sawerror=0;

    if ($#refkeys!=$#testkeys) { 
      print "----------------------------------------------------------------------------\n";
      print "ERROR $numerr found on operation $i\n\n";
      print "Operation is \"$cmd\"\n\n";
      print "Reference implementation has ".($#refkeys+1)." keys\n";
      print "Test implementation has ".($#testkeys+1)." keys\n";
      print "----------------------------------------------------------------------------\n";
      $sawerror=1;
    } else {
      foreach $refkey (@refkeys) { 
	if (!defined $testcontent{$refkey}) { 
	  print "----------------------------------------------------------------------------\n";
	  print "ERROR $numerr found on operation $i\n\n";
	  print "Operation is \"$cmd\"\n\n";
	  print "Reference implementation has key \"$refkey\"\n";
	  print "Test implementation does NOT have this key\n";
	  print "----------------------------------------------------------------------------\n";
	  $sawerror=1;
	} else {
	  if ($testcontent{$refkey} ne $refcontent{$refkey}) {
	    print "----------------------------------------------------------------------------\n";
	    print "ERROR $numerr found on operation $i\n\n";
	    print "Operation is \"$cmd\"\n\n";
	    print "Reference implementation has ($refkey,$refcontent{$refkey})\n";
	    print "Test implementation has      ($refkey,$testcontent{$refkey})\n";
	    print "----------------------------------------------------------------------------\n";
	    $sawerror=1;
	  }
	}
      }
    }
    $numerr++ if $sawerror;
  } else {
    if ($ref ne $test) { 
      print "----------------------------------------------------------------------------\n";
      print "ERROR $numerr found on operation $i\n\n";
      print "Operation is \"$cmd\"\n\n";
      print "Reference implementation says: \"$ref\"\n";
      print "Test implementation says:      \"$test\"\n";
      print "----------------------------------------------------------------------------\n";
      $numerr++;
    }
  }
  $i++;
}


print "Summary:  $numerr errors found on $i operations with error limit set to $maxerrs\n";
if ($numerr==0) {
  print "\n\nCONGRATULATIONS - NO ERRORS FOUND!\n\n";
} else {
  print "\n\nERRORS FOUND\n\n";
}

