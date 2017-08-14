#!/usr/bin/perl
my @files = @ARGV;

use List::MoreUtils qw(uniq);

my @symlist = ( );

for ($filei=0;$filei < @files;$filei++) {
    $file = $files[$filei];

    $state = 0;

    open(X,"<",$file) || die;
    while (my $line = <X>) {
        chomp $line;

        if ($line =~ m/^enum {/) {
            $state = 1;
        }
        elsif ($line =~ m/^};/) {
            $state = 0;
        }
        elsif ($state == 1 && $line =~ m/^\tOPCODE_/) {
            next if $line =~ m/^\tOPCODE__MAX/;
            next if $line =~ m/^\tOPCODE_NONE/;
            $line =~ s/^\tOPCODE_//;
            $line =~ s/,.*$//;
            push(@symlist,$line) if $line ne "";
        }
    }
    close(X);
}

@symlist = sort(uniq(@symlist));

$count = 0;
print "/* auto generated */\n";
print "enum {\n";
print "\tOPCODE_NONE, /* =$count */\n"; $count++;
for ($op=0;$op < @symlist;$op++) {
    $opname = $symlist[$op];
    print "\tOPCODE_$opname, /* =$count */\n"; $count++;
}
print "\tOPCODE__MAX  /* =$count */\n";
print "};\n";
print "\n";

my @offsetlist = ( );
$count = 0;
$offset = 0;
print "#ifdef WANT_OPCODE_NAMES\n";
print "/* auto generated */\n";
print "const char opcode_names[] = \\\n";
print "\t\"\\0\" /* =$count @".$offset." */ \\\n"; push(@offsetlist,$offset); $count++; $offset += 1;
for ($op=0;$op < @symlist;$op++) {
    $opname = $symlist[$op];
    print "\t\"$opname\\0\" /* =$count @".$offset." */ \\\n"; push(@offsetlist,$offset); $count++; $offset += 1+length($opname);
}
print "\t\"\\0\"; /* =$count @".$offset." */\n"; push(@offsetlist,$offset); $count++; $offset += 1;
print "\n";

$count = 0;
if ($offset >= 0x10000) {
    $nameof_t = "uint32_t";
}
elsif ($offset >= 0x100) {
    $nameof_t = "uint16_t";
}
else {
    $nameof_t = "uint8_t";
}
print "/* auto generated */\n";
print "const $nameof_t opcode_name_offset[OPCODE__MAX+1] = {\n";
print "\t".$offsetlist[$count].", /* =$count */\n"; $count++;
for ($op=0;$op < @symlist;$op++) {
    $opname = $symlist[$op];
    print "\t".$offsetlist[$count].", /* =$count */\n"; $count++;
}
print "\t".$offsetlist[$count]."  /* =$count */\n";
print "};\n";
print "#endif /*WANT_OPCODE_NAMES*/\n";
print "\n";

