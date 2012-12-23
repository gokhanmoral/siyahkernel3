/*
   'src_compress_deflate_zutil.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Fri Oct 12 22:15:16 2012
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
#if ( defined( _WIN32) || defined( __WIN32__)) && ! defined( WIN32)
#define WIN32
#endif
#if defined( __GNUC__) || defined( WIN32) || defined( bb1223) ||  \
defined( bb1200)
#ifndef bb397
#define bb397
#endif
#endif
#if defined( __MSDOS__) && ! defined( bb168)
#define bb168
#endif
#if defined( bb168) && ! defined( bb397)
#define bb502
#endif
#ifdef bb168
#define bb1052
#endif
#if ( defined( bb168) || defined( bb1191) || defined( WIN32)) && !  \
defined( bb136)
#define bb136
#endif
#if defined( __STDC__) || defined( __cplusplus) || defined( bb1212)
#ifndef bb136
#define bb136
#endif
#endif
#ifndef bb136
#ifndef bbh
#define bbh
#endif
#endif
#if defined( __BORLANDC__) && ( __BORLANDC__ < 0x500)
#define bb1143
#endif
#ifndef bb273
#ifdef bb502
#define bb273 8
#else
#define bb273 9
#endif
#endif
#ifndef bbp
#ifdef bb136
#define bbp( bb404) bb404
#else
#define bbp( bb404) ()
#endif
#endif
bba bbf bb153;bba bbs bbe bb7;bba bbs bb6 bb24;bba bb153 bb29;bba bbl
bb441;bba bbe bb1139;bba bb7 bb166;bba bb24 bb167;
#ifdef bb136
bba bbb*bb70;bba bbb*bb186;
#else
bba bb153*bb70;bba bb153*bb186;
#endif
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bb70( *bb505)bbp((bb70 bb110,bb7 bb495,bb7 bb47));bba bbb( *bb506
)bbp((bb70 bb110,bb70 bb1097));bbi bb379;bba bbi bb1187{bb29*bb125;
bb7 bb148;bb24 bb192;bb29*bb596;bb7 bb385;bb24 bb611;bbl*bb321;bbi
bb379*bb23;bb505 bb401;bb506 bb370;bb70 bb110;bbe bb977;bb24 bb368;
bb24 bb1154;}bb432;bba bb432*bb17;bbr bbh bbl*bb1159 bbp((bbb));bbr
bbe bb512 bbp((bb17 bb16,bbe bb175));bbr bbe bb946 bbp((bb17 bb16));
bbr bbe bb1050 bbp((bb17 bb16,bbe bb175));bbr bbe bb950 bbp((bb17 bb16
));bbr bbe bb1177 bbp((bb17 bb16,bbh bb29*bb424,bb7 bb436));bbr bbe
bb1150 bbp((bb17 bb131,bb17 bb182));bbr bbe bb1053 bbp((bb17 bb16));
bbr bbe bb1175 bbp((bb17 bb16,bbe bb123,bbe bb300));bbr bbe bb1174 bbp
((bb17 bb16,bbh bb29*bb424,bb7 bb436));bbr bbe bb1148 bbp((bb17 bb16));
bbr bbe bb1008 bbp((bb17 bb16));bbr bbe bb1149 bbp((bb29*bb131,bb167*
bb313,bbh bb29*bb182,bb24 bb325));bbr bbe bb1115 bbp((bb29*bb131,
bb167*bb313,bbh bb29*bb182,bb24 bb325,bbe bb123));bbr bbe bb1161 bbp(
(bb29*bb131,bb167*bb313,bbh bb29*bb182,bb24 bb325));bba bb186 bb33;
bbr bb33 bb1208 bbp((bbh bbl*bb1069,bbh bbl*bb57));bbr bb33 bb1209 bbp
((bbe bb472,bbh bbl*bb57));bbr bbe bb1226 bbp((bb33 bb25,bbe bb123,
bbe bb300));bbr bbe bb1184 bbp((bb33 bb25,bb186 bb39,bbs bb22));bbr
bbe bb1188 bbp((bb33 bb25,bbh bb186 bb39,bbs bb22));bbr bbe bb1197 bbp
((bb33 bb25,bbh bbl*bb1228,...));bbr bbe bb1190 bbp((bb33 bb25,bbh bbl
 *bbg));bbr bbl*bb1181 bbp((bb33 bb25,bbl*bb39,bbe bb22));bbr bbe
bb1196 bbp((bb33 bb25,bbe bbo));bbr bbe bb1182 bbp((bb33 bb25));bbr
bbe bb1216 bbp((bb33 bb25,bbe bb175));bbr bb6 bb1193 bbp((bb33 bb25,
bb6 bb92,bbe bb1204));bbr bbe bb1180 bbp((bb33 bb25));bbr bb6 bb1202
bbp((bb33 bb25));bbr bbe bb1195 bbp((bb33 bb25));bbr bbe bb1205 bbp((
bb33 bb25));bbr bbh bbl*bb1186 bbp((bb33 bb25,bbe*bb1179));bbr bb24
bb985 bbp((bb24 bb368,bbh bb29*bb39,bb7 bb22));bbr bb24 bb1167 bbp((
bb24 bb377,bbh bb29*bb39,bb7 bb22));bbr bbe bb1110 bbp((bb17 bb16,bbe
bb123,bbh bbl*bb190,bbe bb195));bbr bbe bb1122 bbp((bb17 bb16,bbh bbl
 *bb190,bbe bb195));bbr bbe bb1042 bbp((bb17 bb16,bbe bb123,bbe bb566
,bbe bb454,bbe bb960,bbe bb300,bbh bbl*bb190,bbe bb195));bbr bbe
bb1079 bbp((bb17 bb16,bbe bb454,bbh bbl*bb190,bbe bb195));bbr bbh bbl
 *bb1152 bbp((bbe bb18));bbr bbe bb1156 bbp((bb17 bby));bbr bbh bb167
 *bb1162 bbp((bbb));
#ifdef __cplusplus
}
#endif
#define bb962 1
#ifdef bb136
#if defined( bb1741)
#else
#endif
#endif
bba bbs bbl bb155;bba bb155 bb1207;bba bbs bb135 bb126;bba bb126 bb501
;bba bbs bb6 bb395;bbr bbh bbl*bb1066[10 ];
#if bb273 >= 8
#define bb693 8
#else
#define bb693 bb273
#endif
#ifdef bb168
#define bb419 0x00
#if defined( __TURBOC__) || defined( __BORLANDC__)
#if( __STDC__ == 1) && ( defined( bb1793) || defined( bb1773))
bbb bb954 bb1326(bbb*bb101);bbb*bb954 bb1341(bbs bb6 bb1740);
#else
#include"uncobf.h"
#include<alloc.h>
#include"cobf.h"
#endif
#else
#include"uncobf.h"
#include<malloc.h>
#include"cobf.h"
#endif
#endif
#ifdef WIN32
#define bb419 0x0b
#endif
#if ( defined( _MSC_VER) && ( _MSC_VER > 600))
#define bb1753( bb472, bb129) bb1790( bb472, bb129)
#endif
#ifndef bb419
#define bb419 0x03
#endif
#if defined( bb1547) && ! defined( _MSC_VER) && ! defined( bb1776)
#define bb962
#endif
bba bb24( *bb956)bbp((bb24 bb477,bbh bb29*bb39,bb7 bb22));bb70 bb1166
bbp((bb70 bb110,bbs bb495,bbs bb47));bbb bb1163 bbp((bb70 bb110,bb70
bb911));bbh bbl*bb1066[10 ]={"\x6e\x65\x65\x64\x20\x64\x69\x63\x74\x69"
"\x6f\x6e\x61\x72\x79","\x73\x74\x72\x65\x61\x6d\x20\x65\x6e\x64","",""
"\x66\x69\x6c\x65\x20\x65\x72\x72\x6f\x72","\x73\x74\x72\x65\x61\x6d"
"\x20\x65\x72\x72\x6f\x72","\x64\x61\x74\x61\x20\x65\x72\x72\x6f\x72"
,"\x69\x6e\x73\x75\x66\x66\x69\x63\x69\x65\x6e\x74\x20\x6d\x65\x6d"
"\x6f\x72\x79","\x62\x75\x66\x66\x65\x72\x20\x65\x72\x72\x6f\x72",""
"\x69\x6e\x63\x6f\x6d\x70\x61\x74\x69\x62\x6c\x65\x20\x76\x65\x72\x73"
"\x69\x6f\x6e",""};bbi bb379{bbe bb446;};
#ifndef bb136
bbr bbb bb2640 bbp((bbe));
#endif
bbh bbl*bb1159(){bb2"\x31\x2e\x31\x2e\x33";}bbh bbl*bb1152(bb18)bbe
bb18;{bb2 bb1066[2 -(bb18)];}
#ifdef __TURBOC__
#if ( defined( __BORLANDC__) || ! defined( bb1547)) && ! defined(  \
bb397)
#define bb2318
#define bb2399 10
bb41 bbe bb2014=0 ;bba bbi bb2643{bb70 bb2372;bb70 bb2400;}bb2508;bb41
bb2508 bb1018[bb2399];bb70 bb1166(bb70 bb110,bbs bb495,bbs bb47){bb70
bb39=bb110;bb395 bb2259=(bb395)bb495*bb47;bbm(bb2259<65520L ){bb39=
bb1341(bb2259);bbm( * (bb126* )&bb39!=0 )bb2 bb39;}bb54{bb39=bb1341(
bb2259+16L );}bbm(bb39==bb91||bb2014>=bb2399)bb2 bb91;bb1018[bb2014].
bb2372=bb39; * ((bb126* )&bb39+1 )+=((bb126)((bb155* )bb39-0 )+15 )>>4 ; *
(bb126* )&bb39=0 ;bb1018[bb2014++].bb2400=bb39;bb2 bb39;}bbb bb1163(
bb70 bb110,bb70 bb911){bbe bb11;bbm( * (bb126* )&bb911!=0 ){bb1326(
bb911);bb2;}bb90(bb11=0 ;bb11<bb2014;bb11++){bbm(bb911!=bb1018[bb11].
bb2400)bb1683;bb1326(bb1018[bb11].bb2372);bb108(++bb11<bb2014){bb1018
[bb11-1 ]=bb1018[bb11];}bb2014--;bb2;}bb911=bb110;;}
#endif
#endif
#if defined( bb2637) && ! defined( bb397)
#define bb2318
#if (! defined( _MSC_VER) || ( _MSC_VER <= 600))
#define bb2444 bb2619
#define bb2556 bb2646
#endif
bb70 bb1166(bb70 bb110,bbs bb495,bbs bb47){bbm(bb110)bb110=0 ;bb2
bb2444((bb6)bb495,bb47);}bbb bb1163(bb70 bb110,bb70 bb911){bbm(bb110)bb110
=0 ;bb2556(bb911);}
#endif
#ifndef bb2318
#ifndef bb136
bbr bb186 bb137 bbp((bb7 bb495,bb7 bb47));bbr bbb bb105 bbp((bb70
bb911));
#endif
bb70 bb1166(bb110,bb495,bb47)bb70 bb110;bbs bb495;bbs bb47;{(bbb)bb110
;(bbb)bb495;(bbb)bb47;;bb2 bb91;}bbb bb1163(bb110,bb911)bb70 bb110;
bb70 bb911;{(bbb)bb110;(bbb)bb911;;bb2;}
#endif
