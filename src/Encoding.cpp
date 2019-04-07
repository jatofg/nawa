/**
 * \file Encoding.cpp
 * \brief Implementation of the Encoding class.
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of QSF.
 *
 * QSF is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * QSF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QSF.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <boost/algorithm/string.hpp>
#include <codecvt>
#include <sstream>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include <qsf/Encoding.h>
#include <qsf/Utils.h>
#include "../libs/base64/base64.h"

namespace {
    const std::unordered_map<char32_t, std::u32string> htmlEntities =
            {{U'\u00C1',U"&Aacute;"},{U'\u00E1',U"&aacute;"},{U'\u0102',U"&Abreve;"},{U'\u0103',U"&abreve;"},
             {U'\u223E',U"&ac;"},{U'\u223F',U"&acd;"},{U'\u223E',U""},{U'\u00C2',U"&Acirc;"},
             {U'\u00E2',U"&acirc;"},{U'\u00B4',U"&acute;"},{U'\u0410',U"&Acy;"},{U'\u0430',U"&acy;"},
             {U'\u00C6',U"&AElig;"},{U'\u00E6',U"&aelig;"},{U'\u2061',U"&af;"},{U'\U0001d504',U"&Afr;"},
             {U'\U0001d51e',U"&afr;"},{U'\u00C0',U"&Agrave;"},{U'\u00E0',U"&agrave;"},
             {U'\u2135',U"&alefsym;"},{U'\u0391',U"&Alpha;"},{U'\u03B1',U"&alpha;"},{U'\u0100',U"&Amacr;"},
             {U'\u0101',U"&amacr;"},{U'\u2A3F',U"&amalg;"},{U'\u0026',U"&amp;"},{U'\u2A55',U"&andand;"},
             {U'\u2A53',U"&And;"},{U'\u2227',U"&and;"},{U'\u2A5C',U"&andd;"},{U'\u2A58',U"&andslope;"},
             {U'\u2A5A',U"&andv;"},{U'\u2220',U"&ang;"},{U'\u29A4',U"&ange;"},{U'\u29A8',U"&angmsdaa;"},
             {U'\u29A9',U"&angmsdab;"},{U'\u29AA',U"&angmsdac;"},{U'\u29AB',U"&angmsdad;"},
             {U'\u29AC',U"&angmsdae;"},{U'\u29AD',U"&angmsdaf;"},{U'\u29AE',U"&angmsdag;"},
             {U'\u29AF',U"&angmsdah;"},{U'\u2221',U"&angmsd;"},{U'\u221F',U"&angrt;"},
             {U'\u22BE',U"&angrtvb;"},{U'\u299D',U"&angrtvbd;"},{U'\u2222',U"&angsph;"},
             {U'\u00C5',U"&angst;"},{U'\u237C',U"&angzarr;"},{U'\u0104',U"&Aogon;"},{U'\u0105',U"&aogon;"},
             {U'\U0001d538',U"&Aopf;"},{U'\U0001d552',U"&aopf;"},{U'\u2A6F',U"&apacir;"},{U'\u2248',U"&ap;"},
             {U'\u2A70',U"&apE;"},{U'\u224A',U"&ape;"},{U'\u224B',U"&apid;"},{U'\u0027',U"&apos;"},
             {U'\u00E5',U"&aring;"},{U'\U0001d49c',U"&Ascr;"},{U'\U0001d4b6',U"&ascr;"},
             {U'\u2254',U"&Assign;"},{U'\u002A',U"&ast;"},{U'\u224D',U"&asympeq;"},{U'\u00C3',U"&Atilde;"},
             {U'\u00E3',U"&atilde;"},{U'\u00C4',U"&Auml;"},{U'\u00E4',U"&auml;"},{U'\u2233',U"&awconint;"},
             {U'\u2A11',U"&awint;"},{U'\u224C',U"&backcong;"},{U'\u03F6',U"&backepsilon;"},
             {U'\u2035',U"&backprime;"},{U'\u223D',U"&backsim;"},{U'\u22CD',U"&backsimeq;"},
             {U'\u2216',U"&Backslash;"},{U'\u2AE7',U"&Barv;"},{U'\u22BD',U"&barvee;"},{U'\u2305',U"&barwed;"},
             {U'\u2306',U"&Barwed;"},{U'\u23B5',U"&bbrk;"},{U'\u23B6',U"&bbrktbrk;"},{U'\u0411',U"&Bcy;"},
             {U'\u0431',U"&bcy;"},{U'\u201E',U"&bdquo;"},{U'\u2235',U"&becaus;"},{U'\u29B0',U"&bemptyv;"},
             {U'\u212C',U"&bernou;"},{U'\u0392',U"&Beta;"},{U'\u03B2',U"&beta;"},{U'\u2136',U"&beth;"},
             {U'\u226C',U"&between;"},{U'\U0001d505',U"&Bfr;"},{U'\U0001d51f',U"&bfr;"},
             {U'\u22C2',U"&bigcap;"},{U'\u25EF',U"&bigcirc;"},{U'\u22C3',U"&bigcup;"},
             {U'\u2A00',U"&bigodot;"},{U'\u2A01',U"&bigoplus;"},{U'\u2A02',U"&bigotimes;"},
             {U'\u2A06',U"&bigsqcup;"},{U'\u2605',U"&bigstar;"},{U'\u25BD',U"&bigtriangledown;"},
             {U'\u25B3',U"&bigtriangleup;"},{U'\u2A04',U"&biguplus;"},{U'\u22C1',U"&bigvee;"},
             {U'\u22C0',U"&bigwedge;"},{U'\u290D',U"&bkarow;"},{U'\u29EB',U"&blacklozenge;"},
             {U'\u25AA',U"&blacksquare;"},{U'\u25B4',U"&blacktriangle;"},{U'\u25BE',U"&blacktriangledown;"},
             {U'\u25C2',U"&blacktriangleleft;"},{U'\u25B8',U"&blacktriangleright;"},{U'\u2423',U"&blank;"},
             {U'\u2592',U"&blk12;"},{U'\u2591',U"&blk14;"},{U'\u2593',U"&blk34;"},{U'\u2588',U"&block;"},
             {U'\u003D',U""},{U'\u2261',U""},{U'\u2AED',U"&bNot;"},{U'\u2310',U"&bnot;"},
             {U'\U0001d539',U"&Bopf;"},{U'\U0001d553',U"&bopf;"},{U'\u22A5',U"&bot;"},{U'\u22C8',U"&bowtie;"},
             {U'\u29C9',U"&boxbox;"},{U'\u2510',U"&boxdl;"},{U'\u2555',U"&boxdL;"},{U'\u2556',U"&boxDl;"},
             {U'\u2557',U"&boxDL;"},{U'\u250C',U"&boxdr;"},{U'\u2552',U"&boxdR;"},{U'\u2553',U"&boxDr;"},
             {U'\u2554',U"&boxDR;"},{U'\u2500',U"&boxh;"},{U'\u2550',U"&boxH;"},{U'\u252C',U"&boxhd;"},
             {U'\u2564',U"&boxHd;"},{U'\u2565',U"&boxhD;"},{U'\u2566',U"&boxHD;"},{U'\u2534',U"&boxhu;"},
             {U'\u2567',U"&boxHu;"},{U'\u2568',U"&boxhU;"},{U'\u2569',U"&boxHU;"},{U'\u229F',U"&boxminus;"},
             {U'\u229E',U"&boxplus;"},{U'\u22A0',U"&boxtimes;"},{U'\u2518',U"&boxul;"},{U'\u255B',U"&boxuL;"},
             {U'\u255C',U"&boxUl;"},{U'\u255D',U"&boxUL;"},{U'\u2514',U"&boxur;"},{U'\u2558',U"&boxuR;"},
             {U'\u2559',U"&boxUr;"},{U'\u255A',U"&boxUR;"},{U'\u2502',U"&boxv;"},{U'\u2551',U"&boxV;"},
             {U'\u253C',U"&boxvh;"},{U'\u256A',U"&boxvH;"},{U'\u256B',U"&boxVh;"},{U'\u256C',U"&boxVH;"},
             {U'\u2524',U"&boxvl;"},{U'\u2561',U"&boxvL;"},{U'\u2562',U"&boxVl;"},{U'\u2563',U"&boxVL;"},
             {U'\u251C',U"&boxvr;"},{U'\u255E',U"&boxvR;"},{U'\u255F',U"&boxVr;"},{U'\u2560',U"&boxVR;"},
             {U'\u02D8',U"&breve;"},{U'\u00A6',U"&brvbar;"},{U'\U0001d4b7',U"&bscr;"},{U'\u204F',U"&bsemi;"},
             {U'\u29C5',U"&bsolb;"},{U'\u005C',U"&bsol;"},{U'\u27C8',U"&bsolhsub;"},{U'\u2022',U"&bull;"},
             {U'\u224E',U"&bump;"},{U'\u2AAE',U"&bumpE;"},{U'\u224F',U"&bumpe;"},{U'\u0106',U"&Cacute;"},
             {U'\u0107',U"&cacute;"},{U'\u2A44',U"&capand;"},{U'\u2A49',U"&capbrcup;"},
             {U'\u2A4B',U"&capcap;"},{U'\u2229',U"&cap;"},{U'\u22D2',U"&Cap;"},{U'\u2A47',U"&capcup;"},
             {U'\u2A40',U"&capdot;"},{U'\u2145',U"&CapitalDifferentialD;"},{U'\u2229',U""},
             {U'\u2041',U"&caret;"},{U'\u02C7',U"&caron;"},{U'\u212D',U"&Cayleys;"},{U'\u2A4D',U"&ccaps;"},
             {U'\u010C',U"&Ccaron;"},{U'\u010D',U"&ccaron;"},{U'\u00C7',U"&Ccedil;"},{U'\u00E7',U"&ccedil;"},
             {U'\u0108',U"&Ccirc;"},{U'\u0109',U"&ccirc;"},{U'\u2230',U"&Cconint;"},{U'\u2A4C',U"&ccups;"},
             {U'\u2A50',U"&ccupssm;"},{U'\u010A',U"&Cdot;"},{U'\u010B',U"&cdot;"},{U'\u00B8',U"&cedil;"},
             {U'\u29B2',U"&cemptyv;"},{U'\u00A2',U"&cent;"},{U'\u00B7',U"&centerdot;"},
             {U'\U0001d520',U"&cfr;"},{U'\u0427',U"&CHcy;"},{U'\u0447',U"&chcy;"},{U'\u2713',U"&check;"},
             {U'\u03A7',U"&Chi;"},{U'\u03C7',U"&chi;"},{U'\u02C6',U"&circ;"},{U'\u2257',U"&circeq;"},
             {U'\u21BA',U"&circlearrowleft;"},{U'\u21BB',U"&circlearrowright;"},{U'\u229B',U"&circledast;"},
             {U'\u229A',U"&circledcirc;"},{U'\u229D',U"&circleddash;"},{U'\u2299',U"&CircleDot;"},
             {U'\u00AE',U"&circledR;"},{U'\u24C8',U"&circledS;"},{U'\u2296',U"&CircleMinus;"},
             {U'\u2295',U"&CirclePlus;"},{U'\u2297',U"&CircleTimes;"},{U'\u25CB',U"&cir;"},
             {U'\u29C3',U"&cirE;"},{U'\u2A10',U"&cirfnint;"},{U'\u2AEF',U"&cirmid;"},{U'\u29C2',U"&cirscir;"},
             {U'\u2232',U"&ClockwiseContourIntegral;"},{U'\u201D',U"&CloseCurlyDoubleQuote;"},
             {U'\u2019',U"&CloseCurlyQuote;"},{U'\u2663',U"&clubs;"},{U'\u003A',U"&colon;"},
             {U'\u2237',U"&Colon;"},{U'\u2A74',U"&Colone;"},{U'\u002C',U"&comma;"},{U'\u0040',U"&commat;"},
             {U'\u2201',U"&comp;"},{U'\u2218',U"&compfn;"},{U'\u2102',U"&complexes;"},{U'\u2245',U"&cong;"},
             {U'\u2A6D',U"&congdot;"},{U'\u2261',U"&Congruent;"},{U'\u222E',U"&conint;"},
             {U'\u222F',U"&Conint;"},{U'\U0001d554',U"&copf;"},{U'\u2210',U"&coprod;"},{U'\u00A9',U"&copy;"},
             {U'\u2117',U"&copysr;"},{U'\u21B5',U"&crarr;"},{U'\u2717',U"&cross;"},{U'\u2A2F',U"&Cross;"},
             {U'\U0001d49e',U"&Cscr;"},{U'\U0001d4b8',U"&cscr;"},{U'\u2ACF',U"&csub;"},{U'\u2AD1',U"&csube;"},
             {U'\u2AD0',U"&csup;"},{U'\u2AD2',U"&csupe;"},{U'\u22EF',U"&ctdot;"},{U'\u2938',U"&cudarrl;"},
             {U'\u2935',U"&cudarrr;"},{U'\u22DE',U"&cuepr;"},{U'\u22DF',U"&cuesc;"},{U'\u21B6',U"&cularr;"},
             {U'\u293D',U"&cularrp;"},{U'\u2A48',U"&cupbrcap;"},{U'\u2A46',U"&cupcap;"},{U'\u222A',U"&cup;"},
             {U'\u22D3',U"&Cup;"},{U'\u2A4A',U"&cupcup;"},{U'\u228D',U"&cupdot;"},{U'\u2A45',U"&cupor;"},
             {U'\u222A',U""},{U'\u21B7',U"&curarr;"},{U'\u293C',U"&curarrm;"},{U'\u22CE',U"&curlyvee;"},
             {U'\u22CF',U"&curlywedge;"},{U'\u00A4',U"&curren;"},{U'\u2231',U"&cwint;"},
             {U'\u232D',U"&cylcty;"},{U'\u2020',U"&dagger;"},{U'\u2021',U"&Dagger;"},{U'\u2138',U"&daleth;"},
             {U'\u2193',U"&darr;"},{U'\u21A1',U"&Darr;"},{U'\u21D3',U"&dArr;"},{U'\u2010',U"&dash;"},
             {U'\u2AE4',U"&Dashv;"},{U'\u22A3',U"&dashv;"},{U'\u290F',U"&dbkarow;"},{U'\u02DD',U"&dblac;"},
             {U'\u010E',U"&Dcaron;"},{U'\u010F',U"&dcaron;"},{U'\u0414',U"&Dcy;"},{U'\u0434',U"&dcy;"},
             {U'\u21CA',U"&ddarr;"},{U'\u2146',U"&dd;"},{U'\u2911',U"&DDotrahd;"},{U'\u2A77',U"&ddotseq;"},
             {U'\u00B0',U"&deg;"},{U'\u2207',U"&Del;"},{U'\u0394',U"&Delta;"},{U'\u03B4',U"&delta;"},
             {U'\u29B1',U"&demptyv;"},{U'\u297F',U"&dfisht;"},{U'\U0001d507',U"&Dfr;"},
             {U'\U0001d521',U"&dfr;"},{U'\u2965',U"&dHar;"},{U'\u21C3',U"&dharl;"},{U'\u21C2',U"&dharr;"},
             {U'\u02D9',U"&DiacriticalDot;"},{U'\u0060',U"&DiacriticalGrave;"},
             {U'\u02DC',U"&DiacriticalTilde;"},{U'\u22C4',U"&diam;"},{U'\u2666',U"&diamondsuit;"},
             {U'\u00A8',U"&die;"},{U'\u03DD',U"&digamma;"},{U'\u22F2',U"&disin;"},{U'\u00F7',U"&div;"},
             {U'\u22C7',U"&divideontimes;"},{U'\u0402',U"&DJcy;"},{U'\u0452',U"&djcy;"},
             {U'\u231E',U"&dlcorn;"},{U'\u230D',U"&dlcrop;"},{U'\u0024',U"&dollar;"},
             {U'\U0001d53b',U"&Dopf;"},{U'\U0001d555',U"&dopf;"},{U'\u20DC',U"&DotDot;"},
             {U'\u2250',U"&doteq;"},{U'\u2251',U"&doteqdot;"},{U'\u2238',U"&dotminus;"},
             {U'\u2214',U"&dotplus;"},{U'\u22A1',U"&dotsquare;"},{U'\u21D0',U"&DoubleLeftArrow;"},
             {U'\u21D4',U"&DoubleLeftRightArrow;"},{U'\u27F8',U"&DoubleLongLeftArrow;"},
             {U'\u27FA',U"&DoubleLongLeftRightArrow;"},{U'\u27F9',U"&DoubleLongRightArrow;"},
             {U'\u21D2',U"&DoubleRightArrow;"},{U'\u22A8',U"&DoubleRightTee;"},{U'\u21D1',U"&DoubleUpArrow;"},
             {U'\u21D5',U"&DoubleUpDownArrow;"},{U'\u2225',U"&DoubleVerticalBar;"},
             {U'\u2913',U"&DownArrowBar;"},{U'\u21F5',U"&DownArrowUpArrow;"},{U'\u0311',U"&DownBreve;"},
             {U'\u2950',U"&DownLeftRightVector;"},{U'\u295E',U"&DownLeftTeeVector;"},
             {U'\u2956',U"&DownLeftVectorBar;"},{U'\u21BD',U"&DownLeftVector;"},
             {U'\u295F',U"&DownRightTeeVector;"},{U'\u2957',U"&DownRightVectorBar;"},
             {U'\u21C1',U"&DownRightVector;"},{U'\u21A7',U"&DownTeeArrow;"},{U'\u22A4',U"&DownTee;"},
             {U'\u2910',U"&drbkarow;"},{U'\u231F',U"&drcorn;"},{U'\u230C',U"&drcrop;"},
             {U'\U0001d49f',U"&Dscr;"},{U'\U0001d4b9',U"&dscr;"},{U'\u0405',U"&DScy;"},{U'\u0455',U"&dscy;"},
             {U'\u29F6',U"&dsol;"},{U'\u0110',U"&Dstrok;"},{U'\u0111',U"&dstrok;"},{U'\u22F1',U"&dtdot;"},
             {U'\u25BF',U"&dtri;"},{U'\u296F',U"&duhar;"},{U'\u29A6',U"&dwangle;"},{U'\u040F',U"&DZcy;"},
             {U'\u045F',U"&dzcy;"},{U'\u27FF',U"&dzigrarr;"},{U'\u00C9',U"&Eacute;"},{U'\u00E9',U"&eacute;"},
             {U'\u2A6E',U"&easter;"},{U'\u011A',U"&Ecaron;"},{U'\u011B',U"&ecaron;"},{U'\u00CA',U"&Ecirc;"},
             {U'\u00EA',U"&ecirc;"},{U'\u2256',U"&ecir;"},{U'\u2255',U"&ecolon;"},{U'\u042D',U"&Ecy;"},
             {U'\u044D',U"&ecy;"},{U'\u0116',U"&Edot;"},{U'\u0117',U"&edot;"},{U'\u2147',U"&ee;"},
             {U'\u2252',U"&efDot;"},{U'\U0001d508',U"&Efr;"},{U'\U0001d522',U"&efr;"},{U'\u2A9A',U"&eg;"},
             {U'\u00C8',U"&Egrave;"},{U'\u00E8',U"&egrave;"},{U'\u2A96',U"&egs;"},{U'\u2A98',U"&egsdot;"},
             {U'\u2A99',U"&el;"},{U'\u2208',U"&Element;"},{U'\u23E7',U"&elinters;"},{U'\u2113',U"&ell;"},
             {U'\u2A95',U"&els;"},{U'\u2A97',U"&elsdot;"},{U'\u0112',U"&Emacr;"},{U'\u0113',U"&emacr;"},
             {U'\u2205',U"&empty;"},{U'\u25FB',U"&EmptySmallSquare;"},{U'\u25AB',U"&EmptyVerySmallSquare;"},
             {U'\u2004',U"&emsp13;"},{U'\u2005',U"&emsp14;"},{U'\u2003',U"&emsp;"},{U'\u014A',U"&ENG;"},
             {U'\u014B',U"&eng;"},{U'\u2002',U"&ensp;"},{U'\u0118',U"&Eogon;"},{U'\u0119',U"&eogon;"},
             {U'\U0001d53c',U"&Eopf;"},{U'\U0001d556',U"&eopf;"},{U'\u22D5',U"&epar;"},
             {U'\u29E3',U"&eparsl;"},{U'\u2A71',U"&eplus;"},{U'\u03B5',U"&epsi;"},{U'\u0395',U"&Epsilon;"},
             {U'\u03F5',U"&epsiv;"},{U'\u2242',U"&eqsim;"},{U'\u2A75',U"&Equal;"},{U'\u003D',U"&equals;"},
             {U'\u225F',U"&equest;"},{U'\u21CC',U"&Equilibrium;"},{U'\u2A78',U"&equivDD;"},
             {U'\u29E5',U"&eqvparsl;"},{U'\u2971',U"&erarr;"},{U'\u2253',U"&erDot;"},{U'\u212F',U"&escr;"},
             {U'\u2130',U"&Escr;"},{U'\u2A73',U"&Esim;"},{U'\u0397',U"&Eta;"},{U'\u03B7',U"&eta;"},
             {U'\u00D0',U"&ETH;"},{U'\u00F0',U"&eth;"},{U'\u00CB',U"&Euml;"},{U'\u00EB',U"&euml;"},
             {U'\u20AC',U"&euro;"},{U'\u0021',U"&excl;"},{U'\u2203',U"&exist;"},{U'\u0424',U"&Fcy;"},
             {U'\u0444',U"&fcy;"},{U'\u2640',U"&female;"},{U'\uFB03',U"&ffilig;"},{U'\uFB00',U"&fflig;"},
             {U'\uFB04',U"&ffllig;"},{U'\U0001d509',U"&Ffr;"},{U'\U0001d523',U"&ffr;"},{U'\uFB01',U"&filig;"},
             {U'\u25FC',U"&FilledSmallSquare;"},{U'\u0066',U""},{U'\u266D',U"&flat;"},{U'\uFB02',U"&fllig;"},
             {U'\u25B1',U"&fltns;"},{U'\u0192',U"&fnof;"},{U'\U0001d53d',U"&Fopf;"},{U'\U0001d557',U"&fopf;"},
             {U'\u2200',U"&forall;"},{U'\u22D4',U"&fork;"},{U'\u2AD9',U"&forkv;"},{U'\u2131',U"&Fouriertrf;"},
             {U'\u2A0D',U"&fpartint;"},{U'\u00BD',U"&frac12;"},{U'\u2153',U"&frac13;"},
             {U'\u00BC',U"&frac14;"},{U'\u2155',U"&frac15;"},{U'\u2159',U"&frac16;"},{U'\u215B',U"&frac18;"},
             {U'\u2154',U"&frac23;"},{U'\u2156',U"&frac25;"},{U'\u00BE',U"&frac34;"},{U'\u2157',U"&frac35;"},
             {U'\u215C',U"&frac38;"},{U'\u2158',U"&frac45;"},{U'\u215A',U"&frac56;"},{U'\u215D',U"&frac58;"},
             {U'\u215E',U"&frac78;"},{U'\u2044',U"&frasl;"},{U'\u2322',U"&frown;"},{U'\U0001d4bb',U"&fscr;"},
             {U'\u01F5',U"&gacute;"},{U'\u0393',U"&Gamma;"},{U'\u03B3',U"&gamma;"},{U'\u03DC',U"&Gammad;"},
             {U'\u2A86',U"&gap;"},{U'\u011E',U"&Gbreve;"},{U'\u011F',U"&gbreve;"},{U'\u0122',U"&Gcedil;"},
             {U'\u011C',U"&Gcirc;"},{U'\u011D',U"&gcirc;"},{U'\u0413',U"&Gcy;"},{U'\u0433',U"&gcy;"},
             {U'\u0120',U"&Gdot;"},{U'\u0121',U"&gdot;"},{U'\u2265',U"&ge;"},{U'\u2267',U"&gE;"},
             {U'\u2A8C',U"&gEl;"},{U'\u22DB',U"&gel;"},{U'\u2A7E',U"&geqslant;"},{U'\u2AA9',U"&gescc;"},
             {U'\u2A80',U"&gesdot;"},{U'\u2A82',U"&gesdoto;"},{U'\u2A84',U"&gesdotol;"},{U'\u22DB',U""},
             {U'\u2A94',U"&gesles;"},{U'\U0001d50a',U"&Gfr;"},{U'\U0001d524',U"&gfr;"},{U'\u226B',U"&gg;"},
             {U'\u22D9',U"&Gg;"},{U'\u2137',U"&gimel;"},{U'\u0403',U"&GJcy;"},{U'\u0453',U"&gjcy;"},
             {U'\u2AA5',U"&gla;"},{U'\u2277',U"&gl;"},{U'\u2A92',U"&glE;"},{U'\u2AA4',U"&glj;"},
             {U'\u2A8A',U"&gnap;"},{U'\u2A88',U"&gne;"},{U'\u2269',U"&gnE;"},{U'\u22E7',U"&gnsim;"},
             {U'\U0001d53e',U"&Gopf;"},{U'\U0001d558',U"&gopf;"},{U'\u2AA2',U"&GreaterGreater;"},
             {U'\u2273',U"&GreaterTilde;"},{U'\U0001d4a2',U"&Gscr;"},{U'\u210A',U"&gscr;"},
             {U'\u2A8E',U"&gsime;"},{U'\u2A90',U"&gsiml;"},{U'\u2AA7',U"&gtcc;"},{U'\u2A7A',U"&gtcir;"},
             {U'\u003E',U"&gt;"},{U'\u22D7',U"&gtdot;"},{U'\u2995',U"&gtlPar;"},{U'\u2A7C',U"&gtquest;"},
             {U'\u2978',U"&gtrarr;"},{U'\u2269',U""},{U'\u200A',U"&hairsp;"},{U'\u210B',U"&hamilt;"},
             {U'\u042A',U"&HARDcy;"},{U'\u044A',U"&hardcy;"},{U'\u2948',U"&harrcir;"},{U'\u2194',U"&harr;"},
             {U'\u21AD',U"&harrw;"},{U'\u005E',U"&Hat;"},{U'\u210F',U"&hbar;"},{U'\u0124',U"&Hcirc;"},
             {U'\u0125',U"&hcirc;"},{U'\u2665',U"&hearts;"},{U'\u2026',U"&hellip;"},{U'\u22B9',U"&hercon;"},
             {U'\U0001d525',U"&hfr;"},{U'\u210C',U"&Hfr;"},{U'\u2925',U"&hksearow;"},
             {U'\u2926',U"&hkswarow;"},{U'\u21FF',U"&hoarr;"},{U'\u223B',U"&homtht;"},
             {U'\u21A9',U"&hookleftarrow;"},{U'\u21AA',U"&hookrightarrow;"},{U'\U0001d559',U"&hopf;"},
             {U'\u210D',U"&Hopf;"},{U'\u2015',U"&horbar;"},{U'\U0001d4bd',U"&hscr;"},{U'\u0126',U"&Hstrok;"},
             {U'\u0127',U"&hstrok;"},{U'\u2043',U"&hybull;"},{U'\u00CD',U"&Iacute;"},{U'\u00ED',U"&iacute;"},
             {U'\u2063',U"&ic;"},{U'\u00CE',U"&Icirc;"},{U'\u00EE',U"&icirc;"},{U'\u0418',U"&Icy;"},
             {U'\u0438',U"&icy;"},{U'\u0130',U"&Idot;"},{U'\u0415',U"&IEcy;"},{U'\u0435',U"&iecy;"},
             {U'\u00A1',U"&iexcl;"},{U'\U0001d526',U"&ifr;"},{U'\u2111',U"&Ifr;"},{U'\u00CC',U"&Igrave;"},
             {U'\u00EC',U"&igrave;"},{U'\u2148',U"&ii;"},{U'\u2A0C',U"&iiiint;"},{U'\u222D',U"&iiint;"},
             {U'\u29DC',U"&iinfin;"},{U'\u2129',U"&iiota;"},{U'\u0132',U"&IJlig;"},{U'\u0133',U"&ijlig;"},
             {U'\u012A',U"&Imacr;"},{U'\u012B',U"&imacr;"},{U'\u2110',U"&imagline;"},{U'\u0131',U"&imath;"},
             {U'\u22B7',U"&imof;"},{U'\u01B5',U"&imped;"},{U'\u2105',U"&incare;"},{U'\u221E',U"&infin;"},
             {U'\u29DD',U"&infintie;"},{U'\u22BA',U"&intcal;"},{U'\u222B',U"&int;"},{U'\u222C',U"&Int;"},
             {U'\u2124',U"&integers;"},{U'\u2A17',U"&intlarhk;"},{U'\u2A3C',U"&intprod;"},
             {U'\u2062',U"&InvisibleTimes;"},{U'\u0401',U"&IOcy;"},{U'\u0451',U"&iocy;"},
             {U'\u012E',U"&Iogon;"},{U'\u012F',U"&iogon;"},{U'\U0001d540',U"&Iopf;"},
             {U'\U0001d55a',U"&iopf;"},{U'\u0399',U"&Iota;"},{U'\u03B9',U"&iota;"},{U'\u00BF',U"&iquest;"},
             {U'\U0001d4be',U"&iscr;"},{U'\u22F5',U"&isindot;"},{U'\u22F9',U"&isinE;"},{U'\u22F4',U"&isins;"},
             {U'\u22F3',U"&isinsv;"},{U'\u0128',U"&Itilde;"},{U'\u0129',U"&itilde;"},{U'\u0406',U"&Iukcy;"},
             {U'\u0456',U"&iukcy;"},{U'\u00CF',U"&Iuml;"},{U'\u00EF',U"&iuml;"},{U'\u0134',U"&Jcirc;"},
             {U'\u0135',U"&jcirc;"},{U'\u0419',U"&Jcy;"},{U'\u0439',U"&jcy;"},{U'\U0001d50d',U"&Jfr;"},
             {U'\U0001d527',U"&jfr;"},{U'\u0237',U"&jmath;"},{U'\U0001d541',U"&Jopf;"},
             {U'\U0001d55b',U"&jopf;"},{U'\U0001d4a5',U"&Jscr;"},{U'\U0001d4bf',U"&jscr;"},
             {U'\u0408',U"&Jsercy;"},{U'\u0458',U"&jsercy;"},{U'\u0404',U"&Jukcy;"},{U'\u0454',U"&jukcy;"},
             {U'\u039A',U"&Kappa;"},{U'\u03BA',U"&kappa;"},{U'\u03F0',U"&kappav;"},{U'\u0136',U"&Kcedil;"},
             {U'\u0137',U"&kcedil;"},{U'\u041A',U"&Kcy;"},{U'\u043A',U"&kcy;"},{U'\U0001d50e',U"&Kfr;"},
             {U'\U0001d528',U"&kfr;"},{U'\u0138',U"&kgreen;"},{U'\u0425',U"&KHcy;"},{U'\u0445',U"&khcy;"},
             {U'\u040C',U"&KJcy;"},{U'\u045C',U"&kjcy;"},{U'\U0001d542',U"&Kopf;"},{U'\U0001d55c',U"&kopf;"},
             {U'\U0001d4a6',U"&Kscr;"},{U'\U0001d4c0',U"&kscr;"},{U'\u21DA',U"&lAarr;"},
             {U'\u0139',U"&Lacute;"},{U'\u013A',U"&lacute;"},{U'\u29B4',U"&laemptyv;"},
             {U'\u2112',U"&lagran;"},{U'\u039B',U"&Lambda;"},{U'\u03BB',U"&lambda;"},{U'\u27E8',U"&lang;"},
             {U'\u27EA',U"&Lang;"},{U'\u2991',U"&langd;"},{U'\u2A85',U"&lap;"},{U'\u00AB',U"&laquo;"},
             {U'\u21E4',U"&larrb;"},{U'\u291F',U"&larrbfs;"},{U'\u2190',U"&larr;"},{U'\u219E',U"&Larr;"},
             {U'\u291D',U"&larrfs;"},{U'\u21AB',U"&larrlp;"},{U'\u2939',U"&larrpl;"},{U'\u2973',U"&larrsim;"},
             {U'\u21A2',U"&larrtl;"},{U'\u2919',U"&latail;"},{U'\u291B',U"&lAtail;"},{U'\u2AAB',U"&lat;"},
             {U'\u2AAD',U"&late;"},{U'\u2AAD',U""},{U'\u290C',U"&lbarr;"},{U'\u290E',U"&lBarr;"},
             {U'\u2772',U"&lbbrk;"},{U'\u007B',U"&lbrace;"},{U'\u005B',U"&lbrack;"},{U'\u298B',U"&lbrke;"},
             {U'\u298F',U"&lbrksld;"},{U'\u298D',U"&lbrkslu;"},{U'\u013D',U"&Lcaron;"},
             {U'\u013E',U"&lcaron;"},{U'\u013B',U"&Lcedil;"},{U'\u013C',U"&lcedil;"},{U'\u2308',U"&lceil;"},
             {U'\u041B',U"&Lcy;"},{U'\u043B',U"&lcy;"},{U'\u2936',U"&ldca;"},{U'\u201C',U"&ldquo;"},
             {U'\u2967',U"&ldrdhar;"},{U'\u294B',U"&ldrushar;"},{U'\u21B2',U"&ldsh;"},{U'\u2264',U"&le;"},
             {U'\u2266',U"&lE;"},{U'\u21C6',U"&LeftArrowRightArrow;"},{U'\u27E6',U"&LeftDoubleBracket;"},
             {U'\u2961',U"&LeftDownTeeVector;"},{U'\u2959',U"&LeftDownVectorBar;"},{U'\u230A',U"&LeftFloor;"},
             {U'\u21BC',U"&leftharpoonup;"},{U'\u21C7',U"&leftleftarrows;"},
             {U'\u21CB',U"&leftrightharpoons;"},{U'\u294E',U"&LeftRightVector;"},
             {U'\u21A4',U"&LeftTeeArrow;"},{U'\u295A',U"&LeftTeeVector;"},{U'\u22CB',U"&leftthreetimes;"},
             {U'\u29CF',U"&LeftTriangleBar;"},{U'\u22B2',U"&LeftTriangle;"},
             {U'\u22B4',U"&LeftTriangleEqual;"},{U'\u2951',U"&LeftUpDownVector;"},
             {U'\u2960',U"&LeftUpTeeVector;"},{U'\u2958',U"&LeftUpVectorBar;"},{U'\u21BF',U"&LeftUpVector;"},
             {U'\u2952',U"&LeftVectorBar;"},{U'\u2A8B',U"&lEg;"},{U'\u22DA',U"&leg;"},
             {U'\u2A7D',U"&leqslant;"},{U'\u2AA8',U"&lescc;"},{U'\u2A7F',U"&lesdot;"},
             {U'\u2A81',U"&lesdoto;"},{U'\u2A83',U"&lesdotor;"},{U'\u22DA',U""},{U'\u2A93',U"&lesges;"},
             {U'\u22D6',U"&lessdot;"},{U'\u2276',U"&LessGreater;"},{U'\u2AA1',U"&LessLess;"},
             {U'\u2272',U"&lesssim;"},{U'\u297C',U"&lfisht;"},{U'\U0001d50f',U"&Lfr;"},
             {U'\U0001d529',U"&lfr;"},{U'\u2A91',U"&lgE;"},{U'\u2962',U"&lHar;"},{U'\u296A',U"&lharul;"},
             {U'\u2584',U"&lhblk;"},{U'\u0409',U"&LJcy;"},{U'\u0459',U"&ljcy;"},{U'\u226A',U"&ll;"},
             {U'\u22D8',U"&Ll;"},{U'\u296B',U"&llhard;"},{U'\u25FA',U"&lltri;"},{U'\u013F',U"&Lmidot;"},
             {U'\u0140',U"&lmidot;"},{U'\u23B0',U"&lmoustache;"},{U'\u2A89',U"&lnap;"},{U'\u2A87',U"&lne;"},
             {U'\u2268',U"&lnE;"},{U'\u22E6',U"&lnsim;"},{U'\u27EC',U"&loang;"},{U'\u21FD',U"&loarr;"},
             {U'\u27F5',U"&longleftarrow;"},{U'\u27F7',U"&longleftrightarrow;"},{U'\u27FC',U"&longmapsto;"},
             {U'\u27F6',U"&longrightarrow;"},{U'\u21AC',U"&looparrowright;"},{U'\u2985',U"&lopar;"},
             {U'\U0001d543',U"&Lopf;"},{U'\U0001d55d',U"&lopf;"},{U'\u2A2D',U"&loplus;"},
             {U'\u2A34',U"&lotimes;"},{U'\u2217',U"&lowast;"},{U'\u005F',U"&lowbar;"},
             {U'\u2199',U"&LowerLeftArrow;"},{U'\u2198',U"&LowerRightArrow;"},{U'\u25CA',U"&loz;"},
             {U'\u0028',U"&lpar;"},{U'\u2993',U"&lparlt;"},{U'\u296D',U"&lrhard;"},{U'\u200E',U"&lrm;"},
             {U'\u22BF',U"&lrtri;"},{U'\u2039',U"&lsaquo;"},{U'\U0001d4c1',U"&lscr;"},{U'\u21B0',U"&lsh;"},
             {U'\u2A8D',U"&lsime;"},{U'\u2A8F',U"&lsimg;"},{U'\u2018',U"&lsquo;"},{U'\u201A',U"&lsquor;"},
             {U'\u0141',U"&Lstrok;"},{U'\u0142',U"&lstrok;"},{U'\u2AA6',U"&ltcc;"},{U'\u2A79',U"&ltcir;"},
             {U'\u003C',U"&lt;"},{U'\u22C9',U"&ltimes;"},{U'\u2976',U"&ltlarr;"},{U'\u2A7B',U"&ltquest;"},
             {U'\u25C3',U"&ltri;"},{U'\u2996',U"&ltrPar;"},{U'\u294A',U"&lurdshar;"},{U'\u2966',U"&luruhar;"},
             {U'\u2268',U""},{U'\u00AF',U"&macr;"},{U'\u2642',U"&male;"},{U'\u2720',U"&malt;"},
             {U'\u2905',U"&Map;"},{U'\u21A6',U"&map;"},{U'\u21A5',U"&mapstoup;"},{U'\u25AE',U"&marker;"},
             {U'\u2A29',U"&mcomma;"},{U'\u041C',U"&Mcy;"},{U'\u043C',U"&mcy;"},{U'\u2014',U"&mdash;"},
             {U'\u223A',U"&mDDot;"},{U'\u205F',U"&MediumSpace;"},{U'\u2133',U"&Mellintrf;"},
             {U'\U0001d510',U"&Mfr;"},{U'\U0001d52a',U"&mfr;"},{U'\u2127',U"&mho;"},{U'\u00B5',U"&micro;"},
             {U'\u2AF0',U"&midcir;"},{U'\u2223',U"&mid;"},{U'\u2212',U"&minus;"},{U'\u2A2A',U"&minusdu;"},
             {U'\u2213',U"&MinusPlus;"},{U'\u2ADB',U"&mlcp;"},{U'\u22A7',U"&models;"},
             {U'\U0001d544',U"&Mopf;"},{U'\U0001d55e',U"&mopf;"},{U'\U0001d4c2',U"&mscr;"},
             {U'\u039C',U"&Mu;"},{U'\u03BC',U"&mu;"},{U'\u22B8',U"&multimap;"},{U'\u0143',U"&Nacute;"},
             {U'\u0144',U"&nacute;"},{U'\u2220',U""},{U'\u2249',U"&nap;"},{U'\u2A70',U""},{U'\u224B',U""},
             {U'\u0149',U"&napos;"},{U'\u266E',U"&natural;"},{U'\u2115',U"&naturals;"},{U'\u00A0',U"&nbsp;"},
             {U'\u224E',U""},{U'\u224F',U""},{U'\u2A43',U"&ncap;"},{U'\u0147',U"&Ncaron;"},
             {U'\u0148',U"&ncaron;"},{U'\u0145',U"&Ncedil;"},{U'\u0146',U"&ncedil;"},{U'\u2247',U"&ncong;"},
             {U'\u2A6D',U""},{U'\u2A42',U"&ncup;"},{U'\u041D',U"&Ncy;"},{U'\u043D',U"&ncy;"},
             {U'\u2013',U"&ndash;"},{U'\u2924',U"&nearhk;"},{U'\u2197',U"&nearr;"},{U'\u21D7',U"&neArr;"},
             {U'\u2260',U"&ne;"},{U'\u2250',U""},{U'\u200B',U"&NegativeMediumSpace;"},{U'\u2262',U"&nequiv;"},
             {U'\u2928',U"&nesear;"},{U'\u2242',U""},{U'\u000A',U"&NewLine;"},{U'\u2204',U"&nexist;"},
             {U'\U0001d511',U"&Nfr;"},{U'\U0001d52b',U"&nfr;"},{U'\u2267',U""},{U'\u2271',U"&nge;"},
             {U'\u2A7E',U""},{U'\u22D9',U""},{U'\u2275',U"&ngsim;"},{U'\u226B',U""},{U'\u226F',U"&ngt;"},
             {U'\u226B',U""},{U'\u21AE',U"&nharr;"},{U'\u21CE',U"&nhArr;"},{U'\u2AF2',U"&nhpar;"},
             {U'\u220B',U"&ni;"},{U'\u22FC',U"&nis;"},{U'\u22FA',U"&nisd;"},{U'\u040A',U"&NJcy;"},
             {U'\u045A',U"&njcy;"},{U'\u219A',U"&nlarr;"},{U'\u21CD',U"&nlArr;"},{U'\u2025',U"&nldr;"},
             {U'\u2266',U""},{U'\u2270',U"&nle;"},{U'\u2A7D',U""},{U'\u226E',U"&nless;"},{U'\u22D8',U""},
             {U'\u2274',U"&nlsim;"},{U'\u226A',U""},{U'\u22EA',U"&nltri;"},{U'\u22EC',U"&nltrie;"},
             {U'\u226A',U""},{U'\u2224',U"&nmid;"},{U'\u2060',U"&NoBreak;"},{U'\U0001d55f',U"&nopf;"},
             {U'\u2AEC',U"&Not;"},{U'\u00AC',U"&not;"},{U'\u226D',U"&NotCupCap;"},
             {U'\u2226',U"&NotDoubleVerticalBar;"},{U'\u2209',U"&NotElement;"},
             {U'\u2279',U"&NotGreaterLess;"},{U'\u22F5',U""},{U'\u22F9',U""},{U'\u22F7',U"&notinvb;"},
             {U'\u22F6',U"&notinvc;"},{U'\u29CF',U""},{U'\u2278',U"&NotLessGreater;"},{U'\u2AA2',U""},
             {U'\u2AA1',U""},{U'\u220C',U"&notni;"},{U'\u22FE',U"&notnivb;"},{U'\u22FD',U"&notnivc;"},
             {U'\u2280',U"&NotPrecedes;"},{U'\u2AAF',U""},{U'\u22E0',U"&NotPrecedesSlantEqual;"},
             {U'\u29D0',U""},{U'\u22EB',U"&NotRightTriangle;"},{U'\u22ED',U"&NotRightTriangleEqual;"},
             {U'\u228F',U""},{U'\u22E2',U"&NotSquareSubsetEqual;"},{U'\u2290',U""},
             {U'\u22E3',U"&NotSquareSupersetEqual;"},{U'\u2282',U""},{U'\u2288',U"&NotSubsetEqual;"},
             {U'\u2281',U"&NotSucceeds;"},{U'\u2AB0',U""},{U'\u22E1',U"&NotSucceedsSlantEqual;"},
             {U'\u227F',U""},{U'\u2283',U""},{U'\u2289',U"&NotSupersetEqual;"},{U'\u2241',U"&NotTilde;"},
             {U'\u2244',U"&NotTildeEqual;"},{U'\u2AFD',U""},{U'\u2202',U""},{U'\u2A14',U"&npolint;"},
             {U'\u2933',U""},{U'\u219B',U"&nrarr;"},{U'\u21CF',U"&nrArr;"},{U'\u219D',U""},
             {U'\U0001d4a9',U"&Nscr;"},{U'\U0001d4c3',U"&nscr;"},{U'\u2284',U"&nsub;"},{U'\u2AC5',U""},
             {U'\u2285',U"&nsup;"},{U'\u2AC6',U""},{U'\u00D1',U"&Ntilde;"},{U'\u00F1',U"&ntilde;"},
             {U'\u039D',U"&Nu;"},{U'\u03BD',U"&nu;"},{U'\u0023',U"&num;"},{U'\u2116',U"&numero;"},
             {U'\u2007',U"&numsp;"},{U'\u224D',U""},{U'\u22AC',U"&nvdash;"},{U'\u22AD',U"&nvDash;"},
             {U'\u22AE',U"&nVdash;"},{U'\u22AF',U"&nVDash;"},{U'\u2265',U""},{U'\u003E',U""},
             {U'\u2904',U"&nvHarr;"},{U'\u29DE',U"&nvinfin;"},{U'\u2902',U"&nvlArr;"},{U'\u2264',U""},
             {U'\u003C',U""},{U'\u22B4',U""},{U'\u2903',U"&nvrArr;"},{U'\u22B5',U""},{U'\u223C',U""},
             {U'\u2923',U"&nwarhk;"},{U'\u2196',U"&nwarr;"},{U'\u21D6',U"&nwArr;"},{U'\u2927',U"&nwnear;"},
             {U'\u00D3',U"&Oacute;"},{U'\u00F3',U"&oacute;"},{U'\u00D4',U"&Ocirc;"},{U'\u00F4',U"&ocirc;"},
             {U'\u041E',U"&Ocy;"},{U'\u043E',U"&ocy;"},{U'\u0150',U"&Odblac;"},{U'\u0151',U"&odblac;"},
             {U'\u2A38',U"&odiv;"},{U'\u29BC',U"&odsold;"},{U'\u0152',U"&OElig;"},{U'\u0153',U"&oelig;"},
             {U'\u29BF',U"&ofcir;"},{U'\U0001d512',U"&Ofr;"},{U'\U0001d52c',U"&ofr;"},{U'\u02DB',U"&ogon;"},
             {U'\u00D2',U"&Ograve;"},{U'\u00F2',U"&ograve;"},{U'\u29C1',U"&ogt;"},{U'\u29B5',U"&ohbar;"},
             {U'\u03A9',U"&ohm;"},{U'\u29BE',U"&olcir;"},{U'\u29BB',U"&olcross;"},{U'\u203E',U"&oline;"},
             {U'\u29C0',U"&olt;"},{U'\u014C',U"&Omacr;"},{U'\u014D',U"&omacr;"},{U'\u03C9',U"&omega;"},
             {U'\u039F',U"&Omicron;"},{U'\u03BF',U"&omicron;"},{U'\u29B6',U"&omid;"},
             {U'\U0001d546',U"&Oopf;"},{U'\U0001d560',U"&oopf;"},{U'\u29B7',U"&opar;"},{U'\u29B9',U"&operp;"},
             {U'\u2A54',U"&Or;"},{U'\u2228',U"&or;"},{U'\u2A5D',U"&ord;"},{U'\u2134',U"&order;"},
             {U'\u00AA',U"&ordf;"},{U'\u00BA',U"&ordm;"},{U'\u22B6',U"&origof;"},{U'\u2A56',U"&oror;"},
             {U'\u2A57',U"&orslope;"},{U'\u2A5B',U"&orv;"},{U'\U0001d4aa',U"&Oscr;"},{U'\u00D8',U"&Oslash;"},
             {U'\u00F8',U"&oslash;"},{U'\u2298',U"&osol;"},{U'\u00D5',U"&Otilde;"},{U'\u00F5',U"&otilde;"},
             {U'\u2A36',U"&otimesas;"},{U'\u2A37',U"&Otimes;"},{U'\u00D6',U"&Ouml;"},{U'\u00F6',U"&ouml;"},
             {U'\u233D',U"&ovbar;"},{U'\u23DE',U"&OverBrace;"},{U'\u23B4',U"&OverBracket;"},
             {U'\u23DC',U"&OverParenthesis;"},{U'\u00B6',U"&para;"},{U'\u2AF3',U"&parsim;"},
             {U'\u2AFD',U"&parsl;"},{U'\u2202',U"&part;"},{U'\u041F',U"&Pcy;"},{U'\u043F',U"&pcy;"},
             {U'\u0025',U"&percnt;"},{U'\u002E',U"&period;"},{U'\u2030',U"&permil;"},{U'\u2031',U"&pertenk;"},
             {U'\U0001d513',U"&Pfr;"},{U'\U0001d52d',U"&pfr;"},{U'\u03A6',U"&Phi;"},{U'\u03C6',U"&phi;"},
             {U'\u03D5',U"&phiv;"},{U'\u260E',U"&phone;"},{U'\u03A0',U"&Pi;"},{U'\u03C0',U"&pi;"},
             {U'\u03D6',U"&piv;"},{U'\u210E',U"&planckh;"},{U'\u2A23',U"&plusacir;"},{U'\u2A22',U"&pluscir;"},
             {U'\u002B',U"&plus;"},{U'\u2A25',U"&plusdu;"},{U'\u2A72',U"&pluse;"},{U'\u00B1',U"&PlusMinus;"},
             {U'\u2A26',U"&plussim;"},{U'\u2A27',U"&plustwo;"},{U'\u2A15',U"&pointint;"},
             {U'\U0001d561',U"&popf;"},{U'\u2119',U"&Popf;"},{U'\u00A3',U"&pound;"},{U'\u2AB7',U"&prap;"},
             {U'\u2ABB',U"&Pr;"},{U'\u227A',U"&pr;"},{U'\u227C',U"&prcue;"},{U'\u2AAF',U"&PrecedesEqual;"},
             {U'\u227E',U"&PrecedesTilde;"},{U'\u2AB9',U"&precnapprox;"},{U'\u2AB5',U"&precneqq;"},
             {U'\u22E8',U"&precnsim;"},{U'\u2AB3',U"&prE;"},{U'\u2032',U"&prime;"},{U'\u2033',U"&Prime;"},
             {U'\u220F',U"&prod;"},{U'\u232E',U"&profalar;"},{U'\u2312',U"&profline;"},
             {U'\u2313',U"&profsurf;"},{U'\u221D',U"&prop;"},{U'\u22B0',U"&prurel;"},
             {U'\U0001d4ab',U"&Pscr;"},{U'\U0001d4c5',U"&pscr;"},{U'\u03A8',U"&Psi;"},{U'\u03C8',U"&psi;"},
             {U'\u2008',U"&puncsp;"},{U'\U0001d514',U"&Qfr;"},{U'\U0001d52e',U"&qfr;"},
             {U'\U0001d562',U"&qopf;"},{U'\u211A',U"&Qopf;"},{U'\u2057',U"&qprime;"},
             {U'\U0001d4ac',U"&Qscr;"},{U'\U0001d4c6',U"&qscr;"},{U'\u2A16',U"&quatint;"},
             {U'\u003F',U"&quest;"},{U'\u0022',U"&quot;"},{U'\u21DB',U"&rAarr;"},{U'\u223D',U""},
             {U'\u0154',U"&Racute;"},{U'\u0155',U"&racute;"},{U'\u221A',U"&radic;"},{U'\u29B3',U"&raemptyv;"},
             {U'\u27E9',U"&rang;"},{U'\u27EB',U"&Rang;"},{U'\u2992',U"&rangd;"},{U'\u29A5',U"&range;"},
             {U'\u00BB',U"&raquo;"},{U'\u2975',U"&rarrap;"},{U'\u21E5',U"&rarrb;"},{U'\u2920',U"&rarrbfs;"},
             {U'\u2933',U"&rarrc;"},{U'\u2192',U"&rarr;"},{U'\u21A0',U"&Rarr;"},{U'\u291E',U"&rarrfs;"},
             {U'\u2945',U"&rarrpl;"},{U'\u2974',U"&rarrsim;"},{U'\u2916',U"&Rarrtl;"},{U'\u21A3',U"&rarrtl;"},
             {U'\u219D',U"&rarrw;"},{U'\u291A',U"&ratail;"},{U'\u291C',U"&rAtail;"},{U'\u2236',U"&ratio;"},
             {U'\u2773',U"&rbbrk;"},{U'\u007D',U"&rbrace;"},{U'\u005D',U"&rbrack;"},{U'\u298C',U"&rbrke;"},
             {U'\u298E',U"&rbrksld;"},{U'\u2990',U"&rbrkslu;"},{U'\u0158',U"&Rcaron;"},
             {U'\u0159',U"&rcaron;"},{U'\u0156',U"&Rcedil;"},{U'\u0157',U"&rcedil;"},{U'\u2309',U"&rceil;"},
             {U'\u0420',U"&Rcy;"},{U'\u0440',U"&rcy;"},{U'\u2937',U"&rdca;"},{U'\u2969',U"&rdldhar;"},
             {U'\u21B3',U"&rdsh;"},{U'\u211C',U"&real;"},{U'\u211B',U"&realine;"},{U'\u211D',U"&reals;"},
             {U'\u25AD',U"&rect;"},{U'\u297D',U"&rfisht;"},{U'\u230B',U"&rfloor;"},{U'\U0001d52f',U"&rfr;"},
             {U'\u2964',U"&rHar;"},{U'\u21C0',U"&rharu;"},{U'\u296C',U"&rharul;"},{U'\u03A1',U"&Rho;"},
             {U'\u03C1',U"&rho;"},{U'\u03F1',U"&rhov;"},{U'\u21C4',U"&RightArrowLeftArrow;"},
             {U'\u27E7',U"&RightDoubleBracket;"},{U'\u295D',U"&RightDownTeeVector;"},
             {U'\u2955',U"&RightDownVectorBar;"},{U'\u21C9',U"&rightrightarrows;"},{U'\u22A2',U"&RightTee;"},
             {U'\u295B',U"&RightTeeVector;"},{U'\u22CC',U"&rightthreetimes;"},
             {U'\u29D0',U"&RightTriangleBar;"},{U'\u22B3',U"&RightTriangle;"},
             {U'\u22B5',U"&RightTriangleEqual;"},{U'\u294F',U"&RightUpDownVector;"},
             {U'\u295C',U"&RightUpTeeVector;"},{U'\u2954',U"&RightUpVectorBar;"},
             {U'\u21BE',U"&RightUpVector;"},{U'\u2953',U"&RightVectorBar;"},{U'\u02DA',U"&ring;"},
             {U'\u200F',U"&rlm;"},{U'\u23B1',U"&rmoustache;"},{U'\u2AEE',U"&rnmid;"},{U'\u27ED',U"&roang;"},
             {U'\u21FE',U"&roarr;"},{U'\u2986',U"&ropar;"},{U'\U0001d563',U"&ropf;"},{U'\u2A2E',U"&roplus;"},
             {U'\u2A35',U"&rotimes;"},{U'\u2970',U"&RoundImplies;"},{U'\u0029',U"&rpar;"},
             {U'\u2994',U"&rpargt;"},{U'\u2A12',U"&rppolint;"},{U'\u203A',U"&rsaquo;"},
             {U'\U0001d4c7',U"&rscr;"},{U'\u21B1',U"&rsh;"},{U'\u22CA',U"&rtimes;"},{U'\u25B9',U"&rtri;"},
             {U'\u29CE',U"&rtriltri;"},{U'\u29F4',U"&RuleDelayed;"},{U'\u2968',U"&ruluhar;"},
             {U'\u211E',U"&rx;"},{U'\u015A',U"&Sacute;"},{U'\u015B',U"&sacute;"},{U'\u2AB8',U"&scap;"},
             {U'\u0160',U"&Scaron;"},{U'\u0161',U"&scaron;"},{U'\u2ABC',U"&Sc;"},{U'\u227B',U"&sc;"},
             {U'\u227D',U"&sccue;"},{U'\u2AB0',U"&sce;"},{U'\u2AB4',U"&scE;"},{U'\u015E',U"&Scedil;"},
             {U'\u015F',U"&scedil;"},{U'\u015C',U"&Scirc;"},{U'\u015D',U"&scirc;"},{U'\u2ABA',U"&scnap;"},
             {U'\u2AB6',U"&scnE;"},{U'\u22E9',U"&scnsim;"},{U'\u2A13',U"&scpolint;"},{U'\u227F',U"&scsim;"},
             {U'\u0421',U"&Scy;"},{U'\u0441',U"&scy;"},{U'\u22C5',U"&sdot;"},{U'\u2A66',U"&sdote;"},
             {U'\u21D8',U"&seArr;"},{U'\u00A7',U"&sect;"},{U'\u003B',U"&semi;"},{U'\u2929',U"&seswar;"},
             {U'\u2736',U"&sext;"},{U'\U0001d516',U"&Sfr;"},{U'\U0001d530',U"&sfr;"},{U'\u266F',U"&sharp;"},
             {U'\u0429',U"&SHCHcy;"},{U'\u0449',U"&shchcy;"},{U'\u0428',U"&SHcy;"},{U'\u0448',U"&shcy;"},
             {U'\u2191',U"&ShortUpArrow;"},{U'\u00AD',U"&shy;"},{U'\u03A3',U"&Sigma;"},{U'\u03C3',U"&sigma;"},
             {U'\u03C2',U"&sigmaf;"},{U'\u223C',U"&sim;"},{U'\u2A6A',U"&simdot;"},{U'\u2243',U"&sime;"},
             {U'\u2A9E',U"&simg;"},{U'\u2AA0',U"&simgE;"},{U'\u2A9D',U"&siml;"},{U'\u2A9F',U"&simlE;"},
             {U'\u2246',U"&simne;"},{U'\u2A24',U"&simplus;"},{U'\u2972',U"&simrarr;"},{U'\u2A33',U"&smashp;"},
             {U'\u29E4',U"&smeparsl;"},{U'\u2323',U"&smile;"},{U'\u2AAA',U"&smt;"},{U'\u2AAC',U"&smte;"},
             {U'\u2AAC',U""},{U'\u042C',U"&SOFTcy;"},{U'\u044C',U"&softcy;"},{U'\u233F',U"&solbar;"},
             {U'\u29C4',U"&solb;"},{U'\u002F',U"&sol;"},{U'\U0001d54a',U"&Sopf;"},{U'\U0001d564',U"&sopf;"},
             {U'\u2660',U"&spades;"},{U'\u2293',U"&sqcap;"},{U'\u2293',U""},{U'\u2294',U"&sqcup;"},
             {U'\u2294',U""},{U'\u228F',U"&sqsub;"},{U'\u2291',U"&sqsube;"},{U'\u2290',U"&sqsup;"},
             {U'\u2292',U"&sqsupe;"},{U'\u25A1',U"&square;"},{U'\U0001d4ae',U"&Sscr;"},
             {U'\U0001d4c8',U"&sscr;"},{U'\u22C6',U"&sstarf;"},{U'\u2606',U"&star;"},{U'\u2282',U"&sub;"},
             {U'\u22D0',U"&Sub;"},{U'\u2ABD',U"&subdot;"},{U'\u2AC5',U"&subE;"},{U'\u2286',U"&sube;"},
             {U'\u2AC3',U"&subedot;"},{U'\u2AC1',U"&submult;"},{U'\u2ACB',U"&subnE;"},{U'\u228A',U"&subne;"},
             {U'\u2ABF',U"&subplus;"},{U'\u2979',U"&subrarr;"},{U'\u2AC7',U"&subsim;"},
             {U'\u2AD5',U"&subsub;"},{U'\u2AD3',U"&subsup;"},{U'\u2211',U"&sum;"},{U'\u266A',U"&sung;"},
             {U'\u00B9',U"&sup1;"},{U'\u00B2',U"&sup2;"},{U'\u00B3',U"&sup3;"},{U'\u2283',U"&sup;"},
             {U'\u22D1',U"&Sup;"},{U'\u2ABE',U"&supdot;"},{U'\u2AD8',U"&supdsub;"},{U'\u2AC6',U"&supE;"},
             {U'\u2287',U"&supe;"},{U'\u2AC4',U"&supedot;"},{U'\u27C9',U"&suphsol;"},{U'\u2AD7',U"&suphsub;"},
             {U'\u297B',U"&suplarr;"},{U'\u2AC2',U"&supmult;"},{U'\u2ACC',U"&supnE;"},{U'\u228B',U"&supne;"},
             {U'\u2AC0',U"&supplus;"},{U'\u2AC8',U"&supsim;"},{U'\u2AD4',U"&supsub;"},{U'\u2AD6',U"&supsup;"},
             {U'\u21D9',U"&swArr;"},{U'\u292A',U"&swnwar;"},{U'\u00DF',U"&szlig;"},{U'\u0009',U"&Tab;"},
             {U'\u2316',U"&target;"},{U'\u03A4',U"&Tau;"},{U'\u03C4',U"&tau;"},{U'\u0164',U"&Tcaron;"},
             {U'\u0165',U"&tcaron;"},{U'\u0162',U"&Tcedil;"},{U'\u0163',U"&tcedil;"},{U'\u0422',U"&Tcy;"},
             {U'\u0442',U"&tcy;"},{U'\u20DB',U"&tdot;"},{U'\u2315',U"&telrec;"},{U'\U0001d517',U"&Tfr;"},
             {U'\U0001d531',U"&tfr;"},{U'\u2234',U"&there4;"},{U'\u0398',U"&Theta;"},{U'\u03B8',U"&theta;"},
             {U'\u03D1',U"&thetasym;"},{U'\u205F',U""},{U'\u2009',U"&ThinSpace;"},{U'\u00DE',U"&THORN;"},
             {U'\u00FE',U"&thorn;"},{U'\u2A31',U"&timesbar;"},{U'\u00D7',U"&times;"},{U'\u2A30',U"&timesd;"},
             {U'\u2336',U"&topbot;"},{U'\u2AF1',U"&topcir;"},{U'\U0001d54b',U"&Topf;"},
             {U'\U0001d565',U"&topf;"},{U'\u2ADA',U"&topfork;"},{U'\u2034',U"&tprime;"},
             {U'\u2122',U"&trade;"},{U'\u25B5',U"&triangle;"},{U'\u225C',U"&triangleq;"},
             {U'\u25EC',U"&tridot;"},{U'\u2A3A',U"&triminus;"},{U'\u2A39',U"&triplus;"},
             {U'\u29CD',U"&trisb;"},{U'\u2A3B',U"&tritime;"},{U'\u23E2',U"&trpezium;"},
             {U'\U0001d4af',U"&Tscr;"},{U'\U0001d4c9',U"&tscr;"},{U'\u0426',U"&TScy;"},{U'\u0446',U"&tscy;"},
             {U'\u040B',U"&TSHcy;"},{U'\u045B',U"&tshcy;"},{U'\u0166',U"&Tstrok;"},{U'\u0167',U"&tstrok;"},
             {U'\u00DA',U"&Uacute;"},{U'\u00FA',U"&uacute;"},{U'\u219F',U"&Uarr;"},{U'\u2949',U"&Uarrocir;"},
             {U'\u040E',U"&Ubrcy;"},{U'\u045E',U"&ubrcy;"},{U'\u016C',U"&Ubreve;"},{U'\u016D',U"&ubreve;"},
             {U'\u00DB',U"&Ucirc;"},{U'\u00FB',U"&ucirc;"},{U'\u0423',U"&Ucy;"},{U'\u0443',U"&ucy;"},
             {U'\u21C5',U"&udarr;"},{U'\u0170',U"&Udblac;"},{U'\u0171',U"&udblac;"},{U'\u296E',U"&udhar;"},
             {U'\u297E',U"&ufisht;"},{U'\U0001d518',U"&Ufr;"},{U'\U0001d532',U"&ufr;"},
             {U'\u00D9',U"&Ugrave;"},{U'\u00F9',U"&ugrave;"},{U'\u2963',U"&uHar;"},{U'\u2580',U"&uhblk;"},
             {U'\u231C',U"&ulcorn;"},{U'\u230F',U"&ulcrop;"},{U'\u25F8',U"&ultri;"},{U'\u016A',U"&Umacr;"},
             {U'\u016B',U"&umacr;"},{U'\u23DF',U"&UnderBrace;"},{U'\u23DD',U"&UnderParenthesis;"},
             {U'\u228E',U"&UnionPlus;"},{U'\u0172',U"&Uogon;"},{U'\u0173',U"&uogon;"},
             {U'\U0001d54c',U"&Uopf;"},{U'\U0001d566',U"&uopf;"},{U'\u2912',U"&UpArrowBar;"},
             {U'\u2195',U"&updownarrow;"},{U'\u03C5',U"&upsi;"},{U'\u03D2',U"&Upsi;"},
             {U'\u03A5',U"&Upsilon;"},{U'\u21C8',U"&upuparrows;"},{U'\u231D',U"&urcorn;"},
             {U'\u230E',U"&urcrop;"},{U'\u016E',U"&Uring;"},{U'\u016F',U"&uring;"},{U'\u25F9',U"&urtri;"},
             {U'\U0001d4b0',U"&Uscr;"},{U'\U0001d4ca',U"&uscr;"},{U'\u22F0',U"&utdot;"},
             {U'\u0168',U"&Utilde;"},{U'\u0169',U"&utilde;"},{U'\u00DC',U"&Uuml;"},{U'\u00FC',U"&uuml;"},
             {U'\u29A7',U"&uwangle;"},{U'\u299C',U"&vangrt;"},{U'\u228A',U""},{U'\u2ACB',U""},{U'\u228B',U""},
             {U'\u2ACC',U""},{U'\u2AE8',U"&vBar;"},{U'\u2AEB',U"&Vbar;"},{U'\u2AE9',U"&vBarv;"},
             {U'\u0412',U"&Vcy;"},{U'\u0432',U"&vcy;"},{U'\u22A9',U"&Vdash;"},{U'\u22AB',U"&VDash;"},
             {U'\u2AE6',U"&Vdashl;"},{U'\u22BB',U"&veebar;"},{U'\u225A',U"&veeeq;"},{U'\u22EE',U"&vellip;"},
             {U'\u007C',U"&verbar;"},{U'\u2016',U"&Verbar;"},{U'\u2758',U"&VerticalSeparator;"},
             {U'\u2240',U"&VerticalTilde;"},{U'\U0001d519',U"&Vfr;"},{U'\U0001d533',U"&vfr;"},
             {U'\U0001d54d',U"&Vopf;"},{U'\U0001d567',U"&vopf;"},{U'\U0001d4b1',U"&Vscr;"},
             {U'\U0001d4cb',U"&vscr;"},{U'\u22AA',U"&Vvdash;"},{U'\u299A',U"&vzigzag;"},
             {U'\u0174',U"&Wcirc;"},{U'\u0175',U"&wcirc;"},{U'\u2A5F',U"&wedbar;"},{U'\u2259',U"&wedgeq;"},
             {U'\u2118',U"&weierp;"},{U'\U0001d51a',U"&Wfr;"},{U'\U0001d534',U"&wfr;"},
             {U'\U0001d54e',U"&Wopf;"},{U'\U0001d568',U"&wopf;"},{U'\U0001d4b2',U"&Wscr;"},
             {U'\U0001d4cc',U"&wscr;"},{U'\U0001d51b',U"&Xfr;"},{U'\U0001d535',U"&xfr;"},{U'\u039E',U"&Xi;"},
             {U'\u03BE',U"&xi;"},{U'\u22FB',U"&xnis;"},{U'\U0001d54f',U"&Xopf;"},{U'\U0001d569',U"&xopf;"},
             {U'\U0001d4b3',U"&Xscr;"},{U'\U0001d4cd',U"&xscr;"},{U'\u00DD',U"&Yacute;"},
             {U'\u00FD',U"&yacute;"},{U'\u042F',U"&YAcy;"},{U'\u044F',U"&yacy;"},{U'\u0176',U"&Ycirc;"},
             {U'\u0177',U"&ycirc;"},{U'\u042B',U"&Ycy;"},{U'\u044B',U"&ycy;"},{U'\u00A5',U"&yen;"},
             {U'\U0001d51c',U"&Yfr;"},{U'\U0001d536',U"&yfr;"},{U'\u0407',U"&YIcy;"},{U'\u0457',U"&yicy;"},
             {U'\U0001d550',U"&Yopf;"},{U'\U0001d56a',U"&yopf;"},{U'\U0001d4b4',U"&Yscr;"},
             {U'\U0001d4ce',U"&yscr;"},{U'\u042E',U"&YUcy;"},{U'\u044E',U"&yucy;"},{U'\u00FF',U"&yuml;"},
             {U'\u0178',U"&Yuml;"},{U'\u0179',U"&Zacute;"},{U'\u017A',U"&zacute;"},{U'\u017D',U"&Zcaron;"},
             {U'\u017E',U"&zcaron;"},{U'\u0417',U"&Zcy;"},{U'\u0437',U"&zcy;"},{U'\u017B',U"&Zdot;"},
             {U'\u017C',U"&zdot;"},{U'\u2128',U"&zeetrf;"},{U'\u0396',U"&Zeta;"},{U'\u03B6',U"&zeta;"},
             {U'\U0001d537',U"&zfr;"},{U'\u0416',U"&ZHcy;"},{U'\u0436',U"&zhcy;"},{U'\u21DD',U"&zigrarr;"},
             {U'\U0001d56b',U"&zopf;"},{U'\U0001d4b5',U"&Zscr;"},{U'\U0001d4cf',U"&zscr;"},
             {U'\u200D',U"&zwj;"},{U'\u200C',U"&zwnj;"}};

    const std::unordered_map<char32_t, std::pair<char32_t, std::u32string>> htmlLookaheads =
            {{U'\u223E',std::make_pair(U'\u0333', U"&acE;")},{U'\u003D',std::make_pair(U'\u20E5', U"&bne;")},
             {U'\u2261',std::make_pair(U'\u20E5', U"&bnequiv;")},
             {U'\u2229',std::make_pair(U'\uFE00', U"&caps;")},
             {U'\u222A',std::make_pair(U'\uFE00', U"&cups;")},
             {U'\u0066',std::make_pair(U'\u006A', U"&fjlig;")},
             {U'\u22DB',std::make_pair(U'\uFE00', U"&gesl;")},
             {U'\u2269',std::make_pair(U'\uFE00', U"&gvertneqq;")},
             {U'\u2AAD',std::make_pair(U'\uFE00', U"&lates;")},
             {U'\u22DA',std::make_pair(U'\uFE00', U"&lesg;")},
             {U'\u2268',std::make_pair(U'\uFE00', U"&lvertneqq;")},
             {U'\u2220',std::make_pair(U'\u20D2', U"&nang;")},
             {U'\u2A70',std::make_pair(U'\u0338', U"&napE;")},
             {U'\u224B',std::make_pair(U'\u0338', U"&napid;")},
             {U'\u224E',std::make_pair(U'\u0338', U"&nbump;")},
             {U'\u224F',std::make_pair(U'\u0338', U"&nbumpe;")},
             {U'\u2A6D',std::make_pair(U'\u0338', U"&ncongdot;")},
             {U'\u2250',std::make_pair(U'\u0338', U"&nedot;")},
             {U'\u2242',std::make_pair(U'\u0338', U"&nesim;")},
             {U'\u2267',std::make_pair(U'\u0338', U"&ngE;")},
             {U'\u2A7E',std::make_pair(U'\u0338', U"&ngeqslant;")},
             {U'\u22D9',std::make_pair(U'\u0338', U"&nGg;")},{U'\u226B',std::make_pair(U'\u20D2', U"&nGt;")},
             {U'\u226B',std::make_pair(U'\u0338', U"&nGtv;")},{U'\u2266',std::make_pair(U'\u0338', U"&nlE;")},
             {U'\u2A7D',std::make_pair(U'\u0338', U"&nleqslant;")},
             {U'\u22D8',std::make_pair(U'\u0338', U"&nLl;")},{U'\u226A',std::make_pair(U'\u20D2', U"&nLt;")},
             {U'\u226A',std::make_pair(U'\u0338', U"&nLtv;")},
             {U'\u22F5',std::make_pair(U'\u0338', U"&notindot;")},
             {U'\u22F9',std::make_pair(U'\u0338', U"&notinE;")},
             {U'\u29CF',std::make_pair(U'\u0338', U"&NotLeftTriangleBar;")},
             {U'\u2AA2',std::make_pair(U'\u0338', U"&NotNestedGreaterGreater;")},
             {U'\u2AA1',std::make_pair(U'\u0338', U"&NotNestedLessLess;")},
             {U'\u2AAF',std::make_pair(U'\u0338', U"&NotPrecedesEqual;")},
             {U'\u29D0',std::make_pair(U'\u0338', U"&NotRightTriangleBar;")},
             {U'\u228F',std::make_pair(U'\u0338', U"&NotSquareSubset;")},
             {U'\u2290',std::make_pair(U'\u0338', U"&NotSquareSuperset;")},
             {U'\u2282',std::make_pair(U'\u20D2', U"&NotSubset;")},
             {U'\u2AB0',std::make_pair(U'\u0338', U"&NotSucceedsEqual;")},
             {U'\u227F',std::make_pair(U'\u0338', U"&NotSucceedsTilde;")},
             {U'\u2283',std::make_pair(U'\u20D2', U"&NotSuperset;")},
             {U'\u2AFD',std::make_pair(U'\u20E5', U"&nparsl;")},
             {U'\u2202',std::make_pair(U'\u0338', U"&npart;")},
             {U'\u2933',std::make_pair(U'\u0338', U"&nrarrc;")},
             {U'\u219D',std::make_pair(U'\u0338', U"&nrarrw;")},
             {U'\u2AC5',std::make_pair(U'\u0338', U"&nsubE;")},
             {U'\u2AC6',std::make_pair(U'\u0338', U"&nsupE;")},
             {U'\u224D',std::make_pair(U'\u20D2', U"&nvap;")},
             {U'\u2265',std::make_pair(U'\u20D2', U"&nvge;")},
             {U'\u003E',std::make_pair(U'\u20D2', U"&nvgt;")},
             {U'\u2264',std::make_pair(U'\u20D2', U"&nvle;")},
             {U'\u003C',std::make_pair(U'\u20D2', U"&nvlt;")},
             {U'\u22B4',std::make_pair(U'\u20D2', U"&nvltrie;")},
             {U'\u22B5',std::make_pair(U'\u20D2', U"&nvrtrie;")},
             {U'\u223C',std::make_pair(U'\u20D2', U"&nvsim;")},
             {U'\u223D',std::make_pair(U'\u0331', U"&race;")},
             {U'\u2AAC',std::make_pair(U'\uFE00', U"&smtes;")},
             {U'\u2293',std::make_pair(U'\uFE00', U"&sqcaps;")},
             {U'\u2294',std::make_pair(U'\uFE00', U"&sqcups;")},
             {U'\u205F',std::make_pair(U'\u200A', U"&ThickSpace;")},
             {U'\u228A',std::make_pair(U'\uFE00', U"&varsubsetneq;")},
             {U'\u2ACB',std::make_pair(U'\uFE00', U"&varsubsetneqq;")},
             {U'\u228B',std::make_pair(U'\uFE00', U"&varsupsetneq;")},
             {U'\u2ACC',std::make_pair(U'\uFE00', U"&varsupsetneqq;")}};

    std::unordered_map<std::u32string, std::pair<char32_t, char32_t>> htmlDecodeTable;

    const std::unordered_set<char> urlUnreserved =
            {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
             'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
             'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4',
             '5', '6', '7', '8', '9', '-', '_', '.', '~'};

    void initializeHtmlDecodeTable() {
        // add entities
        for(auto &e: htmlEntities) {
            // if string is empty, we need to look at htmlLookahead
            if(!e.second.empty()) {
                htmlDecodeTable.insert(std::make_pair(e.second, std::make_pair(e.first, U'\0')));
            }
            else if(htmlLookaheads.count(e.first) == 1) {
                char32_t secondChar = htmlLookaheads.at(e.first).first;
                htmlDecodeTable.insert(std::make_pair(e.second, std::make_pair(e.first, secondChar)));
            }
        }
    }

}

std::string Qsf::Encoding::htmlEncode(std::string input, bool encodeAll) {
    if(!encodeAll) {
        boost::replace_all(input, "&", "&amp;");
        boost::replace_all(input, "\"", "&quot;");
        boost::replace_all(input, "<", "&lt;");
        boost::replace_all(input, ">", "&gt;");
    }
    else {
        // convert to utf32 to iterate through the characters
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
        std::u32string uinput = cv.from_bytes(input);
        std::basic_stringstream<char32_t> uoutputs;
        char32_t lookahead = '\0';
        for(char32_t c: uinput) {
            if(lookahead != '\0') {
                // if we have a lookahead, check the htmlLookahead map if there is an entity for that
                // (there should always be one, if the map is correct)
                if(htmlLookaheads.count(lookahead) != 1) {
                    uoutputs << lookahead;
                    lookahead = '\0';
                }
                else {
                    // check whether the current char matches the second char for the entity
                    auto& currentLA = htmlLookaheads.at(lookahead);
                    if(currentLA.first == c) {
                        uoutputs << currentLA.second;
                        lookahead = '\0';
                        continue;
                    }
                    else {
                        uoutputs << lookahead;
                        lookahead = '\0';
                    }
                }
            }
            if(htmlEntities.count(c) == 1) {
                if(htmlEntities.at(c).empty()) {
                    // empty entity string means that entity stands for 2 characters
                    lookahead = c;
                }
                else {
                    uoutputs << htmlEntities.at(c);
                }
            }
            else {
                uoutputs << c;
            }
        }
        input = cv.to_bytes(uoutputs.str());
    }
    return input;
}

std::string Qsf::Encoding::htmlDecode(std::string input) {

    if(htmlDecodeTable.empty()) initializeHtmlDecodeTable();

    std::regex matchEntity(R"(&[A-Za-z0-9]{1,25}?;)");

    // callback function
    auto replaceEntity = [](const std::vector<std::string>& matches) -> std::string {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
        auto entity32 = cv.from_bytes(matches.at(0));
        if(htmlDecodeTable.count(entity32) != 1) {
            return matches.at(0);
        }
        else {
            std::basic_stringstream<char32_t> ret;
            ret << htmlDecodeTable.at(entity32).first;
            auto secondChar = htmlDecodeTable.at(entity32).second;
            if(secondChar != '\0') {
                ret << secondChar;
            }
            return cv.to_bytes(ret.str());
        }
    };

    // run replacement
    regex_replace_callback(input, matchEntity, replaceEntity);

    // unicode replacement
    std::regex matchUnicode(R"(&#(x([A-Fa-f0-9]{1,5})|([0-9]{1,6}));)");
    auto replaceUnicode = [](const std::vector<std::string>& matches) -> std::string {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
        // first alternative: decimal
        char32_t c;
        if(matches.size() < 3) {
            return "";
        }
        if(matches.size() > 3 && !matches.at(3).empty()) {
            c = (char32_t) std::stoul(matches.at(3));
        }
        // second alternative: hex
        else if(!matches.at(2).empty()) {
            c = (char32_t) std::stoul(matches.at(2), nullptr, 16);
        }
        else {
            return "";
        }
        std::u32string outs(1, c);
        return cv.to_bytes(outs);
    };
    regex_replace_callback(input, matchUnicode, replaceUnicode);

    return input;
}

std::string Qsf::Encoding::urlEncode(const std::string& input) {
    std::stringstream out;

    // check if character is valid, unreserved URL character, otherwise apply url encoding
    for(char c: input) {
        if(urlUnreserved.count(c) == 1) {
            out << c;
        }
        else {
            std::ios init(nullptr);
            init.copyfmt(out);
            out << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)c;
            out.copyfmt(init);
        }
    }

    return out.str();
}

std::string Qsf::Encoding::urlDecode(std::string input) {
    // match url codes by regex
    std::regex matchCode(R"(%([0-9A-F]{2}))");

    // replacement function
    auto replaceCode = [](const std::vector<std::string>& matches) -> std::string {
        return std::string(1, (char) std::stoul(matches.at(1), nullptr, 16));
    };

    regex_replace_callback(input, matchCode, replaceCode);

    return input;
}

bool Qsf::Encoding::isBase64(const std::string &input, bool allowWhitespaces) {
    std::regex rgx;
    if(allowWhitespaces) {
        rgx.assign(R"([A-Za-z0-9\+/ \t\n\r]+={0,2})");
    }
    else {
        rgx.assign(R"([A-Za-z0-9\+/]+={0,2})");
    }
    return (input.length() % 4 == 0) && std::regex_match(input, rgx);
}

std::string Qsf::Encoding::base64Encode(const std::string &input, size_t breakAfter, const std::string &breakSequence) {
    return base64_encode(reinterpret_cast<const unsigned char*>(input.c_str()), input.length(), breakAfter, breakSequence);
}

std::string Qsf::Encoding::base64Decode(const std::string &input) {
    return base64_decode(input);
}

std::string Qsf::Encoding::quotedPrintableEncode(const std::string &input, const std::string &lineEnding, bool replaceCrlf) {
    std::stringstream ret;
    int lineCount = 0;
    for(const char c: input) {
        // TODO do not replace =09 (tabulator)? spaces at the end of a line (shoud be encoded =20)?
        if(c >= 32 && c <= 126 && c != 61) {
            if(lineCount >= 75) {
                ret << "=" << lineEnding;
                lineCount = 0;
            }
            ret << c;
            ++lineCount;
        }
        else if(!replaceCrlf && (c == 10 || c == 13)) {
            ret << c;
            lineCount = 0;
        }
        else {
            if(lineCount >= 73) {
                ret << "=" << lineEnding;
                lineCount = 0;
            }
            ret << "=" << Qsf::to_uppercase(Qsf::hex_dump(std::string(1, c)));
            lineCount += 3;
        }
    }
    return ret.str();
}

std::string Qsf::Encoding::quotedPrintableDecode(std::string input) {
    std::regex matchCode(R"(=[0-9A-F]{2})");

    // replacement function
    auto replaceCode = [](const std::vector<std::string>& matches) -> std::string {
        return std::string(1, (char) std::stoul(matches.at(0), nullptr, 16));
    };

    regex_replace_callback(input, matchCode, replaceCode);

    return input;
}
