/*
   'src_compress_deflate_crc32.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Fri Oct 12 22:15:16 2012
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
bb1079 bbp((bb17 bb16,bbe bb454,bbh bbl*bb190,bbe bb195));bbi bb379{
bbe bb446;};bbr bbh bbl*bb1152 bbp((bbe bb18));bbr bbe bb1156 bbp((
bb17 bby));bbr bbh bb167*bb1162 bbp((bbb));
#ifdef __cplusplus
}
#endif
bb41 bbh bb167 bb1515[256 ]={0x00000000L ,0x77073096L ,0xee0e612cL ,
0x990951baL ,0x076dc419L ,0x706af48fL ,0xe963a535L ,0x9e6495a3L ,
0x0edb8832L ,0x79dcb8a4L ,0xe0d5e91eL ,0x97d2d988L ,0x09b64c2bL ,
0x7eb17cbdL ,0xe7b82d07L ,0x90bf1d91L ,0x1db71064L ,0x6ab020f2L ,
0xf3b97148L ,0x84be41deL ,0x1adad47dL ,0x6ddde4ebL ,0xf4d4b551L ,
0x83d385c7L ,0x136c9856L ,0x646ba8c0L ,0xfd62f97aL ,0x8a65c9ecL ,
0x14015c4fL ,0x63066cd9L ,0xfa0f3d63L ,0x8d080df5L ,0x3b6e20c8L ,
0x4c69105eL ,0xd56041e4L ,0xa2677172L ,0x3c03e4d1L ,0x4b04d447L ,
0xd20d85fdL ,0xa50ab56bL ,0x35b5a8faL ,0x42b2986cL ,0xdbbbc9d6L ,
0xacbcf940L ,0x32d86ce3L ,0x45df5c75L ,0xdcd60dcfL ,0xabd13d59L ,
0x26d930acL ,0x51de003aL ,0xc8d75180L ,0xbfd06116L ,0x21b4f4b5L ,
0x56b3c423L ,0xcfba9599L ,0xb8bda50fL ,0x2802b89eL ,0x5f058808L ,
0xc60cd9b2L ,0xb10be924L ,0x2f6f7c87L ,0x58684c11L ,0xc1611dabL ,
0xb6662d3dL ,0x76dc4190L ,0x01db7106L ,0x98d220bcL ,0xefd5102aL ,
0x71b18589L ,0x06b6b51fL ,0x9fbfe4a5L ,0xe8b8d433L ,0x7807c9a2L ,
0x0f00f934L ,0x9609a88eL ,0xe10e9818L ,0x7f6a0dbbL ,0x086d3d2dL ,
0x91646c97L ,0xe6635c01L ,0x6b6b51f4L ,0x1c6c6162L ,0x856530d8L ,
0xf262004eL ,0x6c0695edL ,0x1b01a57bL ,0x8208f4c1L ,0xf50fc457L ,
0x65b0d9c6L ,0x12b7e950L ,0x8bbeb8eaL ,0xfcb9887cL ,0x62dd1ddfL ,
0x15da2d49L ,0x8cd37cf3L ,0xfbd44c65L ,0x4db26158L ,0x3ab551ceL ,
0xa3bc0074L ,0xd4bb30e2L ,0x4adfa541L ,0x3dd895d7L ,0xa4d1c46dL ,
0xd3d6f4fbL ,0x4369e96aL ,0x346ed9fcL ,0xad678846L ,0xda60b8d0L ,
0x44042d73L ,0x33031de5L ,0xaa0a4c5fL ,0xdd0d7cc9L ,0x5005713cL ,
0x270241aaL ,0xbe0b1010L ,0xc90c2086L ,0x5768b525L ,0x206f85b3L ,
0xb966d409L ,0xce61e49fL ,0x5edef90eL ,0x29d9c998L ,0xb0d09822L ,
0xc7d7a8b4L ,0x59b33d17L ,0x2eb40d81L ,0xb7bd5c3bL ,0xc0ba6cadL ,
0xedb88320L ,0x9abfb3b6L ,0x03b6e20cL ,0x74b1d29aL ,0xead54739L ,
0x9dd277afL ,0x04db2615L ,0x73dc1683L ,0xe3630b12L ,0x94643b84L ,
0x0d6d6a3eL ,0x7a6a5aa8L ,0xe40ecf0bL ,0x9309ff9dL ,0x0a00ae27L ,
0x7d079eb1L ,0xf00f9344L ,0x8708a3d2L ,0x1e01f268L ,0x6906c2feL ,
0xf762575dL ,0x806567cbL ,0x196c3671L ,0x6e6b06e7L ,0xfed41b76L ,
0x89d32be0L ,0x10da7a5aL ,0x67dd4accL ,0xf9b9df6fL ,0x8ebeeff9L ,
0x17b7be43L ,0x60b08ed5L ,0xd6d6a3e8L ,0xa1d1937eL ,0x38d8c2c4L ,
0x4fdff252L ,0xd1bb67f1L ,0xa6bc5767L ,0x3fb506ddL ,0x48b2364bL ,
0xd80d2bdaL ,0xaf0a1b4cL ,0x36034af6L ,0x41047a60L ,0xdf60efc3L ,
0xa867df55L ,0x316e8eefL ,0x4669be79L ,0xcb61b38cL ,0xbc66831aL ,
0x256fd2a0L ,0x5268e236L ,0xcc0c7795L ,0xbb0b4703L ,0x220216b9L ,
0x5505262fL ,0xc5ba3bbeL ,0xb2bd0b28L ,0x2bb45a92L ,0x5cb36a04L ,
0xc2d7ffa7L ,0xb5d0cf31L ,0x2cd99e8bL ,0x5bdeae1dL ,0x9b64c2b0L ,
0xec63f226L ,0x756aa39cL ,0x026d930aL ,0x9c0906a9L ,0xeb0e363fL ,
0x72076785L ,0x05005713L ,0x95bf4a82L ,0xe2b87a14L ,0x7bb12baeL ,
0x0cb61b38L ,0x92d28e9bL ,0xe5d5be0dL ,0x7cdcefb7L ,0x0bdbdf21L ,
0x86d3d2d4L ,0xf1d4e242L ,0x68ddb3f8L ,0x1fda836eL ,0x81be16cdL ,
0xf6b9265bL ,0x6fb077e1L ,0x18b74777L ,0x88085ae6L ,0xff0f6a70L ,
0x66063bcaL ,0x11010b5cL ,0x8f659effL ,0xf862ae69L ,0x616bffd3L ,
0x166ccf45L ,0xa00ae278L ,0xd70dd2eeL ,0x4e048354L ,0x3903b3c2L ,
0xa7672661L ,0xd06016f7L ,0x4969474dL ,0x3e6e77dbL ,0xaed16a4aL ,
0xd9d65adcL ,0x40df0b66L ,0x37d83bf0L ,0xa9bcae53L ,0xdebb9ec5L ,
0x47b2cf7fL ,0x30b5ffe9L ,0xbdbdf21cL ,0xcabac28aL ,0x53b39330L ,
0x24b4a3a6L ,0xbad03605L ,0xcdd70693L ,0x54de5729L ,0x23d967bfL ,
0xb3667a2eL ,0xc4614ab8L ,0x5d681b02L ,0x2a6f2b94L ,0xb40bbe37L ,
0xc30c8ea1L ,0x5a05df1bL ,0x2d02ef8dL };bbh bb167*bb1162(){bb2(bbh bb167
 * )bb1515;}bb24 bb1167(bb377,bb39,bb22)bb24 bb377;bbh bb29*bb39;bb7
bb22;{bbm(bb39==0 )bb2 0L ;bb377=bb377^0xffffffffL ;bb108(bb22>=8 ){bb377
=bb1515[((bbe)bb377^( *bb39++))&0xff ]^(bb377>>8 );;bb377=bb1515[((bbe)bb377
^( *bb39++))&0xff ]^(bb377>>8 );;;bb377=bb1515[((bbe)bb377^( *bb39++))&
0xff ]^(bb377>>8 );;bb377=bb1515[((bbe)bb377^( *bb39++))&0xff ]^(bb377>>
8 );;;;bb377=bb1515[((bbe)bb377^( *bb39++))&0xff ]^(bb377>>8 );;bb377=
bb1515[((bbe)bb377^( *bb39++))&0xff ]^(bb377>>8 );;;bb377=bb1515[((bbe)bb377
^( *bb39++))&0xff ]^(bb377>>8 );;bb377=bb1515[((bbe)bb377^( *bb39++))&
0xff ]^(bb377>>8 );;;;;bb22-=8 ;}bbm(bb22)bb564{bb377=bb1515[((bbe)bb377
^( *bb39++))&0xff ]^(bb377>>8 );;}bb108(--bb22);bb2 bb377^0xffffffffL ;}
