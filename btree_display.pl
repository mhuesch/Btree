#!/usr/bin/perl -w

$#ARGV==0 || die "usage: btree_display.pl disk\n";

$disk=shift;


system("btree_display $disk 10 dot > _btree_in.dot");

system("dot _btree_in.dot > _btree_out.dot");

system("dotty _btree_out.dot &");
