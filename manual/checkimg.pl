#!/usr/bin/perl

use strict;
use warnings;

die "Usage: $0 FILENAMEs\n" if not @ARGV;
foreach my $file (@ARGV) {
    open my $fh, '<:encoding(UTF-8)', $file or die;
    while (my $line = <$fh>) {
        if ($line =~ /image\:(.+)\[/) {
            my $img = $1;

            my @langs = ("en", "de", "nl");

            my $res = sprintf "Image %-40s: ", $img;
            foreach my $l (@langs) {
                my $f = "./images/$l/$img";

                if (-e $f) {
                    $res .= "$l ok\t";
                } else {
                    $res .= "$l FAIL\t";
                }
            }

            print "$res\n";
        }
    }
}
