#! /usr/bin/perl -w
use Fcntl qw();
use Getopt::Std;
use POSIX;

sub usage() {
  print STDERR "Usage: $0 [-b chunksize] [-h] [-n] <oldfile> <newfile> \n";
  print STDERR " -b <chunksize> in bytes\n";
  print STDERR " -h html output\n";
  print STDERR " -n do not list unchanged blocks\n";
  exit(1);
}

our($opt_b, $opt_h, $opt_n);
getopts('nhb:');

my $oldfile = $ARGV[0] || usage();
my $newfile = $ARGV[1] || usage();
my $chunksize = $opt_b || (1024 * 1024);

my $oldread = 0;
my $oldend = 0;
my $newread = 0;
my $newend = 0;
my $numchunks = 0;

# 8MB/s with :raw:perlio !?
# 9MB/s with :raw:unix !?
open (OF, '<:raw:unix', $oldfile) or die "can't open $oldfile: $!\n";
open (NF, '<:raw:unix', $newfile) or die "can't open $newfile: $!\n";
#sysopen(OF, $oldfile, Fcntl::O_RDONLY | Fcntl::O_BINARY) or die "can't open $oldfile: $!\n";
#sysopen(NF, $newfile, Fcntl::O_RDONLY | Fcntl::O_BINARY) or die "can't open $newfile: $!\n";

html_head() if $opt_h;

my $oldbuf;
my $newbuf;

my $numSame = 0;
my $numChange = 0;
my $numGrown = 0;
my $numShrunk = 0;

my $oldtotal = 0;
my $newtotal = 0;
my $smetotal = 0;
my $chgtotal = 0;
my $blktotal = 0;

while (!($oldend && $newend)) {
  my $type = 'change';

  if ($oldend) {
    $type = 'grown';
    $numGrown++;
  } else {
    $oldread = read(OF, $oldbuf, $chunksize);
    #print "old: $oldbuf\n";
    $oldend = ($oldread != $chunksize);
    $oldtotal += $oldread;
  }

  if ($newend) {
    $type = 'shrunk';
    $numShrunk++;
  } else {
    $newread = read(NF, $newbuf, $chunksize);
    #print "new: $newbuf\n";
    $newend = ($newread != $chunksize);
    $newtotal += $newread;
  }

  if ($type eq 'change') {
    my $cmpsize = $oldread < $newread ? $oldread : $newread;
    $blktotal += $cmpsize;
    my $cmpdiff = 0;
    for (my $i = 0; $i < $cmpsize; $i++) {
      $cmpdiff++ unless (substr($oldbuf, $i, 1) eq substr($newbuf, $i, 1));
    }

    if ($cmpdiff == 0) {
      $smetotal += $cmpsize;
      $numSame++;
      ### Gleicher Block
      if ($opt_h) {
        print '<div class="c e"></div>';
      } else {
        printf "%d\t%s\n", $numchunks, "EQ" if $opt_n;
      }
    } else {
      $chgtotal += $cmpdiff;
      $numChange++;
      ### Anderer Block
      if ($opt_h) {
        my $cl = ceil($cmpdiff * 100/ $cmpsize);
        printf '<div class="c c%s"></div>', $cl;
      } else {
        printf "%d\t%d\t%s\n", $numchunks, $cmpdiff, pct($cmpdiff, $cmpsize);
      }
    }
  } elsif ($type eq 'grown') {
    ### Grown
    if ($opt_h) {
      print '<div class="c g"></div>';
    } else {
      printf "%d\t%s\n", $numchunks, $type;
    }
  } elsif ($type eq 'shrunk') {
    ### Shrunk
    if ($opt_h) {
      print '<div class="c s"></div>';
    } else {
      printf "%d\t%s\n", $numchunks, $type;
    }
  } else {
    die;
  }

  $numchunks++;
}

# Summary output
printf "\n<br><pre>\n" if $opt_h;
printf "Old file   : %s (%d b)\n", $oldfile, $oldtotal;
printf "New file   : %s (%d b)\n", $newfile, $newtotal;
printf "Chunk size : %d\n", $chunksize;
printf "Common bytes   : %d\t%s of %d\n", $smetotal, pct($smetotal, $blktotal), $blktotal;
printf "Changed bytes  : %d\t%s of %d\n", $chgtotal, pct($chgtotal, $blktotal), $blktotal;
printf "Equal Chunks   : %d\t%s\n", $numSame, pct($numSame, $numchunks);
printf "Changed Chunks : %d\t%s\n", $numChange, pct($numChange, $numchunks);
printf "Grown Chunks   : %d\t%s\n", $numGrown, pct($numGrown, $numchunks) if $numGrown;
printf "Shrunk Chunks  : %d\t%s\n", $numShrunk, pct($numShrunk, $numchunks) if $numShrunk;
printf "</pre>\n" if $opt_h;
html_end() if $opt_h;
exit 0;

sub pct {
  my ($num, $total) = @_;
  return sprintf "%2.1f%%", ($num * 100/ $total);
}

sub html_head {
  print <<EOF;
<!html>
<html>
  <head>
    <title>Color Blocks</title>
<style>
.c { width: 3px; height: 3px; display: inline-block; margin: 0; padding: 0;}
.g { background-color: #00FF00; } /* grow */
.s { background-color: #0000FF; } /* shrunk */
.e { background-color: #AAAAAA; } /* equal */
EOF

#.c1 { background-color: rgb(155,0,0); }
#.c2 { background-color: rgb(200,0,0); }
#.c3 { background-color: rgb(255,0,0); }
for (1 .. 100) {
  printf ".c%d { background-color: rgb(%d,0,0); }\n", $_, ($_ + 155);
}

  print <<EOF;
</style>
  </head>
  <body>
EOF
}

sub html_end {
  print <<EOF;
  </body>
</html>
EOF
}
