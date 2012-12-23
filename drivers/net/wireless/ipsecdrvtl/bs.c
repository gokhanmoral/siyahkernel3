/*
   'src_nic_vpnsupport.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Fri Oct 12 22:15:16 2012
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
#ifdef UNDER_CE
#define bb1915 64
#endif
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
bbk bb921(bbh bbb*bb295);bbk bb886(bbh bbb*bb510,bbe bb22);bba bb83
bb4;bb9{bb98=0 ,bb364=-12000 ,bb357=-11999 ,bb375=-11998 ,bb671=-11997 ,
bb797=-11996 ,bb724=-11995 ,bb871=-11994 ,bb788=-11992 ,bb806=-11991 ,
bb668=-11990 ,bb712=-11989 ,bb833=-11988 ,bb637=-11987 ,bb672=-11986 ,
bb773=-11985 ,bb851=-11984 ,bb623=-11983 ,bb617=-11982 ,bb764=-11981 ,
bb903=-11980 ,bb791=-11979 ,bb722=-11978 ,bb843=-11977 ,bb583=-11976 ,
bb844=-11975 ,bb767=-11960 ,bb678=-11959 ,bb689=-11500 ,bb732=-11499 ,
bb856=-11498 ,bb798=-11497 ,bb879=-11496 ,bb869=-11495 ,bb828=-11494 ,
bb774=-11493 ,bb858=-11492 ,bb885=-11491 ,bb705=-11490 ,bb745=-11489 ,
bb702=-11488 ,bb892=-11487 ,bb872=-11486 ,bb706=-11485 ,bb645=-11484 ,
bb902=-11483 ,bb769=-11482 ,bb905=-11481 ,bb846=-11480 ,bb759=-11479 ,
bb644=-11478 ,bb717=-11477 ,bb657=-11476 ,bb631=-11475 ,bb864=-11474 ,
bb789=-11473 ,bb707=-11472 ,bb809=-11460 ,bb652=-11450 ,bb740=-11449 ,
bb710=-11448 ,bb733=-11447 ,bb790=-11446 ,bb635=-11445 ,bb887=-11444 ,
bb824=-11443 ,bb842=-11440 ,bb865=-11439 ,bb801=-11438 ,bb800=-11437 ,
bb673=-11436 ,bb688=-11435 ,bb620=-11420 ,bb531=-11419 ,bb571=-11418 ,
bb685=-11417 ,bb835=-11416 ,bb667=-11415 ,bb795=-11414 ,bb731=-11413 ,
bb633=-11412 ,bb823=-11411 ,bb674=-11410 ,bb636=-11409 ,bb708=-11408 ,
bb900=-11407 ,bb898=-11406 ,bb803=-11405 ,bb720=-11404 ,bb658=-11403 ,
bb761=-11402 ,bb634=-11401 ,bb679=-11400 ,bb878=-11399 ,bb754=-11398 ,
bb762=-11397 ,bb683=-11396 ,bb866=-11395 ,bb889=-11394 ,bb615=-11393 ,
bb894=-11392 ,bb692=-11391 ,bb784=-11390 ,bb727=-11389 ,bb711=-11388 ,
bb749=-11387 ,bb904=-11386 ,bb627=-11385 ,bb700=-11384 ,bb786=-11383 ,
bb648=-11382 ,bb814=-11381 ,bb736=-11380 ,bb785=-11379 ,bb669=-11378 ,
bb752=-11377 ,bb808=-11376 ,bb709=-11375 ,bb763=-11374 ,bb699=-11373 ,
bb897=-11372 ,bb862=-11371 ,bb802=-11370 ,bb777=-11369 ,bb841=-11368 ,
bb756=-11367 ,bb794=-11366 ,bb719=-11365 ,bb860=-11364 ,bb845=-11363 ,
bb388=-11350 ,bb883=bb388,bb714=-11349 ,bb834=-11348 ,bb836=-11347 ,bb643
=-11346 ,bb649=-11345 ,bb906=-11344 ,bb822=-11343 ,bb779=-11342 ,bb680=-
11341 ,bb770=-11340 ,bb901=-11339 ,bb398=-11338 ,bb663=-11337 ,bb687=bb398
,bb799=-11330 ,bb817=-11329 ,bb781=-11328 ,bb632=-11327 ,bb718=-11326 ,
bb650=-11325 ,bb821=-11324 ,bb698=-11320 ,bb819=-11319 ,bb859=-11318 ,
bb690=-11317 ,bb626=-11316 ,bb681=-11315 ,bb825=-11314 ,bb723=-11313 ,
bb641=-11312 ,bb642=-11300 ,bb741=-11299 ,bb796=-11298 ,bb703=-11297 ,
bb852=-11296 ,bb811=-11295 ,bb832=-11294 ,bb654=-11293 ,bb847=-11292 ,
bb882=-11291 ,bb618=-11290 ,bb804=-11289 ,bb857=-11288 ,bb849=-11287 ,
bb734=-11286 ,bb653=-11285 ,bb646=-11284 ,bb812=-11283 ,bb738=-11282 ,
bb704=-11281 ,bb661=-11280 ,bb713=-11279 ,bb701=-11250 ,bb850=-11249 ,
bb848=-11248 ,bb748=-11247 ,bb737=-11246 ,bb805=-11245 ,bb778=-11244 ,
bb755=-11243 ,bb621=-11242 ,bb839=-11240 ,bb651=-11239 ,bb729=-11238 ,
bb792=-11237 ,bb677=-11150 ,bb855=-11100 ,bb820=-11099 ,bb655=-11098 ,
bb744=-11097 ,bb782=-11096 ,bb793=-11095 ,bb768=-11094 ,bb628=-11093 ,
bb830=-11092 ,bb899=-11091 ,bb666=-11090 ,bb877=-11089 ,bb884=-11088 ,
bb853=-11087 ,bb638=-11086 ,bb780=-11085 ,bb783=-11050 ,bb751=-11049 ,
bb691=-10999 ,bb639=-10998 ,bb656=-10997 ,bb753=-10996 ,bb893=-10995 ,
bb682=-10994 ,bb694=-10993 ,bb840=-10992 ,bb771=-10991 ,bb735=-10990 ,
bb630=-10989 ,bb907=-10988 ,bb728=-10979 ,bb660=-10978 ,bb765=-10977 ,
bb873=-10976 ,bb695=-10975 ,bb826=-10974 ,};bba bbi bb455{bb1 bb74;bbd
bb127;bbd bb181;bbi bb455*bb94;}bbx;bb4 bb465(bbx*bb670,bbd bb909,bbx
 *bb696,bbd bb895,bbd bb538);bb4 bb532(bbx*bbj,bbd bb92,bbh bbb*bb95,
bbd bb47);bb4 bb577(bbx*bbj,bbd bb92,bbb*bb131,bbd bb47);bbu bb827(
bbx*bbj,bbd bb92,bbh bbb*bb95,bbd bb47);bb4 bb595(bbx*bb85,bbf bb102,
bbx*bb58);bb4 bb686(bbx*bb85,bbu bb177,bbf*bb408);bb4 bb955(bbx*bb58,
bbf*bb389);bb4 bb941(bbh bbf*bb389,bbx*bb58);bb4 bb541(bbx*bb51,bbf
bb102,bbd*bb939);bb4 bb929(bbx*bb85,bbf bb102,bbf bb408,bbx*bb58);bbd
bb517(bbx*bb51);bbk bb534(bbx*bb51);bbb bb527(bbk bb151,bbx*bb51);bbb
bb536(bbx*bb51);bbb bb968(bbx*bb51,bbd*bb26);bbb bb999(bbx*bb51,bbd*
bb26);bbb bb1020(bbx*bb51,bbd bb26);bbb bb931(bbx*bb51,bbd bb26);bbb
bb983(bbx*bb51);bbu bb1013(bbf*bb51);bba bbi bb991*bb989;bba bbi
bb1026*bb1027;bba bbi bb993*bb1021;bba bbi bb1000*bb1010;bba bbi
bb1012*bb1023;bba bbi bb990*bb986;bba bb9{bb552=0 ,bb581=1 ,bb593=2 ,
bb813=3 ,bb585=4 ,bb570=5 ,bb575=6 ,bb562=7 ,bb579=9 ,}bb422;bba bb9{bb606=
0 ,bb992,bb598,bb1009,bb930,bb923,bb926,bb916,bb924,bb922,bb915,}bb519
;
#pragma pack(push, 8)
#ifdef _MSC_VER
#pragma warning (disable:4200)
#endif
bba bbf bb178[4 ];bba bb9{bb1651=0 ,bb1466=1 ,}bb1390;bba bb9{bb1527=0 ,
bb1718=1 ,bb1559=2 ,bb1437=3 ,bb1657=4 ,bb1495=5 ,bb1634=6 ,bb1514=7 ,bb1605
=8 ,bb1518=9 ,bb1677=10 ,bb1506=11 ,bb1694=12 ,bb1710=13 ,bb1716=14 ,bb1422=
15 ,bb1452=16 ,bb1393=17 ,bb1598=18 ,bb1688=19 ,bb1642=20 ,bb1491=21 ,bb1504
=22 ,bb1473=23 ,bb1601=24 ,bb1604=25 ,bb1449=26 ,bb1579=27 ,bb1373=28 ,
bb1612=29 ,bb1686=30 ,bb1630=16300 ,bb1429=16301 ,bb1727=16384 ,bb1535=
24576 ,bb1462=24577 ,bb1436=24578 ,bb1477=34793 ,bb1380=40500 ,}bb629;bba
bb9{bb1460=0 ,bb1523=1 ,bb1456=2 ,bb1424=3 ,bb1397=4 ,bb1386=5 ,bb1668=6 ,
bb1474=7 ,bb1528=8 ,bb1399=9 ,bb1443=21 ,bb1487=22 ,bb1488=23 ,bb1445=24 ,
bb1540=25 ,bb1508=26 ,bb1461=27 ,bb1382=28 ,bb1479=29 ,bb1489=80 ,}bb775;
bba bb9{bb1562=0 ,bb1697=1 ,bb1693=2 ,bb1483=3 ,bb1520=4 ,}bb1646;bba bb9{
bb1685=0 ,bb1347=1 ,bb1165=2 ,bb1225=3 ,bb1289=4 ,bb1049=61440 ,bb1329=
61441 ,bb1113=61443 ,bb1301=61444 ,}bb486;bba bb9{bb1700=0 ,bb1494=1 ,
bb1560=2 ,}bb1679;bba bb9{bb1388=0 ,bb1726,bb1438,bb1453,bb1565,bb1496,
bb1635,bb1464,bb1603,bb1492,bb1396,bb1728,}bb743;bba bb9{bb1507=0 ,
bb1361=2 ,bb1328=3 ,bb1375=4 ,bb1325=9 ,bb1297=12 ,bb1359=13 ,bb1310=14 ,
bb1348=249 ,}bb715;bba bb9{bb1357=0 ,bb1300=1 ,bb1285=2 ,bb1421=3 ,bb1633=
4 ,bb1356=5 ,bb1342=12 ,bb1320=13 ,bb1368=14 ,bb1286=61440 ,}bb481;bba bb9{
bb1293=1 ,bb1369=2 ,bb1338=3 ,bb1539=4 ,bb1600=5 ,bb1448=6 ,bb1427=7 ,bb1469
=8 ,bb1454=9 ,bb1538=10 ,bb1305=11 ,bb391=12 ,bb1339=13 ,bb390=240 ,bb1344=(
128 <<16 )|bb390,bb1343=(192 <<16 )|bb390,bb1331=(256 <<16 )|bb390,bb1303=(
128 <<16 )|bb391,bb1294=(192 <<16 )|bb391,bb1346=(256 <<16 )|bb391,}bb616;
bba bb9{bb1296=0 ,bb1501=1 ,bb1365=2 ,bb1330=3 ,bb1458=4 ,}bb890;bba bb9{
bb1434=0 ,bb1573=1 ,bb1222=2 ,bb604=3 ,bb1236=4 ,}bb716;bba bb9{bb1576=0 ,
bb1526=1 ,bb1406=2 ,bb1669=5 ,bb1706=7 ,}bb483;bba bb9{bb1425=0 ,bb1513=1 ,
bb1618=2 ,bb1711=3 ,bb1482=4 ,bb1690=5 ,bb1647=6 ,bb386=7 ,bb1543=65001 ,
bb396=240 ,bb1484=(128 <<16 )|bb396,bb1502=(192 <<16 )|bb396,bb1511=(256 <<
16 )|bb396,bb1542=(128 <<16 )|bb386,bb1556=(192 <<16 )|bb386,bb1614=(256 <<
16 )|bb386,}bb810;bba bb9{bb1717=0 ,bb1457=1 ,bb1662=2 ,bb1572=3 ,bb1472=4
,bb1529=5 ,bb1566=6 ,bb1644=65001 ,}bb625;bba bb9{bb1684=0 ,bb1524=1 ,
bb1661=2 ,bb1551=3 ,bb1654=4 ,bb1611=5 ,bb1553=64221 ,bb1617=64222 ,bb1658=
64223 ,bb1672=64224 ,bb1709=65001 ,bb1680=65002 ,bb1549=65003 ,bb1440=
65004 ,bb1722=65005 ,bb1486=65006 ,bb1510=65007 ,bb1476=65008 ,bb1708=
65009 ,bb1475=65010 ,}bb896;bba bb9{bb1698=0 ,bb1416=1 ,bb1431=2 ,}bb891;
bba bb9{bb1409=0 ,bb1638=1 ,bb1478=2 ,bb1682=3 ,}bb739;bba bb9{bb1590=0 ,
bb1418=1 ,bb1433=2 ,bb1648=3 ,bb1660=4 ,bb1641=5 ,bb1500=21 ,bb1569=6 ,
bb1615=7 ,bb1536=8 ,bb1378=1000 ,}bb490;bba bb9{bb1410=0 ,bb1561=1 ,bb1666
=2 ,}bb730;bba bb9{bb1665=0 ,bb1629=1 ,bb1715=2 ,bb1435=3 ,bb1471=4 ,}bb676
;bba bb9{bb1530=0 ,bb1673=1 ,bb1392=1001 ,bb1713=1002 ,}bb837;bba bb9{
bb1558=0 ,bb1134=1 ,bb1040=2 ,bb1051=3 ,bb1112=4 ,bb1126=5 ,bb1094=6 ,bb1695
=100 ,bb1581=101 ,}bb480;bba bbi bb394{bb616 bb154;bb481 bb580;bb486
bb57;}bb394;bba bbi bb400{bb715 bb1349;bb481 bb580;bb486 bb57;}bb400;
bba bbi bb392{bb890 bb1003;}bb392;bba bbi bb476{bb896 bb1606;bb625
bb414;bb810 bb154;bbu bb1485;bb483 bb647;}bb476;bba bbi bb485{bbu
bb599;bb394 bb308;bbu bb640;bb400 bb560;bbu bb772;bb392 bb607;bb483
bb647;}bb485;bba bbi bb450{bb178 bb957;bb178 bb1211;bb716 bb102;bb556
{bbi{bb400 bb45;bbf bb558[64 ];bbf bb551[64 ];}bb560;bbi{bb394 bb45;bbf
bb1218[32 ];bbf bb1229[32 ];bbf bb558[64 ];bbf bb551[64 ];bbf bb1199[16 ];
}bb308;bbi{bb392 bb45;}bb607;}bb317;}bb450;bba bbi{bbd bb818,bb589;
bbf bb1132:1 ;bbf bb1168:1 ;bbf bb102;bbk bb440;}bb185;bba bbi bb507{
bbd bb11;bb185 bbc[64 *2 ];}bb507;
#ifdef UNDER_CE
bba bb43 bb378;
#else
bba bb83 bb378;
#endif
bba bbi bb199{bbi bb199*bb1467, *bb1385;bbd bb26;bbd bb1114;bb185
bb914[64 ];bb480 bb504;bbd bb1299;bbk bb1062;bbd bb554;bbd bb675;bbd
bb816;bbf bb488;bbf bb1335;bbf bb1103;bbd bb1029;bbd bb1381;bb378
bb568;bbk bb1280;bb450 bb409[3 ];bb378 bb1568;bbf bb1503[40 ];bbd bb590
;bbd bb1577;}bb199;bba bbi bb399{bbi bb399*bb1723;bb185 bb475;}bb399;
bba bbi bb746{bbu bb473;bbu bb488;bbd bb26;bbd bb590;bbf bb1512;bbk
bb1592;bbf*bb1544;bbd bb1423;bbf*bb1583;bbd bb1714;bbf*bb1376;bbd
bb1412;bbu bb1645;bbu bb1570;bb399*bb131;bbu bb1470;bb676 bb1519;bbd
bb1593;bb891 bb1707;bb480 bb504;bbk bb1371;bbd bb1533;bb837 bb1403;
bbd bb1650;bbd bb1720;bb743 bb1417;bbf*bb1405;bbd bb1413;bb490 bb867;
bbd bb1656;bbd bb1623;bbd bb1408;bbd bb1699;bbd bb1493;bb476*bb1537;
bbd bb1610;bb485*bb1505;bbd bb1395;bbd bb1531;bbd bb1441;}bb746;bba
bbi bb684{bbu bb473;bbd bb26;bb185 bb475;}bb684;bba bbi bb665{bb199*
bb316;bbu bb1571;bbf*bb1701;bbd bb1667;}bb665;bba bbi bb876{bbd bb26;
bb185 bb475;bbf bb1432;bbf bb1447;}bb876;bba bbi bb829{bbu bb473;bbu
bb1119;bbd bb26;bbf*bb1625;bbd bb1541;}bb829;bba bbi bb662{bbd bb26;
bbk bb1702;bbk bb1729;bbd bb151;bbf*bb48;}bb662;bba bbi bb831{bbu
bb1586;bbd bb26;bbd bb554;bbd bb675;bbd bb816;}bb831;bba bbi bb868{
bb629 bb1490;bbd bb26;bb775 bb1323;bbu bb1555;}bb868;bba bbi bb622{
bbf bb1674;bbf bb1389;bbf bb1687;bbf bb1394;bbf bb1575;bbf bb1602;bbf
bb1584;bbf bb1459;bbf bb1374;bbf bb1521;bbf bb1411;bbf bb1627;bbf
bb1725;bbf bb1407;bbf bb1676;bbf bb1439;bbf bb1620;bbf bb1383;bbf
bb1450;bbf bb512;bbf bb1548;bbf bb1663;bbf bb1532;bbf bb1689;bbf
bb1415;bbf bb1430;bbf bb1414;}bb622;bba bbi bb742{bbu bb1637;bbd bb489
;bbd bb1621;bb739 bb1426;bbk bb1632;bbu bb1516;bbu bb1563;bbu bb1652;
bbu bb1451;bbu bb1631;bbu bb1659;bbu bb1398;bbl bb1624[128 ];bbl bb1670
[128 ];bbl bb1595[128 ];bbl bb1419[256 ];bbl bb1636[128 ];bbl bb1444[128 ]
;bbd bb1589;bbf bb1564[8 ];bbf bb1404[8 ];}bb742;bba bbi bb659{bbd bb26
;bbd bb1696;}bb659;bba bbi bb861{bbd bb26;bbu bb488;}bb861;bba bbi
bb747{bbu bb1522;bbd bb510;bbd bb1171;}bb747;bba bbi bb757{bbd bb26;
bb490 bb867;bb730 bb1599;bbf*bb1578;bbd bb1587;}bb757;bba bb9{bb1400=
0 ,bb1552,bb1671,bb1384,bb1613,bb1534,bb1594,bb1391,bb1525,bb1580,
bb1582,bb1692,bb1705,bb1653,bb1401,bb1585,bb1463,bb1402,bb1622,bb1640
,}bb664;bba bbi bb1649 bb815;bba bb4( *bb1550)(bb815*bb1567,bbb*
bb1588,bb664 bb321,bbb*bb74);
#pragma pack(pop)
#ifdef _WIN32
#ifdef UNDER_CE
#define bb468 bb1703 bb603("1:")
#else
#define bb468 bb603("\\\\.\\IPSecTL")
#endif
#else
#define bb614 "ipsecdrvtl"
#define bb468 "/dev/" bb614
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
#include"uncobf.h"
#include<linux/ioctl.h>
#include"cobf.h"
bba bbi{bb1 bb1367;bbd bb1319;bb1 bb1240;bbd bb1144;bbd bb448;}bb1198
;
#define bb1364 1
#endif
#pragma pack(push, 8)
bb9{bb1355=3 ,bb1353,bb1354,bb1420,};bba bbi{bbf bb103[4 ];}bb1268;bba
bbi{bbf bb103[4 ];}bb1235;bba bbi{bbd bb948;bbd bb26;}bb1265;bba bbi{
bbd bb129;bbf bb1224[8 ];}bb402;bba bb9{bb1220=0 ,bb1232,bb1250,bb1262,
bb1731}bb1233;bba bbi{bbf bb1120;bbd bb1070;bbf bb1360;}bb474;
#pragma pack(pop)
#pragma pack(push, 8)
bb9{bb1135=-5000 ,bb1104=-4000 ,bb997=-4999 ,bb988=-4998 ,bb1011=-4997 ,
bb981=-4996 ,bb1141=-4995 ,bb1087=-4994 ,bb1096=-4993 ,bb1024=-4992 ,
bb1031=-4991 };bb4 bb1129(bb4 bb1133,bbd bb1116,bbl*bb1100);bba bbi{
bb199 bb180;bbd bb1194;bbd bb1084;bbd bb1370;bbd bb1081;bbd bb1239;
bbd bb1279;bbd bb1256;bbd bb1238;bbd bb1248;bbd bb1281;bbd bb1249;bbu
bb1221;bb43 bb568,bb1172,bb1170;bbf bb371[6 ];}bb160;bba bbi bb478{bbi
bb478*bb94;bbf bb102;bbk bb1275;bbk bb1276;bbk bb1271;bbk bb1274;}
bb430;bba bbi bb787{bbi bb787*bb94;bbi bb478*bb1095;bbd bb26;bbf bb371
[6 ];}bb411;bba bb9{bb1146=0 ,bb1730,bb1037,bb1017,bb1007}bb204;bba bbi
{bbd bb382;bbd bb448;bbd bb513;bb402*bb912;bb96 bb980;}bb302;bba bbi{
bb474*bb457;bb411*bb1142;bbd bb584;bb430*bb542;bb96 bb601;bbq bb1118;
bbq bb548;bb160*bb508;bbu bb1269;bbk bb1157;bbk bb1106;bb302 bb1041;}
bb32, *bb1607;
#pragma pack(pop)
bba bbi bb967 bb1334, *bb78;bba bbi bb838{bbi bb838*bb323;bb1 bb466;
bbq bb565;bbd bb26;bbk bb440;bbq bb92;bb1 bb315;bbq bb442;bb1 bb546;
bbq bb543;bb1 bb1498;bb100 bb1362;bbf bb1307[6 ];bb100 bb964;bb100
bb1140;bb100 bb521;bb100 bb533;}bb174, *bb87;bba bbi bb870{bbi bb870*
bb94;bb174*bb323;bbd bb26;bbk bb537;bbk bb1468;bbq bb1442;bbq bb1517;
bbk bb1428;}bb1455, *bb459;bbu bb1284(bb32* *bb1215);bbb bb1283(bb32*
bbj);bb204 bb1267(bb32*bb109,bb376 bb451,bb312 bb138,bb341 bb413,
bb318 bb200);bb204 bb1246(bb32*bb109,bb376 bb451,bb312 bb138,bb341
bb413,bb318 bb200);bb204 bb1254(bb32*bb109,bb174*bb48,bb78 bb76);
bb204 bb1234(bb32*bb109,bb174*bb48,bb78 bb76);bb4 bb1244(bb32*bb109,
bb174*bb48,bbd*bb103);bb4 bb1153(bb78 bb76,bb32*bb109,bb174*bb48,
bb160*bb316,bbu bb594,bbu bb947);bba bbi bb1886{bb121 bb1908;bb121
bb1919;bb32*bb971;}bb1044, *bb1889;bbr bb1044 bb949;bbi bb967{bb121
bb1881;bbq bb1891;bbd bb1952;bb87 bb998;bb87 bb1934;bb87 bb1858;bb87
bb1890;bb87 bb1937;bb459 bb1857;bb459 bb1951;bb459 bb1901;bb96 bb1124
;bb100 bb1902;bb100 bb1946;bb100 bb1925;bb121 bb1948;bb121 bb1869;};
bbr bb78 bb1956;bbr bb96 bb1912;bbd bb1862(bbb*bb518,bbb*bb1878,bb161
 *bb1117);bb161 bb1944(bb121 bb1955,bb121 bb1906,bb77 bb546,bbq bb543
,bb77 bb1111,bbq bb1102,bbq bb1145);
#ifdef UNDER_CE
#define bb591 16
#define bb1125 32
#else
#define bb591 128
#define bb1125 256
#endif
#define bb1105 bb591  *2
#define bb563 ( bb1105  *  2)
#define bb1900 bb563  *  2
#define bb1860 bb563  *  2
bbr bbq bb958;bb161 bb1784(bb60 bb959,bbb*bb39,bbq bb1089,bb122 bb1681
);bb140 bb1921(IN bb78 bb76,IN bb121 bb1916,IN bb1 bb546,IN bbq bb543
,IN bb77 bb1111,IN bbq bb1102,IN bbq bb1145);bb140 bb1903(IN bb78 bb76
);bbd bb1898(bb77 bb518,bb121 bb1917,bb77 bb1907,bbq bb1958,bb77
bb1854,bbq bb1850,bbq bb1910,bb161*bb1117);bbb bb1230(bb78 bb76,bb87*
bb540,bb87 bb48);bb87 bb1264(bb78 bb76,bb87*bb540);bbu bb1783(bb78
bb76);bbb bb1794(bb78 bb76);bb87 bb1465(bb173 bb359,bb78 bb76);bb87
bb1828(bb173 bb359,bb78 bb76);bb87 bb1775(bb173 bb359,bb78 bb76);
bb140 bb1664(bb78 bb76,bb87 bb48);bb140 bb1799(bb78 bb76,bb87 bb48);
bb140 bb1845(bb78 bb76,bb87 bb48);bbb bb1913();bbb bb1877();bbb bb157
(bbh bbl*bb19,...);bbb bb1999(bb187 bbg);bbb bb2034(bbb*bb27,bbq bb11
);bbb bb1379(bbb*bb5,bbq bb10);bbb bb1810(bbb*bb5,bbq bb10);bbb bb1332
(bbb*bb5,bbq bb10);bbb bb1896();bbb bb1747();bbb bb1976(bb366*bb1920);
bbb bb1574(bb326*bb27);bbb bb1263(bb326*bb880,bb487*bb150);bbb bb1481
(bb326*bb880,bb416*bb1609);bba bb9{bb407,bb1497,}bb296;bbk bb1214(
bb296 bb758,bbh bbf*bb447);bbd bb544(bb296 bb758,bbh bbf*bb447);bbb
bb1158(bbk bb159,bb296 bb555,bbf bb439[2 ]);bbb bb972(bbd bb159,bb296
bb555,bbf bb439[4 ]);bbb bb1230(bb78 bb76,bb87*bb540,bb87 bb48){bb134(
&bb76->bb1124);bbm( *bb540==bb91)bb48->bb323=bb48;bb54{bb48->bb323=( *
bb540)->bb323;( *bb540)->bb323=bb48;} *bb540=bb48;bb132(&bb76->bb1124
);}bb87 bb1264(bb78 bb76,bb87*bb540){bb87 bb48;bb134(&bb76->bb1124);
bbm( *bb540==bb91)bb48=bb91;bb54{bb48=( *bb540)->bb323;bbm(bb48== *
bb540) *bb540=bb91;bb54( *bb540)->bb323=bb48->bb323;}bb132(&bb76->
bb1124);bb2 bb48;}bbu bb1783(bb78 bb76){bbe bbz;bbq bb2052;bb76->
bb998=bb91;bb2052=2000 ;bb90(bbz=0 ;bbz<bb563;bbz++){bb87 bb48=bb137(1 ,
bb12( *bb48));bbm(!bb48)bb2 0 ;bb48->bb466=bb128(bb2052);bbm(!bb48->
bb466)bb2 0 ;bb48->bb315=bb128(bb2052);bbm(!bb48->bb315)bb2 0 ;bb1230(
bb76,&bb76->bb998,bb48);}bb2 1 ;}bbb bb1794(bb78 bb76){bbq bb2052;bb87
bb48;bb2052=2000 ;bb108((bb48=bb1264(bb76,&bb76->bb998))!=bb91){bb105(
bb48->bb315);bb105(bb48->bb466);bb105(bb48);}bb76->bb998=bb91;}bb87
bb1465(bb173 bb359,bb78 bb76){bb87 bb48=bb1264(bb76,&bb76->bb998);bbm
(!bb48){ *bb359=bb324;bb2 bb91;} *bb359=bb209;bb48->bb323=bb91;bb2
bb48;}bb87 bb1828(bb173 bb359,bb78 bb76){bb87 bb48=bb1264(bb76,&bb76
->bb998);bbm(!bb48){ *bb359=bb324;bb2 bb91;}bb48->bb323=bb91;bb48->
bb964=0 ;bb48->bb1140=0 ;bb48->bb521=0 ;bb48->bb533=0 ;bb2 bb48;}bb87
bb1775(bb173 bb359,bb78 bb76){bb87 bb48=bb1264(bb76,&bb76->bb998);bbm
(!bb48){ *bb359=bb324;bb2 bb91;}bb48->bb323=bb91;bb48->bb964=0 ;bb48->
bb1140=0 ;bb48->bb521=0 ;bb48->bb533=0 ;bb2 bb48;}bb140 bb1664(bb78 bb76
,bb87 bb48){bbm(!bb48)bb2;bb1230(bb76,&bb76->bb998,bb48);}bb140 bb1799
(bb78 bb76,bb87 bb48){bbm(!bb48)bb2;bb1230(bb76,&bb76->bb998,bb48);}
bb140 bb1845(bb78 bb76,bb87 bb48){bbm(!bb48)bb2;bb1230(bb76,&bb76->
bb998,bb48);}
