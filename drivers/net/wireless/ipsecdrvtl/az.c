/*
   'src_compress_deflate_inftrees.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Fri Oct 12 22:15:16 2012
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
((bb166* ,bb166* ,bb152* * ,bb152* * ,bb17));
#if ! defined( bb2248) && ! defined( bb136)
#define bb2248
#endif
bbi bb379{bbe bb446;};bb41 bbe bb1974 bbp((bb166* ,bb7,bb7,bbh bb166*
,bbh bb166* ,bb152* * ,bb166* ,bb152* ,bb7* ,bb166* ));bb41 bbh bb7
bb2368[31 ]={3 ,4 ,5 ,6 ,7 ,8 ,9 ,10 ,11 ,13 ,15 ,17 ,19 ,23 ,27 ,31 ,35 ,43 ,51 ,59 ,67 ,
83 ,99 ,115 ,131 ,163 ,195 ,227 ,258 ,0 ,0 };bb41 bbh bb7 bb2358[31 ]={0 ,0 ,0 ,0 ,0
,0 ,0 ,0 ,1 ,1 ,1 ,1 ,2 ,2 ,2 ,2 ,3 ,3 ,3 ,3 ,4 ,4 ,4 ,4 ,5 ,5 ,5 ,5 ,0 ,112 ,112 };bb41 bbh bb7
bb2404[30 ]={1 ,2 ,3 ,4 ,5 ,7 ,9 ,13 ,17 ,25 ,33 ,49 ,65 ,97 ,129 ,193 ,257 ,385 ,513 ,
769 ,1025 ,1537 ,2049 ,3073 ,4097 ,6145 ,8193 ,12289 ,16385 ,24577 };bb41 bbh bb7
bb2405[30 ]={0 ,0 ,0 ,0 ,1 ,1 ,2 ,2 ,3 ,3 ,4 ,4 ,5 ,5 ,6 ,6 ,7 ,7 ,8 ,8 ,9 ,9 ,10 ,10 ,11 ,11 ,
12 ,12 ,13 ,13 };bb41 bbe bb1974(bbn,bb11,bbg,bbt,bbw,bb45,bb97,bb1788,
bb1789,bb438)bb166*bbn;bb7 bb11;bb7 bbg;bbh bb166*bbt;bbh bb166*bbw;
bb152* *bb45;bb166*bb97;bb152*bb1788;bb7*bb1789;bb166*bb438;{bb7 bbc;
bb7 bbo[15 +1 ];bb7 bb19;bbe bb52;bbe bb42;bb920 bb7 bbz;bb920 bb7 bb75
;bb920 bbe bb3;bbe bb197;bb7 bb1171;bb920 bb166*bb27;bb152*bb84;bbi
bb1372 bb35={{{0 }},0 };bb152*bb317[15 ];bb920 bbe bbv;bb7 bb8[15 +1 ];
bb166*bb2084;bbe bb193;bb7 bby;bb27=bbo; *bb27++=0 ; *bb27++=0 ; *bb27
++=0 ; *bb27++=0 ; *bb27++=0 ; *bb27++=0 ; *bb27++=0 ; *bb27++=0 ; *bb27++=
0 ; *bb27++=0 ; *bb27++=0 ; *bb27++=0 ; *bb27++=0 ; *bb27++=0 ; *bb27++=0 ; *
bb27++=0 ;bb27=bbn;bbz=bb11;bb564{bbo[ *bb27++]++;}bb108(--bbz);bbm(
bbo[0 ]==bb11){ *bb45=(bb152* )0 ; *bb97=0 ;bb2 0 ;}bb197= *bb97;bb90(
bb75=1 ;bb75<=15 ;bb75++)bbm(bbo[bb75])bb21;bb3=bb75;bbm((bb7)bb197<
bb75)bb197=bb75;bb90(bbz=15 ;bbz;bbz--)bbm(bbo[bbz])bb21;bb52=bbz;bbm(
(bb7)bb197>bbz)bb197=bbz; *bb97=bb197;bb90(bb193=1 <<bb75;bb75<bbz;
bb75++,bb193<<=1 )bbm((bb193-=bbo[bb75])<0 )bb2(-3 );bbm((bb193-=bbo[bbz
])<0 )bb2(-3 );bbo[bbz]+=bb193;bb8[1 ]=bb75=0 ;bb27=bbo+1 ;bb2084=bb8+2 ;
bb108(--bbz){ *bb2084++=(bb75+= *bb27++);}bb27=bbn;bbz=0 ;bb564{bbm((
bb75= *bb27++)!=0 )bb438[bb8[bb75]++]=bbz;}bb108(++bbz<bb11);bb11=bb8[
bb52];bb8[0 ]=bbz=0 ;bb27=bb438;bb42=-1 ;bbv=-bb197;bb317[0 ]=(bb152* )0 ;
bb84=(bb152* )0 ;bby=0 ;bb90(;bb3<=bb52;bb3++){bbc=bbo[bb3];bb108(bbc--
){bb108(bb3>bbv+bb197){bb42++;bbv+=bb197;bby=bb52-bbv;bby=bby>(bb7)bb197
?(bb7)bb197:bby;bbm((bb19=1 <<(bb75=bb3-bbv))>bbc+1 ){bb19-=bbc+1 ;
bb2084=bbo+bb3;bbm(bb75<bby)bb108(++bb75<bby){bbm((bb19<<=1 )<= * ++
bb2084)bb21;bb19-= *bb2084;}}bby=1 <<bb75;bbm( *bb1789+bby>1440 )bb2(-4
);bb317[bb42]=bb84=bb1788+ *bb1789; *bb1789+=bby;bbm(bb42){bb8[bb42]=
bbz;bb35.bb509.bb503.bb961=(bb153)bb197;bb35.bb509.bb503.bb1169=(
bb153)bb75;bb75=bbz>>(bbv-bb197);bb35.bb608=(bb7)(bb84-bb317[bb42-1 ]-
bb75);bb317[bb42-1 ][bb75]=bb35;}bb54*bb45=bb84;}bb35.bb509.bb503.
bb961=(bb153)(bb3-bbv);bbm(bb27>=bb438+bb11)bb35.bb509.bb503.bb1169=
128 +64 ;bb54 bbm( *bb27<bbg){bb35.bb509.bb503.bb1169=(bb153)( *bb27<
256 ?0 :32 +64 );bb35.bb608= *bb27++;}bb54{bb35.bb509.bb503.bb1169=(bb153
)(bbw[ *bb27-bbg]+16 +64 );bb35.bb608=bbt[ *bb27++-bbg];}bb19=1 <<(bb3-
bbv);bb90(bb75=bbz>>bbv;bb75<bby;bb75+=bb19)bb84[bb75]=bb35;bb90(bb75
=1 <<(bb3-1 );bbz&bb75;bb75>>=1 )bbz^=bb75;bbz^=bb75;bb1171=(1 <<bbv)-1 ;
bb108((bbz&bb1171)!=bb8[bb42]){bb42--;bbv-=bb197;bb1171=(1 <<bbv)-1 ;}}
}bb2 bb193!=0 &&bb52!=1 ?(-5 ):0 ;}bbe bb2013(bbo,bb1721,bb1771,bb1788,
bby)bb166*bbo;bb166*bb1721;bb152* *bb1771;bb152*bb1788;bb17 bby;{bbe
bb35;bb7 bb1789=0 ;bb166*bb438;bbm((bb438=(bb166* )( * ((bby)->bb401))(
(bby)->bb110,(19 ),(bb12(bb7))))==0 )bb2(-4 );bb35=bb1974(bbo,19 ,19 ,(
bb166* )0 ,(bb166* )0 ,bb1771,bb1721,bb1788,&bb1789,bb438);bbm(bb35==(-
3 ))bby->bb321=(bbl* )"";bb54 bbm(bb35==(-5 )|| *bb1721==0 ){bby->bb321=
(bbl* )"";bb35=(-3 );}( * ((bby)->bb370))((bby)->bb110,(bb70)(bb438));
bb2 bb35;}bbe bb1996(bb2174,bb2390,bbo,bb55,bb940,bb1014,bb1015,
bb1788,bby)bb7 bb2174;bb7 bb2390;bb166*bbo;bb166*bb55;bb166*bb940;
bb152* *bb1014;bb152* *bb1015;bb152*bb1788;bb17 bby;{bbe bb35;bb7
bb1789=0 ;bb166*bb438;bbm((bb438=(bb166* )( * ((bby)->bb401))((bby)->
bb110,(288 ),(bb12(bb7))))==0 )bb2(-4 );bb35=bb1974(bbo,bb2174,257 ,
bb2368,bb2358,bb1014,bb55,bb1788,&bb1789,bb438);bbm(bb35!=0 || *bb55==
0 ){bbm(bb35==(-3 ))bby->bb321=(bbl* )"";bb54 bbm(bb35!=(-4 )){bby->
bb321=(bbl* )"";bb35=(-3 );}( * ((bby)->bb370))((bby)->bb110,(bb70)(
bb438));bb2 bb35;}bb35=bb1974(bbo+bb2174,bb2390,0 ,bb2404,bb2405,
bb1015,bb940,bb1788,&bb1789,bb438);bbm(bb35!=0 ||( *bb940==0 &&bb2174>
257 )){bbm(bb35==(-3 ))bby->bb321=(bbl* )"";bb54 bbm(bb35==(-5 )){bby->
bb321=(bbl* )"";bb35=(-3 );}bb54 bbm(bb35!=(-4 )){bby->bb321=(bbl* )"";
bb35=(-3 );}( * ((bby)->bb370))((bby)->bb110,(bb70)(bb438));bb2 bb35;}
( * ((bby)->bb370))((bby)->bb110,(bb70)(bb438));bb2 0 ;}
#ifdef bb2248
bb41 bbe bb2334=0 ;
#define bb2539 544
bb41 bb152 bb2427[bb2539];bb41 bb7 bb2128;bb41 bb7 bb2130;bb41 bb152*
bb2263;bb41 bb152*bb2265;
#else
bb41 bb7 bb2128=9 ;bb41 bb7 bb2130=5 ;bb41 bb152 bb2263[]={{{{96 ,7 }},
256 },{{{0 ,8 }},80 },{{{0 ,8 }},16 },{{{84 ,8 }},115 },{{{82 ,7 }},31 },{{{0 ,8 }},
112 },{{{0 ,8 }},48 },{{{0 ,9 }},192 },{{{80 ,7 }},10 },{{{0 ,8 }},96 },{{{0 ,8 }},
32 },{{{0 ,9 }},160 },{{{0 ,8 }},0 },{{{0 ,8 }},128 },{{{0 ,8 }},64 },{{{0 ,9 }},224
},{{{80 ,7 }},6 },{{{0 ,8 }},88 },{{{0 ,8 }},24 },{{{0 ,9 }},144 },{{{83 ,7 }},59 },
{{{0 ,8 }},120 },{{{0 ,8 }},56 },{{{0 ,9 }},208 },{{{81 ,7 }},17 },{{{0 ,8 }},104 },
{{{0 ,8 }},40 },{{{0 ,9 }},176 },{{{0 ,8 }},8 },{{{0 ,8 }},136 },{{{0 ,8 }},72 },{{{
0 ,9 }},240 },{{{80 ,7 }},4 },{{{0 ,8 }},84 },{{{0 ,8 }},20 },{{{85 ,8 }},227 },{{{
83 ,7 }},43 },{{{0 ,8 }},116 },{{{0 ,8 }},52 },{{{0 ,9 }},200 },{{{81 ,7 }},13 },{{{
0 ,8 }},100 },{{{0 ,8 }},36 },{{{0 ,9 }},168 },{{{0 ,8 }},4 },{{{0 ,8 }},132 },{{{0 ,
8 }},68 },{{{0 ,9 }},232 },{{{80 ,7 }},8 },{{{0 ,8 }},92 },{{{0 ,8 }},28 },{{{0 ,9 }}
,152 },{{{84 ,7 }},83 },{{{0 ,8 }},124 },{{{0 ,8 }},60 },{{{0 ,9 }},216 },{{{82 ,7 }
},23 },{{{0 ,8 }},108 },{{{0 ,8 }},44 },{{{0 ,9 }},184 },{{{0 ,8 }},12 },{{{0 ,8 }},
140 },{{{0 ,8 }},76 },{{{0 ,9 }},248 },{{{80 ,7 }},3 },{{{0 ,8 }},82 },{{{0 ,8 }},18
},{{{85 ,8 }},163 },{{{83 ,7 }},35 },{{{0 ,8 }},114 },{{{0 ,8 }},50 },{{{0 ,9 }},
196 },{{{81 ,7 }},11 },{{{0 ,8 }},98 },{{{0 ,8 }},34 },{{{0 ,9 }},164 },{{{0 ,8 }},2
},{{{0 ,8 }},130 },{{{0 ,8 }},66 },{{{0 ,9 }},228 },{{{80 ,7 }},7 },{{{0 ,8 }},90 },
{{{0 ,8 }},26 },{{{0 ,9 }},148 },{{{84 ,7 }},67 },{{{0 ,8 }},122 },{{{0 ,8 }},58 },{
{{0 ,9 }},212 },{{{82 ,7 }},19 },{{{0 ,8 }},106 },{{{0 ,8 }},42 },{{{0 ,9 }},180 },{
{{0 ,8 }},10 },{{{0 ,8 }},138 },{{{0 ,8 }},74 },{{{0 ,9 }},244 },{{{80 ,7 }},5 },{{{
0 ,8 }},86 },{{{0 ,8 }},22 },{{{192 ,8 }},0 },{{{83 ,7 }},51 },{{{0 ,8 }},118 },{{{0
,8 }},54 },{{{0 ,9 }},204 },{{{81 ,7 }},15 },{{{0 ,8 }},102 },{{{0 ,8 }},38 },{{{0 ,
9 }},172 },{{{0 ,8 }},6 },{{{0 ,8 }},134 },{{{0 ,8 }},70 },{{{0 ,9 }},236 },{{{80 ,7
}},9 },{{{0 ,8 }},94 },{{{0 ,8 }},30 },{{{0 ,9 }},156 },{{{84 ,7 }},99 },{{{0 ,8 }},
126 },{{{0 ,8 }},62 },{{{0 ,9 }},220 },{{{82 ,7 }},27 },{{{0 ,8 }},110 },{{{0 ,8 }},
46 },{{{0 ,9 }},188 },{{{0 ,8 }},14 },{{{0 ,8 }},142 },{{{0 ,8 }},78 },{{{0 ,9 }},
252 },{{{96 ,7 }},256 },{{{0 ,8 }},81 },{{{0 ,8 }},17 },{{{85 ,8 }},131 },{{{82 ,7 }
},31 },{{{0 ,8 }},113 },{{{0 ,8 }},49 },{{{0 ,9 }},194 },{{{80 ,7 }},10 },{{{0 ,8 }}
,97 },{{{0 ,8 }},33 },{{{0 ,9 }},162 },{{{0 ,8 }},1 },{{{0 ,8 }},129 },{{{0 ,8 }},65
},{{{0 ,9 }},226 },{{{80 ,7 }},6 },{{{0 ,8 }},89 },{{{0 ,8 }},25 },{{{0 ,9 }},146 },
{{{83 ,7 }},59 },{{{0 ,8 }},121 },{{{0 ,8 }},57 },{{{0 ,9 }},210 },{{{81 ,7 }},17 },
{{{0 ,8 }},105 },{{{0 ,8 }},41 },{{{0 ,9 }},178 },{{{0 ,8 }},9 },{{{0 ,8 }},137 },{{
{0 ,8 }},73 },{{{0 ,9 }},242 },{{{80 ,7 }},4 },{{{0 ,8 }},85 },{{{0 ,8 }},21 },{{{80
,8 }},258 },{{{83 ,7 }},43 },{{{0 ,8 }},117 },{{{0 ,8 }},53 },{{{0 ,9 }},202 },{{{
81 ,7 }},13 },{{{0 ,8 }},101 },{{{0 ,8 }},37 },{{{0 ,9 }},170 },{{{0 ,8 }},5 },{{{0 ,
8 }},133 },{{{0 ,8 }},69 },{{{0 ,9 }},234 },{{{80 ,7 }},8 },{{{0 ,8 }},93 },{{{0 ,8 }
},29 },{{{0 ,9 }},154 },{{{84 ,7 }},83 },{{{0 ,8 }},125 },{{{0 ,8 }},61 },{{{0 ,9 }}
,218 },{{{82 ,7 }},23 },{{{0 ,8 }},109 },{{{0 ,8 }},45 },{{{0 ,9 }},186 },{{{0 ,8 }}
,13 },{{{0 ,8 }},141 },{{{0 ,8 }},77 },{{{0 ,9 }},250 },{{{80 ,7 }},3 },{{{0 ,8 }},
83 },{{{0 ,8 }},19 },{{{85 ,8 }},195 },{{{83 ,7 }},35 },{{{0 ,8 }},115 },{{{0 ,8 }},
51 },{{{0 ,9 }},198 },{{{81 ,7 }},11 },{{{0 ,8 }},99 },{{{0 ,8 }},35 },{{{0 ,9 }},
166 },{{{0 ,8 }},3 },{{{0 ,8 }},131 },{{{0 ,8 }},67 },{{{0 ,9 }},230 },{{{80 ,7 }},7
},{{{0 ,8 }},91 },{{{0 ,8 }},27 },{{{0 ,9 }},150 },{{{84 ,7 }},67 },{{{0 ,8 }},123 }
,{{{0 ,8 }},59 },{{{0 ,9 }},214 },{{{82 ,7 }},19 },{{{0 ,8 }},107 },{{{0 ,8 }},43 },
{{{0 ,9 }},182 },{{{0 ,8 }},11 },{{{0 ,8 }},139 },{{{0 ,8 }},75 },{{{0 ,9 }},246 },{
{{80 ,7 }},5 },{{{0 ,8 }},87 },{{{0 ,8 }},23 },{{{192 ,8 }},0 },{{{83 ,7 }},51 },{{{
0 ,8 }},119 },{{{0 ,8 }},55 },{{{0 ,9 }},206 },{{{81 ,7 }},15 },{{{0 ,8 }},103 },{{{
0 ,8 }},39 },{{{0 ,9 }},174 },{{{0 ,8 }},7 },{{{0 ,8 }},135 },{{{0 ,8 }},71 },{{{0 ,9
}},238 },{{{80 ,7 }},9 },{{{0 ,8 }},95 },{{{0 ,8 }},31 },{{{0 ,9 }},158 },{{{84 ,7 }
},99 },{{{0 ,8 }},127 },{{{0 ,8 }},63 },{{{0 ,9 }},222 },{{{82 ,7 }},27 },{{{0 ,8 }}
,111 },{{{0 ,8 }},47 },{{{0 ,9 }},190 },{{{0 ,8 }},15 },{{{0 ,8 }},143 },{{{0 ,8 }},
79 },{{{0 ,9 }},254 },{{{96 ,7 }},256 },{{{0 ,8 }},80 },{{{0 ,8 }},16 },{{{84 ,8 }},
115 },{{{82 ,7 }},31 },{{{0 ,8 }},112 },{{{0 ,8 }},48 },{{{0 ,9 }},193 },{{{80 ,7 }}
,10 },{{{0 ,8 }},96 },{{{0 ,8 }},32 },{{{0 ,9 }},161 },{{{0 ,8 }},0 },{{{0 ,8 }},128
},{{{0 ,8 }},64 },{{{0 ,9 }},225 },{{{80 ,7 }},6 },{{{0 ,8 }},88 },{{{0 ,8 }},24 },{
{{0 ,9 }},145 },{{{83 ,7 }},59 },{{{0 ,8 }},120 },{{{0 ,8 }},56 },{{{0 ,9 }},209 },{
{{81 ,7 }},17 },{{{0 ,8 }},104 },{{{0 ,8 }},40 },{{{0 ,9 }},177 },{{{0 ,8 }},8 },{{{
0 ,8 }},136 },{{{0 ,8 }},72 },{{{0 ,9 }},241 },{{{80 ,7 }},4 },{{{0 ,8 }},84 },{{{0 ,
8 }},20 },{{{85 ,8 }},227 },{{{83 ,7 }},43 },{{{0 ,8 }},116 },{{{0 ,8 }},52 },{{{0 ,
9 }},201 },{{{81 ,7 }},13 },{{{0 ,8 }},100 },{{{0 ,8 }},36 },{{{0 ,9 }},169 },{{{0 ,
8 }},4 },{{{0 ,8 }},132 },{{{0 ,8 }},68 },{{{0 ,9 }},233 },{{{80 ,7 }},8 },{{{0 ,8 }}
,92 },{{{0 ,8 }},28 },{{{0 ,9 }},153 },{{{84 ,7 }},83 },{{{0 ,8 }},124 },{{{0 ,8 }},
60 },{{{0 ,9 }},217 },{{{82 ,7 }},23 },{{{0 ,8 }},108 },{{{0 ,8 }},44 },{{{0 ,9 }},
185 },{{{0 ,8 }},12 },{{{0 ,8 }},140 },{{{0 ,8 }},76 },{{{0 ,9 }},249 },{{{80 ,7 }},
3 },{{{0 ,8 }},82 },{{{0 ,8 }},18 },{{{85 ,8 }},163 },{{{83 ,7 }},35 },{{{0 ,8 }},
114 },{{{0 ,8 }},50 },{{{0 ,9 }},197 },{{{81 ,7 }},11 },{{{0 ,8 }},98 },{{{0 ,8 }},
34 },{{{0 ,9 }},165 },{{{0 ,8 }},2 },{{{0 ,8 }},130 },{{{0 ,8 }},66 },{{{0 ,9 }},229
},{{{80 ,7 }},7 },{{{0 ,8 }},90 },{{{0 ,8 }},26 },{{{0 ,9 }},149 },{{{84 ,7 }},67 },
{{{0 ,8 }},122 },{{{0 ,8 }},58 },{{{0 ,9 }},213 },{{{82 ,7 }},19 },{{{0 ,8 }},106 },
{{{0 ,8 }},42 },{{{0 ,9 }},181 },{{{0 ,8 }},10 },{{{0 ,8 }},138 },{{{0 ,8 }},74 },{{
{0 ,9 }},245 },{{{80 ,7 }},5 },{{{0 ,8 }},86 },{{{0 ,8 }},22 },{{{192 ,8 }},0 },{{{
83 ,7 }},51 },{{{0 ,8 }},118 },{{{0 ,8 }},54 },{{{0 ,9 }},205 },{{{81 ,7 }},15 },{{{
0 ,8 }},102 },{{{0 ,8 }},38 },{{{0 ,9 }},173 },{{{0 ,8 }},6 },{{{0 ,8 }},134 },{{{0 ,
8 }},70 },{{{0 ,9 }},237 },{{{80 ,7 }},9 },{{{0 ,8 }},94 },{{{0 ,8 }},30 },{{{0 ,9 }}
,157 },{{{84 ,7 }},99 },{{{0 ,8 }},126 },{{{0 ,8 }},62 },{{{0 ,9 }},221 },{{{82 ,7 }
},27 },{{{0 ,8 }},110 },{{{0 ,8 }},46 },{{{0 ,9 }},189 },{{{0 ,8 }},14 },{{{0 ,8 }},
142 },{{{0 ,8 }},78 },{{{0 ,9 }},253 },{{{96 ,7 }},256 },{{{0 ,8 }},81 },{{{0 ,8 }},
17 },{{{85 ,8 }},131 },{{{82 ,7 }},31 },{{{0 ,8 }},113 },{{{0 ,8 }},49 },{{{0 ,9 }},
195 },{{{80 ,7 }},10 },{{{0 ,8 }},97 },{{{0 ,8 }},33 },{{{0 ,9 }},163 },{{{0 ,8 }},1
},{{{0 ,8 }},129 },{{{0 ,8 }},65 },{{{0 ,9 }},227 },{{{80 ,7 }},6 },{{{0 ,8 }},89 },
{{{0 ,8 }},25 },{{{0 ,9 }},147 },{{{83 ,7 }},59 },{{{0 ,8 }},121 },{{{0 ,8 }},57 },{
{{0 ,9 }},211 },{{{81 ,7 }},17 },{{{0 ,8 }},105 },{{{0 ,8 }},41 },{{{0 ,9 }},179 },{
{{0 ,8 }},9 },{{{0 ,8 }},137 },{{{0 ,8 }},73 },{{{0 ,9 }},243 },{{{80 ,7 }},4 },{{{0
,8 }},85 },{{{0 ,8 }},21 },{{{80 ,8 }},258 },{{{83 ,7 }},43 },{{{0 ,8 }},117 },{{{0
,8 }},53 },{{{0 ,9 }},203 },{{{81 ,7 }},13 },{{{0 ,8 }},101 },{{{0 ,8 }},37 },{{{0 ,
9 }},171 },{{{0 ,8 }},5 },{{{0 ,8 }},133 },{{{0 ,8 }},69 },{{{0 ,9 }},235 },{{{80 ,7
}},8 },{{{0 ,8 }},93 },{{{0 ,8 }},29 },{{{0 ,9 }},155 },{{{84 ,7 }},83 },{{{0 ,8 }},
125 },{{{0 ,8 }},61 },{{{0 ,9 }},219 },{{{82 ,7 }},23 },{{{0 ,8 }},109 },{{{0 ,8 }},
45 },{{{0 ,9 }},187 },{{{0 ,8 }},13 },{{{0 ,8 }},141 },{{{0 ,8 }},77 },{{{0 ,9 }},
251 },{{{80 ,7 }},3 },{{{0 ,8 }},83 },{{{0 ,8 }},19 },{{{85 ,8 }},195 },{{{83 ,7 }},
35 },{{{0 ,8 }},115 },{{{0 ,8 }},51 },{{{0 ,9 }},199 },{{{81 ,7 }},11 },{{{0 ,8 }},
99 },{{{0 ,8 }},35 },{{{0 ,9 }},167 },{{{0 ,8 }},3 },{{{0 ,8 }},131 },{{{0 ,8 }},67 }
,{{{0 ,9 }},231 },{{{80 ,7 }},7 },{{{0 ,8 }},91 },{{{0 ,8 }},27 },{{{0 ,9 }},151 },{
{{84 ,7 }},67 },{{{0 ,8 }},123 },{{{0 ,8 }},59 },{{{0 ,9 }},215 },{{{82 ,7 }},19 },{
{{0 ,8 }},107 },{{{0 ,8 }},43 },{{{0 ,9 }},183 },{{{0 ,8 }},11 },{{{0 ,8 }},139 },{{
{0 ,8 }},75 },{{{0 ,9 }},247 },{{{80 ,7 }},5 },{{{0 ,8 }},87 },{{{0 ,8 }},23 },{{{
192 ,8 }},0 },{{{83 ,7 }},51 },{{{0 ,8 }},119 },{{{0 ,8 }},55 },{{{0 ,9 }},207 },{{{
81 ,7 }},15 },{{{0 ,8 }},103 },{{{0 ,8 }},39 },{{{0 ,9 }},175 },{{{0 ,8 }},7 },{{{0 ,
8 }},135 },{{{0 ,8 }},71 },{{{0 ,9 }},239 },{{{80 ,7 }},9 },{{{0 ,8 }},95 },{{{0 ,8 }
},31 },{{{0 ,9 }},159 },{{{84 ,7 }},99 },{{{0 ,8 }},127 },{{{0 ,8 }},63 },{{{0 ,9 }}
,223 },{{{82 ,7 }},27 },{{{0 ,8 }},111 },{{{0 ,8 }},47 },{{{0 ,9 }},191 },{{{0 ,8 }}
,15 },{{{0 ,8 }},143 },{{{0 ,8 }},79 },{{{0 ,9 }},255 }};bb41 bb152 bb2265[]={{
{{80 ,5 }},1 },{{{87 ,5 }},257 },{{{83 ,5 }},17 },{{{91 ,5 }},4097 },{{{81 ,5 }},5 }
,{{{89 ,5 }},1025 },{{{85 ,5 }},65 },{{{93 ,5 }},16385 },{{{80 ,5 }},3 },{{{88 ,5 }
},513 },{{{84 ,5 }},33 },{{{92 ,5 }},8193 },{{{82 ,5 }},9 },{{{90 ,5 }},2049 },{{{
86 ,5 }},129 },{{{192 ,5 }},24577 },{{{80 ,5 }},2 },{{{87 ,5 }},385 },{{{83 ,5 }},
25 },{{{91 ,5 }},6145 },{{{81 ,5 }},7 },{{{89 ,5 }},1537 },{{{85 ,5 }},97 },{{{93 ,
5 }},24577 },{{{80 ,5 }},4 },{{{88 ,5 }},769 },{{{84 ,5 }},49 },{{{92 ,5 }},12289 }
,{{{82 ,5 }},13 },{{{90 ,5 }},3073 },{{{86 ,5 }},193 },{{{192 ,5 }},24577 }};
#endif
bbe bb1966(bb55,bb940,bb1014,bb1015,bby)bb166*bb55;bb166*bb940;bb152*
 *bb1014;bb152* *bb1015;bb17 bby;{(bbb)bby;
#ifdef bb2248
bbm(!bb2334){bbe bb3;bb7 bb19=0 ;bb166*bbo;bb166*bb438;bbm((bbo=(bb166
 * )( * ((bby)->bb401))((bby)->bb110,(288 ),(bb12(bb7))))==0 )bb2(-4 );
bbm((bb438=(bb166* )( * ((bby)->bb401))((bby)->bb110,(288 ),(bb12(bb7))))==
0 ){( * ((bby)->bb370))((bby)->bb110,(bb70)(bbo));bb2(-4 );}bb90(bb3=0 ;
bb3<144 ;bb3++)bbo[bb3]=8 ;bb90(;bb3<256 ;bb3++)bbo[bb3]=9 ;bb90(;bb3<280
;bb3++)bbo[bb3]=7 ;bb90(;bb3<288 ;bb3++)bbo[bb3]=8 ;bb2128=9 ;bb1974(bbo,
288 ,257 ,bb2368,bb2358,&bb2263,&bb2128,bb2427,&bb19,bb438);bb90(bb3=0 ;
bb3<30 ;bb3++)bbo[bb3]=5 ;bb2130=5 ;bb1974(bbo,30 ,0 ,bb2404,bb2405,&
bb2265,&bb2130,bb2427,&bb19,bb438);( * ((bby)->bb370))((bby)->bb110,(
bb70)(bb438));( * ((bby)->bb370))((bby)->bb110,(bb70)(bbo));bb2334=1 ;
}
#endif
 *bb55=bb2128; *bb940=bb2130; *bb1014=bb2263; *bb1015=bb2265;bb2 0 ;}
