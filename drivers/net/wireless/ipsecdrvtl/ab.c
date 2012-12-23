/*
   'aes.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Fri Oct 12 22:15:16 2012
*/
#include"cobf.h"
#ifdef _WIN32
#if defined( UNDER_CE) && defined( bb337) || ! defined( bb329)
#define bb355 1
#define bb332 1
#else
#define bb351 bb343
#define bb333 1
#define bb331 1
#endif
#define bb348 1
#include"uncobf.h"
#include<ndis.h>
#include"cobf.h"
#ifdef UNDER_CE
#include"uncobf.h"
#include<ndiswan.h>
#include"cobf.h"
#endif
#include"uncobf.h"
#include<stdio.h>
#include<basetsd.h>
#include"cobf.h"
bba bbs bbl bbf, *bb1;bba bbs bbe bbq, *bb93;bba bb135 bb124, *bb334;
bba bbs bbl bb40, *bb72;bba bbs bb135 bbk, *bb59;bba bbe bbu, *bb133;
bba bbh bbf*bb89;
#ifdef bb311
bba bbd bb60, *bb122;
#endif
#else
#include"uncobf.h"
#include<linux/module.h>
#include<linux/ctype.h>
#include<linux/time.h>
#include<linux/slab.h>
#include"cobf.h"
#ifndef bb116
#define bb116
#ifdef _WIN32
#include"uncobf.h"
#include<wtypes.h>
#include"cobf.h"
#else
#ifdef bb120
#include"uncobf.h"
#include<linux/types.h>
#include"cobf.h"
#else
#include"uncobf.h"
#include<stddef.h>
#include<sys/types.h>
#include"cobf.h"
#endif
#endif
#ifdef _WIN32
bba bb119 bb215;
#else
bba bbe bbu, *bb133, *bb246;
#define bb201 1
#define bb202 0
bba bb251 bb205, *bb240, *bb208;bba bbe bb285, *bb283, *bb262;bba bbs
bbq, *bb93, *bb270;bba bb6 bb238, *bb216;bba bbs bb6 bb263, *bb250;
bba bb6 bb111, *bb222;bba bbs bb6 bb63, *bb289;bba bb63 bb264, *bb207
;bba bb63 bb219, *bb254;bba bb111 bb119, *bb226;bba bb243 bb247;bba
bb279 bb124;bba bb230 bb83;bba bb118 bb112;bba bb118 bb253;
#ifdef bb211
bba bb282 bb40, *bb72;bba bb258 bbk, *bb59;bba bb232 bbd, *bb28;bba
bb256 bb56, *bb113;
#else
bba bb271 bb40, *bb72;bba bb229 bbk, *bb59;bba bb233 bbd, *bb28;bba
bb277 bb56, *bb113;
#endif
bba bb40 bbf, *bb1, *bb214;bba bbk bb237, *bb245, *bb224;bba bbk bb255
, *bb220, *bb248;bba bbd bb60, *bb122, *bb206;bba bb83 bb37, *bb274, *
bb252;bba bbd bb290, *bb275, *bb210;bba bb112 bb265, *bb291, *bb269;
bba bb56 bb227, *bb261, *bb223;
#define bb140 bbb
bba bbb*bb221, *bb77;bba bbh bbb*bb225;bba bbl bb287;bba bbl*bb276;
bba bbh bbl*bb82;
#if defined( bb120)
bba bbe bb115;
#endif
bba bb115 bb20;bba bb20*bb218;bba bbh bb20*bb187;
#if defined( bb213) || defined( bb266)
bba bb20 bb36;bba bb20 bb114;
#else
bba bbl bb36;bba bbs bbl bb114;
#endif
bba bbh bb36*bb257;bba bb36*bb244;bba bb60 bb212, *bb239;bba bbb*
bb106;bba bb106*bb241;
#define bb281( bb34) bbi bb34##__ { bbe bb228; }; bba bbi bb34##__  * \
 bb34
