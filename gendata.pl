#!/usr/bin/perl -w

$#ARGV==0 or die "usage: gendata.pl numbytes|_ > data\n";

$raw="0123456789abcdefghijklmnopqrstuvwxyz";

$i=0;

while ($ARGV[0] eq "_" || $i<$ARGV[0]) {
  print substr($raw, $i % 36,1);
  $i++;
}

