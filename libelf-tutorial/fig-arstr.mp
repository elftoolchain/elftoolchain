% Show the structure of an ar(1) archive.
%
% Copyright (c) 2006-2009 Joseph Koshy.  All rights reserved.
%
% Redistribution and use in source and binary forms, with or without
% modification, are permitted provided that the following conditions
% are met:
% 1. Redistributions of source code must retain the above copyright
%    notice, this list of conditions and the following disclaimer.
% 2. Redistributions in binary form must reproduce the above copyright
%    notice, this list of conditions and the following disclaimer in the
%    documentation and/or other materials provided with the distribution.
%
% This software is provided by Joseph Koshy ``as is'' and
% any express or implied warranties, including, but not limited to, the
% implied warranties of merchantability and fitness for a particular purpose
% are disclaimed.  in no event shall Joseph Koshy be liable
% for any direct, indirect, incidental, special, exemplary, or consequential
% damages (including, but not limited to, procurement of substitute goods
% or services; loss of use, data, or profits; or business interruption)
% however caused and on any theory of liability, whether in contract, strict
% liability, or tort (including negligence or otherwise) arising in any way
% out of the use of this software, even if advised of the possibility of
% such damage.
%
% $Id$
%
defaultfont:= "Times-Roman";
prologues:= 2;
u := 3cm;

input boxes

pair arboxwd, boxht, boxwd, fudge, hfudge, m[], t, xlate;

boxwd = (1u,0);
boxht = (0,.5u);
arboxwd = (.1u,0);
t = (.05u,0);
hfudge = (1.5pt,0);
markoff = 1.8pt;
fudge = (0,3pt);
xlate = (0,.3u);

arrowoff = 2pt;						% offset for arrows
arrowangle = 135;

verbatimtex
\documentclass{article}
\usepackage{times}
\begin{document}
etex;

def elflabel (expr p, off) (suffix pos, d) text $ =
  begingroup
    save n,q,wd,fudge,da; pair n; picture q;
    n = p + off; q = $ infont defaultfont;
    wd = xpart (lrcorner q - llcorner q) + labeloffset;	% width of underline
    label pos (q, n);
    if d = up:
      da = arrowangle; fudge = -arrowoff;
    else:
      da = -arrowangle; fudge = arrowoff;
    fi;
    drawarrow (n + (wd,0)) -- n {left} .. {dir da} (p + (0,fudge));
  endgroup
enddef;

def makebox (suffix b, wd) text $ =
  boxit b ($);
  b ne - b nw = wd;
enddef;

beginfig(1);

  % Draw the layout of an ar(1) archive.
  boxjoin(a.ne = b.nw; a.se = b.sw);

  makebox (armag, arboxwd);
  makebox (arhdr1, arboxwd);
  makebox (file1, boxwd) "File 1";
  makebox (arhdr2, arboxwd);
  makebox (file2, boxwd) "File 2";
  makebox (arhdr3, arboxwd);
  makebox (file3, boxwd) "File 3";

  armag ne - armag se = boxht;

  % Show ARMAGIC and ar(1) headers using a different color.
  fill bpath armag withcolor .6 white;
  forsuffixes v = 1,2,3:
    fill bpath arhdr v withcolor .8 white;
  endfor;

  drawboxed(armag, arhdr1, arhdr2, arhdr3, file1, file2, file3);

  elflabel (armag.n, (.5u,.75u), urt, down) "Archive `magic'";

  z1 = file2.n shifted (.1u,.5u);
  label top ("Archive Headers", z1);
  forsuffixes $ = 1,2,3:
    drawarrow z1 -- arhdr$.n;
  endfor;

  % Draw the expanded ar_hdr entry
  boxjoin(a.sw = b.nw; a.se = b.ne);
  boxit ah1 (btex ar\_name etex);
  boxit ah2 (btex ar\_date etex);
  boxit ah3 (btex ar\_uid etex);
  boxit ah4 (btex ar\_gid etex);
  boxit ah5 (btex ar\_mode etex);
  boxit ah6 (btex ar\_size etex);
  boxit ah7 (btex ar\_fmag etex);

  ah1 c = file3 c + (.5u,-.5u);

  forsuffixes v = 1,2,3,4,5,6,7:
    fill bpath ah.v withcolor .6 white;
  endfor
  drawboxed(ah1,ah2,ah3,ah4,ah5,ah6,ah7);

  path cutline, boxline, l[];
  cutline = (armag.sw -- file3.se) shifted (0,-markoff);
  boxline = (ah1.nw + (0,u) -- ah7.sw + (0,-u)) shifted(-markoff,0);

  l1 = cutline intersectionpoint (ah1.nw -- arhdr3.se);
  l2 = boxline intersectionpoint (ah1.nw -- arhdr3.se);
  l3 = cutline intersectionpoint (ah7.sw -- arhdr3.sw);
  l4 = boxline intersectionpoint (ah7.sw -- arhdr3.sw);
  draw l1--l2; draw l3 -- l4;

  label bot ("Archive Header Entry", ah7.s);
  
endfig;

end
