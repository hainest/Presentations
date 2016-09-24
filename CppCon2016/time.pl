use strict;
use warnings;
use IO::Tee;
use Getopt::Long qw(GetOptions);

sub execute($) {
	my $name = shift;
	my $res = `$name`;
	die if (($? >> 8) != 0 || $? == -1 || ($? & 127) != 0);
	return $res;
}

sub build($$) {
	my ($options,$name) = @_;
	execute("make $options clean $name >> build.out 2>&1");
}

# Force ordering of compiler usage
my @compilers = ('g++-6', 'clang++-3.9', 'icc');

my %config = (
	'g++-6' => {
		'arch'  => ['', '-march=native'],
		'opt'   => ['-O2', '-O3', '-O3 -ffast-math']
	},
	'clang++-3.9' => {
		'arch'  => ['', '-march=native'],
		'opt'   => ['-O2', '-O3', '-O3 -ffast-math']
	},
	'icc' => {
		'arch'  => ['', '-march=native'],
		# The default -fp-model is fast=1, so disable it unless we want 'fast-math'
		'opt'   => ['-O2 -fp-model strict', '-O3 -fp-model strict', '-O3 -fp-model fast=2']
	},
);

my $do_timings = 0;
GetOptions('timings' => \$do_timings);

if($do_timings) {
	die "Usage: $0 [--timings input comparison_sum output]\n" unless @ARGV == 3;
	my $test_input = $ARGV[0];
	my $comparison_sum = $ARGV[1];
	
	open my $fdOut, '>', $ARGV[2] or die "Unable to open $ARGV[2]\n";
	my $tee = IO::Tee->new($fdOut, \*STDOUT);
	
	print "Running full test suite (this could take some time): \n";
	
	for my $name ('streaming', 'cstyle', 'intrinsics') {
	for my $c (@compilers) {
	for my $o (@{$config{$c}{'opt'}}) {
		for my $a (@{$config{$c}{'arch'}}) {
			print $tee "# $name $c $o $a\n" ;
			my $config = "CXX=$c OPT=\"$o\" ARCH=\"$a\"";
			build($config, $name);
			chomp(my $time = execute("./streaming $test_input $comparison_sum"));
			print $fdOut "$time\n";
		}
	}}}
	exit();
}

die "Usage: $0 test compiler arch_index opt_index" if @ARGV < 3;

my $c = $ARGV[1];							# compiler
my $a = @{$config{$c}{'arch'}}[$ARGV[2]];	# arch
my $o = @{$config{$c}{'opt'}}[$ARGV[3]];	# opt

my $config = "CXX=$c OPT=\"$o\" ARCH=\"$a\"";
build($config, $ARGV[0]);
execute("make dump | c++filt > __temp__");

open my $fdIn, '<', '__temp__' or die;
while(<$fdIn>) {
	last if /\<saxpy_loop_begin\d{3}\>\:/;
}
my $start_pos = $.;
while(<$fdIn>) {
	last if /\<saxpy_loop_end\d{3}\>\:/ || $. - $start_pos >= 200 || /vzeroupper/;
	print substr($_, 32) if length > 32;
}

