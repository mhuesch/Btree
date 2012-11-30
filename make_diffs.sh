 perl -e 'foreach $i (`ls *.cc *.h *.p?`) { print $i; chomp $i; system("diff $i pdinda/$i"); } ' > diffs.txt

