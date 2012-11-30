#!/usr/bin/perl -w

# this disk config is for simple correctness testing
# not for performance testing.
$diskstem="__test";
$numblocks=1024;
$blocksize=1024;
$heads=1;
$blockspertrack=1024;
$tracks=1;
$avgseek=10;
$trackseek=1;
$rotlat=10;
$cachesize=64;


$maxerr=10;

$#ARGV==3 or die "usage: test_me.pl keysize valuesize seed numops\n";

($keysize,$valuesize,$seed,$numops)=@ARGV;

$ENV{PATH}.=":.";

system "deletedisk $diskstem";
system "makedisk $diskstem $numblocks $blocksize $heads $blockspertrack $tracks $avgseek $trackseek $rotlat";


$cmd="test.pl \"ref_impl.pl nodebug 0\" \"sim $diskstem $cachesize\" $keysize $valuesize $seed $numops $maxerr";

system $cmd;

