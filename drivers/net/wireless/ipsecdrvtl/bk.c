/*
   'src_ipsec_pgpIPsecComp.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Fri Oct 12 22:15:16 2012
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
bba bbi bb991*bb989;bba bbi bb1026*bb1027;bba bbi bb993*bb1021;bba bbi
bb1000*bb1010;bba bbi bb1012*bb1023;bba bbi bb990*bb986;bba bb9{bb552
=0 ,bb581=1 ,bb593=2 ,bb813=3 ,bb585=4 ,bb570=5 ,bb575=6 ,bb562=7 ,bb579=9 ,}
bb422;bba bb9{bb606=0 ,bb992,bb598,bb1009,bb930,bb923,bb926,bb916,
bb924,bb922,bb915,}bb519;bba bb83 bb4;bb9{bb98=0 ,bb364=-12000 ,bb357=-
11999 ,bb375=-11998 ,bb671=-11997 ,bb797=-11996 ,bb724=-11995 ,bb871=-
11994 ,bb788=-11992 ,bb806=-11991 ,bb668=-11990 ,bb712=-11989 ,bb833=-
11988 ,bb637=-11987 ,bb672=-11986 ,bb773=-11985 ,bb851=-11984 ,bb623=-
11983 ,bb617=-11982 ,bb764=-11981 ,bb903=-11980 ,bb791=-11979 ,bb722=-
11978 ,bb843=-11977 ,bb583=-11976 ,bb844=-11975 ,bb767=-11960 ,bb678=-
11959 ,bb689=-11500 ,bb732=-11499 ,bb856=-11498 ,bb798=-11497 ,bb879=-
11496 ,bb869=-11495 ,bb828=-11494 ,bb774=-11493 ,bb858=-11492 ,bb885=-
11491 ,bb705=-11490 ,bb745=-11489 ,bb702=-11488 ,bb892=-11487 ,bb872=-
11486 ,bb706=-11485 ,bb645=-11484 ,bb902=-11483 ,bb769=-11482 ,bb905=-
11481 ,bb846=-11480 ,bb759=-11479 ,bb644=-11478 ,bb717=-11477 ,bb657=-
11476 ,bb631=-11475 ,bb864=-11474 ,bb789=-11473 ,bb707=-11472 ,bb809=-
11460 ,bb652=-11450 ,bb740=-11449 ,bb710=-11448 ,bb733=-11447 ,bb790=-
11446 ,bb635=-11445 ,bb887=-11444 ,bb824=-11443 ,bb842=-11440 ,bb865=-
11439 ,bb801=-11438 ,bb800=-11437 ,bb673=-11436 ,bb688=-11435 ,bb620=-
11420 ,bb531=-11419 ,bb571=-11418 ,bb685=-11417 ,bb835=-11416 ,bb667=-
11415 ,bb795=-11414 ,bb731=-11413 ,bb633=-11412 ,bb823=-11411 ,bb674=-
11410 ,bb636=-11409 ,bb708=-11408 ,bb900=-11407 ,bb898=-11406 ,bb803=-
11405 ,bb720=-11404 ,bb658=-11403 ,bb761=-11402 ,bb634=-11401 ,bb679=-
11400 ,bb878=-11399 ,bb754=-11398 ,bb762=-11397 ,bb683=-11396 ,bb866=-
11395 ,bb889=-11394 ,bb615=-11393 ,bb894=-11392 ,bb692=-11391 ,bb784=-
11390 ,bb727=-11389 ,bb711=-11388 ,bb749=-11387 ,bb904=-11386 ,bb627=-
11385 ,bb700=-11384 ,bb786=-11383 ,bb648=-11382 ,bb814=-11381 ,bb736=-
11380 ,bb785=-11379 ,bb669=-11378 ,bb752=-11377 ,bb808=-11376 ,bb709=-
11375 ,bb763=-11374 ,bb699=-11373 ,bb897=-11372 ,bb862=-11371 ,bb802=-
11370 ,bb777=-11369 ,bb841=-11368 ,bb756=-11367 ,bb794=-11366 ,bb719=-
11365 ,bb860=-11364 ,bb845=-11363 ,bb388=-11350 ,bb883=bb388,bb714=-11349
,bb834=-11348 ,bb836=-11347 ,bb643=-11346 ,bb649=-11345 ,bb906=-11344 ,
bb822=-11343 ,bb779=-11342 ,bb680=-11341 ,bb770=-11340 ,bb901=-11339 ,
bb398=-11338 ,bb663=-11337 ,bb687=bb398,bb799=-11330 ,bb817=-11329 ,bb781
=-11328 ,bb632=-11327 ,bb718=-11326 ,bb650=-11325 ,bb821=-11324 ,bb698=-
11320 ,bb819=-11319 ,bb859=-11318 ,bb690=-11317 ,bb626=-11316 ,bb681=-
11315 ,bb825=-11314 ,bb723=-11313 ,bb641=-11312 ,bb642=-11300 ,bb741=-
11299 ,bb796=-11298 ,bb703=-11297 ,bb852=-11296 ,bb811=-11295 ,bb832=-
11294 ,bb654=-11293 ,bb847=-11292 ,bb882=-11291 ,bb618=-11290 ,bb804=-
11289 ,bb857=-11288 ,bb849=-11287 ,bb734=-11286 ,bb653=-11285 ,bb646=-
11284 ,bb812=-11283 ,bb738=-11282 ,bb704=-11281 ,bb661=-11280 ,bb713=-
11279 ,bb701=-11250 ,bb850=-11249 ,bb848=-11248 ,bb748=-11247 ,bb737=-
11246 ,bb805=-11245 ,bb778=-11244 ,bb755=-11243 ,bb621=-11242 ,bb839=-
11240 ,bb651=-11239 ,bb729=-11238 ,bb792=-11237 ,bb677=-11150 ,bb855=-
11100 ,bb820=-11099 ,bb655=-11098 ,bb744=-11097 ,bb782=-11096 ,bb793=-
11095 ,bb768=-11094 ,bb628=-11093 ,bb830=-11092 ,bb899=-11091 ,bb666=-
11090 ,bb877=-11089 ,bb884=-11088 ,bb853=-11087 ,bb638=-11086 ,bb780=-
11085 ,bb783=-11050 ,bb751=-11049 ,bb691=-10999 ,bb639=-10998 ,bb656=-
10997 ,bb753=-10996 ,bb893=-10995 ,bb682=-10994 ,bb694=-10993 ,bb840=-
10992 ,bb771=-10991 ,bb735=-10990 ,bb630=-10989 ,bb907=-10988 ,bb728=-
10979 ,bb660=-10978 ,bb765=-10977 ,bb873=-10976 ,bb695=-10975 ,bb826=-
10974 ,};bb9{bb550=1 ,};bbb*bb496(bbd bb1213,bbd bb372);bb4 bb458(bbb*
bb970);bba bbi bb1985 bb1971, *bb383;bba bb9{bb1994=0 ,bb1754=1 ,bb1772
=2 }bb624;bb4 bb1814(bb624 bb1871,bb383*bb360);bb4 bb1949(bb383 bb360,
bbf*bb434,bbd bb418,bbf*bb309,bbd bb293,bbd*bb437,bbd*bb306);bb4
bb1942(bb383 bb360,bbf*bb309,bbd bb293,bbd*bb306,bbu*bb973);bb4 bb1950
(bb383 bb360,bbf*bb434,bbd bb418,bbf*bb309,bbd bb293,bbd*bb437,bbd*
bb306,bbu*bb963);bb4 bb1823(bb383 bb360);bba bbi bb455{bb1 bb74;bbd
bb127;bbd bb181;bbi bb455*bb94;}bbx;bb4 bb465(bbx*bb670,bbd bb909,bbx
 *bb696,bbd bb895,bbd bb538);bb4 bb532(bbx*bbj,bbd bb92,bbh bbb*bb95,
bbd bb47);bb4 bb577(bbx*bbj,bbd bb92,bbb*bb131,bbd bb47);bbu bb827(
bbx*bbj,bbd bb92,bbh bbb*bb95,bbd bb47);bb4 bb2184(bbx*bb85,bbu bb177
,bbd bb489,bb624 bb1358,bbx*bb58,bbu*bb2086);bb4 bb2099(bbx*bb85,bbu
bb177,bb624 bb1358,bbx*bb58);
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
bb159,bb296 bb555,bbf bb439[4 ]);bb4 bb2184(bbx*bb85,bbu bb177,bbd
bb489,bb624 bb1358,bbx*bb58,bbu*bb2086){bb4 bb18=bb98;bbd bb1219;bbd
bb169;bbd bb147;bbd bb1932;bbd bb1554;bbd bb156;bbd bb2104;bbd bb1879
;bbk bb151;bbk bb1991;bbf bb932;bbu bb2109=0 ;bbx*bb73=bb91;bbx*bb46=
bb91;bbx*bb2101=bb91;bb383 bb1626=bb91; *bb2086=0 ;bb151=bb534(bb85);
bbm(bb151<90 )bb2 bb465(bb85,0 ,bb58,0 ,bb151);bb297(bb1358){bb15 bb1754
:bb1991=2 ;bb21;bb15 bb1772:bb1991=3 ;bb21;bb421:bb2 bb364;}bb18=bb1814
(bb1358,&bb1626);bbm(((bb18)!=bb98))bb107 bb164;bb1219=bb517(bb85);
bb169=bb177?0 :bb1219;bb73=bb85;bb46=bb58;bb147=bb1219+4 ;bb2104=bb151-
bb169;bb1879=0 ;bb156=0 ;bb108(bb73&&bb46&&bb156<bb2104){bbm(bb46->
bb181<bb147){bb18=bb375;bb107 bb164;}bb18=bb1949(bb1626,&bb73->bb74[
bb169],bb73->bb127-bb169,&bb46->bb74[bb147],bb46->bb181-bb147,&bb1932
,&bb1554);bbm(((bb18)!=bb98))bb107 bb164;bb169+=bb1932;bb156+=bb1932;
bb147+=bb1554;bb1879+=bb1554;bbm(bb1879>=bb2104&&!bb2101)bb2101=bb46;
bbm(bb169==bb73->bb127){bb73=bb73->bb94;bb169=0 ;}bbm(bb147==bb46->
bb181){bb46->bb127=bb46->bb181;bb46=bb46->bb94;bb147=0 ;}}bb2109=1 ;
bb108(bb2109&&bb46){bb18=bb1942(bb1626,&(bb46->bb74[bb147]),bb46->
bb181-bb147,&bb1554,&bb2109);bbm(((bb18)!=bb98))bb107 bb164;bb147+=
bb1554;bb1879+=bb1554;bbm(bb1879>=bb2104&&!bb2101)bb2101=bb46;bbm(
bb147==bb46->bb181&&bb2109){bb46->bb127=bb46->bb181;bb46=bb46->bb94;
bb147=0 ;}}bbm(bb2109){bb18=bb375;bb107 bb164;}bbm(bb1879>=bb2104){
bb2101->bb127=0 ;bb18=bb465(bb85,0 ,bb58,0 ,bb151);bb107 bb164;}bb18=
bb595(bb85,108 ,bb58);bbm(((bb18)!=bb98))bb107 bb164;bb686(bb85,bb177,
&bb932);bb58->bb74[bb1219+0 ]=bb932;bb58->bb74[bb1219+1 ]=0 ;bb1158(
bb1991,bb407,&bb58->bb74[bb1219+2 ]);bb46->bb127=bb147;bb151=bb1219+4 +
bb1879;bbm(bb177)bb931(bb58,bb489);bb527(bb151,bb58);bb536(bb58); *
bb2086=1 ;bb164:bbm(bb1626)bb1823(bb1626);bb2 bb18;}bb4 bb2099(bbx*
bb85,bbu bb177,bb624 bb1358,bbx*bb58){bb4 bb18=bb98;bbd bb1219;bbd
bb169;bbd bb147;bbd bb1932;bbd bb1554;bbd bb2140;bbk bb151;bbk bb1991
;bbu bb1258=0 ;bbx*bb73=bb91;bbx*bb46=bb91;bb383 bb1626=bb91;bb151=
bb534(bb85);bb297(bb1358){bb15 bb1754:bb1991=2 ;bb21;bb15 bb1772:
bb1991=3 ;bb21;bb421:bb2 bb364;}bb18=bb1814(bb1358,&bb1626);bbm(((bb18
)!=bb98))bb107 bb164;bb18=bb541(bb85,108 ,&bb1219);bbm(bb18==bb981){
bb18=bb465(bb85,0 ,bb58,0 ,bb151);bb107 bb164;}bbm(((bb18)!=bb98))bb107
bb164;{bb147=bb177?0 :bb1219;bb2140=0 ;bb73=bb85;bb169=bb1219+4 ;bb46=
bb58;bb108(bb73&&bb46&&!bb1258){bb18=bb1950(bb1626,&bb73->bb74[bb169]
,bb73->bb127-bb169,&bb46->bb74[bb147],bb46->bb181-bb147,&bb1932,&
bb1554,&bb1258);bbm(bb18==bb375&&bb169==0 &&bb73->bb127==0 ){bb18=bb98;
bb1258=1 ;bb21;}bbm(((bb18)!=bb98))bb107 bb164;bb169+=bb1932;bb147+=
bb1554;bb2140+=bb1554;bbm(bb169==bb73->bb127&&!bb1258&&bb1932!=0 ){
bb73=bb73->bb94;bb169=0 ;bbm(!bb73){bb1258=1 ;bb21;}}bbm(bb147==bb46->
bb181&&!bb1258){bb46->bb127=bb46->bb181;bb46=bb46->bb94;bb147=0 ;}}bbm
(!bb1258){bb18=bb375;bb107 bb164;}}bbm(bb177)bb151=bb2140;bb54{bb151=
bb1219+bb2140;bb18=bb929(bb85,108 ,bb85->bb74[bb1219+0 ],bb58);bbm(((
bb18)!=bb98))bb107 bb164;}bb46->bb127=bb147;bb527(bb151,bb58);bb536(
bb58);bb164:bbm(bb1626)bb1823(bb1626);bb2 bb18;}
