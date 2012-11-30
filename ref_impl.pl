#!/usr/bin/perl -w

$#ARGV==1 || die "usage: ref_impl.pl debug|nodebug bugprobability < test_sequence\n";

srand time();


$debug=substr($ARGV[0],0,1) eq 'd';
$bugprob=$ARGV[1];

$line=<STDIN>;
($op, $keysize, $valuesize) = split(/\s+/, $line);

if (!($op eq "INIT")) { 
  die "First operation is not an init!";
} else {
  print STDERR "Initialized with keysize=$keysize and valuesize=$valuesize\n" if $debug;
  print "OK\n";
}

%content=();

while ($line=<STDIN>) { 
  $line=~/^(\S+)\s+(.*)$/;
  $op=$1; $rest=$2; 
  if ($op eq "INSERT") {
    ($key, $value) = split(/\s+/,$rest);
    if (defined $content{$key} || Bug()) { 
      print STDERR "Inserting ($key, $value) failed because $key already exists\n" if $debug;
      print "FAIL\n";
    } else {
      $content{$key}=$value;
      print STDERR "Inserted ($key, $value)\n" if $debug;
      print "OK\n";
    }
  } elsif ($op eq "UPDATE") { 
    ($key, $value) = split(/\s+/,$rest);
    if (!(defined $content{$key}) || Bug()) { 
      print STDERR "Updating ($key, $value) failed because $key does not exist\n" if $debug;
      print "FAIL\n";
    } else {
      $content{$key}=$value;
      print STDERR "Updated ($key, $value)\n" if $debug;
      print "OK\n";
    }
  } elsif ($op eq "DELETE") { 
    ($key)=split(/\s+/,$rest);
    if (!(defined $content{$key}) || Bug() ) { 
      print STDERR "Deleting ($key) failed because $key does not exist\n" if $debug;
      print "FAIL\n";
    } else {
      delete $content{$key};
      print STDERR "Deleted ($key)\n" if $debug;
      print "OK\n";
    }
  } elsif ($op eq "LOOKUP") { 
    ($key)=split(/\s+/,$rest);
    if (!(defined $content{$key}) || Bug() ) { 
      print STDERR "Looking up ($key) failed because $key does not exist\n" if $debug;
      print "FAIL\n";
    } else {
      $value= $content{$key};
      print STDERR "Lookup ($key) found $value\n" if $debug;
      print "OK $value\n";
    }
  } elsif ($op eq "DISPLAY") { 
    print STDERR "Displaying content in sorted order\n" if $debug;
    print "OK BEGIN DISPLAY\n";
    foreach $key (sort keys %content) {
      print "($key, $content{$key})\n";
    }
    print "OK END DISPLAY\n";
  } elsif ($op eq "DEINIT") {
    print STDERR "Got a deinit.  Finishing up now\n" if $debug;
    print "OK\n";
    exit;
  } else {
    print STDERR "Unknown request $op\n failed\n" if $debug;
    print "FAIL\n";
  }
}


sub Bug { 
  return (rand(1) < $bugprob);
}
