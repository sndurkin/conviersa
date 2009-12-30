# Ragel IRC Machine Specification (RFC-2812)
# http://tools.ietf.org/html/rfc2812.
# Licensed under the MIT license.
# Written by triton in December 2009.

%%{

machine irc;

SPACE		=  " ";
crlf		=  "\r" . "\n";

letter		= alpha;
hexdigit	= xdigit;
special		= [[-`] | [{-}];

# any octet except NUL, CR, LF, " " and "@"
user		= ( extend - [\0\r\n @] )+;

# any 7-bit US-ASCII character,
# except NUL, CR, LF, FF, h/v TABs, and " "
# Note: BNF and comments are ambiguous on FF.
key			= ( ascii - ( [\0\t\n\v\r ] | 0x06 ) ){1,23};

# any octet except NUL, CR, LF, " " and ":"
nospcrlfcl	= extend - [\0\r\n :];
middle		= nospcrlfcl ( ":" | nospcrlfcl )*;
trailing	= ( [: ] | nospcrlfcl )*;

wildone		=  "?";
wildmany	=  "*";
nowild		=  extend - [\0*?]; # any octet except NUL, "*", "?"
noesc		=  extend - [\0\\]; # any octet except NUL and "\"
mask		=  ( nowild | noesc wildone | noesc wildmany )*;
matchone	=  extend - "\0"; # matches wildone
matchmany	=  matchone*;

ip4addr		=  digit{1,3} "." digit{1,3} "." digit{1,3} "." digit{1,3};
ip6addr		=  ( hexdigit+ ( ":" hexdigit+ ){7} )
				| ( "0:0:0:0:0:" ( "0" | "FFFF"i ) ":" ip4addr );
hostaddr	=  ip4addr | ip6addr;
targetmask	=  ( "$" | "#" ) mask;

nickname	= ( letter | special ) ( letter | digit | special | "-" ){,15};
shortname	= ( letter | digit ) ( letter | digit | "-" )* ( letter | digit )*;
hostname	= shortname ( "." shortname )*;
servername	= hostname;
host		= hostname | hostaddr;

target		= nickname | servername;

channelid	= ( [A-Z] | digit ){5};
chanstring	=  extend - [\0\a\r\n ,:];
channel		=  ( "#" | "+" | ( "!" channelid ) | "&" ) chanstring ( ":" chanstring )?;

msgto		=  ( channel | ( user ( "%" host )? "@" servername ) )
				| ( ( user "%" host ) | targetmask )
				| ( nickname | ( nickname "!" user "@" host ) );

msgtarget = msgto ( "," msgto )*;

params = ( ( SPACE middle ){,14} ( SPACE ":" trailing )? )
			| ( ( SPACE middle ){14} ( SPACE ( ":" )? trailing )? );

command = letter+ | digit{3};

prefix = servername | ( nickname ( ( "!" user )? "@" host )? );

message = ( ":" prefix SPACE )? command ( params )? crlf;

main := message;

}%%

%% write data;
%% write init;
%% write exec;

# QRegExp URL("lol(([\\w:]+)[/]{2})?(\\w|.)+");