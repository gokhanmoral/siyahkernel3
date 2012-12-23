/*
   'hmac.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Fri Oct 12 22:15:16 2012
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
bbb bb1961(bb492*bbj,bbe bb587){bb1855(&bbj->bbn,bb587);}bbb bb2005(
bb492*bbj,bbh bbb*bb1304,bbq bb143){bb449 bbn=bbj->bbn;bb1 bb537=bbj
->bb537,bb1340=bbj->bb1340;bbh bbf*bb71=(bbh bbf* )bb1304;bbf bb2271[
256 ],bb1545[256 ];bbn.bb888(bb537);bbm(bb143>bbn.bb38){bbn.bb179(bb537
,bb71,bb143);bb31(bbn.bb393<=bb12(bb2271));bbn.bb725(bb537,bb2271);
bb71=bb2271;bb143=bbn.bb393;bb31(bb143<=bbn.bb38);}{bbq bbz;bb31(bbn.
bb38<=bb12(bb1545));bb90(bbz=0 ;bbz<bbn.bb38;bbz++)bb1545[bbz]=0x36 ^(
bbz<bb143?bb71[bbz]:0 );bbn.bb888(bb537);bbn.bb179(bb537,bb1545,bbn.
bb38);}{bbq bbz;bb90(bbz=0 ;bbz<bbn.bb38;bbz++)bb1545[bbz]=0x5c ^(bbz<
bb143?bb71[bbz]:0 );bbn.bb888(bb1340);bbn.bb179(bb1340,bb1545,bbn.bb38
);}}bbb bb2107(bb492*bbj,bbe bb587,bbh bbb*bb71,bbq bb143){bb1961(bbj
,bb587);bb2005(bbj,bb71,bb143);}bbb bb1987(bb492*bbj,bbh bbb*bb5,bbq
bb10){bbj->bbn.bb179(bbj->bb537,bb5,bb10);}bbb bb2006(bb492*bbj,bbb*
bb14){bb449 bbn=bbj->bbn;bb1 bb537=bbj->bb537,bb1340=bbj->bb1340;bbn.
bb725(bb537,bb14);bbn.bb179(bb1340,bb14,bbn.bb393);bbn.bb725(bb1340,
bb14);}bbb bb2106(bbe bb587,bbh bbb*bb71,bbq bb143,bbb*bb14,bbh bbb*
bb5,bbq bb10){bb492 bb97;bb2107(&bb97,bb587,bb71,bb143);bb1987(&bb97,
bb5,bb10);bb2006(&bb97,bb14);}bbb bb2188(bbe bb587,bb82 bb71,bbb*bb14
,bb82 bb5){bb2106(bb587,bb71,(bbq)bb1306(bb71),bb14,bb5,(bbq)bb1306(
bb5));}
