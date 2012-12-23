/*
   'src_compress_deflate_trees.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Fri Oct 12 22:15:16 2012
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
bb1290,bbe bb1108));bb41 bbh bbe bb2412[29 ]={0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,
2 ,2 ,2 ,2 ,3 ,3 ,3 ,3 ,4 ,4 ,4 ,4 ,5 ,5 ,5 ,5 ,0 };bb41 bbh bbe bb2376[30 ]={0 ,0 ,0 ,0 ,1
,1 ,2 ,2 ,3 ,3 ,4 ,4 ,5 ,5 ,6 ,6 ,7 ,7 ,8 ,8 ,9 ,9 ,10 ,10 ,11 ,11 ,12 ,12 ,13 ,13 };bb41 bbh
bbe bb2477[19 ]={0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,3 ,7 };bb41 bbh bb155
bb2195[19 ]={16 ,17 ,18 ,0 ,8 ,7 ,9 ,6 ,10 ,5 ,11 ,4 ,12 ,3 ,13 ,2 ,14 ,1 ,15 };bb41 bbh
bb460 bb1738[(256 +1 +29 )+2 ]={{{12 },{8 }},{{140 },{8 }},{{76 },{8 }},{{204 },
{8 }},{{44 },{8 }},{{172 },{8 }},{{108 },{8 }},{{236 },{8 }},{{28 },{8 }},{{156 }
,{8 }},{{92 },{8 }},{{220 },{8 }},{{60 },{8 }},{{188 },{8 }},{{124 },{8 }},{{252
},{8 }},{{2 },{8 }},{{130 },{8 }},{{66 },{8 }},{{194 },{8 }},{{34 },{8 }},{{162 }
,{8 }},{{98 },{8 }},{{226 },{8 }},{{18 },{8 }},{{146 },{8 }},{{82 },{8 }},{{210 }
,{8 }},{{50 },{8 }},{{178 },{8 }},{{114 },{8 }},{{242 },{8 }},{{10 },{8 }},{{138
},{8 }},{{74 },{8 }},{{202 },{8 }},{{42 },{8 }},{{170 },{8 }},{{106 },{8 }},{{
234 },{8 }},{{26 },{8 }},{{154 },{8 }},{{90 },{8 }},{{218 },{8 }},{{58 },{8 }},{{
186 },{8 }},{{122 },{8 }},{{250 },{8 }},{{6 },{8 }},{{134 },{8 }},{{70 },{8 }},{{
198 },{8 }},{{38 },{8 }},{{166 },{8 }},{{102 },{8 }},{{230 },{8 }},{{22 },{8 }},{
{150 },{8 }},{{86 },{8 }},{{214 },{8 }},{{54 },{8 }},{{182 },{8 }},{{118 },{8 }},
{{246 },{8 }},{{14 },{8 }},{{142 },{8 }},{{78 },{8 }},{{206 },{8 }},{{46 },{8 }},
{{174 },{8 }},{{110 },{8 }},{{238 },{8 }},{{30 },{8 }},{{158 },{8 }},{{94 },{8 }}
,{{222 },{8 }},{{62 },{8 }},{{190 },{8 }},{{126 },{8 }},{{254 },{8 }},{{1 },{8 }}
,{{129 },{8 }},{{65 },{8 }},{{193 },{8 }},{{33 },{8 }},{{161 },{8 }},{{97 },{8 }}
,{{225 },{8 }},{{17 },{8 }},{{145 },{8 }},{{81 },{8 }},{{209 },{8 }},{{49 },{8 }}
,{{177 },{8 }},{{113 },{8 }},{{241 },{8 }},{{9 },{8 }},{{137 },{8 }},{{73 },{8 }}
,{{201 },{8 }},{{41 },{8 }},{{169 },{8 }},{{105 },{8 }},{{233 },{8 }},{{25 },{8 }
},{{153 },{8 }},{{89 },{8 }},{{217 },{8 }},{{57 },{8 }},{{185 },{8 }},{{121 },{8
}},{{249 },{8 }},{{5 },{8 }},{{133 },{8 }},{{69 },{8 }},{{197 },{8 }},{{37 },{8 }
},{{165 },{8 }},{{101 },{8 }},{{229 },{8 }},{{21 },{8 }},{{149 },{8 }},{{85 },{8
}},{{213 },{8 }},{{53 },{8 }},{{181 },{8 }},{{117 },{8 }},{{245 },{8 }},{{13 },{
8 }},{{141 },{8 }},{{77 },{8 }},{{205 },{8 }},{{45 },{8 }},{{173 },{8 }},{{109 },
{8 }},{{237 },{8 }},{{29 },{8 }},{{157 },{8 }},{{93 },{8 }},{{221 },{8 }},{{61 },
{8 }},{{189 },{8 }},{{125 },{8 }},{{253 },{8 }},{{19 },{9 }},{{275 },{9 }},{{147
},{9 }},{{403 },{9 }},{{83 },{9 }},{{339 },{9 }},{{211 },{9 }},{{467 },{9 }},{{
51 },{9 }},{{307 },{9 }},{{179 },{9 }},{{435 },{9 }},{{115 },{9 }},{{371 },{9 }},
{{243 },{9 }},{{499 },{9 }},{{11 },{9 }},{{267 },{9 }},{{139 },{9 }},{{395 },{9 }
},{{75 },{9 }},{{331 },{9 }},{{203 },{9 }},{{459 },{9 }},{{43 },{9 }},{{299 },{9
}},{{171 },{9 }},{{427 },{9 }},{{107 },{9 }},{{363 },{9 }},{{235 },{9 }},{{491 }
,{9 }},{{27 },{9 }},{{283 },{9 }},{{155 },{9 }},{{411 },{9 }},{{91 },{9 }},{{347
},{9 }},{{219 },{9 }},{{475 },{9 }},{{59 },{9 }},{{315 },{9 }},{{187 },{9 }},{{
443 },{9 }},{{123 },{9 }},{{379 },{9 }},{{251 },{9 }},{{507 },{9 }},{{7 },{9 }},{
{263 },{9 }},{{135 },{9 }},{{391 },{9 }},{{71 },{9 }},{{327 },{9 }},{{199 },{9 }}
,{{455 },{9 }},{{39 },{9 }},{{295 },{9 }},{{167 },{9 }},{{423 },{9 }},{{103 },{9
}},{{359 },{9 }},{{231 },{9 }},{{487 },{9 }},{{23 },{9 }},{{279 },{9 }},{{151 },
{9 }},{{407 },{9 }},{{87 },{9 }},{{343 },{9 }},{{215 },{9 }},{{471 },{9 }},{{55 }
,{9 }},{{311 },{9 }},{{183 },{9 }},{{439 },{9 }},{{119 },{9 }},{{375 },{9 }},{{
247 },{9 }},{{503 },{9 }},{{15 },{9 }},{{271 },{9 }},{{143 },{9 }},{{399 },{9 }},
{{79 },{9 }},{{335 },{9 }},{{207 },{9 }},{{463 },{9 }},{{47 },{9 }},{{303 },{9 }}
,{{175 },{9 }},{{431 },{9 }},{{111 },{9 }},{{367 },{9 }},{{239 },{9 }},{{495 },{
9 }},{{31 },{9 }},{{287 },{9 }},{{159 },{9 }},{{415 },{9 }},{{95 },{9 }},{{351 },
{9 }},{{223 },{9 }},{{479 },{9 }},{{63 },{9 }},{{319 },{9 }},{{191 },{9 }},{{447
},{9 }},{{127 },{9 }},{{383 },{9 }},{{255 },{9 }},{{511 },{9 }},{{0 },{7 }},{{64
},{7 }},{{32 },{7 }},{{96 },{7 }},{{16 },{7 }},{{80 },{7 }},{{48 },{7 }},{{112 },
{7 }},{{8 },{7 }},{{72 },{7 }},{{40 },{7 }},{{104 },{7 }},{{24 },{7 }},{{88 },{7 }
},{{56 },{7 }},{{120 },{7 }},{{4 },{7 }},{{68 },{7 }},{{36 },{7 }},{{100 },{7 }},
{{20 },{7 }},{{84 },{7 }},{{52 },{7 }},{{116 },{7 }},{{3 },{8 }},{{131 },{8 }},{{
67 },{8 }},{{195 },{8 }},{{35 },{8 }},{{163 },{8 }},{{99 },{8 }},{{227 },{8 }}};
bb41 bbh bb460 bb2232[30 ]={{{0 },{5 }},{{16 },{5 }},{{8 },{5 }},{{24 },{5 }},
{{4 },{5 }},{{20 },{5 }},{{12 },{5 }},{{28 },{5 }},{{2 },{5 }},{{18 },{5 }},{{10 }
,{5 }},{{26 },{5 }},{{6 },{5 }},{{22 },{5 }},{{14 },{5 }},{{30 },{5 }},{{1 },{5 }}
,{{17 },{5 }},{{9 },{5 }},{{25 },{5 }},{{5 },{5 }},{{21 },{5 }},{{13 },{5 }},{{29
},{5 }},{{3 },{5 }},{{19 },{5 }},{{11 },{5 }},{{27 },{5 }},{{7 },{5 }},{{23 },{5 }
}};bb41 bbh bb155 bb1735[512 ]={0 ,1 ,2 ,3 ,4 ,4 ,5 ,5 ,6 ,6 ,6 ,6 ,7 ,7 ,7 ,7 ,8 ,8 ,8 ,
8 ,8 ,8 ,8 ,8 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,10 ,
10 ,10 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,12 ,12 ,12 ,12 ,12 ,
12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,12 ,
12 ,12 ,12 ,12 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,
13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,13 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,
14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,
14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,
14 ,14 ,14 ,14 ,14 ,14 ,14 ,14 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,
15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,
15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,15 ,
15 ,15 ,15 ,0 ,0 ,16 ,17 ,18 ,18 ,19 ,19 ,20 ,20 ,20 ,20 ,21 ,21 ,21 ,21 ,22 ,22 ,22 ,22 ,22
,22 ,22 ,22 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24
,24 ,24 ,24 ,24 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,26 ,26 ,26
,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26
,26 ,26 ,26 ,26 ,26 ,26 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27
,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28
,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28
,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28
,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,28 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29
,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29
,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29 ,29
,29 ,29 ,29 ,29 ,29 };bb41 bbh bb155 bb2050[258 -3 +1 ]={0 ,1 ,2 ,3 ,4 ,5 ,6 ,7 ,8 ,8 ,
9 ,9 ,10 ,10 ,11 ,11 ,12 ,12 ,12 ,12 ,13 ,13 ,13 ,13 ,14 ,14 ,14 ,14 ,15 ,15 ,15 ,15 ,16 ,16
,16 ,16 ,16 ,16 ,16 ,16 ,17 ,17 ,17 ,17 ,17 ,17 ,17 ,17 ,18 ,18 ,18 ,18 ,18 ,18 ,18 ,18 ,19
,19 ,19 ,19 ,19 ,19 ,19 ,19 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20
,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,21 ,22 ,22 ,22 ,22 ,22 ,22 ,22
,22 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,22 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23 ,23
,23 ,23 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24
,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,24 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25
,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,25 ,26 ,26 ,26
,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26 ,26
,26 ,26 ,26 ,26 ,26 ,26 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27
,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,27 ,28 };bb41 bbh bbe bb2331[29
]={0 ,1 ,2 ,3 ,4 ,5 ,6 ,7 ,8 ,10 ,12 ,14 ,16 ,20 ,24 ,28 ,32 ,40 ,48 ,56 ,64 ,80 ,96 ,112 ,
128 ,160 ,192 ,224 ,0 };bb41 bbh bbe bb2414[30 ]={0 ,1 ,2 ,3 ,4 ,6 ,8 ,12 ,16 ,24 ,32
,48 ,64 ,96 ,128 ,192 ,256 ,384 ,512 ,768 ,1024 ,1536 ,2048 ,3072 ,4096 ,6144 ,8192 ,
12288 ,16384 ,24576 };bbi bb2275{bbh bb460*bb2342;bbh bb1139*bb2470;bbe
bb2451;bbe bb2172;bbe bb1941;};bb41 bb2004 bb2489={bb1738,bb2412,256 +
1 ,(256 +1 +29 ),15 };bb41 bb2004 bb2555={bb2232,bb2376,0 ,30 ,15 };bb41
bb2004 bb2495={(bbh bb460* )0 ,bb2477,0 ,19 ,7 };bb41 bbb bb2243 bbp((
bb191*bbg));bb41 bbb bb2165 bbp((bb191*bbg,bb460*bb305,bbe bb3));bb41
bbb bb2364 bbp((bb191*bbg,bb1734*bb1083));bb41 bbb bb2391 bbp((bb460*
bb305,bbe bb511,bb501*bb1192));bb41 bbb bb2182 bbp((bb191*bbg,bb1734*
bb1083));bb41 bbb bb2304 bbp((bb191*bbg,bb460*bb305,bbe bb511));bb41
bbb bb2229 bbp((bb191*bbg,bb460*bb305,bbe bb511));bb41 bbe bb2382 bbp
((bb191*bbg));bb41 bbb bb2406 bbp((bb191*bbg,bbe bb2051,bbe bb2091,
bbe bb2044));bb41 bbb bb2267 bbp((bb191*bbg,bb460*bb1045,bb460*bb1746
));bb41 bbb bb2311 bbp((bb191*bbg));bb41 bbs bb2367 bbp((bbs bb1323,
bbe bb469));bb41 bbb bb2257 bbp((bb191*bbg));bb41 bbb bb2301 bbp((
bb191*bbg));bb41 bbb bb2316 bbp((bb191*bbg,bb441*bb39,bbs bb22,bbe
bb984));bbb bb2206(bbg)bb191*bbg;{bbg->bb1936.bb1748=bbg->bb976;bbg->
bb1936.bb1719=&bb2489;bbg->bb1863.bb1748=bbg->bb1678;bbg->bb1863.
bb1719=&bb2555;bbg->bb2061.bb1748=bbg->bb522;bbg->bb2061.bb1719=&
bb2495;bbg->bb99=0 ;bbg->bb79=0 ;bbg->bb1992=8 ;bb2243(bbg);}bb41 bbb
bb2243(bbg)bb191*bbg;{bbe bb11;bb90(bb11=0 ;bb11<(256 +1 +29 );bb11++)bbg
->bb976[bb11].bb268.bb426=0 ;bb90(bb11=0 ;bb11<30 ;bb11++)bbg->bb1678[
bb11].bb268.bb426=0 ;bb90(bb11=0 ;bb11<19 ;bb11++)bbg->bb522[bb11].bb268
.bb426=0 ;bbg->bb976[256 ].bb268.bb426=1 ;bbg->bb1904=bbg->bb2117=0L ;bbg
->bb605=bbg->bb2228=0 ;}bb41 bbb bb2165(bbg,bb305,bb3)bb191*bbg;bb460*
bb305;bbe bb3;{bbe bb438=bbg->bb520[bb3];bbe bb75=bb3<<1 ;bb108(bb75<=
bbg->bb1499){bbm(bb75<bbg->bb1499&&(bb305[bbg->bb520[bb75+1 ]].bb268.
bb426<bb305[bbg->bb520[bb75]].bb268.bb426||(bb305[bbg->bb520[bb75+1 ]]
.bb268.bb426==bb305[bbg->bb520[bb75]].bb268.bb426&&bbg->bb1237[bbg->
bb520[bb75+1 ]]<=bbg->bb1237[bbg->bb520[bb75]]))){bb75++;}bbm((bb305[
bb438].bb268.bb426<bb305[bbg->bb520[bb75]].bb268.bb426||(bb305[bb438]
.bb268.bb426==bb305[bbg->bb520[bb75]].bb268.bb426&&bbg->bb1237[bb438]
<=bbg->bb1237[bbg->bb520[bb75]])))bb21;bbg->bb520[bb3]=bbg->bb520[
bb75];bb3=bb75;bb75<<=1 ;}bbg->bb520[bb3]=bb438;}bb41 bbb bb2364(bbg,
bb1083)bb191*bbg;bb1734*bb1083;{bb460*bb305=bb1083->bb1748;bbe bb511=
bb1083->bb511;bbh bb460*bb2113=bb1083->bb1719->bb2342;bbh bb1139*
bb1800=bb1083->bb1719->bb2470;bbe bb608=bb1083->bb1719->bb2451;bbe
bb1941=bb1083->bb1719->bb1941;bbe bb42;bbe bb11,bb97;bbe bb514;bbe
bb2142;bb126 bb19;bbe bb2137=0 ;bb90(bb514=0 ;bb514<=15 ;bb514++)bbg->
bb1192[bb514]=0 ;bb305[bbg->bb520[bbg->bb1945]].bb49.bb22=0 ;bb90(bb42=
bbg->bb1945+1 ;bb42<(2 * (256 +1 +29 )+1 );bb42++){bb11=bbg->bb520[bb42];
bb514=bb305[bb305[bb11].bb49.bb2161].bb49.bb22+1 ;bbm(bb514>bb1941)bb514
=bb1941,bb2137++;bb305[bb11].bb49.bb22=(bb126)bb514;bbm(bb11>bb511)bb1683
;bbg->bb1192[bb514]++;bb2142=0 ;bbm(bb11>=bb608)bb2142=bb1800[bb11-
bb608];bb19=bb305[bb11].bb268.bb426;bbg->bb1904+=(bb395)bb19* (bb514+
bb2142);bbm(bb2113)bbg->bb2117+=(bb395)bb19* (bb2113[bb11].bb49.bb22+
bb2142);}bbm(bb2137==0 )bb2;;bb564{bb514=bb1941-1 ;bb108(bbg->bb1192[
bb514]==0 )bb514--;bbg->bb1192[bb514]--;bbg->bb1192[bb514+1 ]+=2 ;bbg->
bb1192[bb1941]--;bb2137-=2 ;}bb108(bb2137>0 );bb90(bb514=bb1941;bb514!=
0 ;bb514--){bb11=bbg->bb1192[bb514];bb108(bb11!=0 ){bb97=bbg->bb520[--
bb42];bbm(bb97>bb511)bb1683;bbm(bb305[bb97].bb49.bb22!=(bbs)bb514){;
bbg->bb1904+=((bb6)bb514-(bb6)bb305[bb97].bb49.bb22) * (bb6)bb305[
bb97].bb268.bb426;bb305[bb97].bb49.bb22=(bb126)bb514;}bb11--;}}}bb41
bbb bb2391(bb305,bb511,bb1192)bb460*bb305;bbe bb511;bb501*bb1192;{
bb126 bb2401[15 +1 ];bb126 bb172=0 ;bbe bb514;bbe bb11;bb90(bb514=1 ;
bb514<=15 ;bb514++){bb2401[bb514]=bb172=(bb172+bb1192[bb514-1 ])<<1 ;};;
bb90(bb11=0 ;bb11<=bb511;bb11++){bbe bb22=bb305[bb11].bb49.bb22;bbm(
bb22==0 )bb1683;bb305[bb11].bb268.bb172=bb2367(bb2401[bb22]++,bb22);;}
}bb41 bbb bb2182(bbg,bb1083)bb191*bbg;bb1734*bb1083;{bb460*bb305=
bb1083->bb1748;bbh bb460*bb2113=bb1083->bb1719->bb2342;bbe bb2172=
bb1083->bb1719->bb2172;bbe bb11,bb97;bbe bb511=-1 ;bbe bb1777;bbg->
bb1499=0 ,bbg->bb1945=(2 * (256 +1 +29 )+1 );bb90(bb11=0 ;bb11<bb2172;bb11++
){bbm(bb305[bb11].bb268.bb426!=0 ){bbg->bb520[++(bbg->bb1499)]=bb511=
bb11;bbg->bb1237[bb11]=0 ;}bb54{bb305[bb11].bb49.bb22=0 ;}}bb108(bbg->
bb1499<2 ){bb1777=bbg->bb520[++(bbg->bb1499)]=(bb511<2 ?++bb511:0 );
bb305[bb1777].bb268.bb426=1 ;bbg->bb1237[bb1777]=0 ;bbg->bb1904--;bbm(
bb2113)bbg->bb2117-=bb2113[bb1777].bb49.bb22;}bb1083->bb511=bb511;
bb90(bb11=bbg->bb1499/2 ;bb11>=1 ;bb11--)bb2165(bbg,bb305,bb11);bb1777=
bb2172;bb564{{bb11=bbg->bb520[1 ];bbg->bb520[1 ]=bbg->bb520[bbg->bb1499
--];bb2165(bbg,bb305,1 );};bb97=bbg->bb520[1 ];bbg->bb520[--(bbg->
bb1945)]=bb11;bbg->bb520[--(bbg->bb1945)]=bb97;bb305[bb1777].bb268.
bb426=bb305[bb11].bb268.bb426+bb305[bb97].bb268.bb426;bbg->bb1237[
bb1777]=(bb155)((bbg->bb1237[bb11]>=bbg->bb1237[bb97]?bbg->bb1237[
bb11]:bbg->bb1237[bb97])+1 );bb305[bb11].bb49.bb2161=bb305[bb97].bb49.
bb2161=(bb126)bb1777;bbg->bb520[1 ]=bb1777++;bb2165(bbg,bb305,1 );}
bb108(bbg->bb1499>=2 );bbg->bb520[--(bbg->bb1945)]=bbg->bb520[1 ];
bb2364(bbg,(bb1734* )bb1083);bb2391((bb460* )bb305,bb511,bbg->bb1192);
}bb41 bbb bb2304(bbg,bb305,bb511)bb191*bbg;bb460*bb305;bbe bb511;{bbe
bb11;bbe bb2083=-1 ;bbe bb854;bbe bb1206=bb305[0 ].bb49.bb22;bbe bb969=
0 ;bbe bb1336=7 ;bbe bb1302=4 ;bbm(bb1206==0 )bb1336=138 ,bb1302=3 ;bb305[
bb511+1 ].bb49.bb22=(bb126)0xffff ;bb90(bb11=0 ;bb11<=bb511;bb11++){
bb854=bb1206;bb1206=bb305[bb11+1 ].bb49.bb22;bbm(++bb969<bb1336&&bb854
==bb1206){bb1683;}bb54 bbm(bb969<bb1302){bbg->bb522[bb854].bb268.
bb426+=bb969;}bb54 bbm(bb854!=0 ){bbm(bb854!=bb2083)bbg->bb522[bb854].
bb268.bb426++;bbg->bb522[16 ].bb268.bb426++;}bb54 bbm(bb969<=10 ){bbg->
bb522[17 ].bb268.bb426++;}bb54{bbg->bb522[18 ].bb268.bb426++;}bb969=0 ;
bb2083=bb854;bbm(bb1206==0 ){bb1336=138 ,bb1302=3 ;}bb54 bbm(bb854==
bb1206){bb1336=6 ,bb1302=3 ;}bb54{bb1336=7 ,bb1302=4 ;}}}bb41 bbb bb2229(
bbg,bb305,bb511)bb191*bbg;bb460*bb305;bbe bb511;{bbe bb11;bbe bb2083=
-1 ;bbe bb854;bbe bb1206=bb305[0 ].bb49.bb22;bbe bb969=0 ;bbe bb1336=7 ;
bbe bb1302=4 ;bbm(bb1206==0 )bb1336=138 ,bb1302=3 ;bb90(bb11=0 ;bb11<=
bb511;bb11++){bb854=bb1206;bb1206=bb305[bb11+1 ].bb49.bb22;bbm(++bb969
<bb1336&&bb854==bb1206){bb1683;}bb54 bbm(bb969<bb1302){bb564{{bbe bb22
=bbg->bb522[bb854].bb49.bb22;bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl))-bb22){
bbe bb170=bbg->bb522[bb854].bb268.bb172;bbg->bb99|=(bb170<<bbg->bb79);
{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&0xff ));};{bbg->bb171[
bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));};};bbg->bb99=(bb126)bb170
>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79+=bb22-(8 *2 *bb12(bbl));}
bb54{bbg->bb99|=(bbg->bb522[bb854].bb268.bb172)<<bbg->bb79;bbg->bb79
+=bb22;}};}bb108(--bb969!=0 );}bb54 bbm(bb854!=0 ){bbm(bb854!=bb2083){{
bbe bb22=bbg->bb522[bb854].bb49.bb22;bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl
))-bb22){bbe bb170=bbg->bb522[bb854].bb268.bb172;bbg->bb99|=(bb170<<
bbg->bb79);{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&0xff ));};{
bbg->bb171[bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));};};bbg->
bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79+=bb22-(
8 *2 *bb12(bbl));}bb54{bbg->bb99|=(bbg->bb522[bb854].bb268.bb172)<<bbg
->bb79;bbg->bb79+=bb22;}};bb969--;};{bbe bb22=bbg->bb522[16 ].bb49.
bb22;bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bbg->bb522[16
].bb268.bb172;bbg->bb99|=(bb170<<bbg->bb79);{{bbg->bb171[bbg->bb184++
]=((bb155)((bbg->bb99)&0xff ));};{bbg->bb171[bbg->bb184++]=((bb155)((
bb126)(bbg->bb99)>>8 ));};};bbg->bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(
bbl))-bbg->bb79);bbg->bb79+=bb22-(8 *2 *bb12(bbl));}bb54{bbg->bb99|=(
bbg->bb522[16 ].bb268.bb172)<<bbg->bb79;bbg->bb79+=bb22;}};{bbe bb22=2
;bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bb969-3 ;bbg->bb99
|=(bb170<<bbg->bb79);{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&
0xff ));};{bbg->bb171[bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));}
;};bbg->bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79
+=bb22-(8 *2 *bb12(bbl));}bb54{bbg->bb99|=(bb969-3 )<<bbg->bb79;bbg->
bb79+=bb22;}};}bb54 bbm(bb969<=10 ){{bbe bb22=bbg->bb522[17 ].bb49.bb22
;bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bbg->bb522[17 ].
bb268.bb172;bbg->bb99|=(bb170<<bbg->bb79);{{bbg->bb171[bbg->bb184++]=
((bb155)((bbg->bb99)&0xff ));};{bbg->bb171[bbg->bb184++]=((bb155)((
bb126)(bbg->bb99)>>8 ));};};bbg->bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(
bbl))-bbg->bb79);bbg->bb79+=bb22-(8 *2 *bb12(bbl));}bb54{bbg->bb99|=(
bbg->bb522[17 ].bb268.bb172)<<bbg->bb79;bbg->bb79+=bb22;}};{bbe bb22=3
;bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bb969-3 ;bbg->bb99
|=(bb170<<bbg->bb79);{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&
0xff ));};{bbg->bb171[bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));}
;};bbg->bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79
+=bb22-(8 *2 *bb12(bbl));}bb54{bbg->bb99|=(bb969-3 )<<bbg->bb79;bbg->
bb79+=bb22;}};}bb54{{bbe bb22=bbg->bb522[18 ].bb49.bb22;bbm(bbg->bb79>
(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bbg->bb522[18 ].bb268.bb172;bbg->
bb99|=(bb170<<bbg->bb79);{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->
bb99)&0xff ));};{bbg->bb171[bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>
8 ));};};bbg->bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg
->bb79+=bb22-(8 *2 *bb12(bbl));}bb54{bbg->bb99|=(bbg->bb522[18 ].bb268.
bb172)<<bbg->bb79;bbg->bb79+=bb22;}};{bbe bb22=7 ;bbm(bbg->bb79>(bbe)(
8 *2 *bb12(bbl))-bb22){bbe bb170=bb969-11 ;bbg->bb99|=(bb170<<bbg->bb79);
{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&0xff ));};{bbg->bb171[
bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));};};bbg->bb99=(bb126)bb170
>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79+=bb22-(8 *2 *bb12(bbl));}
bb54{bbg->bb99|=(bb969-11 )<<bbg->bb79;bbg->bb79+=bb22;}};}bb969=0 ;
bb2083=bb854;bbm(bb1206==0 ){bb1336=138 ,bb1302=3 ;}bb54 bbm(bb854==
bb1206){bb1336=6 ,bb1302=3 ;}bb54{bb1336=7 ,bb1302=4 ;}}}bb41 bbe bb2382(
bbg)bb191*bbg;{bbe bb1786;bb2304(bbg,(bb460* )bbg->bb976,bbg->bb1936.
bb511);bb2304(bbg,(bb460* )bbg->bb1678,bbg->bb1863.bb511);bb2182(bbg,
(bb1734* )(&(bbg->bb2061)));bb90(bb1786=19 -1 ;bb1786>=3 ;bb1786--){bbm(
bbg->bb522[bb2195[bb1786]].bb49.bb22!=0 )bb21;}bbg->bb1904+=3 * (bb1786
+1 )+5 +5 +4 ;;bb2 bb1786;}bb41 bbb bb2406(bbg,bb2051,bb2091,bb2044)bb191
 *bbg;bbe bb2051,bb2091,bb2044;{bbe bb2120;;;;{bbe bb22=5 ;bbm(bbg->
bb79>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bb2051-257 ;bbg->bb99|=(
bb170<<bbg->bb79);{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&
0xff ));};{bbg->bb171[bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));}
;};bbg->bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79
+=bb22-(8 *2 *bb12(bbl));}bb54{bbg->bb99|=(bb2051-257 )<<bbg->bb79;bbg->
bb79+=bb22;}};{bbe bb22=5 ;bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl))-bb22){
bbe bb170=bb2091-1 ;bbg->bb99|=(bb170<<bbg->bb79);{{bbg->bb171[bbg->
bb184++]=((bb155)((bbg->bb99)&0xff ));};{bbg->bb171[bbg->bb184++]=((
bb155)((bb126)(bbg->bb99)>>8 ));};};bbg->bb99=(bb126)bb170>>((bbe)(8 *2
 *bb12(bbl))-bbg->bb79);bbg->bb79+=bb22-(8 *2 *bb12(bbl));}bb54{bbg->
bb99|=(bb2091-1 )<<bbg->bb79;bbg->bb79+=bb22;}};{bbe bb22=4 ;bbm(bbg->
bb79>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bb2044-4 ;bbg->bb99|=(bb170
<<bbg->bb79);{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&0xff ));}
;{bbg->bb171[bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));};};bbg->
bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79+=bb22-(
8 *2 *bb12(bbl));}bb54{bbg->bb99|=(bb2044-4 )<<bbg->bb79;bbg->bb79+=bb22
;}};bb90(bb2120=0 ;bb2120<bb2044;bb2120++){;{bbe bb22=3 ;bbm(bbg->bb79>
(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bbg->bb522[bb2195[bb2120]].bb49.
bb22;bbg->bb99|=(bb170<<bbg->bb79);{{bbg->bb171[bbg->bb184++]=((bb155
)((bbg->bb99)&0xff ));};{bbg->bb171[bbg->bb184++]=((bb155)((bb126)(bbg
->bb99)>>8 ));};};bbg->bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->
bb79);bbg->bb79+=bb22-(8 *2 *bb12(bbl));}bb54{bbg->bb99|=(bbg->bb522[
bb2195[bb2120]].bb49.bb22)<<bbg->bb79;bbg->bb79+=bb22;}};};bb2229(bbg
,(bb460* )bbg->bb976,bb2051-1 );;bb2229(bbg,(bb460* )bbg->bb1678,
bb2091-1 );;}bbb bb2141(bbg,bb39,bb1290,bb1108)bb191*bbg;bb441*bb39;
bb395 bb1290;bbe bb1108;{{bbe bb22=3 ;bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl
))-bb22){bbe bb170=(0 <<1 )+bb1108;bbg->bb99|=(bb170<<bbg->bb79);{{bbg
->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&0xff ));};{bbg->bb171[bbg->
bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));};};bbg->bb99=(bb126)bb170
>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79+=bb22-(8 *2 *bb12(bbl));}
bb54{bbg->bb99|=((0 <<1 )+bb1108)<<bbg->bb79;bbg->bb79+=bb22;}};bb2316(
bbg,bb39,(bbs)bb1290,1 );}bbb bb2253(bbg)bb191*bbg;{{bbe bb22=3 ;bbm(
bbg->bb79>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=1 <<1 ;bbg->bb99|=(bb170
<<bbg->bb79);{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&0xff ));}
;{bbg->bb171[bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));};};bbg->
bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79+=bb22-(
8 *2 *bb12(bbl));}bb54{bbg->bb99|=(1 <<1 )<<bbg->bb79;bbg->bb79+=bb22;}};
{bbe bb22=bb1738[256 ].bb49.bb22;bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl))-
bb22){bbe bb170=bb1738[256 ].bb268.bb172;bbg->bb99|=(bb170<<bbg->bb79);
{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&0xff ));};{bbg->bb171[
bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));};};bbg->bb99=(bb126)bb170
>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79+=bb22-(8 *2 *bb12(bbl));}
bb54{bbg->bb99|=(bb1738[256 ].bb268.bb172)<<bbg->bb79;bbg->bb79+=bb22;
}};bb2301(bbg);bbm(1 +bbg->bb1992+10 -bbg->bb79<9 ){{bbe bb22=3 ;bbm(bbg
->bb79>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=1 <<1 ;bbg->bb99|=(bb170<<
bbg->bb79);{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&0xff ));};{
bbg->bb171[bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));};};bbg->
bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79+=bb22-(
8 *2 *bb12(bbl));}bb54{bbg->bb99|=(1 <<1 )<<bbg->bb79;bbg->bb79+=bb22;}};
{bbe bb22=bb1738[256 ].bb49.bb22;bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl))-
bb22){bbe bb170=bb1738[256 ].bb268.bb172;bbg->bb99|=(bb170<<bbg->bb79);
{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&0xff ));};{bbg->bb171[
bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));};};bbg->bb99=(bb126)bb170
>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79+=bb22-(8 *2 *bb12(bbl));}
bb54{bbg->bb99|=(bb1738[256 ].bb268.bb172)<<bbg->bb79;bbg->bb79+=bb22;
}};bb2301(bbg);}bbg->bb1992=7 ;}bbb bb1616(bbg,bb39,bb1290,bb1108)bb191
 *bbg;bb441*bb39;bb395 bb1290;bbe bb1108;{bb395 bb2012,bb2075;bbe
bb1786=0 ;bbm(bbg->bb123>0 ){bbm(bbg->bb977==2 )bb2311(bbg);bb2182(bbg,(
bb1734* )(&(bbg->bb1936)));;bb2182(bbg,(bb1734* )(&(bbg->bb1863)));;
bb1786=bb2382(bbg);bb2012=(bbg->bb1904+3 +7 )>>3 ;bb2075=(bbg->bb2117+3 +
7 )>>3 ;;bbm(bb2075<=bb2012)bb2012=bb2075;}bb54{;bb2012=bb2075=bb1290+5
;}bbm(bb1290+4 <=bb2012&&bb39!=(bbl* )0 ){bb2141(bbg,bb39,bb1290,bb1108
);}bb54 bbm(bb2075==bb2012){{bbe bb22=3 ;bbm(bbg->bb79>(bbe)(8 *2 *bb12(
bbl))-bb22){bbe bb170=(1 <<1 )+bb1108;bbg->bb99|=(bb170<<bbg->bb79);{{
bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&0xff ));};{bbg->bb171[
bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));};};bbg->bb99=(bb126)bb170
>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79+=bb22-(8 *2 *bb12(bbl));}
bb54{bbg->bb99|=((1 <<1 )+bb1108)<<bbg->bb79;bbg->bb79+=bb22;}};bb2267(
bbg,(bb460* )bb1738,(bb460* )bb2232);}bb54{{bbe bb22=3 ;bbm(bbg->bb79>
(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=(2 <<1 )+bb1108;bbg->bb99|=(bb170
<<bbg->bb79);{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&0xff ));}
;{bbg->bb171[bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));};};bbg->
bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79+=bb22-(
8 *2 *bb12(bbl));}bb54{bbg->bb99|=((2 <<1 )+bb1108)<<bbg->bb79;bbg->bb79
+=bb22;}};bb2406(bbg,bbg->bb1936.bb511+1 ,bbg->bb1863.bb511+1 ,bb1786+1
);bb2267(bbg,(bb460* )bbg->bb976,(bb460* )bbg->bb1678);};bb2243(bbg);
bbm(bb1108){bb2257(bbg);};}bbe bb2386(bbg,bb415,bb1136)bb191*bbg;bbs
bb415;bbs bb1136;{bbg->bb1643[bbg->bb605]=(bb126)bb415;bbg->bb1724[
bbg->bb605++]=(bb155)bb1136;bbm(bb415==0 ){bbg->bb976[bb1136].bb268.
bb426++;}bb54{bbg->bb2228++;bb415--;;bbg->bb976[bb2050[bb1136]+256 +1 ]
.bb268.bb426++;bbg->bb1678[((bb415)<256 ?bb1735[bb415]:bb1735[256 +((
bb415)>>7 )])].bb268.bb426++;}bb2(bbg->bb605==bbg->bb1121-1 );}bb41 bbb
bb2267(bbg,bb1045,bb1746)bb191*bbg;bb460*bb1045;bb460*bb1746;{bbs
bb415;bbe bb1136;bbs bb2285=0 ;bbs bb172;bbe bb1800;bbm(bbg->bb605!=0 )bb564
{bb415=bbg->bb1643[bb2285];bb1136=bbg->bb1724[bb2285++];bbm(bb415==0 ){
{bbe bb22=bb1045[bb1136].bb49.bb22;bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl))-
bb22){bbe bb170=bb1045[bb1136].bb268.bb172;bbg->bb99|=(bb170<<bbg->
bb79);{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&0xff ));};{bbg->
bb171[bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));};};bbg->bb99=(
bb126)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79+=bb22-(8 *2 *
bb12(bbl));}bb54{bbg->bb99|=(bb1045[bb1136].bb268.bb172)<<bbg->bb79;
bbg->bb79+=bb22;}};;}bb54{bb172=bb2050[bb1136];{bbe bb22=bb1045[bb172
+256 +1 ].bb49.bb22;bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=
bb1045[bb172+256 +1 ].bb268.bb172;bbg->bb99|=(bb170<<bbg->bb79);{{bbg->
bb171[bbg->bb184++]=((bb155)((bbg->bb99)&0xff ));};{bbg->bb171[bbg->
bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));};};bbg->bb99=(bb126)bb170
>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79+=bb22-(8 *2 *bb12(bbl));}
bb54{bbg->bb99|=(bb1045[bb172+256 +1 ].bb268.bb172)<<bbg->bb79;bbg->
bb79+=bb22;}};bb1800=bb2412[bb172];bbm(bb1800!=0 ){bb1136-=bb2331[
bb172];{bbe bb22=bb1800;bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl))-bb22){bbe
bb170=bb1136;bbg->bb99|=(bb170<<bbg->bb79);{{bbg->bb171[bbg->bb184++]
=((bb155)((bbg->bb99)&0xff ));};{bbg->bb171[bbg->bb184++]=((bb155)((
bb126)(bbg->bb99)>>8 ));};};bbg->bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(
bbl))-bbg->bb79);bbg->bb79+=bb22-(8 *2 *bb12(bbl));}bb54{bbg->bb99|=(
bb1136)<<bbg->bb79;bbg->bb79+=bb22;}};}bb415--;bb172=((bb415)<256 ?
bb1735[bb415]:bb1735[256 +((bb415)>>7 )]);;{bbe bb22=bb1746[bb172].bb49
.bb22;bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bb1746[bb172
].bb268.bb172;bbg->bb99|=(bb170<<bbg->bb79);{{bbg->bb171[bbg->bb184++
]=((bb155)((bbg->bb99)&0xff ));};{bbg->bb171[bbg->bb184++]=((bb155)((
bb126)(bbg->bb99)>>8 ));};};bbg->bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(
bbl))-bbg->bb79);bbg->bb79+=bb22-(8 *2 *bb12(bbl));}bb54{bbg->bb99|=(
bb1746[bb172].bb268.bb172)<<bbg->bb79;bbg->bb79+=bb22;}};bb1800=
bb2376[bb172];bbm(bb1800!=0 ){bb415-=bb2414[bb172];{bbe bb22=bb1800;
bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bb415;bbg->bb99|=(
bb170<<bbg->bb79);{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&
0xff ));};{bbg->bb171[bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));}
;};bbg->bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(bbl))-bbg->bb79);bbg->bb79
+=bb22-(8 *2 *bb12(bbl));}bb54{bbg->bb99|=(bb415)<<bbg->bb79;bbg->bb79
+=bb22;}};}};}bb108(bb2285<bbg->bb605);{bbe bb22=bb1045[256 ].bb49.
bb22;bbm(bbg->bb79>(bbe)(8 *2 *bb12(bbl))-bb22){bbe bb170=bb1045[256 ].
bb268.bb172;bbg->bb99|=(bb170<<bbg->bb79);{{bbg->bb171[bbg->bb184++]=
((bb155)((bbg->bb99)&0xff ));};{bbg->bb171[bbg->bb184++]=((bb155)((
bb126)(bbg->bb99)>>8 ));};};bbg->bb99=(bb126)bb170>>((bbe)(8 *2 *bb12(
bbl))-bbg->bb79);bbg->bb79+=bb22-(8 *2 *bb12(bbl));}bb54{bbg->bb99|=(
bb1045[256 ].bb268.bb172)<<bbg->bb79;bbg->bb79+=bb22;}};bbg->bb1992=
bb1045[256 ].bb49.bb22;}bb41 bbb bb2311(bbg)bb191*bbg;{bbe bb11=0 ;bbs
bb2324=0 ;bbs bb2268=0 ;bb108(bb11<7 )bb2268+=bbg->bb976[bb11++].bb268.
bb426;bb108(bb11<128 )bb2324+=bbg->bb976[bb11++].bb268.bb426;bb108(
bb11<256 )bb2268+=bbg->bb976[bb11++].bb268.bb426;bbg->bb977=(bb153)(
bb2268>(bb2324>>2 )?0 :1 );}bb41 bbs bb2367(bb172,bb22)bbs bb172;bbe bb22
;{bb920 bbs bb2302=0 ;bb564{bb2302|=bb172&1 ;bb172>>=1 ,bb2302<<=1 ;}
bb108(--bb22>0 );bb2 bb2302>>1 ;}bb41 bbb bb2301(bbg)bb191*bbg;{bbm(bbg
->bb79==16 ){{{bbg->bb171[bbg->bb184++]=((bb155)((bbg->bb99)&0xff ));};
{bbg->bb171[bbg->bb184++]=((bb155)((bb126)(bbg->bb99)>>8 ));};};bbg->
bb99=0 ;bbg->bb79=0 ;}bb54 bbm(bbg->bb79>=8 ){{bbg->bb171[bbg->bb184++]=
((bb153)bbg->bb99);};bbg->bb99>>=8 ;bbg->bb79-=8 ;}}bb41 bbb bb2257(bbg
)bb191*bbg;{bbm(bbg->bb79>8 ){{{bbg->bb171[bbg->bb184++]=((bb155)((bbg
->bb99)&0xff ));};{bbg->bb171[bbg->bb184++]=((bb155)((bb126)(bbg->bb99
)>>8 ));};};}bb54 bbm(bbg->bb79>0 ){{bbg->bb171[bbg->bb184++]=((bb153)bbg
->bb99);};}bbg->bb99=0 ;bbg->bb79=0 ;}bb41 bbb bb2316(bbg,bb39,bb22,
bb984)bb191*bbg;bb441*bb39;bbs bb22;bbe bb984;{bb2257(bbg);bbg->
bb1992=8 ;bbm(bb984){{{bbg->bb171[bbg->bb184++]=((bb155)(((bb126)bb22)&
0xff ));};{bbg->bb171[bbg->bb184++]=((bb155)((bb126)((bb126)bb22)>>8 ));
};};{{bbg->bb171[bbg->bb184++]=((bb155)(((bb126)~bb22)&0xff ));};{bbg
->bb171[bbg->bb184++]=((bb155)((bb126)((bb126)~bb22)>>8 ));};};}bb108(
bb22--){{bbg->bb171[bbg->bb184++]=( *bb39++);};}}
