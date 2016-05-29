#!/usr/bin/perl -w

#
# The MIT License (MIT)
#
# Copyright (c) 2016 Michael J. Wouters
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


# Script for testing parsing of SBF scripts

$fin = $ARGV[0];
open(my $FILE, $fin) or die $!;
binmode($FILE);

use constant BLK_SIZE => 512;

$input='';

%GNSS=();

$BEIDOU=0;
$GPS=1;
$GALILEO=2;
$GLONASS=3;
$QZSS=4;
$SBAS=5;

while (read($FILE, my $buf, BLK_SIZE)) {
   
	# add what we got to the current $input, so that we can parse for funs stuff
	$input .= $buf;
	
	# Header structure for SBF packets is 
	# Sync (2b) | CRC (2b) | ID (2b) | length (2b) 
	
	if ($input=~ /\x24\x40(.{6})([\s\S]*)/) { # if we've got a SBF header
		$input=$&; # chuck away the prematch part
		($crc,$msgID,$packetLength) = unpack("SSS",$1);
		$data = $2;
		
		# Have we got the full message ?
		
		$inputLength=length($input);
		if ($packetLength <= $inputLength){ # it's all there ! yay !
			# SatVisibility - 4012
			if (($msgID & 8191) == 4027){
				ParseMeasEpoch($data,$packetLength-8);
				$gotCN=1;
			}
			elsif (($msgID & 8191) == 4012){
				ParseSatVisibility($data,$packetLength-8);
				$gotVis=1;
			}
			$gotVis=1;
			if ($gotCN && $gotVis){
				SendData();
				$gotCN=$gotVis=0;
			}
			
			# Tidy up the input buffer
			if ($packetLength == $inputLength){
				$input="";
			}
			else{
				$input=substr $input,$packetLength-8;
			}
		}
		else{
			# nuffink
		}
	}
}

#----------------------------------------------------------------------------
sub ParseMeasEpoch
{
	my $d=$_[0];
	my @preamble = unpack("ISC4cS",$d);
	my $N1=$preamble[2];
	my $SB1Length=$preamble[3];
	my $SB2Length=$preamble[4];
	printf("%i %i %i %i %i %i\n",$preamble[0],$preamble[1],$N1,$SB1Length,$SB2Length,$N1*$SB1Length+12);
	my $n2cnt=0;
	for ($n=0;$n<$N1;$n++){
		@b1 = unpack("C4IiScCSCC",(substr $d, 12+$n*$SB1Length+$n2cnt*$SB2Length,$SB1Length));
		$nb2=$b1[11];
		
		$sig=$b1[1] & 31;
		$svid=$b1[2];
		$cn=$b1[8];
		$offset = 12+($n+1)*$SB1Length+$n2cnt*$SB2Length;
		
		($constellation,$prn,$reqsig) = SVIDtoGNSSParams($svid);
		
		if ($constellation == $SBAS){
			$reqsig = $sig; # KLUDGE
		}
		
		if ($constellation==-1){
			$n2cnt += $nb2;
			next;
		}
		
		if ($sig != $reqsig){ # search block 2
			for ($m=0;$m<$nb2;$m++){
				@b2 = unpack("C3",(substr $d, $offset+$m*$SB2Length,$SB2Length));
				$sig2=$b2[0] & 31;
				if ($sig2==$reqsig){
					if (defined $GNSS{$svid}){ $GNSS{$svid}[4]=$b2[2];}
					else{$GNSS{$svid}=[($constellation,$prn,-999,-999,$b2[2])];} # new entry
					last;
				}
			}
		}
		else{
			if (defined $GNSS{$svid}){$GNSS{$svid}[4]=$cn;}
			else{$GNSS{$svid}=[($constellation,$prn,-999,-999,$cn)];}
		}
		
		$n2cnt += $nb2;
	}
}

sub SVIDtoGNSSParams{

	my $svid=$_[0];
	my $reqsig=-1,$constellation=-1,$prn=-1;
	
	if ($svid <= 37){ # GPS
		$reqsig=0;
		$constellation=$GPS;
		$prn=$svid;
	}
	elsif ($svid >= 181 && $svid <=187){ # QZSS
		$reqsig=6;
		$constellation=$QZSS;
		$prn=$svid-180;
	}
	elsif ($svid >=38 && $svid <=62){ # GLONASS
		$reqsig=8;
		$constellation=$GLONASS;
		$prn=$svid-37;
	}
	elsif ($svid >=71 && $svid <=102){ # Galileo
		$reqsig=17;
		$constellation=$GALILEO;
		$prn=$svid-70;
	}
	elsif ($svid >=120 && $svid <=140){ # SBAS
		$reqsig=24;
		$constellation=$SBAS;
		$prn=$svid-100;
	}
	elsif ($svid >= 141 && $svid <= 172){ # Beidou
		$reqsig=28;
		$constellation=$BEIDOU;
		$prn=$svid-140;
	}

	return ($constellation,$prn,$reqsig);
	
}
	
#----------------------------------------------------------------------------
sub ParseSatVisibility
{
	my $d=$_[0];
	my @preamble = unpack("ISCC",$d);
	my $N=$preamble[2];
	my $BLength=$preamble[3];
	printf("%i %i %i %i %i %i\n",$preamble[0],$preamble[1],$N,$BLength);
	for ($m=0;$m<$N;$m++){
		@b=unpack("C2Ss",(substr $d,8+$m*$BLength,$BLength));
		$svid = $b1[0];
		$az = $b1[2];
		$el=  $b1[3];
		next if ($az == 65535 || $el == -32768);
		$az = $az*100.0;
		$el = $el*100.0;
		
		($constellation,$prn,$reqsig) = SVIDtoGNSSParams($svid);
		
		if ($constellation==-1){
			next;
		}
		
		if (defined $GNSS{$svid}){
			$cn=$GNSS->{$svid}[4];
			$GNSS{$svid}=[($constellation,$prn,$az,$el,$cn)];
		}
		else{
			$GNSS{$svid}=[($constellation,$prn,$az,$el,-999)];
		}
		
	}
}

#-----------------------------------------------------------------------------
sub SendData
{
	foreach my $key (keys %GNSS){
		print "$GNSS{$key}[0] $GNSS{$key}[1] $GNSS{$key}[2] $GNSS{$key}[3] $GNSS{$key}[4]\n";
	}
	%GNSS=();
}

