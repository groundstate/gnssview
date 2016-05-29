#!/usr/bin/perl -w

# Simulates
# NB libio-socket-multicast-perl needed in Ubuntu

use IO::Socket::Multicast;

sub ReadGPSCCTF;
sub UpdateSatellites;
sub BroadcastData;
sub Interpolate;

use constant DESTINATION => '226.1.1.37:14544'; 
my $sock = IO::Socket::Multicast->new(Proto=>'udp',PeerAddr=>DESTINATION);
$sock->mcast_ttl(10); # time to live

#$teststart=1;
#$testinc=5;

#$teststart=3599;
#$testinc=-5;

##$teststart=3500;
##$testinc=5;

$teststart=100;
$testinc=20;

$test=$teststart;
$elev=150;

while (1)
{
	sleep(1);
	UpdateSatellites();
	BroadcastData();
}

sub UpdateSatellites()
{
	
	splice @birds,0; # empty it
	
	@gmt = gmtime(time);
	$tod = $gmt[0] + $gmt[1]*60+$gmt[2]*3600;
	
	push @birds,[time(),3,10,int($test/10.0),int($elev/10.0),127+rand()*128];
	$test+=$testinc;
	if ($test >= 3600){$test -= 3600;}
	if ($test <0){$test += 3600;}
	$elev += 1;
}

sub BroadcastData()
{
	my $i;
	my $data="";
	for ($i=0;$i<=$#birds;$i++){
		$data .= sprintf("$birds[$i][0],$birds[$i][1],$birds[$i][2],%d,%d,%d\n",$birds[$i][3],$birds[$i][4],$birds[$i][5]);
	}
	$sock->send($data) || print "Couldn't send\n";
	print $data;
}