bba bbi{bb37 bb188,bb242,bb231,bb260;}bb286, *bb234, *bb278;bba bbi{
bb37 bb8,bb193;}bb280, *bb235, *bb259;bba bbi{bb37 bb267,bb249;}bb236
, *bb217, *bb284;
#endif
bba bbh bbf*bb89;
#endif
bba bbf bb100;
#define IN
#define OUT
#ifdef _DEBUG
#define bb139( bbc) bb31( bbc)
#else
#define bb139( bbc) ( bbb)( bbc)
#endif
bba bbe bb161, *bb173;
#define bb209 0
#define bb314 1
#define bb298 2
#define bb324 3
#define bb346 4
bba bbe bb349;bba bbb*bb121;
#endif
#ifdef _WIN32
#ifndef UNDER_CE
#define bb30 bb344
#define bb43 bb335
bba bbs bb6 bb30;bba bb6 bb43;
#endif
#else
#endif
#ifdef _WIN32
bbb*bb128(bb30 bb47);bbb bb105(bbb* );bbb*bb137(bb30 bb159,bb30 bb47);
#else
#define bb128( bbc) bb146(1, bbc, bb142)
#define bb105( bbc) bb342( bbc)
#define bb137( bbc, bbn) bb146( bbc, bbn, bb142)
#endif
#ifdef _WIN32
#define bb31( bbc) bb358( bbc)
#else
#ifdef _DEBUG
bbe bb145(bbh bbl*bb95,bbh bbl*bb25,bbs bb272);
#define bb31( bbc) ( bbb)(( bbc) || ( bb145(# bbc, __FILE__, __LINE__ \
)))
#else
#define bb31( bbc) (( bbb)0)
#endif
#endif
bb43 bb301(bb43*bb320);
#ifndef _WIN32
bbe bb328(bbh bbl*bbg);bbe bb322(bbh bbl*bb19,...);
#endif
#ifdef _WIN32
bba bb353 bb96;
#define bb141( bbc) bb356( bbc)
#define bb144( bbc) bb345( bbc)
#define bb134( bbc) bb350( bbc)
#define bb132( bbc) bb339( bbc)
#else
bba bb347 bb96;
#define bb141( bbc) ( bbb)(  *  bbc = bb330( bbc))
#define bb144( bbc) (( bbb)0)
#define bb134( bbc) bb352( bbc)
#define bb132( bbc) bb354( bbc)
#endif
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbi{bbq bb456;bbd bb417[4 * (14 +1 )];}bb363;bbb bb1098(bb363*bbj,
bbh bbb*bb71,bbq bb143);bbb bb1736(bb363*bbj,bbh bbb*bb71,bbq bb143);
bbb bb1034(bb363*bbj,bbb*bb14,bbh bbb*bb5);bbb bb1779(bb363*bbj,bbb*
bb14,bbh bbb*bb5);
#ifdef __cplusplus
}
#endif
bb41 bbf bb406[256 ]={0x63 ,0x7c ,0x77 ,0x7b ,0xf2 ,0x6b ,0x6f ,0xc5 ,0x30 ,
0x01 ,0x67 ,0x2b ,0xfe ,0xd7 ,0xab ,0x76 ,0xca ,0x82 ,0xc9 ,0x7d ,0xfa ,0x59 ,0x47
,0xf0 ,0xad ,0xd4 ,0xa2 ,0xaf ,0x9c ,0xa4 ,0x72 ,0xc0 ,0xb7 ,0xfd ,0x93 ,0x26 ,
0x36 ,0x3f ,0xf7 ,0xcc ,0x34 ,0xa5 ,0xe5 ,0xf1 ,0x71 ,0xd8 ,0x31 ,0x15 ,0x04 ,0xc7
,0x23 ,0xc3 ,0x18 ,0x96 ,0x05 ,0x9a ,0x07 ,0x12 ,0x80 ,0xe2 ,0xeb ,0x27 ,0xb2 ,
0x75 ,0x09 ,0x83 ,0x2c ,0x1a ,0x1b ,0x6e ,0x5a ,0xa0 ,0x52 ,0x3b ,0xd6 ,0xb3 ,0x29
,0xe3 ,0x2f ,0x84 ,0x53 ,0xd1 ,0x00 ,0xed ,0x20 ,0xfc ,0xb1 ,0x5b ,0x6a ,0xcb ,
0xbe ,0x39 ,0x4a ,0x4c ,0x58 ,0xcf ,0xd0 ,0xef ,0xaa ,0xfb ,0x43 ,0x4d ,0x33 ,0x85
,0x45 ,0xf9 ,0x02 ,0x7f ,0x50 ,0x3c ,0x9f ,0xa8 ,0x51 ,0xa3 ,0x40 ,0x8f ,0x92 ,
0x9d ,0x38 ,0xf5 ,0xbc ,0xb6 ,0xda ,0x21 ,0x10 ,0xff ,0xf3 ,0xd2 ,0xcd ,0x0c ,0x13
,0xec ,0x5f ,0x97 ,0x44 ,0x17 ,0xc4 ,0xa7 ,0x7e ,0x3d ,0x64 ,0x5d ,0x19 ,0x73 ,
0x60 ,0x81 ,0x4f ,0xdc ,0x22 ,0x2a ,0x90 ,0x88 ,0x46 ,0xee ,0xb8 ,0x14 ,0xde ,0x5e
,0x0b ,0xdb ,0xe0 ,0x32 ,0x3a ,0x0a ,0x49 ,0x06 ,0x24 ,0x5c ,0xc2 ,0xd3 ,0xac ,
0x62 ,0x91 ,0x95 ,0xe4 ,0x79 ,0xe7 ,0xc8 ,0x37 ,0x6d ,0x8d ,0xd5 ,0x4e ,0xa9 ,0x6c
,0x56 ,0xf4 ,0xea ,0x65 ,0x7a ,0xae ,0x08 ,0xba ,0x78 ,0x25 ,0x2e ,0x1c ,0xa6 ,
0xb4 ,0xc6 ,0xe8 ,0xdd ,0x74 ,0x1f ,0x4b ,0xbd ,0x8b ,0x8a ,0x70 ,0x3e ,0xb5 ,0x66
,0x48 ,0x03 ,0xf6 ,0x0e ,0x61 ,0x35 ,0x57 ,0xb9 ,0x86 ,0xc1 ,0x1d ,0x9e ,0xe1 ,
0xf8 ,0x98 ,0x11 ,0x69 ,0xd9 ,0x8e ,0x94 ,0x9b ,0x1e ,0x87 ,0xe9 ,0xce ,0x55 ,0x28
,0xdf ,0x8c ,0xa1 ,0x89 ,0x0d ,0xbf ,0xe6 ,0x42 ,0x68 ,0x41 ,0x99 ,0x2d ,0x0f ,
0xb0 ,0x54 ,0xbb ,0x16 };bb41 bbq bb2085(bbq bb438){bb438<<=1 ;bbm(bb438&
0x0100 )bb438^=0x011b ;bb2 bb438;}bb41 bbd bb2080[256 ],bb2079[256 ],
bb2078[256 ],bb2077[256 ];bb41 bbf bb994[256 ];bb41 bbd bb1766[256 ],
bb1767[256 ],bb1765[256 ],bb1764[256 ];bb41 bbb bb2057(){bbq bbz;bb90(
bbz=0 ;bbz<256 ;bbz++){bbq bb75=bb406[bbz];{bbq bb1834=bb2085(bb75),
bb2543=bb1834^bb75;bbq bb45=bb1834<<24 |bb75<<16 |bb75<<8 |bb2543;bb2080
[bbz]=bb45;bb2079[bbz]=((bb45)>>(8 )|(bb45)<<(32 -8 ));bb2078[bbz]=((
bb45)>>(16 )|(bb45)<<(32 -16 ));bb2077[bbz]=((bb45)>>(24 )|(bb45)<<(32 -24
));}bb994[bb75]=bbz;{bbq bb2293=bb2085(bbz),bb2292=bb2085(bb2293),
bb2183=bb2085(bb2292),bb2544=bb2183^bbz,bb2497=bb2183^bb2293^bbz,
bb2500=bb2183^bb2292^bbz,bb2499=bb2183^bb2292^bb2293;bbq bb45=bb2499
<<24 |bb2544<<16 |bb2500<<8 |bb2497;bb1766[bb75]=bb45;bb1767[bb75]=((
bb45)>>(8 )|(bb45)<<(32 -8 ));bb1765[bb75]=((bb45)>>(16 )|(bb45)<<(32 -16 ));
bb1764[bb75]=((bb45)>>(24 )|(bb45)<<(32 -24 ));}}}bbb bb1098(bb363*bbj,
bbh bbb*bb71,bbq bb143){bbq bb1253,bb456,bbz;bb28 bb3=bbj->bb417;bb41
bbu bb1822=1 ;bbm(bb1822){bb2057();bb1822=0 ;}bb31(bb143==16 ||bb143==24
||bb143==32 );bb1253=bb143/4 ;bbj->bb456=bb456=bb1253+6 ;bb90(bbz=0 ;bbz<
bb1253;bbz++) *bb3++=(((bb1)((bb28)bb71+bbz))[3 ]|((bb1)((bb28)bb71+
bbz))[2 ]<<8 |((bb1)((bb28)bb71+bbz))[1 ]<<16 |((bb1)((bb28)bb71+bbz))[0 ]
<<24 );{bbq bbo=1 ;bb90(;bbz<4 * (bb456+1 );bbz++){bbd bb45= * (bb3-1 );
bbm(bbz%bb1253==0 ){bb45=(bb406[bb45>>24 ]^bb406[bb45>>16 &0xff ]<<24 ^
bb406[bb45>>8 &0xff ]<<16 ^bb406[bb45&0xff ]<<8 )^(bbo<<24 );bbo=bb2085(bbo
);}bb54 bbm(bb1253>6 &&bbz%bb1253==4 ){bb45=((bb45)>>(8 )|(bb45)<<(32 -8 ));
bb45=(bb406[bb45>>24 ]^bb406[bb45>>16 &0xff ]<<24 ^bb406[bb45>>8 &0xff ]<<
16 ^bb406[bb45&0xff ]<<8 );} *bb3= * (bb3-bb1253)^bb45;bb3++;}}}bbb
bb1736(bb363*bbj,bbh bbb*bb71,bbq bb143){bb363 bbw;bb28 bb3=bbj->
bb417;bbq bbz;bb1098(&bbw,bb71,bb143);bbj->bb456=bbw.bb456;bb90(bbz=0
;bbz<=bbw.bb456;bbz++){bb81(bb3+4 *bbz,bbw.bb417+4 * (bbw.bb456-bbz),16
);}bb90(bbz=1 ;bbz<bbw.bb456;bbz++){bb3+=4 ;bb3[0 ]=bb1766[bb406[bb3[0 ]
>>24 ]]^bb1767[bb406[bb3[0 ]>>16 &0xff ]]^bb1765[bb406[bb3[0 ]>>8 &0xff ]]^
bb1764[bb406[bb3[0 ]&0xff ]];;bb3[1 ]=bb1766[bb406[bb3[1 ]>>24 ]]^bb1767[
bb406[bb3[1 ]>>16 &0xff ]]^bb1765[bb406[bb3[1 ]>>8 &0xff ]]^bb1764[bb406[
bb3[1 ]&0xff ]];;bb3[2 ]=bb1766[bb406[bb3[2 ]>>24 ]]^bb1767[bb406[bb3[2 ]>>
16 &0xff ]]^bb1765[bb406[bb3[2 ]>>8 &0xff ]]^bb1764[bb406[bb3[2 ]&0xff ]];;
bb3[3 ]=bb1766[bb406[bb3[3 ]>>24 ]]^bb1767[bb406[bb3[3 ]>>16 &0xff ]]^
bb1765[bb406[bb3[3 ]>>8 &0xff ]]^bb1764[bb406[bb3[3 ]&0xff ]];;}}bbb bb1034
(bb363*bbj,bbb*bb14,bbh bbb*bb5){bbd bb750,bb194,bb338,bb760,bb1128,
bb50,bb86,bb1127;bbq bb456=bbj->bb456,bbz;bb28 bb3=(bb28)bbj->bb417;
bb750=(((bb1)((bb28)bb5))[3 ]|((bb1)((bb28)bb5))[2 ]<<8 |((bb1)((bb28)bb5
))[1 ]<<16 |((bb1)((bb28)bb5))[0 ]<<24 )^bb3[0 ];bb194=(((bb1)((bb28)bb5+1
))[3 ]|((bb1)((bb28)bb5+1 ))[2 ]<<8 |((bb1)((bb28)bb5+1 ))[1 ]<<16 |((bb1)((
bb28)bb5+1 ))[0 ]<<24 )^bb3[1 ];bb338=(((bb1)((bb28)bb5+2 ))[3 ]|((bb1)((
bb28)bb5+2 ))[2 ]<<8 |((bb1)((bb28)bb5+2 ))[1 ]<<16 |((bb1)((bb28)bb5+2 ))[0
]<<24 )^bb3[2 ];bb760=(((bb1)((bb28)bb5+3 ))[3 ]|((bb1)((bb28)bb5+3 ))[2 ]
<<8 |((bb1)((bb28)bb5+3 ))[1 ]<<16 |((bb1)((bb28)bb5+3 ))[0 ]<<24 )^bb3[3 ];
bb90(bbz=1 ;bbz<bb456;bbz++){bb3+=4 ;bb1128=bb2080[bb750>>24 ]^bb2079[(
bb194>>16 )&0xff ]^bb2078[(bb338>>8 )&0xff ]^bb2077[(bb760&0xff )]^bb3[0 ];
bb50=bb2080[bb194>>24 ]^bb2079[(bb338>>16 )&0xff ]^bb2078[(bb760>>8 )&
0xff ]^bb2077[(bb750&0xff )]^bb3[1 ];bb86=bb2080[bb338>>24 ]^bb2079[(
bb760>>16 )&0xff ]^bb2078[(bb750>>8 )&0xff ]^bb2077[(bb194&0xff )]^bb3[2 ];
bb1127=bb2080[bb760>>24 ]^bb2079[(bb750>>16 )&0xff ]^bb2078[(bb194>>8 )&
0xff ]^bb2077[(bb338&0xff )]^bb3[3 ];bb750=bb1128;bb194=bb50;bb338=bb86;
bb760=bb1127;}bb3+=4 ;bb1128=bb406[bb750>>24 ]<<24 ^bb406[bb194>>16 &0xff
]<<16 ^bb406[bb338>>8 &0xff ]<<8 ^bb406[bb760&0xff ]^bb3[0 ];bb50=bb406[
bb194>>24 ]<<24 ^bb406[bb338>>16 &0xff ]<<16 ^bb406[bb760>>8 &0xff ]<<8 ^
bb406[bb750&0xff ]^bb3[1 ];bb86=bb406[bb338>>24 ]<<24 ^bb406[bb760>>16 &
0xff ]<<16 ^bb406[bb750>>8 &0xff ]<<8 ^bb406[bb194&0xff ]^bb3[2 ];bb1127=
bb406[bb760>>24 ]<<24 ^bb406[bb750>>16 &0xff ]<<16 ^bb406[bb194>>8 &0xff ]<<
8 ^bb406[bb338&0xff ]^bb3[3 ];((bb28)bb14)[0 ]=(((bb1)(&bb1128))[3 ]|((bb1
)(&bb1128))[2 ]<<8 |((bb1)(&bb1128))[1 ]<<16 |((bb1)(&bb1128))[0 ]<<24 );((
bb28)bb14)[1 ]=(((bb1)(&bb50))[3 ]|((bb1)(&bb50))[2 ]<<8 |((bb1)(&bb50))[
1 ]<<16 |((bb1)(&bb50))[0 ]<<24 );((bb28)bb14)[2 ]=(((bb1)(&bb86))[3 ]|((
bb1)(&bb86))[2 ]<<8 |((bb1)(&bb86))[1 ]<<16 |((bb1)(&bb86))[0 ]<<24 );((
bb28)bb14)[3 ]=(((bb1)(&bb1127))[3 ]|((bb1)(&bb1127))[2 ]<<8 |((bb1)(&
bb1127))[1 ]<<16 |((bb1)(&bb1127))[0 ]<<24 );}bbb bb1779(bb363*bbj,bbb*
bb14,bbh bbb*bb5){bbd bb750,bb194,bb338,bb760,bb1128,bb50,bb86,bb1127
;bbq bb456=bbj->bb456,bbz;bb28 bb3=(bb28)bbj->bb417;bb750=(((bb1)((
bb28)bb5))[3 ]|((bb1)((bb28)bb5))[2 ]<<8 |((bb1)((bb28)bb5))[1 ]<<16 |((
bb1)((bb28)bb5))[0 ]<<24 )^bb3[0 ];bb194=(((bb1)((bb28)bb5+1 ))[3 ]|((bb1)(
(bb28)bb5+1 ))[2 ]<<8 |((bb1)((bb28)bb5+1 ))[1 ]<<16 |((bb1)((bb28)bb5+1 ))[
0 ]<<24 )^bb3[1 ];bb338=(((bb1)((bb28)bb5+2 ))[3 ]|((bb1)((bb28)bb5+2 ))[2 ]
<<8 |((bb1)((bb28)bb5+2 ))[1 ]<<16 |((bb1)((bb28)bb5+2 ))[0 ]<<24 )^bb3[2 ];
bb760=(((bb1)((bb28)bb5+3 ))[3 ]|((bb1)((bb28)bb5+3 ))[2 ]<<8 |((bb1)((
bb28)bb5+3 ))[1 ]<<16 |((bb1)((bb28)bb5+3 ))[0 ]<<24 )^bb3[3 ];bb90(bbz=1 ;
bbz<bb456;bbz++){bb3+=4 ;bb1128=bb1766[bb750>>24 ]^bb1767[bb760>>16 &
0xff ]^bb1765[bb338>>8 &0xff ]^bb1764[bb194&0xff ]^bb3[0 ];bb50=bb1766[
bb194>>24 ]^bb1767[bb750>>16 &0xff ]^bb1765[bb760>>8 &0xff ]^bb1764[bb338&
0xff ]^bb3[1 ];bb86=bb1766[bb338>>24 ]^bb1767[bb194>>16 &0xff ]^bb1765[
bb750>>8 &0xff ]^bb1764[bb760&0xff ]^bb3[2 ];bb1127=bb1766[bb760>>24 ]^
bb1767[bb338>>16 &0xff ]^bb1765[bb194>>8 &0xff ]^bb1764[bb750&0xff ]^bb3[3
];bb750=bb1128;bb194=bb50;bb338=bb86;bb760=bb1127;}bb3+=4 ;bb1128=
bb994[bb750>>24 ]<<24 ^bb994[bb760>>16 &0xff ]<<16 ^bb994[bb338>>8 &0xff ]<<
8 ^bb994[bb194&0xff ]^bb3[0 ];bb50=bb994[bb194>>24 ]<<24 ^bb994[bb750>>16 &
0xff ]<<16 ^bb994[bb760>>8 &0xff ]<<8 ^bb994[bb338&0xff ]^bb3[1 ];bb86=bb994
[bb338>>24 ]<<24 ^bb994[bb194>>16 &0xff ]<<16 ^bb994[bb750>>8 &0xff ]<<8 ^
bb994[bb760&0xff ]^bb3[2 ];bb1127=bb994[bb760>>24 ]<<24 ^bb994[bb338>>16 &
0xff ]<<16 ^bb994[bb194>>8 &0xff ]<<8 ^bb994[bb750&0xff ]^bb3[3 ];((bb28)bb14
)[0 ]=(((bb1)(&bb1128))[3 ]|((bb1)(&bb1128))[2 ]<<8 |((bb1)(&bb1128))[1 ]
<<16 |((bb1)(&bb1128))[0 ]<<24 );((bb28)bb14)[1 ]=(((bb1)(&bb50))[3 ]|((
bb1)(&bb50))[2 ]<<8 |((bb1)(&bb50))[1 ]<<16 |((bb1)(&bb50))[0 ]<<24 );((
bb28)bb14)[2 ]=(((bb1)(&bb86))[3 ]|((bb1)(&bb86))[2 ]<<8 |((bb1)(&bb86))[
1 ]<<16 |((bb1)(&bb86))[0 ]<<24 );((bb28)bb14)[3 ]=(((bb1)(&bb1127))[3 ]|((
bb1)(&bb1127))[2 ]<<8 |((bb1)(&bb1127))[1 ]<<16 |((bb1)(&bb1127))[0 ]<<24 );
}
