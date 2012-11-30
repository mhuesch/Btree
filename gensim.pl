#!/usr/bin/perl -w

$#ARGV==2 or die "usage: gensim.pl seed maxkey numrecords > simrequests\n";

$seed=shift;
$maxkey=shift;
$num=shift;

srand($seed);

for ($i=0;$i<$num;$i++) {
  print join(" ",GenRandomRequest($maxkey)), "\n";
}

sub GenRandomRequest() {
  my $maxkey=shift;
  my ($type,$key,$value);
  my $r=int(rand(4));

  if ($r==0) {
    $type="INSERT";
    $key=int(rand($maxkey));
    $value=int(rand($maxkey));
    return ($type, $key,$value);
  }

  if ($r==1) {
    $type="DELETE";
    $key=int(rand($maxkey));
    $value=0;
    return ($type,$key);
  }

  if ($r==2) {
    $type="UPDATE";
    $key=int(rand($maxkey));
    $value=int(rand($maxkey));
    return ($type,$key,$value);
  }

  if ($r==3) { 
    $type="LOOKUP";
    $key=int(rand($maxkey));
    return ($type, $key);
  }
}


	  
    
