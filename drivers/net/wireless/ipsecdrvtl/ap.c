/*
   'src_compress_LZS_lzsc.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Fri Oct 12 22:15:16 2012
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
bba bbi bb1864 bb1864;bba bbi bb1864*bb479;bbd bb2191(bbb);bbb bb2135
(bb479 bb0,bbb*bb1315);bbk bb2171(bb479 bb0,bbf* *bb1739,bbf* *bb1733
,bbd*bb913,bbd*bb597,bbb*bb1315,bbk bb372,bbk bb2156);bbk bb2290(
bb479 bb0,bbf* *bb1739,bbf* *bb1733,bbd*bb913,bbd*bb597,bbb*bb1315,
bbk bb372);
#define bb201 1
#define bb202 0
#define bb1247 bb201
#define bb2373 bb202
bba bbi{bbf*bb1752;bbd bb1028;bbf*bb1743;bbd bb600;bbd bb1272;bbd
bb1745;bbk bb2047;bbf bb1911;bbf bb2607;bbk bb1691;bbd bb545;bbd
bb1839;bbk bb1257;bbd*bb2269;bbd bb2288;bbk bb1333;bbk bb2246;bbk
bb2049;bbk bb2098;bbk bb2058;bbk bb2114;bbk bb1001;bb124 bb1055;bbf
bb1712;bbf bb2606;bb124 bb1943;bbk bb1998;bbk bb1821;}bb2517;bba bbi{
bbf*bb1752;bbd bb1028;bbf*bb1743;bbd bb600;bbk bb2089;bbk bb1757;bbk
bb1164;bbk bb2438;bb124 bb1151;bbk bb1173;bbk bb995;bb124 bb1043;
bb124 bb92;bbd bb469;bbk bb1970;}bb2534;bba bbi{bbf bb1843[2048 ];bbk
bb1873[2048 ];bbd bb2082[4096 ];bb2517 bb44;bbf bb2162[2048 ];bb2534 bb80
;bbf bb1154[64 ];}bb2045;bbi bb1864{bb2045*bb452;bb2045 bb13;bbk bb2110
;bbk bb1749;};bba bbi bb2580{bbk bb2314;bb124 bb47;}bb2449;bb41 bb2449
bb2154[24 ]={{0x0 ,0 },{0x0 ,0 },{0x0 ,2 },{0x1 ,2 },{0x2 ,2 },{0xC ,4 },{0xD ,4 },{
0xE ,4 },{0xF0 ,8 },{0xF1 ,8 },{0xF2 ,8 },{0xF3 ,8 },{0xF4 ,8 },{0xF5 ,8 },{0xF6 ,8 }
,{0xF7 ,8 },{0xF8 ,8 },{0xF9 ,8 },{0xFA ,8 },{0xFB ,8 },{0xFC ,8 },{0xFD ,8 },{0xFE
,8 }};bb41 bbk bb2479[5 ][3 ]={{1 ,1 ,1 },{1 ,1 ,1 },{0 ,0 ,1 },{0 ,0 ,1 },{0 ,1 ,1 }};
bb9{bb2227,bb2560,bb2486,bb2471,bb2385};bb9{bb2081,bb2344,bb2379,
bb2347,bb2306,bb2305,bb2421,bb2234,bb2363};bb41 bbb bb2222(bb479 bb0);
bb41 bbb bb2371(bb479 bb0);bb41 bbb bb1099(bb479 bb0);bb41 bbb bb2378
(bb479 bb0);bb41 bbb bb2127(bbd*bb2407,bbd bb2322);bb41 bbb bb2076(
bb479 bb0,bbk bb2155,bbd bb2097);bb41 bbb bb2121(bb479 bb0);bb41 bbk
bb2280(bb479 bb0);bb41 bbk bb1847(bb479 bb0);bb41 bbb bb2071(bb479 bb0
,bbf bbo);bbd bb2191(bbb){bb2 bb12(bb1864);}bb41 bbb bb2222(bb479 bb0
){bb0->bb13.bb44.bb1943=8 ;bb0->bb13.bb44.bb1712=0 ;bb0->bb13.bb44.
bb1333=bb0->bb13.bb44.bb1691=0 ;bb0->bb13.bb44.bb545=bb0->bb13.bb44.
bb1839=0 ;bb0->bb13.bb44.bb2114=bb0->bb13.bb44.bb1821=0 ;bb2;}bbb bb2135
(bb479 bb0,bbb*bb1315){bb0->bb452=(bb2045* )bb1315;bb0->bb13.bb44=bb0
->bb452->bb44;bb0->bb13.bb44.bb1272=0xFFFFFFFFL ;bb0->bb13.bb44.bb1745
=bb0->bb13.bb44.bb1272-1 ;bb0->bb13.bb44.bb1998=0 ;bb2222(bb0);bb2127(
bb0->bb452->bb2082,0xC0000000L );bb0->bb452->bb44=bb0->bb13.bb44;bb0->
bb13.bb80=bb0->bb452->bb80;bb0->bb13.bb80.bb2089=0 ;bb2121(bb0);bb0->
bb452->bb80=bb0->bb13.bb80;bb2;}bb41 bbb bb2127(bbd*bb2407,bbd bb2322
){bbk bbz;bb90(bbz=0 ;bbz<4096 ;bbz++) *bb2407++=bb2322;bb2;}bb41 bbb
bb2371(bb479 bb0){bb920 bbk bbz;bbd*bb2131;bbd bb2220;bb2220=
0xC0000000L ;bbm(bb0->bb13.bb44.bb1272!=0 )bb2220=0x40000000L ;bb2131=
bb0->bb452->bb2082;bb90(bbz=0 ;bbz<4096 ;bbz++,bb2131++)bbm(bb0->bb13.
bb44.bb1272- *bb2131>2048 -2 ) *bb2131=bb2220;bb2;}bb41 bbb bb1099(
bb479 bb0){bb920 bb124 bb1968;bbm(bb0->bb13.bb44.bb600==0 )bb0->bb13.
bb44.bb1998=1 ;bbm((bb1968=bb0->bb13.bb44.bb1055-bb0->bb13.bb44.bb1943
)>0 ){bb0->bb13.bb44.bb1712|=(bb0->bb13.bb44.bb1001>>bb1968);bb0->bb13
.bb44.bb1943=8 ; *bb0->bb13.bb44.bb1743++=bb0->bb13.bb44.bb1712;--bb0
->bb13.bb44.bb600;bb0->bb13.bb44.bb1712=0 ;bb0->bb13.bb44.bb1001&=((1
<<bb1968)-1 );bb0->bb13.bb44.bb1055=bb1968;bb1099(bb0);}bb54 bbm(
bb1968<0 ){bb0->bb13.bb44.bb1712|=(bb0->bb13.bb44.bb1001<<-bb1968);bb0
->bb13.bb44.bb1943-=bb0->bb13.bb44.bb1055;}bb54{bb0->bb13.bb44.bb1712
|=bb0->bb13.bb44.bb1001;bb0->bb13.bb44.bb1943=8 ; *bb0->bb13.bb44.
bb1743++=bb0->bb13.bb44.bb1712;--bb0->bb13.bb44.bb600;bb0->bb13.bb44.
bb1712=0 ;}bb2;}bb41 bbb bb2378(bb479 bb0){bb0->bb13.bb44.bb1001=(
0x180 );bb0->bb13.bb44.bb1055=(9 );bb1099(bb0);bbm(bb0->bb13.bb44.
bb1943!=8 ){ *bb0->bb13.bb44.bb1743++=bb0->bb13.bb44.bb1712;--bb0->
bb13.bb44.bb600;bbm(bb0->bb13.bb44.bb600==0 ){bb0->bb13.bb44.bb1998=1 ;
}}bb0->bb13.bb44.bb1943=8 ;bb0->bb13.bb44.bb1712=0 ;bb2;}bb41 bbb bb2076
(bb479 bb0,bbk bb2155,bbd bb2097){bbm(bb0->bb13.bb44.bb1821==0 ){bbm(
bb2155<128 ){bb0->bb13.bb44.bb1001=(0x180 |bb2155);bb0->bb13.bb44.
bb1055=(9 );bb1099(bb0);}bb54{bb0->bb13.bb44.bb1001=(0x1000 |bb2155);
bb0->bb13.bb44.bb1055=(13 );bb1099(bb0);}}bbm(bb2097>=23 ){bb0->bb13.
bb44.bb1001=(((1 <<4 )-1 ));bb0->bb13.bb44.bb1055=(4 );bb1099(bb0);bb2097
-=((1 <<4 )-1 );bb0->bb13.bb44.bb1839+=((1 <<4 )-1 );bb0->bb13.bb44.bb1821=
1 ;}bb54{bb0->bb13.bb44.bb1001=(bb2154[(bbk)bb2097].bb2314);bb0->bb13.
bb44.bb1055=(bb2154[(bbk)bb2097].bb47);bb1099(bb0);bb0->bb13.bb44.
bb1821=0 ;bb0->bb13.bb44.bb545=0 ;bb0->bb13.bb44.bb1839=0 ;bb0->bb1749=
bb2385;}bb2;}bb41 bbb bb2192(bb479 bb0,bbk bb372){bbm(bb0->bb13.bb44.
bb2114==1 ){bbm(bb0->bb13.bb44.bb545==0 ){bb0->bb13.bb44.bb1001=((bb0->
bb13.bb44.bb1911));bb0->bb13.bb44.bb1055=(9 );bb1099(bb0);bb0->bb1749=
bb2227;}bb54 bb2076(bb0,bb0->bb13.bb44.bb1333,bb0->bb13.bb44.bb545);}
bb2378(bb0);bbm((bb372&0x04 )==0 ){bbm(0 -bb0->bb13.bb44.bb1272>=2048 )bb2127
(bb0->bb452->bb2082,0xC0000000L );bb54 bbm(0x80000000L -bb0->bb13.bb44.
bb1272>=2048 )bb2127(bb0->bb452->bb2082,0x40000000L );bb0->bb13.bb44.
bb1272+=2048 ;bb0->bb13.bb44.bb1745+=2048 ;bb2222(bb0);}bb0->bb13.bb44.
bb2114=0 ;bb2;}bbk bb2171(bb479 bb0,bbf* *bb1739,bbf* *bb1733,bbd*
bb913,bbd*bb597,bbb*bb1315,bbk bb372,bbk bb2156){bb920 bbk bb2181;
bb920 bbk bb2039;bbk bb2166;bbk bb1038=0 ;bb0->bb452=(bb2045* )bb1315;
bb0->bb13.bb44=bb0->bb452->bb44;bb0->bb13.bb44.bb1752= *bb1739;bb0->
bb13.bb44.bb1028= *bb913;bb0->bb13.bb44.bb1743= *bb1733;bb0->bb13.
bb44.bb600= *bb597;bb0->bb13.bb44.bb1998=0 ;bb0->bb2110=0X0018 &bb372;
bbm(bb0->bb2110>0x0010 ){bb0->bb2110=0x0010 ;}bb0->bb2110>>=3 ;bbm( *
bb597<=15 )bb1038=0 ;bb54 bbm(bb0->bb13.bb44.bb1028!=0 ){bb0->bb13.bb44.
bb600-=15 ;bbm(bb0->bb13.bb44.bb2114==0 ){bb0->bb13.bb44.bb1911= *bb0->
bb13.bb44.bb1752++;--bb0->bb13.bb44.bb1028;++bb0->bb13.bb44.bb1272;++
bb0->bb13.bb44.bb1745;bb0->bb13.bb44.bb2047=(bbk)bb0->bb13.bb44.
bb1745&(2048 -1 );bb0->bb452->bb1843[(bbk)bb0->bb13.bb44.bb1272&(2048 -1
)]=bb0->bb13.bb44.bb1911;bb0->bb13.bb44.bb1691=(bb0->bb13.bb44.bb1691
<<8 )+bb0->bb13.bb44.bb1911;bb0->bb13.bb44.bb2114=1 ;}bb108((bb0->bb13.
bb44.bb1028!=0 )&&(bb0->bb13.bb44.bb1998==0 )){++bb0->bb13.bb44.bb1272;
++bb0->bb13.bb44.bb1745;bb0->bb13.bb44.bb2047=(bbk)bb0->bb13.bb44.
bb1745&(2048 -1 );bbm(((bb0->bb13.bb44.bb1272&0x7FFFFFFFL )==0 ))bb2371(
bb0);bb0->bb452->bb1843[(bbk)bb0->bb13.bb44.bb1272&(2048 -1 )]=bb0->
bb13.bb44.bb1911= *bb0->bb13.bb44.bb1752++;bb0->bb13.bb44.bb1691=(bb0
->bb13.bb44.bb1691<<8 )+bb0->bb13.bb44.bb1911;--bb0->bb13.bb44.bb1028;
bb0->bb13.bb44.bb2269=bb0->bb452->bb2082+((((bb0->bb13.bb44.bb1691)&
0xFF00 )>>4 )^((bb0->bb13.bb44.bb1691)&0x00FF ));bbm((bb0->bb13.bb44.
bb2288=bb0->bb13.bb44.bb1745- *bb0->bb13.bb44.bb2269)>2048 -2 ){bb0->
bb452->bb1873[bb0->bb13.bb44.bb2047]=0 ;bbm(bb0->bb13.bb44.bb545!=0 ){
bb2076(bb0,bb0->bb13.bb44.bb1333,bb0->bb13.bb44.bb545);}bb54{bb0->
bb13.bb44.bb1001=((bb0->bb13.bb44.bb1691>>8 ));bb0->bb13.bb44.bb1055=(
9 );bb1099(bb0);bb0->bb1749=bb2227;}}bb54{bb0->bb452->bb1873[bb0->bb13
.bb44.bb2047]=(bbk)bb0->bb13.bb44.bb2288;bbm(bb0->bb13.bb44.bb545!=0 ){
bbm((bb0->bb452->bb1843[(bbk)(((bbd)bb0->bb13.bb44.bb1257+bb0->bb13.
bb44.bb545+bb0->bb13.bb44.bb1839)&(bbd)(2048 -1 ))]==bb0->bb13.bb44.
bb1911)&&((bb0->bb13.bb44.bb545+bb0->bb13.bb44.bb1839)<(bbd)0xFFFFFFFFL
)){++bb0->bb13.bb44.bb545;bb0->bb1749=bb2486;bbm(bb0->bb13.bb44.
bb1821){bbm(bb0->bb13.bb44.bb545>=23 ){bb0->bb13.bb44.bb1001=(((1 <<4 )-
1 ));bb0->bb13.bb44.bb1055=(4 );bb1099(bb0);bb0->bb13.bb44.bb545-=((1 <<
4 )-1 );bb0->bb13.bb44.bb1839+=((1 <<4 )-1 );}}bb54 bbm(bb0->bb13.bb44.
bb545>=23 ){bbm(bb0->bb13.bb44.bb1333<128 ){bb0->bb13.bb44.bb1001=(
0x180 |bb0->bb13.bb44.bb1333);bb0->bb13.bb44.bb1055=(9 );bb1099(bb0);}
bb54{bb0->bb13.bb44.bb1001=(0x1000 |bb0->bb13.bb44.bb1333);bb0->bb13.
bb44.bb1055=(13 );bb1099(bb0);}bb0->bb13.bb44.bb1001=(((1 <<4 )-1 ));bb0
->bb13.bb44.bb1055=(4 );bb1099(bb0);bb0->bb13.bb44.bb545-=((1 <<4 )-1 );
bb0->bb13.bb44.bb1839+=((1 <<4 )-1 );bb0->bb13.bb44.bb1821=1 ;}}bb54 bbm(
bb0->bb13.bb44.bb1821){bb0->bb13.bb44.bb1001=(bb2154[(bbk)bb0->bb13.
bb44.bb545].bb2314);bb0->bb13.bb44.bb1055=(bb2154[(bbk)bb0->bb13.bb44
.bb545].bb47);bb1099(bb0);bb0->bb13.bb44.bb545=0 ;bb0->bb13.bb44.
bb1839=0 ;bb0->bb13.bb44.bb1821=0 ;bb0->bb1749=bb2385;}bb54 bbm(bb0->
bb13.bb44.bb545>=8 ){bb2076(bb0,bb0->bb13.bb44.bb1333,bb0->bb13.bb44.
bb545);}bb54{bb2039=0 ;bb0->bb13.bb44.bb2098=bb0->bb13.bb44.bb1333;
bb108((bb0->bb452->bb1873[bb0->bb13.bb44.bb1257]!=0 )&&(bb2039==0 )&&(
bb0->bb13.bb44.bb2058<bb2156)&&(bb0->bb13.bb44.bb2098<(bbk)(2048 -bb0
->bb13.bb44.bb545))){bb0->bb13.bb44.bb2098+=bb0->bb452->bb1873[bb0->
bb13.bb44.bb1257];++bb0->bb13.bb44.bb2058;bbm(bb0->bb13.bb44.bb2098<(
bbk)(2048 -bb0->bb13.bb44.bb545)){bb0->bb13.bb44.bb1257=bb0->bb13.bb44
.bb1257-bb0->bb452->bb1873[bb0->bb13.bb44.bb1257]&(2048 -1 );bbm(bb0->
bb452->bb1843[bb0->bb13.bb44.bb1257]==bb0->bb452->bb1843[bb0->bb13.
bb44.bb2246]){bb2039=1 ;bb90(bb2181=2 ,bb2166=(bb0->bb13.bb44.bb1257+2 )&
(2048 -1 );bb2181<=(bbk)bb0->bb13.bb44.bb545;bb2181++,bb2166=(bb2166+1 )&
(2048 -1 )){bbm(bb0->bb452->bb1843[bb2166]!=bb0->bb452->bb1843[(bb0->
bb13.bb44.bb2246+bb2181)&(2048 -1 )]){bb2039=0 ;bb21;}}}}}bbm(bb2039){
bb0->bb13.bb44.bb1333=bb0->bb13.bb44.bb2098;++bb0->bb13.bb44.bb545;
bb0->bb1749=bb2471;}bb54{bb2076(bb0,bb0->bb13.bb44.bb1333,bb0->bb13.
bb44.bb545);}}}bb54{bb0->bb13.bb44.bb2049=(bbk)bb0->bb13.bb44.bb2288;
bb0->bb13.bb44.bb2058=0 ;bb564{bb0->bb13.bb44.bb1257=(bbk)(bb0->bb13.
bb44.bb1745-bb0->bb13.bb44.bb2049&(2048 -1 ));bbm(bb0->bb452->bb1843[
bb0->bb13.bb44.bb1257]==(bbf)(bb0->bb13.bb44.bb1691>>8 )){bb0->bb13.
bb44.bb545=2 ;bb0->bb13.bb44.bb2246=bb0->bb13.bb44.bb2047;bb0->bb13.
bb44.bb1333=bb0->bb13.bb44.bb2049;bb0->bb1749=bb2560;bb21;}bb54{bb0->
bb13.bb44.bb2049+=bb0->bb452->bb1873[bb0->bb13.bb44.bb1257];++bb0->
bb13.bb44.bb2058;}}bb108((bb0->bb452->bb1873[bb0->bb13.bb44.bb1257]!=
0 )&&(bb0->bb13.bb44.bb2058<bb2156)&&(bb0->bb13.bb44.bb2049<2048 -2 ));
bbm(bb0->bb13.bb44.bb545==0 ){bb0->bb13.bb44.bb1001=((bb0->bb13.bb44.
bb1691>>8 ));bb0->bb13.bb44.bb1055=(9 );bb1099(bb0);bb0->bb1749=bb2227;
}}}bbm(bb2479[bb0->bb1749][bb0->bb2110]){ *bb0->bb13.bb44.bb2269=bb0
->bb13.bb44.bb1745;}}bbm(bb0->bb13.bb44.bb1028==0 ){bb1038=1 ;bbm(bb372
&0x01 ){bb2192(bb0,bb372);bb1038|=4 ;}}bbm((bb0->bb13.bb44.bb600==0 )||(
bb0->bb13.bb44.bb1998)){bbm(!(bb1038&1 )){bb1038=2 ;bbm(bb372&0x02 ){
bb2192(bb0,bb372);bb1038|=4 ;}}bb54{bb1038|=3 ;bbm((!(bb372&0x01 ))&&(
bb372&0x02 )){bb2192(bb0,bb372);bb1038|=4 ;}}}bb0->bb13.bb44.bb600+=15 ;
}bb54{bb1038=1 ;bbm(bb372&0x01 ){bb2192(bb0,bb372);bb1038|=4 ;}}bb0->
bb452->bb44=bb0->bb13.bb44; *bb1739=bb0->bb13.bb44.bb1752; *bb913=bb0
->bb13.bb44.bb1028; *bb1733=bb0->bb13.bb44.bb1743; *bb597=bb0->bb13.
bb44.bb600;bb2(bb1038);}bb41 bbb bb2121(bb479 bb0){bb0->bb13.bb80.
bb2089&=(2048 -1 );bb0->bb13.bb80.bb1043=bb0->bb13.bb80.bb1151=bb0->
bb13.bb80.bb92=0 ;bb0->bb13.bb80.bb995=bb0->bb13.bb80.bb1757=bb0->bb13
.bb80.bb1173=0 ;bb0->bb13.bb80.bb469=0 ;bb0->bb13.bb80.bb1970=0 ;bb0->
bb13.bb80.bb1164=bb2081;bb0->bb13.bb80.bb2438=1 ;bb2;}bb41 bbk bb2280(
bb479 bb0){bbm((bb0->bb13.bb80.bb1028==0 )&&(bb0->bb13.bb80.bb1043==0 ))bb0
->bb13.bb80.bb995=bb2373;bb54{bb0->bb13.bb80.bb995=bb1247;bbm(bb0->
bb13.bb80.bb1043==0 ){bb0->bb13.bb80.bb1757= *bb0->bb13.bb80.bb1752++;
--bb0->bb13.bb80.bb1028;bb0->bb13.bb80.bb1043=7 ;bb0->bb13.bb80.bb1173
=(bb0->bb13.bb80.bb1757>127 )?1 :0 ;bb0->bb13.bb80.bb1757&=((bbf)0x7F );}
bb54{bb0->bb13.bb80.bb1173=(bb0->bb13.bb80.bb1757>>(bb0->bb13.bb80.
bb1043-1 ));--bb0->bb13.bb80.bb1043;bb0->bb13.bb80.bb1757&=((bbf)0xFF
>>(8 -bb0->bb13.bb80.bb1043));}}bb2(bb0->bb13.bb80.bb995);}bb41 bbk
bb1847(bb479 bb0){bbk bb2153;bb124 bb8;bbm(bb0->bb13.bb80.bb995==
bb1247)bb0->bb13.bb80.bb1173=0 ;bb54 bb0->bb13.bb80.bb995=bb1247;bb108
((bb0->bb13.bb80.bb1151>0 )&&(bb0->bb13.bb80.bb995==bb1247)){bbm((bb0
->bb13.bb80.bb1028==0 )&&(bb0->bb13.bb80.bb1043==0 ))bb0->bb13.bb80.
bb995=bb2373;bb54{bbm(bb0->bb13.bb80.bb1043==0 ){bb0->bb13.bb80.bb1757
= *bb0->bb13.bb80.bb1752++;--bb0->bb13.bb80.bb1028;bb0->bb13.bb80.
bb1043=8 ;}bb2153=bb0->bb13.bb80.bb1757;bbm((bb8=bb0->bb13.bb80.bb1151
-bb0->bb13.bb80.bb1043)>0 )bb2153<<=bb8;bb54 bb2153>>=-bb8;bb0->bb13.
bb80.bb1173|=bb2153;bb8=((((8 )<(bb0->bb13.bb80.bb1151)?(8 ):(bb0->bb13
.bb80.bb1151)))<(bb0->bb13.bb80.bb1043)?(((8 )<(bb0->bb13.bb80.bb1151)?
(8 ):(bb0->bb13.bb80.bb1151))):(bb0->bb13.bb80.bb1043));bb0->bb13.bb80
.bb1151-=bb8;bb0->bb13.bb80.bb1043-=bb8;bb0->bb13.bb80.bb1757&=((bbf)0xFF
>>(8 -bb0->bb13.bb80.bb1043));}}bb2(bb0->bb13.bb80.bb995);}bb41 bbb
bb2071(bb479 bb0,bbf bbo){bbm(bb0->bb13.bb80.bb600!=0 ){ *bb0->bb13.
bb80.bb1743++=bbo;--bb0->bb13.bb80.bb600;bb0->bb452->bb2162[bb0->bb13
.bb80.bb2089++]=(bbf)bbo;bb0->bb13.bb80.bb2089&=(2048 -1 );}}bbk bb2290
(bb479 bb0,bbf* *bb1739,bbf* *bb1733,bbd*bb913,bbd*bb597,bbb*bb1315,
bbk bb372){bbk bb2196=0 ;bbk bb1038=0 ;bb0->bb452=(bb2045* )bb1315;bb0
->bb13.bb80=bb0->bb452->bb80;bb0->bb13.bb80.bb1752= *bb1739;bb0->bb13
.bb80.bb1028= *bb913;bb0->bb13.bb80.bb1743= *bb1733;bb0->bb13.bb80.
bb600= *bb597;bbm(bb372&0x01 ){bb2121(bb0);}bbm((bb0->bb13.bb80.bb1028
!=0 )&&(bb0->bb13.bb80.bb600!=0 )){bb108((bb0->bb13.bb80.bb600!=0 )&&((
bb0->bb13.bb80.bb1028!=0 )||(bb0->bb13.bb80.bb1043!=0 ))&&(bb2196==0 )){
bbm(bb0->bb13.bb80.bb1970){bb108((bb0->bb13.bb80.bb600!=0 )&&(bb0->
bb13.bb80.bb469>0 )){bb0->bb13.bb80.bb92&=(2048 -1 );bb2071(bb0,bb0->
bb452->bb2162[bb0->bb13.bb80.bb92++]);--bb0->bb13.bb80.bb469;}bbm(bb0
->bb13.bb80.bb469==0 )bb0->bb13.bb80.bb1970=0 ;bb0->bb13.bb80.bb1164=
bb2081;}bb54{bb297(bb0->bb13.bb80.bb1164){bb15 bb2081:bb2280(bb0);bbm
(bb0->bb13.bb80.bb1173==0 ){bb0->bb13.bb80.bb1151=8 ;bb0->bb13.bb80.
bb1164=bb2344;bb15 bb2344:bb1847(bb0);bbm(bb0->bb13.bb80.bb995==
bb1247){bb2071(bb0,(bbf)bb0->bb13.bb80.bb1173);bb0->bb13.bb80.bb1164=
bb2081;}}bb54{bb0->bb13.bb80.bb1164=bb2379;bb15 bb2379:bb2280(bb0);
bbm(bb0->bb13.bb80.bb995==bb1247){bb0->bb13.bb80.bb1151=bb0->bb13.
bb80.bb1173?7 :11 ;bb0->bb13.bb80.bb1164=bb2347;bb15 bb2347:bb1847(bb0);
bbm(bb0->bb13.bb80.bb995==bb1247){bb0->bb13.bb80.bb92=bb0->bb13.bb80.
bb1173;bbm(bb0->bb13.bb80.bb92==0 )bb2196=1 ;bb54{bb0->bb13.bb80.bb92=
bb0->bb13.bb80.bb2089-bb0->bb13.bb80.bb92;bb0->bb13.bb80.bb1151=2 ;bb0
->bb13.bb80.bb1164=bb2306;bb15 bb2306:bb1847(bb0);bbm(bb0->bb13.bb80.
bb995==bb1247){bb0->bb13.bb80.bb469=2 +bb0->bb13.bb80.bb1173;bbm(bb0->
bb13.bb80.bb469==5 ){bb0->bb13.bb80.bb1151=2 ;bb0->bb13.bb80.bb1164=
bb2305;bb15 bb2305:bb1847(bb0);bbm(bb0->bb13.bb80.bb995==bb1247){bb0
->bb13.bb80.bb469+=bb0->bb13.bb80.bb1173;bbm(bb0->bb13.bb80.bb469==8 ){
bb0->bb13.bb80.bb1151=4 ;bb0->bb13.bb80.bb1164=bb2421;bb15 bb2421:
bb1847(bb0);bbm(bb0->bb13.bb80.bb995==bb1247){bb0->bb13.bb80.bb469+=
bb0->bb13.bb80.bb1173;bbm(bb0->bb13.bb80.bb469==23 ){bb564{bb15 bb2234
:bb108((bb0->bb13.bb80.bb600!=0 )&&(bb0->bb13.bb80.bb469>0 )){bb0->bb13
.bb80.bb92&=(2048 -1 );bb2071(bb0,bb0->bb452->bb2162[bb0->bb13.bb80.
bb92++]);--bb0->bb13.bb80.bb469;}bbm(bb0->bb13.bb80.bb600==0 ){bb0->
bb13.bb80.bb1164=bb2234;bb21;}bb54{bb0->bb13.bb80.bb1151=4 ;bb0->bb13.
bb80.bb1164=bb2363;bb15 bb2363:bb1847(bb0);bbm(bb0->bb13.bb80.bb995==
bb1247)bb0->bb13.bb80.bb469+=bb0->bb13.bb80.bb1173;bb54 bb21;}}bb108(
bb0->bb13.bb80.bb1173==((1 <<4 )-1 ));}}}}}}bbm((bb0->bb13.bb80.bb995==
bb1247)&&(bb0->bb13.bb80.bb1164!=bb2234)){bb0->bb13.bb80.bb1970=1 ;}}}
}}}}}bbm(bb0->bb13.bb80.bb1970){bb108((bb0->bb13.bb80.bb600!=0 )&&(bb0
->bb13.bb80.bb469>0 )){bb0->bb13.bb80.bb92&=(2048 -1 );bb2071(bb0,bb0->
bb452->bb2162[bb0->bb13.bb80.bb92++]);--bb0->bb13.bb80.bb469;}bbm(bb0
->bb13.bb80.bb469==0 )bb0->bb13.bb80.bb1970=0 ;bb0->bb13.bb80.bb1164=
bb2081;}}bbm(bb2196){bb2121(bb0);bb1038|=4 ;}bbm(bb0->bb13.bb80.bb1028
==0 ){bb1038|=1 ;}bbm(bb0->bb13.bb80.bb600==0 ){bb1038|=2 ;}bb0->bb452->
bb80=bb0->bb13.bb80; *bb1739=bb0->bb13.bb80.bb1752; *bb913=bb0->bb13.
bb80.bb1028; *bb1733=bb0->bb13.bb80.bb1743; *bb597=bb0->bb13.bb80.
bb600;bb2(bb1038);}
