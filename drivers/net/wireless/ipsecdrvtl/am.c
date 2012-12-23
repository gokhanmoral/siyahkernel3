/*
   'ripemd.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Fri Oct 12 22:15:16 2012
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
bba bbi{bbd bb10;bbd bb23[5 ];bbf bb101[64 ];}bb523;bbb bb1802(bb523*
bbj);bbb bb1295(bb523*bbj,bbh bbb*bb498,bbq bb10);bbb bb1796(bb523*
bbj,bbb*bb14);bbb bb1922(bbb*bb14,bbh bbb*bb5,bbq bb10);bbb bb1972(
bbb*bb14,bb82 bb5);
#ifdef __cplusplus
}
#endif
bb41 bbb bb1252(bbd bb23[5 ],bbh bbf bb95[64 ]){bb31(bb12(bbe)>=4 );{bbd
bb65,bb61,bb69,bb55,bb64,bb66,bb62,bb49,bb67,bb68;bbd bbv[16 ],bbz;
bb65=bb61=bb23[0 ];bb69=bb55=bb23[1 ];bb64=bb66=bb23[2 ];bb62=bb49=bb23[
3 ];bb67=bb68=bb23[4 ];bb90(bbz=0 ;bbz<16 ;bbz++,bb95+=4 )bbv[bbz]=(bb95[0
]|bb95[1 ]<<8 |bb95[2 ]<<16 |bb95[3 ]<<24 );bb61+=(bb55^bb66^bb49)+bbv[0 ];
bb61=((bb61)<<(11 )|(bb61)>>(32 -11 ))+bb68;bb66=((bb66)<<(10 )|(bb66)>>(
32 -10 ));bb68+=(bb61^bb55^bb66)+bbv[1 ];bb68=((bb68)<<(14 )|(bb68)>>(32 -
14 ))+bb49;bb55=((bb55)<<(10 )|(bb55)>>(32 -10 ));bb49+=(bb68^bb61^bb55)+
bbv[2 ];bb49=((bb49)<<(15 )|(bb49)>>(32 -15 ))+bb66;bb61=((bb61)<<(10 )|(
bb61)>>(32 -10 ));bb66+=(bb49^bb68^bb61)+bbv[3 ];bb66=((bb66)<<(12 )|(
bb66)>>(32 -12 ))+bb55;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb55+=(bb66^
bb49^bb68)+bbv[4 ];bb55=((bb55)<<(5 )|(bb55)>>(32 -5 ))+bb61;bb49=((bb49)<<
(10 )|(bb49)>>(32 -10 ));bb61+=(bb55^bb66^bb49)+bbv[5 ];bb61=((bb61)<<(8 )|
(bb61)>>(32 -8 ))+bb68;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));bb68+=(bb61^
bb55^bb66)+bbv[6 ];bb68=((bb68)<<(7 )|(bb68)>>(32 -7 ))+bb49;bb55=((bb55)<<
(10 )|(bb55)>>(32 -10 ));bb49+=(bb68^bb61^bb55)+bbv[7 ];bb49=((bb49)<<(9 )|
(bb49)>>(32 -9 ))+bb66;bb61=((bb61)<<(10 )|(bb61)>>(32 -10 ));bb66+=(bb49^
bb68^bb61)+bbv[8 ];bb66=((bb66)<<(11 )|(bb66)>>(32 -11 ))+bb55;bb68=((
bb68)<<(10 )|(bb68)>>(32 -10 ));bb55+=(bb66^bb49^bb68)+bbv[9 ];bb55=((
bb55)<<(13 )|(bb55)>>(32 -13 ))+bb61;bb49=((bb49)<<(10 )|(bb49)>>(32 -10 ));
bb61+=(bb55^bb66^bb49)+bbv[10 ];bb61=((bb61)<<(14 )|(bb61)>>(32 -14 ))+
bb68;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));bb68+=(bb61^bb55^bb66)+bbv[
11 ];bb68=((bb68)<<(15 )|(bb68)>>(32 -15 ))+bb49;bb55=((bb55)<<(10 )|(bb55
)>>(32 -10 ));bb49+=(bb68^bb61^bb55)+bbv[12 ];bb49=((bb49)<<(6 )|(bb49)>>
(32 -6 ))+bb66;bb61=((bb61)<<(10 )|(bb61)>>(32 -10 ));bb66+=(bb49^bb68^
bb61)+bbv[13 ];bb66=((bb66)<<(7 )|(bb66)>>(32 -7 ))+bb55;bb68=((bb68)<<(
10 )|(bb68)>>(32 -10 ));bb55+=(bb66^bb49^bb68)+bbv[14 ];bb55=((bb55)<<(9 )|
(bb55)>>(32 -9 ))+bb61;bb49=((bb49)<<(10 )|(bb49)>>(32 -10 ));bb61+=(bb55^
bb66^bb49)+bbv[15 ];bb61=((bb61)<<(8 )|(bb61)>>(32 -8 ))+bb68;bb66=((bb66
)<<(10 )|(bb66)>>(32 -10 ));bb68+=(bb61&bb55|~bb61&bb66)+0x5a827999 +bbv[
7 ];bb68=((bb68)<<(7 )|(bb68)>>(32 -7 ))+bb49;bb55=((bb55)<<(10 )|(bb55)>>
(32 -10 ));bb49+=(bb68&bb61|~bb68&bb55)+0x5a827999 +bbv[4 ];bb49=((bb49)<<
(6 )|(bb49)>>(32 -6 ))+bb66;bb61=((bb61)<<(10 )|(bb61)>>(32 -10 ));bb66+=(
bb49&bb68|~bb49&bb61)+0x5a827999 +bbv[13 ];bb66=((bb66)<<(8 )|(bb66)>>(
32 -8 ))+bb55;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb55+=(bb66&bb49|~
bb66&bb68)+0x5a827999 +bbv[1 ];bb55=((bb55)<<(13 )|(bb55)>>(32 -13 ))+bb61
;bb49=((bb49)<<(10 )|(bb49)>>(32 -10 ));bb61+=(bb55&bb66|~bb55&bb49)+
0x5a827999 +bbv[10 ];bb61=((bb61)<<(11 )|(bb61)>>(32 -11 ))+bb68;bb66=((
bb66)<<(10 )|(bb66)>>(32 -10 ));bb68+=(bb61&bb55|~bb61&bb66)+0x5a827999 +
bbv[6 ];bb68=((bb68)<<(9 )|(bb68)>>(32 -9 ))+bb49;bb55=((bb55)<<(10 )|(
bb55)>>(32 -10 ));bb49+=(bb68&bb61|~bb68&bb55)+0x5a827999 +bbv[15 ];bb49=
((bb49)<<(7 )|(bb49)>>(32 -7 ))+bb66;bb61=((bb61)<<(10 )|(bb61)>>(32 -10 ));
bb66+=(bb49&bb68|~bb49&bb61)+0x5a827999 +bbv[3 ];bb66=((bb66)<<(15 )|(
bb66)>>(32 -15 ))+bb55;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb55+=(bb66&
bb49|~bb66&bb68)+0x5a827999 +bbv[12 ];bb55=((bb55)<<(7 )|(bb55)>>(32 -7 ))+
bb61;bb49=((bb49)<<(10 )|(bb49)>>(32 -10 ));bb61+=(bb55&bb66|~bb55&bb49)+
0x5a827999 +bbv[0 ];bb61=((bb61)<<(12 )|(bb61)>>(32 -12 ))+bb68;bb66=((
bb66)<<(10 )|(bb66)>>(32 -10 ));bb68+=(bb61&bb55|~bb61&bb66)+0x5a827999 +
bbv[9 ];bb68=((bb68)<<(15 )|(bb68)>>(32 -15 ))+bb49;bb55=((bb55)<<(10 )|(
bb55)>>(32 -10 ));bb49+=(bb68&bb61|~bb68&bb55)+0x5a827999 +bbv[5 ];bb49=(
(bb49)<<(9 )|(bb49)>>(32 -9 ))+bb66;bb61=((bb61)<<(10 )|(bb61)>>(32 -10 ));
bb66+=(bb49&bb68|~bb49&bb61)+0x5a827999 +bbv[2 ];bb66=((bb66)<<(11 )|(
bb66)>>(32 -11 ))+bb55;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb55+=(bb66&
bb49|~bb66&bb68)+0x5a827999 +bbv[14 ];bb55=((bb55)<<(7 )|(bb55)>>(32 -7 ))+
bb61;bb49=((bb49)<<(10 )|(bb49)>>(32 -10 ));bb61+=(bb55&bb66|~bb55&bb49)+
0x5a827999 +bbv[11 ];bb61=((bb61)<<(13 )|(bb61)>>(32 -13 ))+bb68;bb66=((
bb66)<<(10 )|(bb66)>>(32 -10 ));bb68+=(bb61&bb55|~bb61&bb66)+0x5a827999 +
bbv[8 ];bb68=((bb68)<<(12 )|(bb68)>>(32 -12 ))+bb49;bb55=((bb55)<<(10 )|(
bb55)>>(32 -10 ));bb49+=((bb68|~bb61)^bb55)+0x6ed9eba1 +bbv[3 ];bb49=((
bb49)<<(11 )|(bb49)>>(32 -11 ))+bb66;bb61=((bb61)<<(10 )|(bb61)>>(32 -10 ));
bb66+=((bb49|~bb68)^bb61)+0x6ed9eba1 +bbv[10 ];bb66=((bb66)<<(13 )|(bb66
)>>(32 -13 ))+bb55;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb55+=((bb66|~
bb49)^bb68)+0x6ed9eba1 +bbv[14 ];bb55=((bb55)<<(6 )|(bb55)>>(32 -6 ))+bb61
;bb49=((bb49)<<(10 )|(bb49)>>(32 -10 ));bb61+=((bb55|~bb66)^bb49)+
0x6ed9eba1 +bbv[4 ];bb61=((bb61)<<(7 )|(bb61)>>(32 -7 ))+bb68;bb66=((bb66)<<
(10 )|(bb66)>>(32 -10 ));bb68+=((bb61|~bb55)^bb66)+0x6ed9eba1 +bbv[9 ];
bb68=((bb68)<<(14 )|(bb68)>>(32 -14 ))+bb49;bb55=((bb55)<<(10 )|(bb55)>>(
32 -10 ));bb49+=((bb68|~bb61)^bb55)+0x6ed9eba1 +bbv[15 ];bb49=((bb49)<<(9
)|(bb49)>>(32 -9 ))+bb66;bb61=((bb61)<<(10 )|(bb61)>>(32 -10 ));bb66+=((
bb49|~bb68)^bb61)+0x6ed9eba1 +bbv[8 ];bb66=((bb66)<<(13 )|(bb66)>>(32 -13
))+bb55;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb55+=((bb66|~bb49)^bb68)+
0x6ed9eba1 +bbv[1 ];bb55=((bb55)<<(15 )|(bb55)>>(32 -15 ))+bb61;bb49=((
bb49)<<(10 )|(bb49)>>(32 -10 ));bb61+=((bb55|~bb66)^bb49)+0x6ed9eba1 +bbv
[2 ];bb61=((bb61)<<(14 )|(bb61)>>(32 -14 ))+bb68;bb66=((bb66)<<(10 )|(bb66
)>>(32 -10 ));bb68+=((bb61|~bb55)^bb66)+0x6ed9eba1 +bbv[7 ];bb68=((bb68)<<
(8 )|(bb68)>>(32 -8 ))+bb49;bb55=((bb55)<<(10 )|(bb55)>>(32 -10 ));bb49+=((
bb68|~bb61)^bb55)+0x6ed9eba1 +bbv[0 ];bb49=((bb49)<<(13 )|(bb49)>>(32 -13
))+bb66;bb61=((bb61)<<(10 )|(bb61)>>(32 -10 ));bb66+=((bb49|~bb68)^bb61)+
0x6ed9eba1 +bbv[6 ];bb66=((bb66)<<(6 )|(bb66)>>(32 -6 ))+bb55;bb68=((bb68)<<
(10 )|(bb68)>>(32 -10 ));bb55+=((bb66|~bb49)^bb68)+0x6ed9eba1 +bbv[13 ];
bb55=((bb55)<<(5 )|(bb55)>>(32 -5 ))+bb61;bb49=((bb49)<<(10 )|(bb49)>>(32
-10 ));bb61+=((bb55|~bb66)^bb49)+0x6ed9eba1 +bbv[11 ];bb61=((bb61)<<(12 )|
(bb61)>>(32 -12 ))+bb68;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));bb68+=((
bb61|~bb55)^bb66)+0x6ed9eba1 +bbv[5 ];bb68=((bb68)<<(7 )|(bb68)>>(32 -7 ))+
bb49;bb55=((bb55)<<(10 )|(bb55)>>(32 -10 ));bb49+=((bb68|~bb61)^bb55)+
0x6ed9eba1 +bbv[12 ];bb49=((bb49)<<(5 )|(bb49)>>(32 -5 ))+bb66;bb61=((bb61
)<<(10 )|(bb61)>>(32 -10 ));bb66+=(bb49&bb61|bb68&~bb61)+0x8f1bbcdc +bbv[
1 ];bb66=((bb66)<<(11 )|(bb66)>>(32 -11 ))+bb55;bb68=((bb68)<<(10 )|(bb68)>>
(32 -10 ));bb55+=(bb66&bb68|bb49&~bb68)+0x8f1bbcdc +bbv[9 ];bb55=((bb55)<<
(12 )|(bb55)>>(32 -12 ))+bb61;bb49=((bb49)<<(10 )|(bb49)>>(32 -10 ));bb61+=
(bb55&bb49|bb66&~bb49)+0x8f1bbcdc +bbv[11 ];bb61=((bb61)<<(14 )|(bb61)>>
(32 -14 ))+bb68;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));bb68+=(bb61&bb66|
bb55&~bb66)+0x8f1bbcdc +bbv[10 ];bb68=((bb68)<<(15 )|(bb68)>>(32 -15 ))+
bb49;bb55=((bb55)<<(10 )|(bb55)>>(32 -10 ));bb49+=(bb68&bb55|bb61&~bb55)+
0x8f1bbcdc +bbv[0 ];bb49=((bb49)<<(14 )|(bb49)>>(32 -14 ))+bb66;bb61=((
bb61)<<(10 )|(bb61)>>(32 -10 ));bb66+=(bb49&bb61|bb68&~bb61)+0x8f1bbcdc +
bbv[8 ];bb66=((bb66)<<(15 )|(bb66)>>(32 -15 ))+bb55;bb68=((bb68)<<(10 )|(
bb68)>>(32 -10 ));bb55+=(bb66&bb68|bb49&~bb68)+0x8f1bbcdc +bbv[12 ];bb55=
((bb55)<<(9 )|(bb55)>>(32 -9 ))+bb61;bb49=((bb49)<<(10 )|(bb49)>>(32 -10 ));
bb61+=(bb55&bb49|bb66&~bb49)+0x8f1bbcdc +bbv[4 ];bb61=((bb61)<<(8 )|(
bb61)>>(32 -8 ))+bb68;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));bb68+=(bb61&
bb66|bb55&~bb66)+0x8f1bbcdc +bbv[13 ];bb68=((bb68)<<(9 )|(bb68)>>(32 -9 ))+
bb49;bb55=((bb55)<<(10 )|(bb55)>>(32 -10 ));bb49+=(bb68&bb55|bb61&~bb55)+
0x8f1bbcdc +bbv[3 ];bb49=((bb49)<<(14 )|(bb49)>>(32 -14 ))+bb66;bb61=((
bb61)<<(10 )|(bb61)>>(32 -10 ));bb66+=(bb49&bb61|bb68&~bb61)+0x8f1bbcdc +
bbv[7 ];bb66=((bb66)<<(5 )|(bb66)>>(32 -5 ))+bb55;bb68=((bb68)<<(10 )|(
bb68)>>(32 -10 ));bb55+=(bb66&bb68|bb49&~bb68)+0x8f1bbcdc +bbv[15 ];bb55=
((bb55)<<(6 )|(bb55)>>(32 -6 ))+bb61;bb49=((bb49)<<(10 )|(bb49)>>(32 -10 ));
bb61+=(bb55&bb49|bb66&~bb49)+0x8f1bbcdc +bbv[14 ];bb61=((bb61)<<(8 )|(
bb61)>>(32 -8 ))+bb68;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));bb68+=(bb61&
bb66|bb55&~bb66)+0x8f1bbcdc +bbv[5 ];bb68=((bb68)<<(6 )|(bb68)>>(32 -6 ))+
bb49;bb55=((bb55)<<(10 )|(bb55)>>(32 -10 ));bb49+=(bb68&bb55|bb61&~bb55)+
0x8f1bbcdc +bbv[6 ];bb49=((bb49)<<(5 )|(bb49)>>(32 -5 ))+bb66;bb61=((bb61)<<
(10 )|(bb61)>>(32 -10 ));bb66+=(bb49&bb61|bb68&~bb61)+0x8f1bbcdc +bbv[2 ];
bb66=((bb66)<<(12 )|(bb66)>>(32 -12 ))+bb55;bb68=((bb68)<<(10 )|(bb68)>>(
32 -10 ));bb55+=(bb66^(bb49|~bb68))+0xa953fd4e +bbv[4 ];bb55=((bb55)<<(9 )|
(bb55)>>(32 -9 ))+bb61;bb49=((bb49)<<(10 )|(bb49)>>(32 -10 ));bb61+=(bb55^
(bb66|~bb49))+0xa953fd4e +bbv[0 ];bb61=((bb61)<<(15 )|(bb61)>>(32 -15 ))+
bb68;bb66=((bb66)<<(10 )|(bb66)>>(32 -10 ));bb68+=(bb61^(bb55|~bb66))+
0xa953fd4e +bbv[5 ];bb68=((bb68)<<(5 )|(bb68)>>(32 -5 ))+bb49;bb55=((bb55)<<
(10 )|(bb55)>>(32 -10 ));bb49+=(bb68^(bb61|~bb55))+0xa953fd4e +bbv[9 ];
bb49=((bb49)<<(11 )|(bb49)>>(32 -11 ))+bb66;bb61=((bb61)<<(10 )|(bb61)>>(
32 -10 ));bb66+=(bb49^(bb68|~bb61))+0xa953fd4e +bbv[7 ];bb66=((bb66)<<(6 )|
(bb66)>>(32 -6 ))+bb55;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb55+=(bb66^
(bb49|~bb68))+0xa953fd4e +bbv[12 ];bb55=((bb55)<<(8 )|(bb55)>>(32 -8 ))+
bb61;bb49=((bb49)<<(10 )|(bb49)>>(32 -10 ));bb61+=(bb55^(bb66|~bb49))+
0xa953fd4e +bbv[2 ];bb61=((bb61)<<(13 )|(bb61)>>(32 -13 ))+bb68;bb66=((
bb66)<<(10 )|(bb66)>>(32 -10 ));bb68+=(bb61^(bb55|~bb66))+0xa953fd4e +bbv
[10 ];bb68=((bb68)<<(12 )|(bb68)>>(32 -12 ))+bb49;bb55=((bb55)<<(10 )|(
bb55)>>(32 -10 ));bb49+=(bb68^(bb61|~bb55))+0xa953fd4e +bbv[14 ];bb49=((
bb49)<<(5 )|(bb49)>>(32 -5 ))+bb66;bb61=((bb61)<<(10 )|(bb61)>>(32 -10 ));
bb66+=(bb49^(bb68|~bb61))+0xa953fd4e +bbv[1 ];bb66=((bb66)<<(12 )|(bb66)>>
(32 -12 ))+bb55;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb55+=(bb66^(bb49|~
bb68))+0xa953fd4e +bbv[3 ];bb55=((bb55)<<(13 )|(bb55)>>(32 -13 ))+bb61;
bb49=((bb49)<<(10 )|(bb49)>>(32 -10 ));bb61+=(bb55^(bb66|~bb49))+
0xa953fd4e +bbv[8 ];bb61=((bb61)<<(14 )|(bb61)>>(32 -14 ))+bb68;bb66=((
bb66)<<(10 )|(bb66)>>(32 -10 ));bb68+=(bb61^(bb55|~bb66))+0xa953fd4e +bbv
[11 ];bb68=((bb68)<<(11 )|(bb68)>>(32 -11 ))+bb49;bb55=((bb55)<<(10 )|(
bb55)>>(32 -10 ));bb49+=(bb68^(bb61|~bb55))+0xa953fd4e +bbv[6 ];bb49=((
bb49)<<(8 )|(bb49)>>(32 -8 ))+bb66;bb61=((bb61)<<(10 )|(bb61)>>(32 -10 ));
bb66+=(bb49^(bb68|~bb61))+0xa953fd4e +bbv[15 ];bb66=((bb66)<<(5 )|(bb66)>>
(32 -5 ))+bb55;bb68=((bb68)<<(10 )|(bb68)>>(32 -10 ));bb55+=(bb66^(bb49|~
bb68))+0xa953fd4e +bbv[13 ];bb55=((bb55)<<(6 )|(bb55)>>(32 -6 ))+bb61;bb49
=((bb49)<<(10 )|(bb49)>>(32 -10 ));bb65+=(bb69^(bb64|~bb62))+0x50a28be6 +
bbv[5 ];bb65=((bb65)<<(8 )|(bb65)>>(32 -8 ))+bb67;bb64=((bb64)<<(10 )|(
bb64)>>(32 -10 ));bb67+=(bb65^(bb69|~bb64))+0x50a28be6 +bbv[14 ];bb67=((
bb67)<<(9 )|(bb67)>>(32 -9 ))+bb62;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));
bb62+=(bb67^(bb65|~bb69))+0x50a28be6 +bbv[7 ];bb62=((bb62)<<(9 )|(bb62)>>
(32 -9 ))+bb64;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));bb64+=(bb62^(bb67|~
bb65))+0x50a28be6 +bbv[0 ];bb64=((bb64)<<(11 )|(bb64)>>(32 -11 ))+bb69;
bb67=((bb67)<<(10 )|(bb67)>>(32 -10 ));bb69+=(bb64^(bb62|~bb67))+
0x50a28be6 +bbv[9 ];bb69=((bb69)<<(13 )|(bb69)>>(32 -13 ))+bb65;bb62=((
bb62)<<(10 )|(bb62)>>(32 -10 ));bb65+=(bb69^(bb64|~bb62))+0x50a28be6 +bbv
[2 ];bb65=((bb65)<<(15 )|(bb65)>>(32 -15 ))+bb67;bb64=((bb64)<<(10 )|(bb64
)>>(32 -10 ));bb67+=(bb65^(bb69|~bb64))+0x50a28be6 +bbv[11 ];bb67=((bb67)<<
(15 )|(bb67)>>(32 -15 ))+bb62;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb62+=
(bb67^(bb65|~bb69))+0x50a28be6 +bbv[4 ];bb62=((bb62)<<(5 )|(bb62)>>(32 -5
))+bb64;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));bb64+=(bb62^(bb67|~bb65))+
0x50a28be6 +bbv[13 ];bb64=((bb64)<<(7 )|(bb64)>>(32 -7 ))+bb69;bb67=((bb67
)<<(10 )|(bb67)>>(32 -10 ));bb69+=(bb64^(bb62|~bb67))+0x50a28be6 +bbv[6 ];
bb69=((bb69)<<(7 )|(bb69)>>(32 -7 ))+bb65;bb62=((bb62)<<(10 )|(bb62)>>(32
-10 ));bb65+=(bb69^(bb64|~bb62))+0x50a28be6 +bbv[15 ];bb65=((bb65)<<(8 )|
(bb65)>>(32 -8 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));bb67+=(bb65^
(bb69|~bb64))+0x50a28be6 +bbv[8 ];bb67=((bb67)<<(11 )|(bb67)>>(32 -11 ))+
bb62;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb62+=(bb67^(bb65|~bb69))+
0x50a28be6 +bbv[1 ];bb62=((bb62)<<(14 )|(bb62)>>(32 -14 ))+bb64;bb65=((
bb65)<<(10 )|(bb65)>>(32 -10 ));bb64+=(bb62^(bb67|~bb65))+0x50a28be6 +bbv
[10 ];bb64=((bb64)<<(14 )|(bb64)>>(32 -14 ))+bb69;bb67=((bb67)<<(10 )|(
bb67)>>(32 -10 ));bb69+=(bb64^(bb62|~bb67))+0x50a28be6 +bbv[3 ];bb69=((
bb69)<<(12 )|(bb69)>>(32 -12 ))+bb65;bb62=((bb62)<<(10 )|(bb62)>>(32 -10 ));
bb65+=(bb69^(bb64|~bb62))+0x50a28be6 +bbv[12 ];bb65=((bb65)<<(6 )|(bb65)>>
(32 -6 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));bb67+=(bb65&bb64|
bb69&~bb64)+0x5c4dd124 +bbv[6 ];bb67=((bb67)<<(9 )|(bb67)>>(32 -9 ))+bb62;
bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb62+=(bb67&bb69|bb65&~bb69)+
0x5c4dd124 +bbv[11 ];bb62=((bb62)<<(13 )|(bb62)>>(32 -13 ))+bb64;bb65=((
bb65)<<(10 )|(bb65)>>(32 -10 ));bb64+=(bb62&bb65|bb67&~bb65)+0x5c4dd124 +
bbv[3 ];bb64=((bb64)<<(15 )|(bb64)>>(32 -15 ))+bb69;bb67=((bb67)<<(10 )|(
bb67)>>(32 -10 ));bb69+=(bb64&bb67|bb62&~bb67)+0x5c4dd124 +bbv[7 ];bb69=(
(bb69)<<(7 )|(bb69)>>(32 -7 ))+bb65;bb62=((bb62)<<(10 )|(bb62)>>(32 -10 ));
bb65+=(bb69&bb62|bb64&~bb62)+0x5c4dd124 +bbv[0 ];bb65=((bb65)<<(12 )|(
bb65)>>(32 -12 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));bb67+=(bb65&
bb64|bb69&~bb64)+0x5c4dd124 +bbv[13 ];bb67=((bb67)<<(8 )|(bb67)>>(32 -8 ))+
bb62;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb62+=(bb67&bb69|bb65&~bb69)+
0x5c4dd124 +bbv[5 ];bb62=((bb62)<<(9 )|(bb62)>>(32 -9 ))+bb64;bb65=((bb65)<<
(10 )|(bb65)>>(32 -10 ));bb64+=(bb62&bb65|bb67&~bb65)+0x5c4dd124 +bbv[10 ]
;bb64=((bb64)<<(11 )|(bb64)>>(32 -11 ))+bb69;bb67=((bb67)<<(10 )|(bb67)>>
(32 -10 ));bb69+=(bb64&bb67|bb62&~bb67)+0x5c4dd124 +bbv[14 ];bb69=((bb69)<<
(7 )|(bb69)>>(32 -7 ))+bb65;bb62=((bb62)<<(10 )|(bb62)>>(32 -10 ));bb65+=(
bb69&bb62|bb64&~bb62)+0x5c4dd124 +bbv[15 ];bb65=((bb65)<<(7 )|(bb65)>>(
32 -7 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));bb67+=(bb65&bb64|bb69
&~bb64)+0x5c4dd124 +bbv[8 ];bb67=((bb67)<<(12 )|(bb67)>>(32 -12 ))+bb62;
bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb62+=(bb67&bb69|bb65&~bb69)+
0x5c4dd124 +bbv[12 ];bb62=((bb62)<<(7 )|(bb62)>>(32 -7 ))+bb64;bb65=((bb65
)<<(10 )|(bb65)>>(32 -10 ));bb64+=(bb62&bb65|bb67&~bb65)+0x5c4dd124 +bbv[
4 ];bb64=((bb64)<<(6 )|(bb64)>>(32 -6 ))+bb69;bb67=((bb67)<<(10 )|(bb67)>>
(32 -10 ));bb69+=(bb64&bb67|bb62&~bb67)+0x5c4dd124 +bbv[9 ];bb69=((bb69)<<
(15 )|(bb69)>>(32 -15 ))+bb65;bb62=((bb62)<<(10 )|(bb62)>>(32 -10 ));bb65+=
(bb69&bb62|bb64&~bb62)+0x5c4dd124 +bbv[1 ];bb65=((bb65)<<(13 )|(bb65)>>(
32 -13 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));bb67+=(bb65&bb64|
bb69&~bb64)+0x5c4dd124 +bbv[2 ];bb67=((bb67)<<(11 )|(bb67)>>(32 -11 ))+
bb62;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb62+=((bb67|~bb65)^bb69)+
0x6d703ef3 +bbv[15 ];bb62=((bb62)<<(9 )|(bb62)>>(32 -9 ))+bb64;bb65=((bb65
)<<(10 )|(bb65)>>(32 -10 ));bb64+=((bb62|~bb67)^bb65)+0x6d703ef3 +bbv[5 ];
bb64=((bb64)<<(7 )|(bb64)>>(32 -7 ))+bb69;bb67=((bb67)<<(10 )|(bb67)>>(32
-10 ));bb69+=((bb64|~bb62)^bb67)+0x6d703ef3 +bbv[1 ];bb69=((bb69)<<(15 )|
(bb69)>>(32 -15 ))+bb65;bb62=((bb62)<<(10 )|(bb62)>>(32 -10 ));bb65+=((
bb69|~bb64)^bb62)+0x6d703ef3 +bbv[3 ];bb65=((bb65)<<(11 )|(bb65)>>(32 -11
))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));bb67+=((bb65|~bb69)^bb64)+
0x6d703ef3 +bbv[7 ];bb67=((bb67)<<(8 )|(bb67)>>(32 -8 ))+bb62;bb69=((bb69)<<
(10 )|(bb69)>>(32 -10 ));bb62+=((bb67|~bb65)^bb69)+0x6d703ef3 +bbv[14 ];
bb62=((bb62)<<(6 )|(bb62)>>(32 -6 ))+bb64;bb65=((bb65)<<(10 )|(bb65)>>(32
-10 ));bb64+=((bb62|~bb67)^bb65)+0x6d703ef3 +bbv[6 ];bb64=((bb64)<<(6 )|(
bb64)>>(32 -6 ))+bb69;bb67=((bb67)<<(10 )|(bb67)>>(32 -10 ));bb69+=((bb64|
~bb62)^bb67)+0x6d703ef3 +bbv[9 ];bb69=((bb69)<<(14 )|(bb69)>>(32 -14 ))+
bb65;bb62=((bb62)<<(10 )|(bb62)>>(32 -10 ));bb65+=((bb69|~bb64)^bb62)+
0x6d703ef3 +bbv[11 ];bb65=((bb65)<<(12 )|(bb65)>>(32 -12 ))+bb67;bb64=((
bb64)<<(10 )|(bb64)>>(32 -10 ));bb67+=((bb65|~bb69)^bb64)+0x6d703ef3 +bbv
[8 ];bb67=((bb67)<<(13 )|(bb67)>>(32 -13 ))+bb62;bb69=((bb69)<<(10 )|(bb69
)>>(32 -10 ));bb62+=((bb67|~bb65)^bb69)+0x6d703ef3 +bbv[12 ];bb62=((bb62)<<
(5 )|(bb62)>>(32 -5 ))+bb64;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));bb64+=((
bb62|~bb67)^bb65)+0x6d703ef3 +bbv[2 ];bb64=((bb64)<<(14 )|(bb64)>>(32 -14
))+bb69;bb67=((bb67)<<(10 )|(bb67)>>(32 -10 ));bb69+=((bb64|~bb62)^bb67)+
0x6d703ef3 +bbv[10 ];bb69=((bb69)<<(13 )|(bb69)>>(32 -13 ))+bb65;bb62=((
bb62)<<(10 )|(bb62)>>(32 -10 ));bb65+=((bb69|~bb64)^bb62)+0x6d703ef3 +bbv
[0 ];bb65=((bb65)<<(13 )|(bb65)>>(32 -13 ))+bb67;bb64=((bb64)<<(10 )|(bb64
)>>(32 -10 ));bb67+=((bb65|~bb69)^bb64)+0x6d703ef3 +bbv[4 ];bb67=((bb67)<<
(7 )|(bb67)>>(32 -7 ))+bb62;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb62+=((
bb67|~bb65)^bb69)+0x6d703ef3 +bbv[13 ];bb62=((bb62)<<(5 )|(bb62)>>(32 -5 ))+
bb64;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));bb64+=(bb62&bb67|~bb62&bb65)+
0x7a6d76e9 +bbv[8 ];bb64=((bb64)<<(15 )|(bb64)>>(32 -15 ))+bb69;bb67=((
bb67)<<(10 )|(bb67)>>(32 -10 ));bb69+=(bb64&bb62|~bb64&bb67)+0x7a6d76e9 +
bbv[6 ];bb69=((bb69)<<(5 )|(bb69)>>(32 -5 ))+bb65;bb62=((bb62)<<(10 )|(
bb62)>>(32 -10 ));bb65+=(bb69&bb64|~bb69&bb62)+0x7a6d76e9 +bbv[4 ];bb65=(
(bb65)<<(8 )|(bb65)>>(32 -8 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));
bb67+=(bb65&bb69|~bb65&bb64)+0x7a6d76e9 +bbv[1 ];bb67=((bb67)<<(11 )|(
bb67)>>(32 -11 ))+bb62;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb62+=(bb67&
bb65|~bb67&bb69)+0x7a6d76e9 +bbv[3 ];bb62=((bb62)<<(14 )|(bb62)>>(32 -14 ))+
bb64;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));bb64+=(bb62&bb67|~bb62&bb65)+
0x7a6d76e9 +bbv[11 ];bb64=((bb64)<<(14 )|(bb64)>>(32 -14 ))+bb69;bb67=((
bb67)<<(10 )|(bb67)>>(32 -10 ));bb69+=(bb64&bb62|~bb64&bb67)+0x7a6d76e9 +
bbv[15 ];bb69=((bb69)<<(6 )|(bb69)>>(32 -6 ))+bb65;bb62=((bb62)<<(10 )|(
bb62)>>(32 -10 ));bb65+=(bb69&bb64|~bb69&bb62)+0x7a6d76e9 +bbv[0 ];bb65=(
(bb65)<<(14 )|(bb65)>>(32 -14 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));
bb67+=(bb65&bb69|~bb65&bb64)+0x7a6d76e9 +bbv[5 ];bb67=((bb67)<<(6 )|(
bb67)>>(32 -6 ))+bb62;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb62+=(bb67&
bb65|~bb67&bb69)+0x7a6d76e9 +bbv[12 ];bb62=((bb62)<<(9 )|(bb62)>>(32 -9 ))+
bb64;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));bb64+=(bb62&bb67|~bb62&bb65)+
0x7a6d76e9 +bbv[2 ];bb64=((bb64)<<(12 )|(bb64)>>(32 -12 ))+bb69;bb67=((
bb67)<<(10 )|(bb67)>>(32 -10 ));bb69+=(bb64&bb62|~bb64&bb67)+0x7a6d76e9 +
bbv[13 ];bb69=((bb69)<<(9 )|(bb69)>>(32 -9 ))+bb65;bb62=((bb62)<<(10 )|(
bb62)>>(32 -10 ));bb65+=(bb69&bb64|~bb69&bb62)+0x7a6d76e9 +bbv[9 ];bb65=(
(bb65)<<(12 )|(bb65)>>(32 -12 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));
bb67+=(bb65&bb69|~bb65&bb64)+0x7a6d76e9 +bbv[7 ];bb67=((bb67)<<(5 )|(
bb67)>>(32 -5 ))+bb62;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb62+=(bb67&
bb65|~bb67&bb69)+0x7a6d76e9 +bbv[10 ];bb62=((bb62)<<(15 )|(bb62)>>(32 -15
))+bb64;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));bb64+=(bb62&bb67|~bb62&
bb65)+0x7a6d76e9 +bbv[14 ];bb64=((bb64)<<(8 )|(bb64)>>(32 -8 ))+bb69;bb67=
((bb67)<<(10 )|(bb67)>>(32 -10 ));bb69+=(bb64^bb62^bb67)+bbv[12 ];bb69=((
bb69)<<(8 )|(bb69)>>(32 -8 ))+bb65;bb62=((bb62)<<(10 )|(bb62)>>(32 -10 ));
bb65+=(bb69^bb64^bb62)+bbv[15 ];bb65=((bb65)<<(5 )|(bb65)>>(32 -5 ))+bb67
;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));bb67+=(bb65^bb69^bb64)+bbv[10 ];
bb67=((bb67)<<(12 )|(bb67)>>(32 -12 ))+bb62;bb69=((bb69)<<(10 )|(bb69)>>(
32 -10 ));bb62+=(bb67^bb65^bb69)+bbv[4 ];bb62=((bb62)<<(9 )|(bb62)>>(32 -9
))+bb64;bb65=((bb65)<<(10 )|(bb65)>>(32 -10 ));bb64+=(bb62^bb67^bb65)+
bbv[1 ];bb64=((bb64)<<(12 )|(bb64)>>(32 -12 ))+bb69;bb67=((bb67)<<(10 )|(
bb67)>>(32 -10 ));bb69+=(bb64^bb62^bb67)+bbv[5 ];bb69=((bb69)<<(5 )|(bb69
)>>(32 -5 ))+bb65;bb62=((bb62)<<(10 )|(bb62)>>(32 -10 ));bb65+=(bb69^bb64^
bb62)+bbv[8 ];bb65=((bb65)<<(14 )|(bb65)>>(32 -14 ))+bb67;bb64=((bb64)<<(
10 )|(bb64)>>(32 -10 ));bb67+=(bb65^bb69^bb64)+bbv[7 ];bb67=((bb67)<<(6 )|
(bb67)>>(32 -6 ))+bb62;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb62+=(bb67^
bb65^bb69)+bbv[6 ];bb62=((bb62)<<(8 )|(bb62)>>(32 -8 ))+bb64;bb65=((bb65)<<
(10 )|(bb65)>>(32 -10 ));bb64+=(bb62^bb67^bb65)+bbv[2 ];bb64=((bb64)<<(13
)|(bb64)>>(32 -13 ))+bb69;bb67=((bb67)<<(10 )|(bb67)>>(32 -10 ));bb69+=(
bb64^bb62^bb67)+bbv[13 ];bb69=((bb69)<<(6 )|(bb69)>>(32 -6 ))+bb65;bb62=(
(bb62)<<(10 )|(bb62)>>(32 -10 ));bb65+=(bb69^bb64^bb62)+bbv[14 ];bb65=((
bb65)<<(5 )|(bb65)>>(32 -5 ))+bb67;bb64=((bb64)<<(10 )|(bb64)>>(32 -10 ));
bb67+=(bb65^bb69^bb64)+bbv[0 ];bb67=((bb67)<<(15 )|(bb67)>>(32 -15 ))+
bb62;bb69=((bb69)<<(10 )|(bb69)>>(32 -10 ));bb62+=(bb67^bb65^bb69)+bbv[3
];bb62=((bb62)<<(13 )|(bb62)>>(32 -13 ))+bb64;bb65=((bb65)<<(10 )|(bb65)>>
(32 -10 ));bb64+=(bb62^bb67^bb65)+bbv[9 ];bb64=((bb64)<<(11 )|(bb64)>>(32
-11 ))+bb69;bb67=((bb67)<<(10 )|(bb67)>>(32 -10 ));bb69+=(bb64^bb62^bb67)+
bbv[11 ];bb69=((bb69)<<(11 )|(bb69)>>(32 -11 ))+bb65;bb62=((bb62)<<(10 )|(
bb62)>>(32 -10 ));bb62+=bb23[1 ]+bb66;bb23[1 ]=bb23[2 ]+bb49+bb67;bb23[2 ]=
bb23[3 ]+bb68+bb65;bb23[3 ]=bb23[4 ]+bb61+bb69;bb23[4 ]=bb23[0 ]+bb55+bb64
;bb23[0 ]=bb62;}}bbb bb1802(bb523*bbj){bb41 bbd bb23[5 ]={0x67452301 ,
0xefcdab89 ,0x98badcfe ,0x10325476 ,0xc3d2e1f0 };bbj->bb10=0 ;bb81(bbj->
bb23,bb23,bb12(bb23));}bbb bb1295(bb523*bbj,bbh bbb*bb498,bbq bb10){
bbh bbf*bb5=(bbh bbf* )bb498;bbq bb384=bbj->bb10%bb12(bbj->bb101);bbj
->bb10+=bb10;bbm(bb384){bbq bb11=bb12(bbj->bb101)-bb384;bb81(bbj->
bb101+bb384,bb5,((bb10)<(bb11)?(bb10):(bb11)));bbm(bb10<bb11)bb2;bb5
+=bb11;bb10-=bb11;bb1252(bbj->bb23,bbj->bb101);}bb90(;bb10>=bb12(bbj
->bb101);bb10-=bb12(bbj->bb101),bb5+=bb12(bbj->bb101))bb1252(bbj->
bb23,bb5);bb81(bbj->bb101,bb5,bb10);}bbb bb1796(bb523*bbj,bbb*bb14){
bbd bb1025[2 ]={(bbd)(bbj->bb10<<3 ),(bbd)(bbj->bb10>>29 )};bbf bb433[
bb12(bb1025)];bbq bbz;bb90(bbz=0 ;bbz<bb12(bb433);bbz++)bb433[bbz]=
bb1025[bbz/4 ]>>((bbz%4 ) *8 )&0xff ;{bbf bb1316[]={0x80 },bb1314[bb12(bbj
->bb101)]={0 };bbq bb384=bbj->bb10%bb12(bbj->bb101);bb1295(bbj,bb1316,
1 );bb1295(bbj,bb1314,(bb12(bbj->bb101) *2 -1 -bb12(bb433)-bb384)%bb12(
bbj->bb101));}bb1295(bbj,bb433,bb12(bb433));bb90(bbz=0 ;bbz<bb12(bbj->
bb23);bbz++)((bbf* )bb14)[bbz]=bbj->bb23[bbz/4 ]>>((bbz%4 ) *8 )&0xff ;}
bbb bb1922(bbb*bb14,bbh bbb*bb5,bbq bb10){bb523 bb97;bb1802(&bb97);
bb1295(&bb97,bb5,bb10);bb1796(&bb97,bb14);}bbb bb1972(bbb*bb14,bb82
bb5){bb1922(bb14,bb5,(bbq)bb1306(bb5));}
