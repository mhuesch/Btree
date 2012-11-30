#!/usr/bin/perl -w

$#ARGV==6 or die "usage: test.pl \"reference implementation command line\" \"your implementation command line\" keysize valsize seed num maxerrs\n";

($refcmd,$testcmd,$keysize,$valsize,$seed,$num,$maxerrs)=@ARGV;

$t=time();
$pid=$$;

system "gen_test_sequence.pl $keysize $valsize $seed $num > TEST.$t.$pid.input";

system "$refcmd < TEST.$t.$pid.input > TEST.$t.$pid.refout";

system "$testcmd < TEST.$t.$pid.input > TEST.$t.$pid.yourout";

system "compare.pl TEST.$t.$pid.input TEST.$t.$pid.refout TEST.$t.$pid.yourout $maxerrs";


