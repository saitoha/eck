#!/usr/bin/perl

#print "\x1B[2J\x1B[1;1H";
hr();
for($i=0; $i<=7; $i++){
    $title = "";
    $seq = "";
    print "\x1B[m";
    if($i & 1){ $seq .= "\x1B[1m"; $title .= "bold "; }
    if($i & 2){ $seq .= "\x1B[4m"; $title .= "uline "; }
    if($i & 4){ $seq .= "\x1B[7m"; $title .= "invert "; }
    #if($i & 8){ $seq .= "\x1B[5m"; $title .= "blink "; }
    if($title eq ""){ $title = "normal "; }
    print $title . $seq . " ck \n";
    c16(30);
    c16(90);
}
print "\x1B[m\n";
c256(38);
c256(48);
print "\x1B[m";
hr();

sub hr{
    print "--------------------------------------------------------------------\n";
}

sub c16{
    my $id = shift;
    my $i;
    print "\x1B[2C";
    for($i=0; $i<=7; $i++){
	print sprintf("\x1B[%d;%dm ck ", $id+$i, $id+10+$i);
    }
    print "\x1B[2C";
    for($i=0; $i<=7; $i++){
	print sprintf("\x1B[%d;%dm ck ", $id+(($i+1) & 7), $id+10+$i);
    }
    print "\n";
}

sub c256{
    my $id = shift;
    my $i;
    for($i=0; $i<216; $i++){
	print sprintf("\x1B[%d;5;%dmck", $id, $i+16);
	if(($i % 36)==35){ print "\n"; }
    }
    for($i=0; $i<24; $i++){
	print sprintf("\x1B[%d;5;%dmck", $id, $i+16+216);
    }
    print "\n";
}
