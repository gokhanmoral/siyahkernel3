/*
   'aes_xcbc.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Fri Oct 12 22:15:16 2012
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
bba bbi{bb363 bb2116;bbq bb10;bbf bb101[16 ];bbf bb1927[16 ];bbf bb1926
[16 ];bbf bb1842[16 ];}bb944;bbb bb2041(bb944*bbj,bbh bbb*bb71,bbq bb143
);bbb bb2092(bb944*bbj,bbh bbb*bb5,bbq bb10);bbb bb2102(bb944*bbj,bbb
 *bb14);
#ifdef __cplusplus
}
#endif
bbb bb2041(bb944*bbj,bbh bbb*bb71,bbq bb143){bb363 bb2148;bbf bb2179[
16 ];bbj->bb10=0 ;bb31(bb143==16 );bb1098(&bb2148,bb71,bb143);bb996(bbj
->bb1842,0 ,16 );bb996(bb2179,1 ,16 );bb1034(&bb2148,bb2179,bb2179);bb996
(bbj->bb1927,2 ,16 );bb1034(&bb2148,bbj->bb1927,bbj->bb1927);bb996(bbj
->bb1926,3 ,16 );bb1034(&bb2148,bbj->bb1926,bbj->bb1926);bb1098(&bbj->
bb2116,bb2179,bb143);}bb41 bbb bb1252(bb944*bbj,bbh bbf*bb5){bbq bbz;
bb90(bbz=0 ;bbz<16 ;bbz++)bbj->bb1842[bbz]^=bb5[bbz];bb1034(&bbj->
bb2116,bbj->bb1842,bbj->bb1842);}bbb bb2092(bb944*bbj,bbh bbb*bb498,
bbq bb10){bbh bbf*bb5=(bbh bbf* )bb498;bbq bb384=bbj->bb10?(bbj->bb10
-1 )%16 +1 :0 ;bbj->bb10+=bb10;bbm(bb384){bbq bb11=16 -bb384;bb81(bbj->
bb101+bb384,bb5,((bb10)<(bb11)?(bb10):(bb11)));bbm(bb10<=bb11)bb2;bb5
+=bb11;bb10-=bb11;bb1252(bbj,bbj->bb101);}bb90(;bb10>16 ;bb10-=16 ,bb5
+=16 )bb1252(bbj,bb5);bb81(bbj->bb101,bb5,bb10);}bbb bb2102(bb944*bbj,
bbb*bb14){bb1 bb3;bbq bbz,bb384=bbj->bb10?(bbj->bb10-1 )%16 +1 :0 ;bbm(
bb384<16 ){bbj->bb101[bb384++]=0x80 ;bb996(bbj->bb101+bb384,0 ,16 -bb384);
bb3=bbj->bb1926;}bb54 bb3=bbj->bb1927;bb90(bbz=0 ;bbz<16 ;bbz++)bbj->
bb101[bbz]^=bb3[bbz];bb1252(bbj,bbj->bb101);bb81(bb14,bbj->bb1842,16 );
}
