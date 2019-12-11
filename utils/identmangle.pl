#!/usr/bin/perl
#
use Data::Dumper;


%full=undef; # Forward mapping: Full identifier names to mangled names
%newn=undef; # reverse mapping

# predefined
$full{'operator'}="oprator";
$newn{'oprator'}="operator";

# domangle3 - mangle names by: splitting names at `-'.  The mangled name is constructed
#    by using 2 characters from the first 4 parts. Example:  the_quick_brown_fox -> thqubrfo
sub domangle3 {
  ($name)=@_;
  $rv="";
  (@parts)=split(/_+/,$name);
  if($#parts==0) {
    $rv=$name;
    $rv=~s/^(.{9}).*$/$1/;
  } else {
    splice @parts,4;
    foreach $part (@parts) {
      $part=~s/^(.{2}).*$/$1/;
      $rv=$rv.$part;
    }
  }
  return $rv;
}

# domangle2 - mangle names by: 1) remove all vowels 2) split names at `_' 3) using 3 characters from
#    the first 3 parts part: Example:  the_quick_brown_fox -> thqckbrw
sub domangle2 {
  ($name)=@_;
  $rv="";
  $name=~s/[aeiouAEIOU]+//g;
  (@parts)=split(/_+/,$name);
  splice @parts,3;
  foreach $part (@parts) {
    $part=~s/^(.{3}).*$/$1/;
    $rv=$rv.$part;
    
  }
  return $rv;
}

# domangle1 - mangle names by: split names at `_' and use 3 characters from the first 3 parts.
#    Example: the_quick_brown_fox -> thequibro
sub domangle1 {
  ($name)=@_;
  $rv="";
  (@parts)=split(/_+/,$name);
  if($#parts==0) {
    $rv=$name;
    $rv=~s/^(.{9}).*$/$1/;
  } else {
    splice @parts,3;
    foreach $part (@parts) {
      $part=~s/^(.{3}).*$/$1/;
      $rv=$rv.$part;
    }
  }
  return $rv;
}

# mangle - main routine for name mangling
sub mangle {
  ($orig)=@_;
  
  # Check if the name exists already and use it
  if (exists ($full{$orig})) {
    $m=$full{$orig};
  } else {
    # if the original name was shorter than 9, we don't need to mangle the name at all.  make sure the 
    # forward and reverse mappings of the original name exist
    if(length($orig)<=9) {
	$newn{$orig}=$orig;
	$full{$orig}=$orig;
        return $orig;
    };
    #if(length($orig)<=9) { $newn{$m}=$orig; return $m; };

    # name doesn't exist - try domangle1
    $m=domangle1($orig);
    # check if the mangled name already exists
    if(exists($newn{$m})) {
      # we have a conflict where two identifiers domangle1 to the same value
      print "$orig->$m EXISTS ";
      $m=domangle2($orig);
      # check if there's a domangle2 conflict as well and use domangle3
      if(exists($newn{$m})) { $m=domangle3($orig); }
      print "->$m\n";
    #while(exists($newn{$m})) {
    #  $m=domangleinc($m);
    }

    # set reverse mapping
    $newn{$m}=$orig;
    # set forward mapping
    $full{$orig}=$m;
  }

  # return the mangled name
  return $m; 
}


# getNames - go through the file and run all the identifiers through the name mangler
sub getNames {
  ($file)=@_;

  if(! open(FH,$file)) {
    print STDERR "can't open $file\n";
    return;
  }
  if(! open(OH,">mod/$file")) {
    print STDERR "can't open mod/$file\n";
    return;
  }

  $lc=1;
  $comment=0; # tracks if we are inside a comment
  while($line=<FH>) {

    # get rid of all CR and LF
    chomp($line);$line=~s/\r//g;
    $line2=$line;

    # deal with comments - does not handle nested comments
    $ec=0;
    if ($line=~/\*\//) {
      $ec=1; # found end of comment marker
    }
    $bc=0;
    if ($line=~/\/\*/) {
      $bc=1;  # found beginning of comment marker
      $comment=1;
    }
    if ($comment==1) {
      # inside of a comment
      if($bc && $ec) {
        # comment is fully contained on one line, remove it
        $line=~s/\/\*.*\*\///;
        $comment=0;
      } elsif($bc) {
        # comment begins on this line and ends somewhere else
        $line=~s/\/\*.*$//;
	$bc=0;
      } elsif ($ec) {
        # comment ends on this line
        $line=~s/^.*\*\///;
	$comment=0;
      } else {
        # remove junk from the inside of a multi-line comment
        $line=~s/^.*$//;
      }
      
    }

    # throw out string literals
    $line=~s/\"[^\"]*\"//g;
    # throw out character literals
    $line=~s/\'[^\']*\'//g;

    # split the line into tokens seperated by various white space, C operators and control structures
    (@tokens)= split(/[\t\ \,\(\)\{\}\[\]\<\>\=\+\*\/&!%\^\.\\;|-:?~]+/,$line);
    #print "$lc: ";
    foreach $tok (@tokens) {
      # mangle the name
      $rep=mangle($tok);
      #if (length($tok)>9) {
      if ($tok -ne $rep) {
        # do the replacement only if the name was actually mangled
        #print "($tok)->($rep)\n";
    	$line2=~s/$tok/$rep/g;
      }
    }
    print OH "$line2\n";
    $lc++;
  }
  close(FH);
  close(OH);
}
#PASS 1: get unique mangled names
foreach $file (@ARGV) {
  # print "Working on $file\n";
  getNames($file);
}

print STDERR Dumper(\%full);
print STDERR Dumper(\%newn);
