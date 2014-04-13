#!/usr/bin/perl -w

# Simulates GNSS satellite systems using CCTF files as input
# NB libio-socket-multicast-perl needed in Ubuntu
use Time::HiRes qw(usleep);
use IO::Socket::Multicast;

sub ReadGPSCCTF;
sub UpdateSatellites;
sub BroadcastData;
sub Interpolate;

use constant DESTINATION => '226.1.1.37:14544'; 
my $sock = IO::Socket::Multicast->new(Proto=>'udp',PeerAddr=>DESTINATION);
$sock->mcast_ttl(10); # time to live

$opt_f=1;
$tstart=4200;
$tod=$tstart;
$tau=3;
if ($opt_f){$tau=24;}

ReadGPSCCTF("./test.cctf");

while (1)
{
	usleep(1000000/$tau);
	UpdateSatellites();
	BroadcastData();
}

sub ReadGPSCCTF()
{
	open(IN,"< $_[0]");
	while ($line=<IN>)
	{
		last if ($line=~/^\s+hhmmss/);
	}
	while ($line=<IN>)
	{
		@data=split " ",$line;
		$data[3]=~/(\d{2})(\d{2})(\d{2})/;
		$hh=$1;
		$mm=$2;
		$ss=$3;
		#print "$data[0] $hh $mm $ss $data[5] $data[6]\n";
		# Accumulate by PRN - this will make tracking simpler
		push @{ $gps{$data[0]} },[$data[0],$hh*3600+$mm*60+$ss,$data[5]/10.0,$data[6]/10.0,999];
		
	}
	
	# print $#{$gps{1}},"\n";
	close IN;
	
}


sub UpdateSatellites()
{
	
	splice @birds,0; # empty it
	
	if ($opt_f){
		$tod++;
	}
	else{
		@gmt = gmtime(time);
		$tod = $tstart+$gmt[0] + $gmt[1]*60+$gmt[2]*3600;
	}	
	foreach $sat ( keys %gps ){
		for ($t=0;$t<=$#{$gps{$sat}};$t++)
		{
			last if ($gps{$sat}[$t][1] > $tod);
		}
		if ($t>0 && $t<=$#{$gps{$sat}})
		{
			if ($gps{$sat}[$t][1] - $gps{$sat}[$t-1][1]==960){ 
				# print "$gps{$sat}[$t-1][1] $gps{$sat}[$t][1] $gps{$sat}[$t][0]\n";
				$az   = Interpolate($tod,$gps{$sat}[$t-1][1]+390,$gps{$sat}[$t-1][3],$gps{$sat}[$t][1]+390,$gps{$sat}[$t][3])/10.0;
				$elev = Interpolate($tod,$gps{$sat}[$t-1][1]+390,$gps{$sat}[$t-1][2],$gps{$sat}[$t][1]+390,$gps{$sat}[$t][2])/10.0;
				#print "$gps{$sat}[$t][0] $az $elev\n";
				if ($gps{$sat}[$t][0] < 10){
					push @birds,[time(),1,$gps{$sat}[$t][0],$az,$elev,127+rand()*128];# 
				}
				elsif ($gps{$sat}[$t][0] < 20){
					push @birds,[time(),1,$gps{$sat}[$t][0],$az,$elev,127+rand()*128];# 1==GPS
				}
				else{
					push @birds,[time(),1,$gps{$sat}[$t][0],$az,$elev,127+rand()*128];# 3==GLONASS
				}
				
			}
		}
		elsif ($t==0 && ($tod >= $gps{$sat}[$t][1])) # no prior track to extrapolate
		{
			$az = $gps{$sat}[$t][3]/10.0;
			$elev=$gps{$sat}[$t][2]/10.0;
			if ($gps{$sat}[$t][0] < 10){
				push @birds,[time(),0,$gps{$sat}[$t][0],$az,$elev,127+rand()*128]; # 0==Beidou
			}
			elsif ($gps{$sat}[$t][0] < 20){
				push @birds,[time(),1,$gps{$sat}[$t][0],$az,$elev,127+rand()*128]; # 1==GPS
			}
			else{
				push @birds,[time(),3,$gps{$sat}[$t][0],$az,$elev,127+rand()*128];# 3==GLONASS
			}
		}
		
	}
}

sub BroadcastData()
{
	my $i;
	my $data="";
	for ($i=0;$i<=$#birds;$i++)
	{
		$data .= sprintf("$birds[$i][0],$birds[$i][1],$birds[$i][2],%d,%d,%d\n",$birds[$i][3]*10,$birds[$i][4]*10,$birds[$i][5]);
	}
	$sock->send($data) || print "Couldn't send\n";
	#print $tod,":",$data;
}

sub Interpolate()
{
	my ($x,$x0,$y0,$x1,$y1)=@_;
  my ($m,$c,$y);
  #print "$x,$x0,$y0,$x1,$y1\n";
  if ($x0 != $x1){ # shouldn't happen !
		$m = ($y1-$y0)/($x1-$x0);
		$c = $y0 - $m*$x0;
		$y = $m*$x+$c;
  }
  else
  {
		$y=$y0;
  }
  return $y;
}
