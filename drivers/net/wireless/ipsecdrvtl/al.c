/*
   'md5.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Fri Oct 12 22:15:16 2012
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
bba bbi{bbd bb10;bbd bb23[4 ];bbf bb101[64 ];}bb526;bbb bb1817(bb526*
bbj);bbb bb1311(bb526*bbj,bbh bbb*bb498,bbq bb10);bbb bb1820(bb526*
bbj,bbb*bb14);bbb bb1852(bbb*bb14,bbh bbb*bb5,bbq bb10);bbb bb1963(
bbb*bb14,bb82 bb5);
#ifdef __cplusplus
}
#endif
bb41 bbb bb1252(bbd bb23[4 ],bbh bbf bb95[64 ]){bb41 bbd bb3[64 ]={
0xd76aa478 ,0xe8c7b756 ,0x242070db ,0xc1bdceee ,0xf57c0faf ,0x4787c62a ,
0xa8304613 ,0xfd469501 ,0x698098d8 ,0x8b44f7af ,0xffff5bb1 ,0x895cd7be ,
0x6b901122 ,0xfd987193 ,0xa679438e ,0x49b40821 ,0xf61e2562 ,0xc040b340 ,
0x265e5a51 ,0xe9b6c7aa ,0xd62f105d ,0x02441453 ,0xd8a1e681 ,0xe7d3fbc8 ,
0x21e1cde6 ,0xc33707d6 ,0xf4d50d87 ,0x455a14ed ,0xa9e3e905 ,0xfcefa3f8 ,
0x676f02d9 ,0x8d2a4c8a ,0xfffa3942 ,0x8771f681 ,0x6d9d6122 ,0xfde5380c ,
0xa4beea44 ,0x4bdecfa9 ,0xf6bb4b60 ,0xbebfbc70 ,0x289b7ec6 ,0xeaa127fa ,
0xd4ef3085 ,0x04881d05 ,0xd9d4d039 ,0xe6db99e5 ,0x1fa27cf8 ,0xc4ac5665 ,
0xf4292244 ,0x432aff97 ,0xab9423a7 ,0xfc93a039 ,0x655b59c3 ,0x8f0ccc92 ,
0xffeff47d ,0x85845dd1 ,0x6fa87e4f ,0xfe2ce6e0 ,0xa3014314 ,0x4e0811a1 ,
0xf7537e82 ,0xbd3af235 ,0x2ad7d2bb ,0xeb86d391 ,};bb31(bb12(bbe)>=4 );{bbd
bbc=bb23[0 ],bbn=bb23[1 ],bbo=bb23[2 ],bbt=bb23[3 ],bb8;bbd bbv[16 ],bbz;
bb90(bbz=0 ;bbz<16 ;bbz++,bb95+=4 )bbv[bbz]=(bb95[0 ]|bb95[1 ]<<8 |bb95[2 ]
<<16 |bb95[3 ]<<24 );bb8=bbc+(bbn&bbo|~bbn&bbt)+bb3[0 ]+bbv[(0 )];bbc=bbn+
((bb8)<<(7 )|(bb8)>>(32 -7 ));bb8=bbt+(bbc&bbn|~bbc&bbo)+bb3[0 +1 ]+bbv[(0
+1 )];bbt=bbc+((bb8)<<(12 )|(bb8)>>(32 -12 ));bb8=bbo+(bbt&bbc|~bbt&bbn)+
bb3[0 +2 ]+bbv[(0 +2 )];bbo=bbt+((bb8)<<(17 )|(bb8)>>(32 -17 ));bb8=bbn+(bbo
&bbt|~bbo&bbc)+bb3[0 +3 ]+bbv[(0 +3 )];bbn=bbo+((bb8)<<(22 )|(bb8)>>(32 -22
));bb8=bbc+(bbn&bbo|~bbn&bbt)+bb3[0 +4 ]+bbv[(0 +4 )];bbc=bbn+((bb8)<<(7 )|
(bb8)>>(32 -7 ));bb8=bbt+(bbc&bbn|~bbc&bbo)+bb3[0 +4 +1 ]+bbv[(0 +4 +1 )];bbt
=bbc+((bb8)<<(12 )|(bb8)>>(32 -12 ));bb8=bbo+(bbt&bbc|~bbt&bbn)+bb3[0 +4 +
2 ]+bbv[(0 +4 +2 )];bbo=bbt+((bb8)<<(17 )|(bb8)>>(32 -17 ));bb8=bbn+(bbo&bbt
|~bbo&bbc)+bb3[0 +4 +3 ]+bbv[(0 +4 +3 )];bbn=bbo+((bb8)<<(22 )|(bb8)>>(32 -22
));bb8=bbc+(bbn&bbo|~bbn&bbt)+bb3[0 +8 ]+bbv[(0 +8 )];bbc=bbn+((bb8)<<(7 )|
(bb8)>>(32 -7 ));bb8=bbt+(bbc&bbn|~bbc&bbo)+bb3[0 +8 +1 ]+bbv[(0 +8 +1 )];bbt
=bbc+((bb8)<<(12 )|(bb8)>>(32 -12 ));bb8=bbo+(bbt&bbc|~bbt&bbn)+bb3[0 +8 +
2 ]+bbv[(0 +8 +2 )];bbo=bbt+((bb8)<<(17 )|(bb8)>>(32 -17 ));bb8=bbn+(bbo&bbt
|~bbo&bbc)+bb3[0 +8 +3 ]+bbv[(0 +8 +3 )];bbn=bbo+((bb8)<<(22 )|(bb8)>>(32 -22
));bb8=bbc+(bbn&bbo|~bbn&bbt)+bb3[0 +12 ]+bbv[(0 +12 )];bbc=bbn+((bb8)<<(
7 )|(bb8)>>(32 -7 ));bb8=bbt+(bbc&bbn|~bbc&bbo)+bb3[0 +12 +1 ]+bbv[(0 +12 +1 )]
;bbt=bbc+((bb8)<<(12 )|(bb8)>>(32 -12 ));bb8=bbo+(bbt&bbc|~bbt&bbn)+bb3[
0 +12 +2 ]+bbv[(0 +12 +2 )];bbo=bbt+((bb8)<<(17 )|(bb8)>>(32 -17 ));bb8=bbn+(
bbo&bbt|~bbo&bbc)+bb3[0 +12 +3 ]+bbv[(0 +12 +3 )];bbn=bbo+((bb8)<<(22 )|(bb8
)>>(32 -22 ));bb8=bbc+(bbt&bbn|~bbt&bbo)+bb3[16 ]+bbv[(5 * (16 )+1 )%16 ];
bbc=bbn+((bb8)<<(5 )|(bb8)>>(32 -5 ));bb8=bbt+(bbo&bbc|~bbo&bbn)+bb3[16 +
1 ]+bbv[(5 * (16 +1 )+1 )%16 ];bbt=bbc+((bb8)<<(9 )|(bb8)>>(32 -9 ));bb8=bbo+(
bbn&bbt|~bbn&bbc)+bb3[16 +2 ]+bbv[(5 * (16 +2 )+1 )%16 ];bbo=bbt+((bb8)<<(14
)|(bb8)>>(32 -14 ));bb8=bbn+(bbc&bbo|~bbc&bbt)+bb3[16 +3 ]+bbv[(5 * (16 +3 )+
1 )%16 ];bbn=bbo+((bb8)<<(20 )|(bb8)>>(32 -20 ));bb8=bbc+(bbt&bbn|~bbt&bbo
)+bb3[16 +4 ]+bbv[(5 * (16 +4 )+1 )%16 ];bbc=bbn+((bb8)<<(5 )|(bb8)>>(32 -5 ));
bb8=bbt+(bbo&bbc|~bbo&bbn)+bb3[16 +4 +1 ]+bbv[(5 * (16 +4 +1 )+1 )%16 ];bbt=
bbc+((bb8)<<(9 )|(bb8)>>(32 -9 ));bb8=bbo+(bbn&bbt|~bbn&bbc)+bb3[16 +4 +2 ]
+bbv[(5 * (16 +4 +2 )+1 )%16 ];bbo=bbt+((bb8)<<(14 )|(bb8)>>(32 -14 ));bb8=bbn
+(bbc&bbo|~bbc&bbt)+bb3[16 +4 +3 ]+bbv[(5 * (16 +4 +3 )+1 )%16 ];bbn=bbo+((bb8
)<<(20 )|(bb8)>>(32 -20 ));bb8=bbc+(bbt&bbn|~bbt&bbo)+bb3[16 +8 ]+bbv[(5 * (
16 +8 )+1 )%16 ];bbc=bbn+((bb8)<<(5 )|(bb8)>>(32 -5 ));bb8=bbt+(bbo&bbc|~bbo
&bbn)+bb3[16 +8 +1 ]+bbv[(5 * (16 +8 +1 )+1 )%16 ];bbt=bbc+((bb8)<<(9 )|(bb8)>>
(32 -9 ));bb8=bbo+(bbn&bbt|~bbn&bbc)+bb3[16 +8 +2 ]+bbv[(5 * (16 +8 +2 )+1 )%16
];bbo=bbt+((bb8)<<(14 )|(bb8)>>(32 -14 ));bb8=bbn+(bbc&bbo|~bbc&bbt)+bb3
[16 +8 +3 ]+bbv[(5 * (16 +8 +3 )+1 )%16 ];bbn=bbo+((bb8)<<(20 )|(bb8)>>(32 -20 ));
bb8=bbc+(bbt&bbn|~bbt&bbo)+bb3[16 +12 ]+bbv[(5 * (16 +12 )+1 )%16 ];bbc=bbn+
((bb8)<<(5 )|(bb8)>>(32 -5 ));bb8=bbt+(bbo&bbc|~bbo&bbn)+bb3[16 +12 +1 ]+
bbv[(5 * (16 +12 +1 )+1 )%16 ];bbt=bbc+((bb8)<<(9 )|(bb8)>>(32 -9 ));bb8=bbo+(
bbn&bbt|~bbn&bbc)+bb3[16 +12 +2 ]+bbv[(5 * (16 +12 +2 )+1 )%16 ];bbo=bbt+((bb8
)<<(14 )|(bb8)>>(32 -14 ));bb8=bbn+(bbc&bbo|~bbc&bbt)+bb3[16 +12 +3 ]+bbv[(
5 * (16 +12 +3 )+1 )%16 ];bbn=bbo+((bb8)<<(20 )|(bb8)>>(32 -20 ));bb8=bbc+(bbn
^bbo^bbt)+bb3[32 ]+bbv[(3 * (32 )+5 )%16 ];bbc=bbn+((bb8)<<(4 )|(bb8)>>(32 -
4 ));bb8=bbt+(bbc^bbn^bbo)+bb3[32 +1 ]+bbv[(3 * (32 +1 )+5 )%16 ];bbt=bbc+((
bb8)<<(11 )|(bb8)>>(32 -11 ));bb8=bbo+(bbt^bbc^bbn)+bb3[32 +2 ]+bbv[(3 * (
32 +2 )+5 )%16 ];bbo=bbt+((bb8)<<(16 )|(bb8)>>(32 -16 ));bb8=bbn+(bbo^bbt^
bbc)+bb3[32 +3 ]+bbv[(3 * (32 +3 )+5 )%16 ];bbn=bbo+((bb8)<<(23 )|(bb8)>>(32 -
23 ));bb8=bbc+(bbn^bbo^bbt)+bb3[32 +4 ]+bbv[(3 * (32 +4 )+5 )%16 ];bbc=bbn+((
bb8)<<(4 )|(bb8)>>(32 -4 ));bb8=bbt+(bbc^bbn^bbo)+bb3[32 +4 +1 ]+bbv[(3 * (
32 +4 +1 )+5 )%16 ];bbt=bbc+((bb8)<<(11 )|(bb8)>>(32 -11 ));bb8=bbo+(bbt^bbc^
bbn)+bb3[32 +4 +2 ]+bbv[(3 * (32 +4 +2 )+5 )%16 ];bbo=bbt+((bb8)<<(16 )|(bb8)>>
(32 -16 ));bb8=bbn+(bbo^bbt^bbc)+bb3[32 +4 +3 ]+bbv[(3 * (32 +4 +3 )+5 )%16 ];
bbn=bbo+((bb8)<<(23 )|(bb8)>>(32 -23 ));bb8=bbc+(bbn^bbo^bbt)+bb3[32 +8 ]+
bbv[(3 * (32 +8 )+5 )%16 ];bbc=bbn+((bb8)<<(4 )|(bb8)>>(32 -4 ));bb8=bbt+(bbc
^bbn^bbo)+bb3[32 +8 +1 ]+bbv[(3 * (32 +8 +1 )+5 )%16 ];bbt=bbc+((bb8)<<(11 )|(
bb8)>>(32 -11 ));bb8=bbo+(bbt^bbc^bbn)+bb3[32 +8 +2 ]+bbv[(3 * (32 +8 +2 )+5 )%
16 ];bbo=bbt+((bb8)<<(16 )|(bb8)>>(32 -16 ));bb8=bbn+(bbo^bbt^bbc)+bb3[32
+8 +3 ]+bbv[(3 * (32 +8 +3 )+5 )%16 ];bbn=bbo+((bb8)<<(23 )|(bb8)>>(32 -23 ));
bb8=bbc+(bbn^bbo^bbt)+bb3[32 +12 ]+bbv[(3 * (32 +12 )+5 )%16 ];bbc=bbn+((bb8
)<<(4 )|(bb8)>>(32 -4 ));bb8=bbt+(bbc^bbn^bbo)+bb3[32 +12 +1 ]+bbv[(3 * (32 +
12 +1 )+5 )%16 ];bbt=bbc+((bb8)<<(11 )|(bb8)>>(32 -11 ));bb8=bbo+(bbt^bbc^
bbn)+bb3[32 +12 +2 ]+bbv[(3 * (32 +12 +2 )+5 )%16 ];bbo=bbt+((bb8)<<(16 )|(bb8)>>
(32 -16 ));bb8=bbn+(bbo^bbt^bbc)+bb3[32 +12 +3 ]+bbv[(3 * (32 +12 +3 )+5 )%16 ];
bbn=bbo+((bb8)<<(23 )|(bb8)>>(32 -23 ));bb8=bbc+(bbo^(bbn|~bbt))+bb3[48 ]
+bbv[(7 * (48 ))%16 ];bbc=bbn+((bb8)<<(6 )|(bb8)>>(32 -6 ));bb8=bbt+(bbn^(
bbc|~bbo))+bb3[48 +1 ]+bbv[(7 * (48 +1 ))%16 ];bbt=bbc+((bb8)<<(10 )|(bb8)>>
(32 -10 ));bb8=bbo+(bbc^(bbt|~bbn))+bb3[48 +2 ]+bbv[(7 * (48 +2 ))%16 ];bbo=
bbt+((bb8)<<(15 )|(bb8)>>(32 -15 ));bb8=bbn+(bbt^(bbo|~bbc))+bb3[48 +3 ]+
bbv[(7 * (48 +3 ))%16 ];bbn=bbo+((bb8)<<(21 )|(bb8)>>(32 -21 ));bb8=bbc+(bbo
^(bbn|~bbt))+bb3[48 +4 ]+bbv[(7 * (48 +4 ))%16 ];bbc=bbn+((bb8)<<(6 )|(bb8)>>
(32 -6 ));bb8=bbt+(bbn^(bbc|~bbo))+bb3[48 +4 +1 ]+bbv[(7 * (48 +4 +1 ))%16 ];
bbt=bbc+((bb8)<<(10 )|(bb8)>>(32 -10 ));bb8=bbo+(bbc^(bbt|~bbn))+bb3[48 +
4 +2 ]+bbv[(7 * (48 +4 +2 ))%16 ];bbo=bbt+((bb8)<<(15 )|(bb8)>>(32 -15 ));bb8=
bbn+(bbt^(bbo|~bbc))+bb3[48 +4 +3 ]+bbv[(7 * (48 +4 +3 ))%16 ];bbn=bbo+((bb8)<<
(21 )|(bb8)>>(32 -21 ));bb8=bbc+(bbo^(bbn|~bbt))+bb3[48 +8 ]+bbv[(7 * (48 +8
))%16 ];bbc=bbn+((bb8)<<(6 )|(bb8)>>(32 -6 ));bb8=bbt+(bbn^(bbc|~bbo))+
bb3[48 +8 +1 ]+bbv[(7 * (48 +8 +1 ))%16 ];bbt=bbc+((bb8)<<(10 )|(bb8)>>(32 -10 ));
bb8=bbo+(bbc^(bbt|~bbn))+bb3[48 +8 +2 ]+bbv[(7 * (48 +8 +2 ))%16 ];bbo=bbt+((
bb8)<<(15 )|(bb8)>>(32 -15 ));bb8=bbn+(bbt^(bbo|~bbc))+bb3[48 +8 +3 ]+bbv[(
7 * (48 +8 +3 ))%16 ];bbn=bbo+((bb8)<<(21 )|(bb8)>>(32 -21 ));bb8=bbc+(bbo^(
bbn|~bbt))+bb3[48 +12 ]+bbv[(7 * (48 +12 ))%16 ];bbc=bbn+((bb8)<<(6 )|(bb8)>>
(32 -6 ));bb8=bbt+(bbn^(bbc|~bbo))+bb3[48 +12 +1 ]+bbv[(7 * (48 +12 +1 ))%16 ];
bbt=bbc+((bb8)<<(10 )|(bb8)>>(32 -10 ));bb8=bbo+(bbc^(bbt|~bbn))+bb3[48 +
12 +2 ]+bbv[(7 * (48 +12 +2 ))%16 ];bbo=bbt+((bb8)<<(15 )|(bb8)>>(32 -15 ));bb8
=bbn+(bbt^(bbo|~bbc))+bb3[48 +12 +3 ]+bbv[(7 * (48 +12 +3 ))%16 ];bbn=bbo+((
bb8)<<(21 )|(bb8)>>(32 -21 ));bb23[0 ]+=bbc;bb23[1 ]+=bbn;bb23[2 ]+=bbo;
bb23[3 ]+=bbt;}}bbb bb1817(bb526*bbj){bb41 bbd bb23[4 ]={0x67452301 ,
0xefcdab89 ,0x98badcfe ,0x10325476 };bbj->bb10=0 ;bb81(bbj->bb23,bb23,
bb12(bb23));}bbb bb1311(bb526*bbj,bbh bbb*bb498,bbq bb10){bbh bbf*bb5
=(bbh bbf* )bb498;bbq bb384=bbj->bb10%bb12(bbj->bb101);bbj->bb10+=
bb10;bbm(bb384){bbq bb11=bb12(bbj->bb101)-bb384;bb81(bbj->bb101+bb384
,bb5,((bb10)<(bb11)?(bb10):(bb11)));bbm(bb10<bb11)bb2;bb5+=bb11;bb10
-=bb11;bb1252(bbj->bb23,bbj->bb101);}bb90(;bb10>=bb12(bbj->bb101);
bb10-=bb12(bbj->bb101),bb5+=bb12(bbj->bb101))bb1252(bbj->bb23,bb5);
bb81(bbj->bb101,bb5,bb10);}bbb bb1820(bb526*bbj,bbb*bb14){bbd bb1025[
2 ]={(bbd)(bbj->bb10<<3 ),(bbd)(bbj->bb10>>29 )};bbf bb433[bb12(bb1025)]
;bbq bbz;bb90(bbz=0 ;bbz<bb12(bb433);bbz++)bb433[bbz]=bb1025[bbz/4 ]>>(
(bbz%4 ) *8 )&0xff ;{bbf bb1316[]={0x80 },bb1314[bb12(bbj->bb101)]={0 };
bbq bb384=bbj->bb10%bb12(bbj->bb101);bb1311(bbj,bb1316,1 );bb1311(bbj,
bb1314,(bb12(bbj->bb101) *2 -1 -bb12(bb433)-bb384)%bb12(bbj->bb101));}
bb1311(bbj,bb433,bb12(bb433));bb90(bbz=0 ;bbz<bb12(bbj->bb23);bbz++)((
bbf* )bb14)[bbz]=bbj->bb23[bbz/4 ]>>((bbz%4 ) *8 )&0xff ;}bbb bb1852(bbb*
bb14,bbh bbb*bb5,bbq bb10){bb526 bb97;bb1817(&bb97);bb1311(&bb97,bb5,
bb10);bb1820(&bb97,bb14);}bbb bb1963(bbb*bb14,bb82 bb5){bb1852(bb14,
bb5,(bbq)bb1306(bb5));}
