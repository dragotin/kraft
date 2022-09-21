#!/usr/bin/perl

use strict;
use warnings;

use LWP::Simple;

# extract icons




foreach my $line ( <STDIN> ) {
    chomp( $line );
    # ![arrow-down](https://tabler-icons.io/static/tabler-icons/icons/arrow-down.svg) arrow-down| Down | prefswages.cpp |

    if ( $line =~ /\((.*)\)/ ) {
        my $url = $1;
        my @parts = split('/', $url);
        my $file = $parts[-1];

        if ( $file =~ /.svg$/ ) {
            # print "$file -> $url\n";
            print "<file>custom-icons/$file</file>\n";
        }
    }

    # to download all files, remove comment from next line:
    # getstore($url, $file);

}

