/*
   'src_ipsec_pgpIPsecESP.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Fri Oct 12 22:15:16 2012
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
bba bb83 bb4;bb9{bb98=0 ,bb364=-12000 ,bb357=-11999 ,bb375=-11998 ,bb671=
-11997 ,bb797=-11996 ,bb724=-11995 ,bb871=-11994 ,bb788=-11992 ,bb806=-
11991 ,bb668=-11990 ,bb712=-11989 ,bb833=-11988 ,bb637=-11987 ,bb672=-
11986 ,bb773=-11985 ,bb851=-11984 ,bb623=-11983 ,bb617=-11982 ,bb764=-
11981 ,bb903=-11980 ,bb791=-11979 ,bb722=-11978 ,bb843=-11977 ,bb583=-
11976 ,bb844=-11975 ,bb767=-11960 ,bb678=-11959 ,bb689=-11500 ,bb732=-
11499 ,bb856=-11498 ,bb798=-11497 ,bb879=-11496 ,bb869=-11495 ,bb828=-
11494 ,bb774=-11493 ,bb858=-11492 ,bb885=-11491 ,bb705=-11490 ,bb745=-
11489 ,bb702=-11488 ,bb892=-11487 ,bb872=-11486 ,bb706=-11485 ,bb645=-
11484 ,bb902=-11483 ,bb769=-11482 ,bb905=-11481 ,bb846=-11480 ,bb759=-
11479 ,bb644=-11478 ,bb717=-11477 ,bb657=-11476 ,bb631=-11475 ,bb864=-
11474 ,bb789=-11473 ,bb707=-11472 ,bb809=-11460 ,bb652=-11450 ,bb740=-
11449 ,bb710=-11448 ,bb733=-11447 ,bb790=-11446 ,bb635=-11445 ,bb887=-
11444 ,bb824=-11443 ,bb842=-11440 ,bb865=-11439 ,bb801=-11438 ,bb800=-
11437 ,bb673=-11436 ,bb688=-11435 ,bb620=-11420 ,bb531=-11419 ,bb571=-
11418 ,bb685=-11417 ,bb835=-11416 ,bb667=-11415 ,bb795=-11414 ,bb731=-
11413 ,bb633=-11412 ,bb823=-11411 ,bb674=-11410 ,bb636=-11409 ,bb708=-
11408 ,bb900=-11407 ,bb898=-11406 ,bb803=-11405 ,bb720=-11404 ,bb658=-
11403 ,bb761=-11402 ,bb634=-11401 ,bb679=-11400 ,bb878=-11399 ,bb754=-
11398 ,bb762=-11397 ,bb683=-11396 ,bb866=-11395 ,bb889=-11394 ,bb615=-
11393 ,bb894=-11392 ,bb692=-11391 ,bb784=-11390 ,bb727=-11389 ,bb711=-
11388 ,bb749=-11387 ,bb904=-11386 ,bb627=-11385 ,bb700=-11384 ,bb786=-
11383 ,bb648=-11382 ,bb814=-11381 ,bb736=-11380 ,bb785=-11379 ,bb669=-
11378 ,bb752=-11377 ,bb808=-11376 ,bb709=-11375 ,bb763=-11374 ,bb699=-
11373 ,bb897=-11372 ,bb862=-11371 ,bb802=-11370 ,bb777=-11369 ,bb841=-
11368 ,bb756=-11367 ,bb794=-11366 ,bb719=-11365 ,bb860=-11364 ,bb845=-
11363 ,bb388=-11350 ,bb883=bb388,bb714=-11349 ,bb834=-11348 ,bb836=-11347
,bb643=-11346 ,bb649=-11345 ,bb906=-11344 ,bb822=-11343 ,bb779=-11342 ,
bb680=-11341 ,bb770=-11340 ,bb901=-11339 ,bb398=-11338 ,bb663=-11337 ,
bb687=bb398,bb799=-11330 ,bb817=-11329 ,bb781=-11328 ,bb632=-11327 ,bb718
=-11326 ,bb650=-11325 ,bb821=-11324 ,bb698=-11320 ,bb819=-11319 ,bb859=-
11318 ,bb690=-11317 ,bb626=-11316 ,bb681=-11315 ,bb825=-11314 ,bb723=-
11313 ,bb641=-11312 ,bb642=-11300 ,bb741=-11299 ,bb796=-11298 ,bb703=-
11297 ,bb852=-11296 ,bb811=-11295 ,bb832=-11294 ,bb654=-11293 ,bb847=-
11292 ,bb882=-11291 ,bb618=-11290 ,bb804=-11289 ,bb857=-11288 ,bb849=-
11287 ,bb734=-11286 ,bb653=-11285 ,bb646=-11284 ,bb812=-11283 ,bb738=-
11282 ,bb704=-11281 ,bb661=-11280 ,bb713=-11279 ,bb701=-11250 ,bb850=-
11249 ,bb848=-11248 ,bb748=-11247 ,bb737=-11246 ,bb805=-11245 ,bb778=-
11244 ,bb755=-11243 ,bb621=-11242 ,bb839=-11240 ,bb651=-11239 ,bb729=-
11238 ,bb792=-11237 ,bb677=-11150 ,bb855=-11100 ,bb820=-11099 ,bb655=-
11098 ,bb744=-11097 ,bb782=-11096 ,bb793=-11095 ,bb768=-11094 ,bb628=-
11093 ,bb830=-11092 ,bb899=-11091 ,bb666=-11090 ,bb877=-11089 ,bb884=-
11088 ,bb853=-11087 ,bb638=-11086 ,bb780=-11085 ,bb783=-11050 ,bb751=-
11049 ,bb691=-10999 ,bb639=-10998 ,bb656=-10997 ,bb753=-10996 ,bb893=-
10995 ,bb682=-10994 ,bb694=-10993 ,bb840=-10992 ,bb771=-10991 ,bb735=-
10990 ,bb630=-10989 ,bb907=-10988 ,bb728=-10979 ,bb660=-10978 ,bb765=-
10977 ,bb873=-10976 ,bb695=-10975 ,bb826=-10974 ,};bb9{bb550=1 ,};bbb*
bb496(bbd bb1213,bbd bb372);bb4 bb458(bbb*bb970);bba bbi bb991*bb989;
bba bbi bb1026*bb1027;bba bbi bb993*bb1021;bba bbi bb1000*bb1010;bba
bbi bb1012*bb1023;bba bbi bb990*bb986;bba bb9{bb552=0 ,bb581=1 ,bb593=2
,bb813=3 ,bb585=4 ,bb570=5 ,bb575=6 ,bb562=7 ,bb579=9 ,}bb422;bba bb9{bb606
=0 ,bb992,bb598,bb1009,bb930,bb923,bb926,bb916,bb924,bb922,bb915,}
bb519;bba bbi bb455{bb1 bb74;bbd bb127;bbd bb181;bbi bb455*bb94;}bbx;
bb4 bb465(bbx*bb670,bbd bb909,bbx*bb696,bbd bb895,bbd bb538);bb4 bb532
(bbx*bbj,bbd bb92,bbh bbb*bb95,bbd bb47);bb4 bb577(bbx*bbj,bbd bb92,
bbb*bb131,bbd bb47);bbu bb827(bbx*bbj,bbd bb92,bbh bbb*bb95,bbd bb47);
bb4 bb2056(bbx*bb304,bbd*bb103);bb4 bb2115(bbx*bb85,bbu bb177,bbd
bb489,bb519 bb154,bbh bbf*bb1312,bbf*bb130,bb422 bb414,bbf*bb557,bbd
bb103,bbd bb491,bbx*bb58);bb4 bb2043(bbx*bb85,bbu bb177,bb519 bb154,
bbh bbf*bb1312,bb422 bb414,bbf*bb557,bbd*bb484,bbd*bb463,bbd*bb535,
bbx*bb58);
#define bb951 bb53(0x0800)
#define bb1138 bb53(0x0806)
#define bb937 bb53(0x01f4)
#define bb945 bb53(0x1194)
#define bb1131 bb53(0x4000)
#define bb1137 bb53(0x2000)
#define bb1107 bb53(0x1FFF)
#define bb1064( bb8) (( bb8) & bb53(0x2000 | 0x1FFF))
#define bb1022( bb8) ((( bb196( bb8)) & 0x1FFF) << 3)
#define bb979( bb8) ((( bb8) & bb53(0x1FFF)) == 0)
#define bb494( bb8) (( bb8) & bb53(0x2000))
#define bb987( bb8) (!( bb494( bb8)))
#pragma pack(push, 1)
bba bbi{bbf bb371[6 ];bbf bb1006[6 ];bbk bb373;}bb366, *bb376;bba bbi{
bbf bb446[6 ];bbk bb373;}bb1082, *bb1091;bba bbi{bbf bb938:4 ;bbf bb1088
:4 ;bbf bb1048;bbk bb361;bbk bb881;bbk bb567;bbf bb1002;bbf bb292;bbk
bb610;bbd bb310;bbd bb203;}bb326, *bb312;bba bbi{bbk bb1085;bbk bb1092
;bbf bb1047;bbf bb1039;bbk bb1057;bbf bb1074[6 ];bbd bb1035;bbf bb1090
[6 ];bbd bb1061;}bb1071, *bb1077;
#pragma pack(pop)
bba bbi{bbk bb288;bbk bb427;bbk bb1005;bbk bb319;}bb416, *bb341;bba
bbi{bbk bb288;bbk bb592;bbd bb549;bbd bb918;bbf bb92;bbf bb172;bbk
bb158;bbk bb319;bbk bb1019;}bb487, *bb318;bba bbi{bbf bb1075;bbf
bb1067;bbf bb1058;bbf bb1036;bbd bb1060;bbk bb1072;bbk bb372;bbd
bb1032;bbd bb1078;bbd bb1063;bbd bb1056;bbf bb1076[16 ];bbf bb1046[64 ]
;bbf bb25[128 ];bbf bb1033[64 ];}bb1080, *bb1086;bba bbi{bbd bb310;bbd
bb203;bbf bb910;bbf bb292;bbk bb919;}bb612, *bb569;
#if defined( _WIN32)
#define bb53( bbc) (((( bbc) & 0XFF00) >> 8) | ((( bbc) & 0X00FF) <<  \
8))
#define bb196( bbc) ( bb53( bbc))
#define bb445( bbc) (((( bbc) & 0XFF000000) >> 24) | ((( bbc) &  \
0X00FF0000) >> 8) | ((( bbc) & 0X0000FF00) << 8) | ((( bbc) &  \
0X000000FF) << 24))
#define bb499( bbc) ( bb445( bbc))
#endif
bbk bb921(bbh bbb*bb295);bbk bb886(bbh bbb*bb510,bbe bb22);bb4 bb595(
bbx*bb85,bbf bb102,bbx*bb58);bb4 bb686(bbx*bb85,bbu bb177,bbf*bb408);
bb4 bb955(bbx*bb58,bbf*bb389);bb4 bb941(bbh bbf*bb389,bbx*bb58);bb4
bb541(bbx*bb51,bbf bb102,bbd*bb939);bb4 bb929(bbx*bb85,bbf bb102,bbf
bb408,bbx*bb58);bbd bb517(bbx*bb51);bbk bb534(bbx*bb51);bbb bb527(bbk
bb151,bbx*bb51);bbb bb536(bbx*bb51);bbb bb968(bbx*bb51,bbd*bb26);bbb
bb999(bbx*bb51,bbd*bb26);bbb bb1020(bbx*bb51,bbd bb26);bbb bb931(bbx*
bb51,bbd bb26);bbb bb983(bbx*bb51);bbu bb1013(bbf*bb51);bb9{bb1135=-
5000 ,bb1104=-4000 ,bb997=-4999 ,bb988=-4998 ,bb1011=-4997 ,bb981=-4996 ,
bb1141=-4995 ,bb1087=-4994 ,bb1096=-4993 ,bb1024=-4992 ,bb1031=-4991 };bb4
bb1129(bb4 bb1133,bbd bb1116,bbl*bb1100);bba bb9{bb407,bb1497,}bb296;
bbk bb1214(bb296 bb758,bbh bbf*bb447);bbd bb544(bb296 bb758,bbh bbf*
bb447);bbb bb1158(bbk bb159,bb296 bb555,bbf bb439[2 ]);bbb bb972(bbd
bb159,bb296 bb555,bbf bb439[4 ]);
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbi{bbf bb417[8 *16 ];}bb917;bbb bb1750(bb917*bbj,bbh bbb*bb71);bbb
bb1930(bb917*bbj,bbh bbb*bb71);bbb bb1245(bb917*bbj,bbb*bb14,bbh bbb*
bb5);bba bbi{bb917 bb2033,bb2032,bb2031;}bb697;bbb bb2100(bb697*bbj,
bbh bbb*bb1304);bbb bb2136(bb697*bbj,bbh bbb*bb1304);bbb bb2042(bb697
 *bbj,bbb*bb14,bbh bbb*bb5);bbb bb2176(bb697*bbj,bbb*bb14,bbh bbb*bb5
);
#ifdef __cplusplus
}
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
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbi{bbd bb417[2 *16 ];}bb982;bbb bb2000(bb982*bbj,bbh bbb*bb71);bbb
bb2145(bb982*bbj,bbh bbb*bb71);bbb bb1981(bb982*bbj,bbb*bb14,bbh bbb*
bb5);
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbi{bbq bb456;bbd bb417[4 * (16 +1 )];}bb574;bbb bb1591(bb574*bbj,
bbh bbb*bb71,bbq bb143);bbb bb2018(bb574*bbj,bbh bbb*bb71,bbq bb143);
bbb bb1241(bb574*bbj,bbb*bb14,bbh bbb*bb5);
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbb( *bb467)(bbb*bbj,bbb*bb14,bbh bbb*bb5);bba bbi{bbe bb57;bbq
bb38;bbq bb143;bbf bb150[16 ];bbq bb88;bbf bb130[16 ];bb467 bb179;bbf
bb154[512 ];}bb431;bbb bb2066(bb431*bbj,bbe bb57);bbb bb2064(bb431*bbj
,bbe bb2217,bbh bbb*bb71,bbh bbb*bb1273);bbb bb2299(bb431*bbj,bbe bb57
,bbh bbb*bb71,bbh bbb*bb1273);bbb bb1597(bb431*bbj,bbb*bb14,bb93 bb176
,bbh bbb*bb5,bbq bb10);bbu bb2096(bb431*bbj,bbb*bb14,bb93 bb176);bbb
bb2389(bbe bb57,bbh bbb*bb71,bbh bbb*bb1273,bbb*bb2124,bb93 bb176,bbh
bbb*bb5,bbq bb10);bb82 bb2264(bbe bb154);bb82 bb2240(bbe bb1655);bb82
bb2413(bbe bb57);bbu bb2511(bbe bb57);
#ifdef __cplusplus
}
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
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbi{bbd bb10;bbd bb23[5 ];bbf bb101[64 ];}bb530;bbb bb1795(bb530*
bbj);bbb bb1288(bb530*bbj,bbh bbb*bb5,bbq bb10);bbb bb1803(bb530*bbj,
bbb*bb14);bba bbi{bbd bb10;bbd bb23[8 ];bbf bb101[64 ];}bb529;bbb bb1819
(bb529*bbj);bbb bb1291(bb529*bbj,bbh bbb*bb5,bbq bb10);bbb bb1815(
bb529*bbj,bbb*bb14);bba bbi{bbd bb10;bb56 bb23[8 ];bbf bb101[128 ];}
bb461;bbb bb1809(bb461*bbj);bbb bb1227(bb461*bbj,bbh bbb*bb5,bbq bb10
);bbb bb1835(bb461*bbj,bbb*bb14);bba bb461 bb925;bbb bb1798(bb925*bbj
);bbb bb1811(bb925*bbj,bbb*bb14);bbb bb1905(bbb*bb14,bbh bbb*bb5,bbq
bb10);bbb bb1866(bbb*bb14,bbh bbb*bb5,bbq bb10);bbb bb1849(bbb*bb14,
bbh bbb*bb5,bbq bb10);bbb bb1929(bbb*bb14,bbh bbb*bb5,bbq bb10);bbb
bb2015(bbb*bb14,bb82 bb5);bbb bb1964(bbb*bb14,bb82 bb5);bbb bb2024(
bbb*bb14,bb82 bb5);bbb bb2016(bbb*bb14,bb82 bb5);
#ifdef __cplusplus
}
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
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbi{bbd bb10;bbd bb23[5 ];bbf bb101[64 ];}bb528;bbb bb1805(bb528*
bbj);bbb bb1351(bb528*bbj,bbh bbb*bb498,bbq bb10);bbb bb1838(bb528*
bbj,bbb*bb14);bbb bb1872(bbb*bb14,bbh bbb*bb5,bbq bb10);bbb bb2001(
bbb*bb14,bb82 bb5);
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbb( *bb1054)(bbb*bbj);bba bbb( *bb807)(bbb*bbj,bbh bbb*bb5,bbq
bb10);bba bbb( *bb776)(bbb*bbj,bbb*bb14);bba bbi{bbe bb129;bbq bb38;
bbq bb393;bb1054 bb888;bb807 bb179;bb776 bb725;}bb449;bbb bb1855(
bb449*bbj,bbe bb129);bba bbi{bb449 bbn;bbf bbt[256 -bb12(bb449)];}
bb453;bbb bb1983(bb453*bbj,bbe bb129);bbb bb1990(bb453*bbj);bbb bb2020
(bb453*bbj,bbe bb129);bbb bb1982(bb453*bbj,bbh bbb*bb5,bbq bb10);bbb
bb1975(bb453*bbj,bbb*bb14);bbb bb1988(bbe bb129,bbb*bb14,bbh bbb*bb5,
bbq bb10);bbb bb2048(bbe bb129,bbb*bb14,bb82 bb5);bb82 bb1969(bbe
bb129);
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbi{bb449 bbn;bbf bb537[(512 -bb12(bb449))/2 ];bbf bb1340[(512 -bb12
(bb449))/2 ];}bb492;bbb bb1961(bb492*bbj,bbe bb587);bbb bb2005(bb492*
bbj,bbh bbb*bb71,bbq bb143);bbb bb2107(bb492*bbj,bbe bb587,bbh bbb*
bb71,bbq bb143);bbb bb1987(bb492*bbj,bbh bbb*bb5,bbq bb10);bbb bb2006
(bb492*bbj,bbb*bb14);bbb bb2106(bbe bb587,bbh bbb*bb71,bbq bb143,bbb*
bb14,bbh bbb*bb5,bbq bb10);bbb bb2188(bbe bb587,bb82 bb71,bbb*bb14,
bb82 bb5);
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbi{bb363 bb2116;bbq bb10;bbf bb101[16 ];bbf bb1927[16 ];bbf bb1926
[16 ];bbf bb1842[16 ];}bb944;bbb bb2041(bb944*bbj,bbh bbb*bb71,bbq bb143
);bbb bb2092(bb944*bbj,bbh bbb*bb5,bbq bb10);bbb bb2102(bb944*bbj,bbb
 *bb14);
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
bbr"\x43"{
#endif
bba bbb( *bb1865)(bbb*bbj,bbh bbb*bb71,bbq bb143);bba bbi{bbe bb129;
bbq bb38;bbq bb393;bb1865 bb888;bb807 bb179;bb776 bb725;}bb2027;bba
bbi{bb2027 bbn;bbf bbt[512 ];}bb524;bbb bb2143(bb524*bbj,bbe bb129);
bbb bb2167(bb524*bbj,bbh bbb*bb71,bbq bb143);bbb bb1833(bb524*bbj,bbe
bb129,bbh bbb*bb71,bbq bb143);bbb bb1242(bb524*bbj,bbh bbb*bb5,bbq
bb10);bbb bb1816(bb524*bbj,bbb*bb14);bbb bb2164(bbe bb129,bbh bbb*
bb71,bbq bb143,bbb*bb14,bbh bbb*bb5,bbq bb10);bbb bb2193(bbe bb129,
bb82 bb71,bbb*bb14,bb82 bb5);bb82 bb2202(bbe bb129);
#ifdef __cplusplus
}
#endif
bb41 bbf bb2349[]={0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 };bb4 bb2056(bbx*
bb304,bbd*bb103){bb4 bb18;bbd bb381;bbf bb74[8 ]; *bb103=0 ;bb18=bb541(
bb304,50 ,&bb381);bbm(((bb18)!=bb98))bb2 bb18;bb577(bb304,bb381,bb74,8
); *bb103=bb544(bb407,&(bb74[0 ]));bb2 bb18;}bb4 bb2115(bbx*bb85,bbu
bb177,bbd bb489,bb519 bb154,bbh bbf*bb1312,bbf*bb130,bb422 bb414,bbf*
bb557,bbd bb103,bbd bb491,bbx*bb58){bbd bb381;bbd bb163;bbd bb2362;
bbk bb151;bbd bb1298;bbd bb1744;bbd bb169;bbd bb147;bbd bb482;bbd
bb500;bbd bb1030;bbd bb156;bbd bb576;bbf bb932;bbf bb578;bbd bb143;
bbd bb2036;bbd bb38;bbd bb965;bbf bb189=0 ;bbf bb420=0 ;bbf bb978[32 ];
bbf bb908[32 ];bbf bb2177[32 ];bbf bb2425[64 ];bbf bb561[8 +64 ];bbf*
bb1914=bb91;bbf*bb602=bb91;bbx*bb73=bb91;bbx*bb46=bb91;bb4 bb18=bb98;
bb431*bb539=bb91;bb524*bb525=bb91;bbe bb721=0x100000 ,bb405;bb151=
bb534(bb85);bb297(bb154){bb15 bb598:bb143=24 ;bb38=8 ;bb721|=0x11 ;bb21;
bb15 bb930:bb143=16 ;bb38=16 ;bb721|=0x80 ;bb21;bb15 bb924:bb143=16 ;bb38
=16 ;bb721|=0x91 ;bb21;bb15 bb922:bb143=24 ;bb38=16 ;bb721|=0x92 ;bb21;
bb15 bb915:bb143=32 ;bb38=16 ;bb721|=0x93 ;bb21;bb15 bb923:bb143=16 ;bb38
=16 ;bb721|=0x20 ;bb21;bb15 bb926:bb143=24 ;bb38=16 ;bb721|=0x21 ;bb21;
bb15 bb916:bb143=32 ;bb38=16 ;bb721|=0x22 ;bb21;bb15 bb606:bb143=0 ;bb38=
0 ;bb721|=0x00 ;bb21;bb421:bb2 bb571;}bb539=bb128(bb12( *bb539));bb2066
(bb539,bb721);bb686(bb85,bb177,&bb932);bb381=bb517(bb85);bb18=bb595(
bb85,50 ,bb58);bbm(((bb18)!=bb98))bb107 bb164;bbm(bb177)bb151+=bb381;
bb2036=bb151-bb381;bb1298=bb177?0 :bb381;bb965=bb2036;bb1744=bb381+8 +
bb38;bbm(bb154!=bb606){bb1 bb1259=bb91;bb2064(bb539,0x0100 |0x1000000 ,
bb1312,bb130?bb130:bb2349);bbm(((bb18)!=bb98))bb107 bb164;bb73=bb85;
bb46=bb58;bb482=0 ;bb500=0 ;{bb156=bb73->bb127;bb576=0 ;bb108(bb156<=
bb1298&&bb73->bb94){bb576=bb156;bb73=bb73->bb94;bb156+=bb73->bb127;}
bb169=bb1298-bb576;bbm(bb156<=bb1298){bb18=bb375;bb107 bb164;}}{bb156
=bb46->bb181;bb576=0 ;bb108(bb156<=bb1744&&bb46->bb94){bb576=bb156;
bb46=bb46->bb94;bb156+=bb46->bb181;}bb147=bb1744-bb576;bbm(bb156<=
bb1744){bb18=bb375;bb107 bb164;}}bb108(bb965>=bb38&&bb73){bbm(!bb46){
bb18=bb375;bb107 bb164;}bb482=0 ;bb500=0 ;bbm(bb73->bb127-bb169>=bb38&&
bb46->bb181-bb147>=bb38){bb1030=((bb73->bb127-bb169)<(bb46->bb181-
bb147)?(bb73->bb127-bb169):(bb46->bb181-bb147));bb1030=bb1030-bb1030%
bb38;{bbq bb393;bb1597(bb539,&bb46->bb74[bb147],&bb393,&bb73->bb74[
bb169],bb1030);bb1259=&bb46->bb74[bb147]+bb393;}bb147+=bb1030;bb169+=
bb1030;bb965-=bb1030;bbm(bb169==bb73->bb127){bb73=bb73->bb94;bb169=0 ;
}bbm(bb147==bb46->bb181){bb46->bb127=bb46->bb181;bb46=bb46->bb94;
bb147=0 ;}}bb54{bbm(bb73->bb127-bb169<bb38){bb1914=bb978;bb108(bb965>=
bb38-bb482&&bb482<bb38&&bb73){bb156=bb73->bb127-bb169;bbm(bb156>bb38-
bb482)bb156=bb38-bb482;bb81(&bb978[bb482],&bb73->bb74[bb169],bb156);
bb482+=bb156;bb169+=bb156;bb965-=bb156;bbm(bb169==bb73->bb127){bb73=
bb73->bb94;bb169=0 ;}}}bb54{bb1914=&bb73->bb74[bb169];bb482=bb38;bb169
+=bb38;bb965-=bb38;}bb602=bb46->bb181-bb147<bb38?bb908:&bb46->bb74[
bb147];bbm(bb482==bb38){bb482=0 ;{bbq bb393;bb1597(bb539,bb602,&bb393,
bb1914,bb38);bb1259=bb602+bb393;}bbm(bb602==bb908){bb500=bb46->bb181-
bb147;bb81(&bb46->bb74[bb147],bb908,bb500);bb46->bb127=bb46->bb181;
bb46=bb46->bb94;bbm(!bb46){bb18=bb375;bb107 bb164;}bb81(bb46->bb74,&
bb908[bb500],bb38-bb500);bb147=bb38-bb500;}bb54 bb147+=bb38;}}}{bbm(
bb965){bbm(!bb73){bb18=bb1031;bb107 bb164;}bbm(bb73->bb127-bb169<
bb965){bb81(&bb978[bb482],&bb73->bb74[bb169],bb73->bb127-bb169);bb482
+=bb73->bb127-bb169;bb965-=bb73->bb127-bb169;bb73=bb73->bb94;bb81(&
bb978[bb482],bb73->bb74,bb965);}bb54 bb81(&bb978[bb482],&bb73->bb74[
bb169],bb965);bb482+=bb965;}bbm(!bb46){bb18=bb375;bb107 bb164;}bb602=
bb46->bb181-bb147<bb38?bb908:&bb46->bb74[bb147];bbm(bb482<bb38-1 ){
bb578=bb38-bb482-2 ;bb90(bb163=0 ;bb163<bb578;bb163++)bb978[bb163+bb482
]=bb163+1 ;bb978[bb482+bb578]=bb578;bb978[bb482+bb578+1 ]=bb932;{bbq
bb393;bb1597(bb539,bb602,&bb393,bb978,bb38);bb1259=bb602+bb393;}bbm(
bb602==bb908){bb500=bb46->bb181-bb147;bb81(&bb46->bb74[bb147],bb908,
bb500);bb46->bb127=bb46->bb181;bb46=bb46->bb94;bb81(bb46->bb74,&bb908
[bb500],bb38-bb500);bb147=bb38-bb500;}bb54 bb147+=bb38;}bb54{bb978[
bb38-1 ]=1 ;{bbq bb393;bb1597(bb539,bb602,&bb393,bb978,bb38);bb1259=
bb602+bb393;}bbm(bb602==bb908){bb500=bb46->bb181-bb147;bb81(&bb46->
bb74[bb147],bb908,bb500);bb46->bb127=bb46->bb181;bb46=bb46->bb94;bb81
(bb46->bb74,&bb908[bb500],bb38-bb500);bb147=bb38-bb500;}bb54 bb147+=
bb38;bb602=bb46->bb181-bb147<bb38?bb908:&bb46->bb74[bb147];{bb90(
bb163=0 ;bb163<bb38-2 ;bb163++)bb978[bb163]=bb163+2 ;bb578=bb38-1 ;bb978[
bb38-2 ]=bb578;bb978[bb38-1 ]=bb932;{bbq bb393;bb1597(bb539,bb602,&
bb393,bb978,bb38);bb1259=bb602+bb393;}}bbm(bb602==bb908){bb500=bb46->
bb181-bb147;bb81(&bb46->bb74[bb147],bb908,bb500);bb46->bb127=bb46->
bb181;bb46=bb46->bb94;bb81(bb46->bb74,&bb908[bb500],bb38-bb500);bb147
=bb38-bb500;}bb54 bb147+=bb38;}}bb151+=bb578+2 +8 +bb38;bb46->bb127=
bb147;bb31(bb1259);bb2096(bb539,bb1259,bb91);}bb54{bb18=bb465(bb85,
bb1298,bb58,bb381+8 ,bb2036);bbm(((bb18)!=bb98))bb107 bb164;bb2362=
bb381+8 +bb2036;bb578=3 -(bb2036+2 +3 )%4 ;bb90(bb163=0 ;bb163<bb578;bb163
++)bb2177[bb163]=bb163+1 ;bb2177[bb578]=bb578;bb2177[bb578+1 ]=bb932;
bb18=bb532(bb58,bb2362,bb2177,bb578+2 );bbm(((bb18)!=bb98))bb107 bb164
;bb151+=bb578+2 +8 ;}bb972(bb103,bb407,&bb561[0 ]);bb972(bb491,bb407,&
bb561[4 ]);bb81(&bb561[8 ],bb130?bb130:bb2349,bb38);bbm(bb130&&bb602)bb81
(bb130,bb602,bb38);bb18=bb532(bb58,bb381,bb561,8 +bb38);bbm(((bb18)!=
bb98))bb107 bb164;bb18=bb595(bb85,50 ,bb58);bbm(((bb18)!=bb98))bb107
bb164;bbm(bb177)bb931(bb58,bb489);bb527(bb151,bb58);bbm(bb414!=bb552){
bb297(bb414){bb15 bb581:bb189=16 ;bb420=16 ;bb405=0x1000 |0x10 ;bb21;bb15
bb593:bb189=20 ;bb420=20 ;bb405=0x1000 |0x20 ;bb21;bb15 bb570:bb189=32 ;
bb420=32 ;bb405=0x1000 |0x21 ;bb21;bb15 bb575:bb189=48 ;bb420=48 ;bb405=
0x1000 |0x22 ;bb21;bb15 bb562:bb189=64 ;bb420=64 ;bb405=0x1000 |0x23 ;bb21;
bb15 bb579:bb189=16 ;bb420=16 ;bb405=0x2000 ;bb21;bb15 bb585:bb189=20 ;
bb420=20 ;bb405=0x1000 |0x80 ;bb21;bb15 bb813:bb189=20 ;bb420=20 ;bb405=
0x1000 |0x30 ;bb21;bb421:bb18=bb531;bb107 bb164;}bbm(bb189>12 )bb189=12 ;
bb525=bb128(bb12( *bb525));bb1833(bb525,bb405,bb557,(bbq)bb420);bb147
=bb381;bb46=bb58;bb108(bb147>bb46->bb127){bb147-=bb46->bb127;bb46=
bb46->bb94;}bb156=bb46->bb127-bb147;bb1242(bb525,bb46->bb74+bb147,
bb156);bb108(bb156<bb151-bb381){bb46=bb46->bb94;bb156+=bb46->bb127;
bb1242(bb525,bb46->bb74,bb46->bb127);}bb1816(bb525,bb2425);bb18=bb532
(bb46,bb46->bb127,bb2425,bb189);bbm(((bb18)!=bb98))bb107 bb164;bb151
+=bb189;bb527(bb151,bb58);}bb536(bb58);bb164:bbm(bb539)bb105(bb539);
bbm(bb525)bb105(bb525);bb2 bb18;}bb4 bb2043(bbx*bb85,bbu bb177,bb519
bb154,bbh bbf*bb1312,bb422 bb414,bbf*bb557,bbd*bb484,bbd*bb463,bbd*
bb535,bbx*bb58){bbd bb381;bbd bb491=0 ;bbk bb151;bbd bb169;bbd bb147;
bbd bb482;bbd bb500;bbd bb1030;bbd bb1298;bbd bb1744;bbd bb156;bbd
bb576;bbd bb143;bbd bb38;bbd bb965;bbf bb578;bbf bb932;bbf bb189=0 ;
bbf bb420=0 ;bbf*bb130=bb91;bbf bb1840[64 ];bbf bb978[32 ];bbf bb908[32 ]
;bbf bb561[8 +64 ];bbf*bb1914=bb91;bbf*bb602=bb91;bbx*bb73=bb91;bbx*
bb46=bb91;bbx*bb470=bb91;bb4 bb18=bb98;bb431*bb539=bb91;bb524*bb525=
bb91;bbe bb721=0x100000 ,bb405;bb151=bb534(bb85);bb297(bb154){bb15
bb598:bb143=24 ;bb38=8 ;bb721|=0x11 ;bb21;bb15 bb930:bb143=16 ;bb38=16 ;
bb721|=0x80 ;bb21;bb15 bb924:bb143=16 ;bb38=16 ;bb721|=0x91 ;bb21;bb15
bb922:bb143=24 ;bb38=16 ;bb721|=0x92 ;bb21;bb15 bb915:bb143=32 ;bb38=16 ;
bb721|=0x93 ;bb21;bb15 bb923:bb143=16 ;bb38=16 ;bb721|=0x20 ;bb21;bb15
bb926:bb143=24 ;bb38=16 ;bb721|=0x21 ;bb21;bb15 bb916:bb143=32 ;bb38=16 ;
bb721|=0x22 ;bb21;bb15 bb606:bb143=0 ;bb38=0 ;bb721|=0x00 ;bb21;bb421:bb2
bb571;}bb539=bb128(bb12( *bb539));bb2066(bb539,bb721);bb18=bb541(bb85
,50 ,&bb381);bbm(((bb18)!=bb98))bb107 bb164;bb577(bb85,bb381,bb561,8 +
bb38);bbm( *bb463<(bb12( *bb484)<<3 )-1 ) *bb463=(bb12( *bb484)<<3 )-1 ; *
bb535= *bb463-(bb12( *bb484)<<3 )+1 ;bb491=bb544(bb407,&bb561[4 ]);bbm(
bb491< *bb535){bb18=bb988;bb107 bb164;}bbm(bb491<= *bb463&& *bb484&1
<<(bb491- *bb535)){bb18=bb997;bb107 bb164;}bbm(bb414!=bb552){bb297(
bb414){bb15 bb581:bb189=16 ;bb420=16 ;bb405=0x1000 |0x10 ;bb21;bb15 bb593
:bb189=20 ;bb420=20 ;bb405=0x1000 |0x20 ;bb21;bb15 bb570:bb189=32 ;bb420=
32 ;bb405=0x1000 |0x21 ;bb21;bb15 bb575:bb189=48 ;bb420=48 ;bb405=0x1000 |
0x22 ;bb21;bb15 bb562:bb189=64 ;bb420=64 ;bb405=0x1000 |0x23 ;bb21;bb15
bb579:bb189=16 ;bb420=16 ;bb405=0x2000 ;bb21;bb15 bb585:bb189=20 ;bb420=
20 ;bb405=0x1000 |0x80 ;bb21;bb15 bb813:bb189=20 ;bb420=20 ;bb405=0x1000 |
0x30 ;bb21;bb421:bb18=bb531;bb107 bb164;}bbm(bb189>12 )bb189=12 ;bb525=
bb128(bb12( *bb525));bb1833(bb525,bb405,bb557,bb420);bb73=bb85;bb169=
bb381;bb156=bb73->bb127>(bbd)(bb151-bb189)?bb151-bb381-bb189:bb73->
bb127-bb169;bb1242(bb525,bb73->bb74+bb169,bb156);bb169+=bb156;bb108(
bb156<bb151-bb381-bb189){bb73=bb73->bb94;bb576=bb156;bb156+=bb73->
bb127;bb169=bb156<=bb151-bb381-bb189?bb73->bb127:bb151-bb381-bb189-
bb576;bb1242(bb525,bb73->bb74,bb169);}bb1816(bb525,bb1840);bbm(!bb827
(bb85,bb151-bb189,bb1840,bb189)){bb18=bb1011;bb107 bb164;}bb151-=
bb189;bbm(bb169+bb189>bb73->bb127)bb73->bb94->bb127=0 ;bb73->bb127=
bb169;}bbm(bb154!=bb606){bb1 bb1259=bb91;bb130=(bbf* )bb496(bb38,
bb550);bbm(!bb130){bb18=bb357;bb107 bb164;}bb81(bb130,&bb561[8 ],bb38);
bb2064(bb539,0x0200 |0x1000000 ,bb1312,bb130);bb73=bb85;bb46=bb58;bb482
=0 ;bb500=0 ;bb1298=bb381+8 +bb38;bb965=bb151-bb1298;bb1744=bb177?0 :
bb381;{bb156=bb73->bb127;bb576=0 ;bb108(bb156<=bb1298&&bb73->bb94){
bb576=bb156;bb73=bb73->bb94;bb156+=bb73->bb127;}bb169=bb1298-bb576;
bbm(bb156<=bb1298){bb18=bb375;bb107 bb164;}}{bb156=bb46->bb181;bb576=
0 ;bb108(bb156<=bb1744&&bb46->bb94){bb576=bb156;bb46=bb46->bb94;bb156
+=bb46->bb181;}bb147=bb1744-bb576;bbm(bb156<=bb1744){bb18=bb375;bb107
bb164;}}bb108(bb965>=bb38&&bb73){bbm(!bb46){bb18=bb375;bb107 bb164;}
bb482=0 ;bb500=0 ;bbm(bb73->bb127-bb169>=bb38&&bb46->bb181-bb147>=bb38){
bb1030=((bb73->bb127-bb169)<(bb46->bb181-bb147)?(bb73->bb127-bb169):(
bb46->bb181-bb147));bb1030=bb1030-bb1030%bb38;{bbq bb393;bb1597(bb539
,&bb46->bb74[bb147],&bb393,&bb73->bb74[bb169],bb1030);bb1259=&bb46->
bb74[bb147]+bb393;}bb147+=bb1030;bb169+=bb1030;bb965-=bb1030;bbm(
bb169==bb73->bb127){bb73=bb73->bb94;bb169=0 ;}bbm(bb147==bb46->bb181){
bb46->bb127=bb46->bb181;bb470=bb46;bb46=bb46->bb94;bb147=0 ;}}bb54{bbm
(bb73->bb127-bb169<bb38){bb1914=bb978;bb108(bb482<bb38&&bb73){bb156=
bb73->bb127-bb169;bbm(bb156>bb38-bb482)bb156=bb38-bb482;bb81(&bb978[
bb482],&bb73->bb74[bb169],bb156);bb482+=bb156;bb169+=bb156;bb965-=
bb156;bbm(bb169==bb73->bb127){bb73=bb73->bb94;bb169=0 ;}}}bb54{bb1914=
&bb73->bb74[bb169];bb482=bb38;bb169+=bb38;bb965-=bb38;}bb602=bb46->
bb181-bb147<bb38?bb908:&bb46->bb74[bb147];bbm(bb482==bb38){bb482=0 ;{
bbq bb393;bb1597(bb539,bb602,&bb393,bb1914,bb38);bb1259=bb602+bb393;}
bbm(bb602==bb908){bb500=bb46->bb181-bb147;bb81(&bb46->bb74[bb147],
bb908,bb500);bb46->bb127=bb46->bb181;bb470=bb46;bb46=bb46->bb94;bbm(!
bb46){bb18=bb375;bb107 bb164;}bb81(bb46->bb74,&bb908[bb500],bb38-
bb500);bb147=bb38-bb500;}bb54 bb147+=bb38;}}}bb31(bb1259);bb2096(
bb539,bb1259,bb91);}bb54{bb18=bb465(bb85,bb381+8 +bb38,bb58,bb177?0 :
bb381,bb151-bb381-8 );bbm(((bb18)!=bb98))bb107 bb164;bb46=bb58;bb470=
bb91;bb108(bb46->bb94){bbm(bb46->bb94->bb127==0 )bb21;bb470=bb46;bb46=
bb46->bb94;}bb147=bb46->bb127;}bbm(bb491> *bb463){ *bb484>>=bb491- *
bb463; *bb484&=0x7fffffff >>(bb491- *bb463-1 ); *bb463=bb491; *bb535= *
bb463-(bb12( *bb484)<<3 )+1 ;} *bb484|=1 <<(bb491- *bb535);bbm(bb147>1 ){
bb578=bb46->bb74[bb147-2 ];bb932=bb46->bb74[bb147-1 ];}bb54 bbm(bb147==
1 ){bb578=bb470->bb74[bb470->bb127-1 ];bb932=bb46->bb74[0 ];}bb54{bb578=
bb470->bb74[bb470->bb127-2 ];bb932=bb470->bb74[bb470->bb127-1 ];}bbm(
bb147>(bbd)(bb578+2 )){bb147-=bb578+2 ;bb46->bb127=bb147;}bb54{bbm(!
bb470){bb18=bb375;bb107 bb164;}bb46->bb127=0 ;bb470->bb127-=bb578+2 -
bb147;bb147=bb470->bb127;bb46=bb470;}bbm(bb932==4 &&!bb177){bb177=1 ;
bb18=bb465(bb58,bb381,bb58,0 ,bb151-(bb381+8 +bb38)-(bb578+2 ));bbm(((
bb18)!=bb98))bb107 bb164;}bb151-=(bb177?bb381:0 )+8 +bb38+bb578+2 ;bbm(!
bb177){bb18=bb929(bb85,50 ,bb932,bb58);bbm(((bb18)!=bb98))bb107 bb164;
}bb527(bb151,bb58);bb536(bb58);bb164:bbm(bb130)bb458(bb130);bbm(bb539
)bb105(bb539);bbm(bb525)bb105(bb525);bb2 bb18;}
