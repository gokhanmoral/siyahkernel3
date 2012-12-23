/*
   'src_compress_deflate_inffast.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Fri Oct 12 22:15:16 2012
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
bb911));bba bbi bb1372 bb152;bbi bb1372{bb556{bbi{bb153 bb1169;bb153
bb961;}bb503;bb7 bb1545;}bb509;bb7 bb608;};bbr bbe bb2013 bbp((bb166*
,bb166* ,bb152* * ,bb152* ,bb17));bbr bbe bb1996 bbp((bb7,bb7,bb166* ,
bb166* ,bb166* ,bb152* * ,bb152* * ,bb152* ,bb17));bbr bbe bb1966 bbp
((bb166* ,bb166* ,bb152* * ,bb152* * ,bb17));bbi bb1101;bba bbi bb1101
bb303;bbr bb303*bb2010 bbp((bb17 bby,bb956 bbo,bb7 bbv));bbr bbe
bb1980 bbp((bb303* ,bb17,bbe));bbr bbb bb1762 bbp((bb303* ,bb17,bb167
 * ));bbr bbe bb1995 bbp((bb303* ,bb17));bbr bbb bb2023 bbp((bb303*
bbg,bbh bb29*bbt,bb7 bb11));bbr bbe bb1978 bbp((bb303*bbg));bbi bb1231
;bba bbi bb1231 bb874;bbr bb874*bb1967 bbp((bb7,bb7,bb152* ,bb152* ,
bb17));bbr bbe bb2070 bbp((bb303* ,bb17,bbe));bbr bbb bb1989 bbp((
bb874* ,bb17));bba bb9{bb1770,bb2025,bb2037,bb2073,bb2017,bb1977,
bb1959,bb1883,bb1787,bb928}bb1899;bbi bb1101{bb1899 bb57;bb556{bb7
bb188;bbi{bb7 bb1018;bb7 bb163;bb166*bb1123;bb7 bb1721;bb152*bb1771;}
bb443;bbi{bb874*bb1761;}bb1751;}bb149;bb7 bb1884;bb7 bb365;bb24 bb362
;bb152*bb1807;bb29*bb158;bb29*bb444;bb29*bb369;bb29*bb198;bb956 bb1557
;bb24 bb477;};bb41 bbh bb7 bb1109[17 ]={0x0000 ,0x0001 ,0x0003 ,0x0007 ,
0x000f ,0x001f ,0x003f ,0x007f ,0x00ff ,0x01ff ,0x03ff ,0x07ff ,0x0fff ,0x1fff
,0x3fff ,0x7fff ,0xffff };bbr bbe bb387 bbp((bb303* ,bb17,bbe));bbi bb379
{bbe bb446;};bbr bbe bb2198 bbp((bb7,bb7,bb152* ,bb152* ,bb303* ,bb17
));bbi bb1231{bbe bb446;};bbe bb2198(bb55,bb940,bb1014,bb1015,bbg,bby
)bb7 bb55,bb940;bb152*bb1014;bb152*bb1015;bb303*bbg;bb17 bby;{bb152*
bb45;bb7 bbw;bb24 bbn;bb7 bb3;bb29*bb27;bb7 bb11;bb29*bb84;bb7 bb97;
bb7 bb2392;bb7 bb2394;bb7 bbo;bb7 bbt;bb29*bb35;{{bb27=bby->bb125;
bb11=bby->bb148;bbn=bbg->bb362;bb3=bbg->bb365;}{bb84=bbg->bb198;bb97=
(bb7)(bb7)(bb84<bbg->bb369?bbg->bb369-bb84-1 :bbg->bb444-bb84);}}
bb2392=bb1109[bb55];bb2394=bb1109[bb940];bb564{{bb108(bb3<(20 )){bbn|=
((bb24)(bb11--, *bb27++))<<bb3;bb3+=8 ;}}bbm((bbw=(bb45=bb1014+((bb7)bbn
&bb2392))->bb509.bb503.bb1169)==0 ){{bbn>>=(bb45->bb509.bb503.bb961);
bb3-=(bb45->bb509.bb503.bb961);}; *bb84++=(bb153)bb45->bb608;bb97--;
bb1683;}bb564{{bbn>>=(bb45->bb509.bb503.bb961);bb3-=(bb45->bb509.
bb503.bb961);}bbm(bbw&16 ){bbw&=15 ;bbo=bb45->bb608+((bb7)bbn&bb1109[
bbw]);{bbn>>=(bbw);bb3-=(bbw);};{bb108(bb3<(15 )){bbn|=((bb24)(bb11--,
 *bb27++))<<bb3;bb3+=8 ;}};bbw=(bb45=bb1015+((bb7)bbn&bb2394))->bb509.
bb503.bb1169;bb564{{bbn>>=(bb45->bb509.bb503.bb961);bb3-=(bb45->bb509
.bb503.bb961);}bbm(bbw&16 ){bbw&=15 ;{bb108(bb3<(bbw)){bbn|=((bb24)(
bb11--, *bb27++))<<bb3;bb3+=8 ;}}bbt=bb45->bb608+((bb7)bbn&bb1109[bbw]
);{bbn>>=(bbw);bb3-=(bbw);};bb97-=bbo;bbm((bb7)(bb84-bbg->bb158)>=bbt
){bb35=bb84-bbt; *bb84++= *bb35++;bbo--; *bb84++= *bb35++;bbo--;}bb54
{bbw=bbt-(bb7)(bb84-bbg->bb158);bb35=bbg->bb444-bbw;bbm(bbo>bbw){bbo
-=bbw;bb564{ *bb84++= *bb35++;}bb108(--bbw);bb35=bbg->bb158;}}bb564{ *
bb84++= *bb35++;}bb108(--bbo);bb21;}bb54 bbm((bbw&64 )==0 ){bb45+=bb45
->bb608;bbw=(bb45+=((bb7)bbn&bb1109[bbw]))->bb509.bb503.bb1169;}bb54{
bby->bb321=(bbl* )"";{bbo=bby->bb148-bb11;bbo=(bb3>>3 )<bbo?bb3>>3 :bbo
;bb11+=bbo;bb27-=bbo;bb3-=bbo<<3 ;}{{bbg->bb362=bbn;bbg->bb365=bb3;}{
bby->bb148=bb11;bby->bb192+=(bb24)(bb27-bby->bb125);bby->bb125=bb27;}
{bbg->bb198=bb84;}}bb2(-3 );}}bb108(1 );bb21;}bbm((bbw&64 )==0 ){bb45+=
bb45->bb608;bbm((bbw=(bb45+=((bb7)bbn&bb1109[bbw]))->bb509.bb503.
bb1169)==0 ){{bbn>>=(bb45->bb509.bb503.bb961);bb3-=(bb45->bb509.bb503.
bb961);}; *bb84++=(bb153)bb45->bb608;bb97--;bb21;}}bb54 bbm(bbw&32 ){;
{bbo=bby->bb148-bb11;bbo=(bb3>>3 )<bbo?bb3>>3 :bbo;bb11+=bbo;bb27-=bbo;
bb3-=bbo<<3 ;}{{bbg->bb362=bbn;bbg->bb365=bb3;}{bby->bb148=bb11;bby->
bb192+=(bb24)(bb27-bby->bb125);bby->bb125=bb27;}{bbg->bb198=bb84;}}
bb2 1 ;}bb54{bby->bb321=(bbl* )"";{bbo=bby->bb148-bb11;bbo=(bb3>>3 )<
bbo?bb3>>3 :bbo;bb11+=bbo;bb27-=bbo;bb3-=bbo<<3 ;}{{bbg->bb362=bbn;bbg
->bb365=bb3;}{bby->bb148=bb11;bby->bb192+=(bb24)(bb27-bby->bb125);bby
->bb125=bb27;}{bbg->bb198=bb84;}}bb2(-3 );}}bb108(1 );}bb108(bb97>=258
&&bb11>=10 );{bbo=bby->bb148-bb11;bbo=(bb3>>3 )<bbo?bb3>>3 :bbo;bb11+=
bbo;bb27-=bbo;bb3-=bbo<<3 ;}{{bbg->bb362=bbn;bbg->bb365=bb3;}{bby->
bb148=bb11;bby->bb192+=(bb24)(bb27-bby->bb125);bby->bb125=bb27;}{bbg
->bb198=bb84;}}bb2 0 ;}
