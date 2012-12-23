/*
   'src_compress_deflate_deflate.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Fri Oct 12 22:15:16 2012
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
bb911));bba bbi bb1954{bb556{bb126 bb426;bb126 bb172;}bb268;bb556{
bb126 bb2161;bb126 bb22;}bb49;}bb460;bba bbi bb2275 bb2004;bba bbi
bb1940{bb460*bb1748;bbe bb511;bb2004*bb1719;}bb1734;bba bb126 bb1059;
bba bb1059 bb1352;bba bbs bb1318;bba bbi bb379{bb17 bb16;bbe bb359;
bb29*bb171;bb395 bb2038;bb29*bb1874;bbe bb184;bbe bb1345;bb153 bb977;
bb153 bb566;bbe bb1893;bb7 bb935;bb7 bb2160;bb7 bb1808;bb29*bb158;
bb395 bb2224;bb1352*bb966;bb1352*bb382;bb7 bb497;bb7 bb1327;bb7 bb2129
;bb7 bb1377;bb7 bb1608;bb6 bb429;bb7 bb953;bb1318 bb2295;bbe bb1939;
bb7 bb183;bb7 bb2022;bb7 bb462;bb7 bb1260;bb7 bb2173;bb7 bb2054;bbe
bb123;bbe bb300;bb7 bb2178;bbe bb1837;bbi bb1954 bb976[(2 * (256 +1 +29 )+
1 )];bbi bb1954 bb1678[2 *30 +1 ];bbi bb1954 bb522[2 *19 +1 ];bbi bb1940
bb1936;bbi bb1940 bb1863;bbi bb1940 bb2061;bb126 bb1192[15 +1 ];bbe
bb520[2 * (256 +1 +29 )+1 ];bbe bb1499;bbe bb1945;bb155 bb1237[2 * (256 +1 +
29 )+1 ];bb1207*bb1724;bb7 bb1121;bb7 bb605;bb501*bb1643;bb395 bb1904;
bb395 bb2117;bb7 bb2228;bbe bb1992;bb126 bb99;bbe bb79;}bb191;bbb
bb2206 bbp((bb191*bbg));bbe bb2386 bbp((bb191*bbg,bbs bb415,bbs bb1136
));bbb bb1616 bbp((bb191*bbg,bb441*bb39,bb395 bb1290,bbe bb1108));bbb
bb2253 bbp((bb191*bbg));bbb bb2141 bbp((bb191*bbg,bb441*bb39,bb395
bb1290,bbe bb1108));bb41 bbh bb460 bb1738[(256 +1 +29 )+2 ]={{{12 },{8 }},{
{140 },{8 }},{{76 },{8 }},{{204 },{8 }},{{44 },{8 }},{{172 },{8 }},{{108 },{8 }},
{{236 },{8 }},{{28 },{8 }},{{156 },{8 }},{{92 },{8 }},{{220 },{8 }},{{60 },{8 }},
{{188 },{8 }},{{124 },{8 }},{{252 },{8 }},{{2 },{8 }},{{130 },{8 }},{{66 },{8 }},
{{194 },{8 }},{{34 },{8 }},{{162 },{8 }},{{98 },{8 }},{{226 },{8 }},{{18 },{8 }},
{{146 },{8 }},{{82 },{8 }},{{210 },{8 }},{{50 },{8 }},{{178 },{8 }},{{114 },{8 }}
,{{242 },{8 }},{{10 },{8 }},{{138 },{8 }},{{74 },{8 }},{{202 },{8 }},{{42 },{8 }}
,{{170 },{8 }},{{106 },{8 }},{{234 },{8 }},{{26 },{8 }},{{154 },{8 }},{{90 },{8 }
},{{218 },{8 }},{{58 },{8 }},{{186 },{8 }},{{122 },{8 }},{{250 },{8 }},{{6 },{8 }
},{{134 },{8 }},{{70 },{8 }},{{198 },{8 }},{{38 },{8 }},{{166 },{8 }},{{102 },{8
}},{{230 },{8 }},{{22 },{8 }},{{150 },{8 }},{{86 },{8 }},{{214 },{8 }},{{54 },{8
}},{{182 },{8 }},{{118 },{8 }},{{246 },{8 }},{{14 },{8 }},{{142 },{8 }},{{78 },{
8 }},{{206 },{8 }},{{46 },{8 }},{{174 },{8 }},{{110 },{8 }},{{238 },{8 }},{{30 },
{8 }},{{158 },{8 }},{{94 },{8 }},{{222 },{8 }},{{62 },{8 }},{{190 },{8 }},{{126 }
,{8 }},{{254 },{8 }},{{1 },{8 }},{{129 },{8 }},{{65 },{8 }},{{193 },{8 }},{{33 },
{8 }},{{161 },{8 }},{{97 },{8 }},{{225 },{8 }},{{17 },{8 }},{{145 },{8 }},{{81 },
{8 }},{{209 },{8 }},{{49 },{8 }},{{177 },{8 }},{{113 },{8 }},{{241 },{8 }},{{9 },
{8 }},{{137 },{8 }},{{73 },{8 }},{{201 },{8 }},{{41 },{8 }},{{169 },{8 }},{{105 }
,{8 }},{{233 },{8 }},{{25 },{8 }},{{153 },{8 }},{{89 },{8 }},{{217 },{8 }},{{57 }
,{8 }},{{185 },{8 }},{{121 },{8 }},{{249 },{8 }},{{5 },{8 }},{{133 },{8 }},{{69 }
,{8 }},{{197 },{8 }},{{37 },{8 }},{{165 },{8 }},{{101 },{8 }},{{229 },{8 }},{{21
},{8 }},{{149 },{8 }},{{85 },{8 }},{{213 },{8 }},{{53 },{8 }},{{181 },{8 }},{{
117 },{8 }},{{245 },{8 }},{{13 },{8 }},{{141 },{8 }},{{77 },{8 }},{{205 },{8 }},{
{45 },{8 }},{{173 },{8 }},{{109 },{8 }},{{237 },{8 }},{{29 },{8 }},{{157 },{8 }},
{{93 },{8 }},{{221 },{8 }},{{61 },{8 }},{{189 },{8 }},{{125 },{8 }},{{253 },{8 }}
,{{19 },{9 }},{{275 },{9 }},{{147 },{9 }},{{403 },{9 }},{{83 },{9 }},{{339 },{9 }
},{{211 },{9 }},{{467 },{9 }},{{51 },{9 }},{{307 },{9 }},{{179 },{9 }},{{435 },{
9 }},{{115 },{9 }},{{371 },{9 }},{{243 },{9 }},{{499 },{9 }},{{11 },{9 }},{{267 }
,{9 }},{{139 },{9 }},{{395 },{9 }},{{75 },{9 }},{{331 },{9 }},{{203 },{9 }},{{
459 },{9 }},{{43 },{9 }},{{299 },{9 }},{{171 },{9 }},{{427 },{9 }},{{107 },{9 }},
{{363 },{9 }},{{235 },{9 }},{{491 },{9 }},{{27 },{9 }},{{283 },{9 }},{{155 },{9 }
},{{411 },{9 }},{{91 },{9 }},{{347 },{9 }},{{219 },{9 }},{{475 },{9 }},{{59 },{9
}},{{315 },{9 }},{{187 },{9 }},{{443 },{9 }},{{123 },{9 }},{{379 },{9 }},{{251 }
,{9 }},{{507 },{9 }},{{7 },{9 }},{{263 },{9 }},{{135 },{9 }},{{391 },{9 }},{{71 }
,{9 }},{{327 },{9 }},{{199 },{9 }},{{455 },{9 }},{{39 },{9 }},{{295 },{9 }},{{
167 },{9 }},{{423 },{9 }},{{103 },{9 }},{{359 },{9 }},{{231 },{9 }},{{487 },{9 }}
,{{23 },{9 }},{{279 },{9 }},{{151 },{9 }},{{407 },{9 }},{{87 },{9 }},{{343 },{9 }
},{{215 },{9 }},{{471 },{9 }},{{55 },{9 }},{{311 },{9 }},{{183 },{9 }},{{439 },{
9 }},{{119 },{9 }},{{375 },{9 }},{{247 },{9 }},{{503 },{9 }},{{15 },{9 }},{{271 }
,{9 }},{{143 },{9 }},{{399 },{9 }},{{79 },{9 }},{{335 },{9 }},{{207 },{9 }},{{
463 },{9 }},{{47 },{9 }},{{303 },{9 }},{{175 },{9 }},{{431 },{9 }},{{111 },{9 }},
{{367 },{9 }},{{239 },{9 }},{{495 },{9 }},{{31 },{9 }},{{287 },{9 }},{{159 },{9 }
},{{415 },{9 }},{{95 },{9 }},{{351 },{9 }},{{223 },{9 }},{{479 },{9 }},{{63 },{9
}},{{319 },{9 }},{{191 },{9 }},{{447 },{9 }},{{127 },{9 }},{{383 },{9 }},{{255 }
,{9 }},{{511 },{9 }},{{0 },{7 }},{{64 },{7 }},{{32 },{7 }},{{96 },{7 }},{{16 },{7
}},{{80 },{7 }},{{48 },{7 }},{{112 },{7 }},{{8 },{7 }},{{72 },{7 }},{{40 },{7 }},
{{104 },{7 }},{{24 },{7 }},{{88 },{7 }},{{56 },{7 }},{{120 },{7 }},{{4 },{7 }},{{
68 },{7 }},{{36 },{7 }},{{100 },{7 }},{{20 },{7 }},{{84 },{7 }},{{52 },{7 }},{{
116 },{7 }},{{3 },{8 }},{{131 },{8 }},{{67 },{8 }},{{195 },{8 }},{{35 },{8 }},{{
163 },{8 }},{{99 },{8 }},{{227 },{8 }}};bb41 bbh bb460 bb2232[30 ]={{{0 },{5 }
},{{16 },{5 }},{{8 },{5 }},{{24 },{5 }},{{4 },{5 }},{{20 },{5 }},{{12 },{5 }},{{
28 },{5 }},{{2 },{5 }},{{18 },{5 }},{{10 },{5 }},{{26 },{5 }},{{6 },{5 }},{{22 },{
5 }},{{14 },{5 }},{{30 },{5 }},{{1 },{5 }},{{17 },{5 }},{{9 },{5 }},{{25 },{5 }},{
{5 },{5 }},{{21 },{5 }},{{13 },{5 }},{{29 },{5 }},{{3 },{5 }},{{19 },{5 }},{{11 },
{5 }},{{27 },{5 }},{{7 },{5 }},{{23 },{5 }}};bb41 bbh bb155 bb1735[512 ]={0 ,1
,2 ,3 ,4 ,4 ,5 ,5 ,6 ,6 ,6 ,6 ,7 ,7 ,7 ,7 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,10 ,10 ,10
,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11
,11 ,11 ,11 ,11 ,11 ,11 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12
,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13
,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13
,13 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14
,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14
,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,15 ,15 ,15 ,15
,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15
,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15
,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,0 ,0 ,16 ,17 ,18 ,18 ,19 ,19 ,20 ,
20 ,20 ,20 ,21 ,21 ,21 ,21 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,
24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,
25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,
26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,27 ,27 ,27 ,27 ,27 ,
27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,
27 ,27 ,27 ,27 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,
28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,
28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,29 ,
29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,
29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,
29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 };bb41 bbh bb155
bb2050[258 -3 +1 ]={0 ,1 ,2 ,3 ,4 ,5 ,6 ,7 ,8 ,8 ,9 ,9 ,10 ,10 ,11 ,11 ,12 ,12 ,12 ,12 ,13 ,
13 ,13 ,13 ,14 ,14 ,14 ,14 ,15 ,15 ,15 ,15 ,16 ,16 ,16 ,16 ,16 ,16 ,16 ,16 ,17 ,17 ,17 ,17 ,
17 ,17 ,17 ,17 ,18 ,18 ,18 ,18 ,18 ,18 ,18 ,18 ,19 ,19 ,19 ,19 ,19 ,19 ,19 ,19 ,20 ,20 ,20 ,
20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,
21 ,21 ,21 ,21 ,21 ,21 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,23 ,
23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,
24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,
24 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,
25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,
26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,27 ,27 ,27 ,27 ,
27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,
27 ,27 ,27 ,27 ,28 };bb41 bbh bbe bb2331[29 ]={0 ,1 ,2 ,3 ,4 ,5 ,6 ,7 ,8 ,10 ,12 ,14 ,
16 ,20 ,24 ,28 ,32 ,40 ,48 ,56 ,64 ,80 ,96 ,112 ,128 ,160 ,192 ,224 ,0 };bb41 bbh bbe
bb2414[30 ]={0 ,1 ,2 ,3 ,4 ,6 ,8 ,12 ,16 ,24 ,32 ,48 ,64 ,96 ,128 ,192 ,256 ,384 ,512 ,
768 ,1024 ,1536 ,2048 ,3072 ,4096 ,6144 ,8192 ,12288 ,16384 ,24576 };bba bb9{
bb1282,bb2157,bb1756,bb2138}bb1824;bba bb1824( *bb2383)bbp((bb191*bbg
,bbe bb175));bb41 bbb bb2187 bbp((bb191*bbg));bb41 bb1824 bb2377 bbp(
(bb191*bbg,bbe bb175));bb41 bb1824 bb2159 bbp((bb191*bbg,bbe bb175));
bb41 bb1824 bb1947 bbp((bb191*bbg,bbe bb175));bb41 bbb bb2426 bbp((
bb191*bbg));bb41 bbb bb2007 bbp((bb191*bbg,bb7 bbn));bb41 bbb bb1266
bbp((bb17 bb16));bb41 bbe bb2431 bbp((bb17 bb16,bb29*bb39,bbs bb47));
bb41 bb7 bb2256 bbp((bb191*bbg,bb1318 bb1993));bba bbi bb2624{bb126
bb2428;bb126 bb2435;bb126 bb2380;bb126 bb2345;bb2383 bb2019;}bb2509;
bb41 bbh bb2509 bb1309[10 ]={{0 ,0 ,0 ,0 ,bb2377},{4 ,4 ,8 ,4 ,bb2159},{4 ,5 ,16
,8 ,bb2159},{4 ,6 ,32 ,32 ,bb2159},{4 ,4 ,16 ,16 ,bb1947},{8 ,16 ,32 ,32 ,bb1947},
{8 ,16 ,128 ,128 ,bb1947},{8 ,32 ,128 ,256 ,bb1947},{32 ,128 ,258 ,1024 ,bb1947},
{32 ,258 ,258 ,4096 ,bb1947}};bbi bb2275{bbe bb446;};bbe bb1110(bb16,
bb123,bb190,bb195)bb17 bb16;bbe bb123;bbh bbl*bb190;bbe bb195;{bb2
bb1042(bb16,bb123,8 ,15 ,bb693,0 ,bb190,bb195);}bbe bb1042(bb16,bb123,
bb566,bb454,bb960,bb300,bb190,bb195)bb17 bb16;bbe bb123;bbe bb566;bbe
bb454;bbe bb960;bbe bb300;bbh bbl*bb190;bbe bb195;{bb191*bbg;bbe
bb1345=0 ;bb41 bbh bbl*bb2440="\x31\x2e\x31\x2e\x33";bb501*bb1882;bbm(
bb190==0 ||bb190[0 ]!=bb2440[0 ]||bb195!=bb12(bb432)){bb2(-6 );}bbm(bb16
==0 )bb2(-2 );bb16->bb321=0 ;bbm(bb16->bb401==0 ){bb16->bb401=bb1166;bb16
->bb110=(bb70)0 ;}bbm(bb16->bb370==0 )bb16->bb370=bb1163;bbm(bb123==(-1
))bb123=6 ;bbm(bb454<0 ){bb1345=1 ;bb454=-bb454;}bbm(bb960<1 ||bb960>
bb273||bb566!=8 ||bb454<8 ||bb454>15 ||bb123<0 ||bb123>9 ||bb300<0 ||bb300>
2 ){bb2(-2 );}bbg=(bb191* )( * ((bb16)->bb401))((bb16)->bb110,(1 ),(bb12
(bb191)));bbm(bbg==0 )bb2(-4 );bb16->bb23=(bbi bb379* )bbg;bbg->bb16=
bb16;bbg->bb1345=bb1345;bbg->bb2160=bb454;bbg->bb935=1 <<bbg->bb2160;
bbg->bb1808=bbg->bb935-1 ;bbg->bb2129=bb960+7 ;bbg->bb1327=1 <<bbg->
bb2129;bbg->bb1377=bbg->bb1327-1 ;bbg->bb1608=((bbg->bb2129+3 -1 )/3 );
bbg->bb158=(bb29* )( * ((bb16)->bb401))((bb16)->bb110,(bbg->bb935),(2
 *bb12(bb153)));bbg->bb966=(bb1352* )( * ((bb16)->bb401))((bb16)->
bb110,(bbg->bb935),(bb12(bb1059)));bbg->bb382=(bb1352* )( * ((bb16)->
bb401))((bb16)->bb110,(bbg->bb1327),(bb12(bb1059)));bbg->bb1121=1 <<(
bb960+6 );bb1882=(bb501* )( * ((bb16)->bb401))((bb16)->bb110,(bbg->
bb1121),(bb12(bb126)+2 ));bbg->bb171=(bb1207* )bb1882;bbg->bb2038=(
bb395)bbg->bb1121* (bb12(bb126)+2L );bbm(bbg->bb158==0 ||bbg->bb966==0
||bbg->bb382==0 ||bbg->bb171==0 ){bb16->bb321=(bbl* )bb1066[2 -((-4 ))];
bb946(bb16);bb2(-4 );}bbg->bb1643=bb1882+bbg->bb1121/bb12(bb126);bbg->
bb1724=bbg->bb171+(1 +bb12(bb126)) *bbg->bb1121;bbg->bb123=bb123;bbg->
bb300=bb300;bbg->bb566=(bb153)bb566;bb2 bb1053(bb16);}bbe bb1177(bb16
,bb424,bb436)bb17 bb16;bbh bb29*bb424;bb7 bb436;{bb191*bbg;bb7 bb469=
bb436;bb7 bb11;bb1318 bb1130=0 ;bbm(bb16==0 ||bb16->bb23==0 ||bb424==0 ||
bb16->bb23->bb359!=42 )bb2(-2 );bbg=bb16->bb23;bb16->bb368=bb985(bb16->
bb368,bb424,bb436);bbm(bb469<3 )bb2 0 ;bbm(bb469>((bbg)->bb935-(258 +3 +1
))){bb469=((bbg)->bb935-(258 +3 +1 ));bb424+=bb436-bb469;}bb81(bbg->
bb158,bb424,bb469);bbg->bb183=bb469;bbg->bb429=(bb6)bb469;bbg->bb497=
bbg->bb158[0 ];(bbg->bb497=(((bbg->bb497)<<bbg->bb1608)^(bbg->bb158[1 ]
))&bbg->bb1377);bb90(bb11=0 ;bb11<=bb469-3 ;bb11++){((bbg->bb497=(((bbg
->bb497)<<bbg->bb1608)^(bbg->bb158[(bb11)+(3 -1 )]))&bbg->bb1377),bbg->
bb966[(bb11)&bbg->bb1808]=bb1130=bbg->bb382[bbg->bb497],bbg->bb382[
bbg->bb497]=(bb1059)(bb11));}bbm(bb1130)bb1130=0 ;bb2 0 ;}bbe bb1053(
bb16)bb17 bb16;{bb191*bbg;bbm(bb16==0 ||bb16->bb23==0 ||bb16->bb401==0
||bb16->bb370==0 )bb2(-2 );bb16->bb192=bb16->bb611=0 ;bb16->bb321=0 ;bb16
->bb977=2 ;bbg=(bb191* )bb16->bb23;bbg->bb184=0 ;bbg->bb1874=bbg->bb171
;bbm(bbg->bb1345<0 ){bbg->bb1345=0 ;}bbg->bb359=bbg->bb1345?113 :42 ;bb16
->bb368=1 ;bbg->bb1893=0 ;bb2206(bbg);bb2426(bbg);bb2 0 ;}bbe bb1175(
bb16,bb123,bb300)bb17 bb16;bbe bb123;bbe bb300;{bb191*bbg;bb2383
bb2019;bbe bb18=0 ;bbm(bb16==0 ||bb16->bb23==0 )bb2(-2 );bbg=bb16->bb23;
bbm(bb123==(-1 )){bb123=6 ;}bbm(bb123<0 ||bb123>9 ||bb300<0 ||bb300>2 ){bb2
(-2 );}bb2019=bb1309[bbg->bb123].bb2019;bbm(bb2019!=bb1309[bb123].
bb2019&&bb16->bb192!=0 ){bb18=bb512(bb16,1 );}bbm(bbg->bb123!=bb123){
bbg->bb123=bb123;bbg->bb2054=bb1309[bb123].bb2435;bbg->bb2178=bb1309[
bb123].bb2428;bbg->bb1837=bb1309[bb123].bb2380;bbg->bb2173=bb1309[
bb123].bb2345;}bbg->bb300=bb300;bb2 bb18;}bb41 bbb bb2007(bbg,bbn)bb191
 *bbg;bb7 bbn;{{bbg->bb171[bbg->bb184++]=((bb153)(bbn>>8 ));};{bbg->
bb171[bbg->bb184++]=((bb153)(bbn&0xff ));};}bb41 bbb bb1266(bb16)bb17
bb16;{bbs bb22=bb16->bb23->bb184;bbm(bb22>bb16->bb385)bb22=bb16->
bb385;bbm(bb22==0 )bb2;bb81(bb16->bb596,bb16->bb23->bb1874,bb22);bb16
->bb596+=bb22;bb16->bb23->bb1874+=bb22;bb16->bb611+=bb22;bb16->bb385
-=bb22;bb16->bb23->bb184-=bb22;bbm(bb16->bb23->bb184==0 ){bb16->bb23->
bb1874=bb16->bb23->bb171;}}bbe bb512(bb16,bb175)bb17 bb16;bbe bb175;{
bbe bb2341;bb191*bbg;bbm(bb16==0 ||bb16->bb23==0 ||bb175>4 ||bb175<0 ){
bb2(-2 );}bbg=bb16->bb23;bbm(bb16->bb596==0 ||(bb16->bb125==0 &&bb16->
bb148!=0 )||(bbg->bb359==666 &&bb175!=4 )){bb2(bb16->bb321=(bbl* )bb1066
[2 -((-2 ))],((-2 )));}bbm(bb16->bb385==0 )bb2(bb16->bb321=(bbl* )bb1066[
2 -((-5 ))],((-5 )));bbg->bb16=bb16;bb2341=bbg->bb1893;bbg->bb1893=bb175
;bbm(bbg->bb359==42 ){bb7 bb984=(8 +((bbg->bb2160-8 )<<4 ))<<8 ;bb7 bb2296
=(bbg->bb123-1 )>>1 ;bbm(bb2296>3 )bb2296=3 ;bb984|=(bb2296<<6 );bbm(bbg->
bb183!=0 )bb984|=0x20 ;bb984+=31 -(bb984%31 );bbg->bb359=113 ;bb2007(bbg,
bb984);bbm(bbg->bb183!=0 ){bb2007(bbg,(bb7)(bb16->bb368>>16 ));bb2007(
bbg,(bb7)(bb16->bb368&0xffff ));}bb16->bb368=1L ;}bbm(bbg->bb184!=0 ){
bb1266(bb16);bbm(bb16->bb385==0 ){bbg->bb1893=-1 ;bb2 0 ;}}bb54 bbm(bb16
->bb148==0 &&bb175<=bb2341&&bb175!=4 ){bb2(bb16->bb321=(bbl* )bb1066[2 -
((-5 ))],((-5 )));}bbm(bbg->bb359==666 &&bb16->bb148!=0 ){bb2(bb16->bb321
=(bbl* )bb1066[2 -((-5 ))],((-5 )));}bbm(bb16->bb148!=0 ||bbg->bb462!=0 ||
(bb175!=0 &&bbg->bb359!=666 )){bb1824 bb1984;bb1984=( * (bb1309[bbg->
bb123].bb2019))(bbg,bb175);bbm(bb1984==bb1756||bb1984==bb2138){bbg->
bb359=666 ;}bbm(bb1984==bb1282||bb1984==bb1756){bbm(bb16->bb385==0 ){
bbg->bb1893=-1 ;}bb2 0 ;}bbm(bb1984==bb2157){bbm(bb175==1 ){bb2253(bbg);
}bb54{bb2141(bbg,(bbl* )0 ,0L ,0 );bbm(bb175==3 ){bbg->bb382[bbg->bb1327-
1 ]=0 ;bb996((bb29* )bbg->bb382,0 ,(bbs)(bbg->bb1327-1 ) *bb12( *bbg->
bb382));;}}bb1266(bb16);bbm(bb16->bb385==0 ){bbg->bb1893=-1 ;bb2 0 ;}}};
bbm(bb175!=4 )bb2 0 ;bbm(bbg->bb1345)bb2 1 ;bb2007(bbg,(bb7)(bb16->bb368
>>16 ));bb2007(bbg,(bb7)(bb16->bb368&0xffff ));bb1266(bb16);bbg->bb1345
=-1 ;bb2 bbg->bb184!=0 ?0 :1 ;}bbe bb946(bb16)bb17 bb16;{bbe bb359;bbm(
bb16==0 ||bb16->bb23==0 )bb2(-2 );bb359=bb16->bb23->bb359;bbm(bb359!=42
&&bb359!=113 &&bb359!=666 ){bb2(-2 );}{bbm(bb16->bb23->bb171)( * ((bb16)->
bb370))((bb16)->bb110,(bb70)(bb16->bb23->bb171));};{bbm(bb16->bb23->
bb382)( * ((bb16)->bb370))((bb16)->bb110,(bb70)(bb16->bb23->bb382));}
;{bbm(bb16->bb23->bb966)( * ((bb16)->bb370))((bb16)->bb110,(bb70)(
bb16->bb23->bb966));};{bbm(bb16->bb23->bb158)( * ((bb16)->bb370))((
bb16)->bb110,(bb70)(bb16->bb23->bb158));};( * ((bb16)->bb370))((bb16)->
bb110,(bb70)(bb16->bb23));bb16->bb23=0 ;bb2 bb359==113 ?(-3 ):0 ;}bbe
bb1150(bb131,bb182)bb17 bb131;bb17 bb182;{
#ifdef bb502
bb2(-2 );
#else
bb191*bb425;bb191*bb1825;bb501*bb1882;bbm(bb182==0 ||bb131==0 ||bb182->
bb23==0 ){bb2(-2 );}bb1825=bb182->bb23; *bb131= *bb182;bb425=(bb191* )(
 * ((bb131)->bb401))((bb131)->bb110,(1 ),(bb12(bb191)));bbm(bb425==0 )bb2
(-4 );bb131->bb23=(bbi bb379* )bb425; *bb425= *bb1825;bb425->bb16=
bb131;bb425->bb158=(bb29* )( * ((bb131)->bb401))((bb131)->bb110,(
bb425->bb935),(2 *bb12(bb153)));bb425->bb966=(bb1352* )( * ((bb131)->
bb401))((bb131)->bb110,(bb425->bb935),(bb12(bb1059)));bb425->bb382=(
bb1352* )( * ((bb131)->bb401))((bb131)->bb110,(bb425->bb1327),(bb12(
bb1059)));bb1882=(bb501* )( * ((bb131)->bb401))((bb131)->bb110,(bb425
->bb1121),(bb12(bb126)+2 ));bb425->bb171=(bb1207* )bb1882;bbm(bb425->
bb158==0 ||bb425->bb966==0 ||bb425->bb382==0 ||bb425->bb171==0 ){bb946(
bb131);bb2(-4 );}bb81(bb425->bb158,bb1825->bb158,bb425->bb935*2 *bb12(
bb153));bb81((bb29* )bb425->bb966,(bb29* )bb1825->bb966,bb425->bb935*
bb12(bb1059));bb81((bb29* )bb425->bb382,(bb29* )bb1825->bb382,bb425->
bb1327*bb12(bb1059));bb81(bb425->bb171,bb1825->bb171,(bb7)bb425->
bb2038);bb425->bb1874=bb425->bb171+(bb1825->bb1874-bb1825->bb171);
bb425->bb1643=bb1882+bb425->bb1121/bb12(bb126);bb425->bb1724=bb425->
bb171+(1 +bb12(bb126)) *bb425->bb1121;bb425->bb1936.bb1748=bb425->
bb976;bb425->bb1863.bb1748=bb425->bb1678;bb425->bb2061.bb1748=bb425->
bb522;bb2 0 ;
#endif
}bb41 bbe bb2431(bb16,bb39,bb47)bb17 bb16;bb29*bb39;bbs bb47;{bbs bb22
=bb16->bb148;bbm(bb22>bb47)bb22=bb47;bbm(bb22==0 )bb2 0 ;bb16->bb148-=
bb22;bbm(!bb16->bb23->bb1345){bb16->bb368=bb985(bb16->bb368,bb16->
bb125,bb22);}bb81(bb39,bb16->bb125,bb22);bb16->bb125+=bb22;bb16->
bb192+=bb22;bb2(bbe)bb22;}bb41 bbb bb2426(bbg)bb191*bbg;{bbg->bb2224=
(bb395)2L *bbg->bb935;bbg->bb382[bbg->bb1327-1 ]=0 ;bb996((bb29* )bbg->
bb382,0 ,(bbs)(bbg->bb1327-1 ) *bb12( *bbg->bb382));;bbg->bb2054=bb1309
[bbg->bb123].bb2435;bbg->bb2178=bb1309[bbg->bb123].bb2428;bbg->bb1837
=bb1309[bbg->bb123].bb2380;bbg->bb2173=bb1309[bbg->bb123].bb2345;bbg
->bb183=0 ;bbg->bb429=0L ;bbg->bb462=0 ;bbg->bb953=bbg->bb1260=3 -1 ;bbg->
bb1939=0 ;bbg->bb497=0 ;}bb41 bb7 bb2256(bbg,bb1993)bb191*bbg;bb1318
bb1993;{bbs bb2315=bbg->bb2173;bb920 bb29*bb493=bbg->bb158+bbg->bb183
;bb920 bb29*bb613;bb920 bbe bb22;bbe bb1189=bbg->bb1260;bbe bb1837=
bbg->bb1837;bb1318 bb2442=bbg->bb183>(bb1318)((bbg)->bb935-(258 +3 +1 ))?
bbg->bb183-(bb1318)((bbg)->bb935-(258 +3 +1 )):0 ;bb1352*bb966=bbg->bb966
;bb7 bb2468=bbg->bb1808;
#ifdef bb1052
bb920 bb29*bb1892=bbg->bb158+bbg->bb183+258 -1 ;bb920 bb126 bb2533= * (
bb501* )bb493;bb920 bb126 bb2119= * (bb501* )(bb493+bb1189-1 );
#else
bb920 bb29*bb1892=bbg->bb158+bbg->bb183+258 ;bb920 bb153 bb2317=bb493[
bb1189-1 ];bb920 bb153 bb2119=bb493[bb1189];
#endif
;bbm(bbg->bb1260>=bbg->bb2178){bb2315>>=2 ;}bbm((bb7)bb1837>bbg->bb462
)bb1837=bbg->bb462;;bb564{;bb613=bbg->bb158+bb1993;
#if ( defined( bb1052) && bb2578 == 258)
bbm( * (bb501* )(bb613+bb1189-1 )!=bb2119|| * (bb501* )bb613!=bb2533)bb1683
;;bb493++,bb613++;bb564{}bb108( * (bb501* )(bb493+=2 )== * (bb501* )(
bb613+=2 )&& * (bb501* )(bb493+=2 )== * (bb501* )(bb613+=2 )&& * (bb501*
)(bb493+=2 )== * (bb501* )(bb613+=2 )&& * (bb501* )(bb493+=2 )== * (
bb501* )(bb613+=2 )&&bb493<bb1892);;bbm( *bb493== *bb613)bb493++;bb22=
(258 -1 )-(bbe)(bb1892-bb493);bb493=bb1892-(258 -1 );
#else
bbm(bb613[bb1189]!=bb2119||bb613[bb1189-1 ]!=bb2317|| *bb613!= *bb493
|| * ++bb613!=bb493[1 ])bb1683;bb493+=2 ,bb613++;;bb564{}bb108( * ++
bb493== * ++bb613&& * ++bb493== * ++bb613&& * ++bb493== * ++bb613&& *
++bb493== * ++bb613&& * ++bb493== * ++bb613&& * ++bb493== * ++bb613&&
 * ++bb493== * ++bb613&& * ++bb493== * ++bb613&&bb493<bb1892);;bb22=
258 -(bbe)(bb1892-bb493);bb493=bb1892-258 ;
#endif
bbm(bb22>bb1189){bbg->bb2022=bb1993;bb1189=bb22;bbm(bb22>=bb1837)bb21
;
#ifdef bb1052
bb2119= * (bb501* )(bb493+bb1189-1 );
#else
bb2317=bb493[bb1189-1 ];bb2119=bb493[bb1189];
#endif
}}bb108((bb1993=bb966[bb1993&bb2468])>bb2442&&--bb2315!=0 );bbm((bb7)bb1189
<=bbg->bb462)bb2(bb7)bb1189;bb2 bbg->bb462;}bb41 bbb bb2187(bbg)bb191
 *bbg;{bb920 bbs bb11,bb97;bb920 bb1352*bb27;bbs bb1928;bb7 bb1203=
bbg->bb935;bb564{bb1928=(bbs)(bbg->bb2224-(bb395)bbg->bb462-(bb395)bbg
->bb183);bbm(bb1928==0 &&bbg->bb183==0 &&bbg->bb462==0 ){bb1928=bb1203;}
bb54 bbm(bb1928==(bbs)(-1 )){bb1928--;}bb54 bbm(bbg->bb183>=bb1203+((
bbg)->bb935-(258 +3 +1 ))){bb81(bbg->bb158,bbg->bb158+bb1203,(bbs)bb1203
);bbg->bb2022-=bb1203;bbg->bb183-=bb1203;bbg->bb429-=(bb6)bb1203;bb11
=bbg->bb1327;bb27=&bbg->bb382[bb11];bb564{bb97= * --bb27; *bb27=(
bb1059)(bb97>=bb1203?bb97-bb1203:0 );}bb108(--bb11);bb11=bb1203;bb27=&
bbg->bb966[bb11];bb564{bb97= * --bb27; *bb27=(bb1059)(bb97>=bb1203?
bb97-bb1203:0 );}bb108(--bb11);bb1928+=bb1203;}bbm(bbg->bb16->bb148==0
)bb2;;bb11=bb2431(bbg->bb16,bbg->bb158+bbg->bb183+bbg->bb462,bb1928);
bbg->bb462+=bb11;bbm(bbg->bb462>=3 ){bbg->bb497=bbg->bb158[bbg->bb183]
;(bbg->bb497=(((bbg->bb497)<<bbg->bb1608)^(bbg->bb158[bbg->bb183+1 ]))&
bbg->bb1377);}}bb108(bbg->bb462<(258 +3 +1 )&&bbg->bb16->bb148!=0 );}bb41
bb1824 bb2377(bbg,bb175)bb191*bbg;bbe bb175;{bb395 bb2270=0xffff ;
bb395 bb2151;bbm(bb2270>bbg->bb2038-5 ){bb2270=bbg->bb2038-5 ;}bb90(;;){
bbm(bbg->bb462<=1 ){;bb2187(bbg);bbm(bbg->bb462==0 &&bb175==0 )bb2 bb1282
;bbm(bbg->bb462==0 )bb21;};bbg->bb183+=bbg->bb462;bbg->bb462=0 ;bb2151=
bbg->bb429+bb2270;bbm(bbg->bb183==0 ||(bb395)bbg->bb183>=bb2151){bbg->
bb462=(bb7)(bbg->bb183-bb2151);bbg->bb183=(bb7)bb2151;{{bb1616(bbg,(
bbg->bb429>=0L ?(bb441* )&bbg->bb158[(bbs)bbg->bb429]:(bb441* )0 ),(
bb395)((bb6)bbg->bb183-bbg->bb429),(0 ));bbg->bb429=bbg->bb183;bb1266(
bbg->bb16);;};bbm(bbg->bb16->bb385==0 )bb2(0 )?bb1756:bb1282;};}bbm(bbg
->bb183-(bb7)bbg->bb429>=((bbg)->bb935-(258 +3 +1 ))){{{bb1616(bbg,(bbg
->bb429>=0L ?(bb441* )&bbg->bb158[(bbs)bbg->bb429]:(bb441* )0 ),(bb395)(
(bb6)bbg->bb183-bbg->bb429),(0 ));bbg->bb429=bbg->bb183;bb1266(bbg->
bb16);;};bbm(bbg->bb16->bb385==0 )bb2(0 )?bb1756:bb1282;};}}{{bb1616(
bbg,(bbg->bb429>=0L ?(bb441* )&bbg->bb158[(bbs)bbg->bb429]:(bb441* )0 ),
(bb395)((bb6)bbg->bb183-bbg->bb429),(bb175==4 ));bbg->bb429=bbg->bb183
;bb1266(bbg->bb16);;};bbm(bbg->bb16->bb385==0 )bb2(bb175==4 )?bb1756:
bb1282;};bb2 bb175==4 ?bb2138:bb2157;}bb41 bb1824 bb2159(bbg,bb175)bb191
 *bbg;bbe bb175;{bb1318 bb1130=0 ;bbe bb1763;bb90(;;){bbm(bbg->bb462<(
258 +3 +1 )){bb2187(bbg);bbm(bbg->bb462<(258 +3 +1 )&&bb175==0 ){bb2 bb1282;
}bbm(bbg->bb462==0 )bb21;}bbm(bbg->bb462>=3 ){((bbg->bb497=(((bbg->
bb497)<<bbg->bb1608)^(bbg->bb158[(bbg->bb183)+(3 -1 )]))&bbg->bb1377),
bbg->bb966[(bbg->bb183)&bbg->bb1808]=bb1130=bbg->bb382[bbg->bb497],
bbg->bb382[bbg->bb497]=(bb1059)(bbg->bb183));}bbm(bb1130!=0 &&bbg->
bb183-bb1130<=((bbg)->bb935-(258 +3 +1 ))){bbm(bbg->bb300!=2 ){bbg->bb953
=bb2256(bbg,bb1130);}}bbm(bbg->bb953>=3 ){;{bb155 bb22=(bbg->bb953-3 );
bb126 bb415=(bbg->bb183-bbg->bb2022);bbg->bb1643[bbg->bb605]=bb415;
bbg->bb1724[bbg->bb605++]=bb22;bb415--;bbg->bb976[bb2050[bb22]+256 +1 ]
.bb268.bb426++;bbg->bb1678[((bb415)<256 ?bb1735[bb415]:bb1735[256 +((
bb415)>>7 )])].bb268.bb426++;bb1763=(bbg->bb605==bbg->bb1121-1 );};bbg
->bb462-=bbg->bb953;bbm(bbg->bb953<=bbg->bb2054&&bbg->bb462>=3 ){bbg->
bb953--;bb564{bbg->bb183++;((bbg->bb497=(((bbg->bb497)<<bbg->bb1608)^
(bbg->bb158[(bbg->bb183)+(3 -1 )]))&bbg->bb1377),bbg->bb966[(bbg->bb183
)&bbg->bb1808]=bb1130=bbg->bb382[bbg->bb497],bbg->bb382[bbg->bb497]=(
bb1059)(bbg->bb183));}bb108(--bbg->bb953!=0 );bbg->bb183++;}bb54{bbg->
bb183+=bbg->bb953;bbg->bb953=0 ;bbg->bb497=bbg->bb158[bbg->bb183];(bbg
->bb497=(((bbg->bb497)<<bbg->bb1608)^(bbg->bb158[bbg->bb183+1 ]))&bbg
->bb1377);}}bb54{;{bb155 bb1841=(bbg->bb158[bbg->bb183]);bbg->bb1643[
bbg->bb605]=0 ;bbg->bb1724[bbg->bb605++]=bb1841;bbg->bb976[bb1841].
bb268.bb426++;bb1763=(bbg->bb605==bbg->bb1121-1 );};bbg->bb462--;bbg->
bb183++;}bbm(bb1763){{bb1616(bbg,(bbg->bb429>=0L ?(bb441* )&bbg->bb158
[(bbs)bbg->bb429]:(bb441* )0 ),(bb395)((bb6)bbg->bb183-bbg->bb429),(0 ));
bbg->bb429=bbg->bb183;bb1266(bbg->bb16);;};bbm(bbg->bb16->bb385==0 )bb2
(0 )?bb1756:bb1282;};}{{bb1616(bbg,(bbg->bb429>=0L ?(bb441* )&bbg->
bb158[(bbs)bbg->bb429]:(bb441* )0 ),(bb395)((bb6)bbg->bb183-bbg->bb429
),(bb175==4 ));bbg->bb429=bbg->bb183;bb1266(bbg->bb16);;};bbm(bbg->
bb16->bb385==0 )bb2(bb175==4 )?bb1756:bb1282;};bb2 bb175==4 ?bb2138:
bb2157;}bb41 bb1824 bb1947(bbg,bb175)bb191*bbg;bbe bb175;{bb1318
bb1130=0 ;bbe bb1763;bb90(;;){bbm(bbg->bb462<(258 +3 +1 )){bb2187(bbg);
bbm(bbg->bb462<(258 +3 +1 )&&bb175==0 ){bb2 bb1282;}bbm(bbg->bb462==0 )bb21
;}bbm(bbg->bb462>=3 ){((bbg->bb497=(((bbg->bb497)<<bbg->bb1608)^(bbg->
bb158[(bbg->bb183)+(3 -1 )]))&bbg->bb1377),bbg->bb966[(bbg->bb183)&bbg
->bb1808]=bb1130=bbg->bb382[bbg->bb497],bbg->bb382[bbg->bb497]=(
bb1059)(bbg->bb183));}bbg->bb1260=bbg->bb953,bbg->bb2295=bbg->bb2022;
bbg->bb953=3 -1 ;bbm(bb1130!=0 &&bbg->bb1260<bbg->bb2054&&bbg->bb183-
bb1130<=((bbg)->bb935-(258 +3 +1 ))){bbm(bbg->bb300!=2 ){bbg->bb953=
bb2256(bbg,bb1130);}bbm(bbg->bb953<=5 &&(bbg->bb300==1 ||(bbg->bb953==3
&&bbg->bb183-bbg->bb2022>4096 ))){bbg->bb953=3 -1 ;}}bbm(bbg->bb1260>=3
&&bbg->bb953<=bbg->bb1260){bb7 bb2490=bbg->bb183+bbg->bb462-3 ;;{bb155
bb22=(bbg->bb1260-3 );bb126 bb415=(bbg->bb183-1 -bbg->bb2295);bbg->
bb1643[bbg->bb605]=bb415;bbg->bb1724[bbg->bb605++]=bb22;bb415--;bbg->
bb976[bb2050[bb22]+256 +1 ].bb268.bb426++;bbg->bb1678[((bb415)<256 ?
bb1735[bb415]:bb1735[256 +((bb415)>>7 )])].bb268.bb426++;bb1763=(bbg->
bb605==bbg->bb1121-1 );};bbg->bb462-=bbg->bb1260-1 ;bbg->bb1260-=2 ;
bb564{bbm(++bbg->bb183<=bb2490){((bbg->bb497=(((bbg->bb497)<<bbg->
bb1608)^(bbg->bb158[(bbg->bb183)+(3 -1 )]))&bbg->bb1377),bbg->bb966[(
bbg->bb183)&bbg->bb1808]=bb1130=bbg->bb382[bbg->bb497],bbg->bb382[bbg
->bb497]=(bb1059)(bbg->bb183));}}bb108(--bbg->bb1260!=0 );bbg->bb1939=
0 ;bbg->bb953=3 -1 ;bbg->bb183++;bbm(bb1763){{bb1616(bbg,(bbg->bb429>=0L
?(bb441* )&bbg->bb158[(bbs)bbg->bb429]:(bb441* )0 ),(bb395)((bb6)bbg->
bb183-bbg->bb429),(0 ));bbg->bb429=bbg->bb183;bb1266(bbg->bb16);;};bbm
(bbg->bb16->bb385==0 )bb2(0 )?bb1756:bb1282;};}bb54 bbm(bbg->bb1939){;{
bb155 bb1841=(bbg->bb158[bbg->bb183-1 ]);bbg->bb1643[bbg->bb605]=0 ;bbg
->bb1724[bbg->bb605++]=bb1841;bbg->bb976[bb1841].bb268.bb426++;bb1763
=(bbg->bb605==bbg->bb1121-1 );};bbm(bb1763){{bb1616(bbg,(bbg->bb429>=
0L ?(bb441* )&bbg->bb158[(bbs)bbg->bb429]:(bb441* )0 ),(bb395)((bb6)bbg
->bb183-bbg->bb429),(0 ));bbg->bb429=bbg->bb183;bb1266(bbg->bb16);;};}
bbg->bb183++;bbg->bb462--;bbm(bbg->bb16->bb385==0 )bb2 bb1282;}bb54{
bbg->bb1939=1 ;bbg->bb183++;bbg->bb462--;}};bbm(bbg->bb1939){;{bb155
bb1841=(bbg->bb158[bbg->bb183-1 ]);bbg->bb1643[bbg->bb605]=0 ;bbg->
bb1724[bbg->bb605++]=bb1841;bbg->bb976[bb1841].bb268.bb426++;bb1763=(
bbg->bb605==bbg->bb1121-1 );};bbg->bb1939=0 ;}{{bb1616(bbg,(bbg->bb429
>=0L ?(bb441* )&bbg->bb158[(bbs)bbg->bb429]:(bb441* )0 ),(bb395)((bb6)bbg
->bb183-bbg->bb429),(bb175==4 ));bbg->bb429=bbg->bb183;bb1266(bbg->
bb16);;};bbm(bbg->bb16->bb385==0 )bb2(bb175==4 )?bb1756:bb1282;};bb2
bb175==4 ?bb2138:bb2157;}
