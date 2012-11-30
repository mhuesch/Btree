#!/usr/bin/perl -w

$#ARGV==3 or die "usage: gen_test_sequence.pl keysize valsize seed num\n";

($keysize,$valuesize,$seed,$num)=@ARGV;

srand $seed;

$keybytes="abcdefghijklmnopqrstuvwxyz0123456789";
$valuebytes="abcdefghijklmnopqrstuvwxyz0123456789";

%ops = ( INSERT_NEW => \&gen_insert_new,
	 INSERT_EXISTS => \&gen_insert_exists,
	 UPDATE_NEW => \&gen_update_new,
	 UPDATE_EXISTS => \&gen_update_exists,
#
# No deletes required for F12
# 
#	 DELETE_NEW => \&gen_delete_new,
#	 DELETE_EXISTS => \&gen_delete_new,
	 LOOKUP_NEW => \&gen_lookup_new,
	 LOOKUP_EXISTS => \&gen_lookup_exists,
	 DISPLAY => \&gen_display
       );

@opnames=keys %ops;


%content= ();

print "INIT $keysize $valuesize\n";

for ($i=1;$i<$num;$i++) { 
  # never try to do an existing key if no keys currently exist
  my $numkeys=keys %content;
  do {
    $op=$opnames[int(rand($#opnames + 1))];
  } while ( $op =~ /EXISTS/ && $numkeys<1 );
  print &{$ops{$op}}(), "\n";
}

print "DEINIT\n";


sub MakeKey {
  return join("", map { substr($keybytes,int(rand(length($keybytes))),1) } (1..$keysize));
}

sub MakeNonExistentKey {
  my $key;
  do {
    $key=MakeKey();
  } while (defined $content{$key});
  return $key;
}

sub MakeExistentKey {
  my @keys=keys %content;
  return $keys[int(rand($#keys+1))];
}

sub MakeValue {
  return join("", map { substr($valuebytes,int(rand(length($valuebytes))),1) } (1..$valuesize));
}


sub gen_insert_new {
  my ($key, $value) = (MakeNonExistentKey(), MakeValue());
  $content{$key}=$value;
  return "INSERT $key $value  # should succeed";
}

sub gen_insert_exists {
  return "INSERT ".MakeExistentKey()." ".MakeValue()."  # should fail";
}

sub gen_update_new {
  return "UPDATE ".MakeNonExistentKey()." ".MakeValue()."  # should fail";
}

sub gen_update_exists {
  my ($key, $value) = (MakeExistentKey(), MakeValue());
  $content{$key}=$value;
  return "UPDATE $key $value  # should succeed";
}

sub gen_delete_new {
  return "DELETE ".MakeNonExistentKey()."  # should fail" ;
}

sub gen_delete_exists {
  my $key=MakeExistentKey();
  delete $content{$key};
  return "DELETE $key  # should succeed";
}

sub gen_lookup_new {
  return "LOOKUP ".MakeNonExistentKey()."  # should fail";
}

sub gen_lookup_exists {
  my $key=MakeExistentKey();
  return "LOOKUP $key  # should succeed and return $content{$key}";
}

sub gen_display {
  return "DISPLAY  # should always succeed";
}
