% Show iteration through an ar(1) archive using elf_begin() and elf_next();
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
defaultfont:= "rptmr";
prologues:= 2;
u := 3cm;

input boxes

pair arboxwd, boxht, boxwd, fudge, xlate;

boxwd = (1u,0);
boxht = (0,.5u);
arboxwd = (.1u,0);

fudge = (0,3pt);
xlate = (0,.3u);

verbatimtex
\documentclass{article}
\usepackage{times}
\begin{document}
etex;

def makebox (suffix b, wd) text $ =
  boxit b ($);
  b ne - b nw = wd;
enddef;

def makecircle (suffix $) =
  circleit c$ (str $);
  c$ c = file.$ n shifted xlate;
enddef;

def curvedarrow (suffix x)(expr s) =
  drawarrow .1 [x, s] .. .5 [x,s] shifted (0,-.1u) .. .95 [x, s];
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

  z1 = arhdr1 nw shifted 1.3 xlate;
  drawarrow z1 -- arhdr1 nw + fudge;
  label top (btex \small elf\_begin() etex, z1);

  % Place `elf_next()' labels.
  boxjoin();
  forsuffixes $ = 1,2,3:
    boxit l $ (btex \small \parbox[t]{.1in}{elf\_next() elf\_begin()} etex);
    l$ s = file$ n shifted (0,.2u);
    drawunboxed(l$);
  endfor;

  forsuffixes $ = 1,2:
    drawarrow arhdr $ n .. l $ s shifted (0,-.05u) .. arhdr[$+1] nw;
  endfor;
  drawarrow arhdr3 n .. l3.s shifted (0,-.05u) .. file3 ne;

  % Mark the ar(1) headers present in the file.
  z2 = arhdr2 c shifted (0,-1u);
  label bot ("Archive headers", z2);

  forsuffixes v = 1,2,3:
    curvedarrow(z2, arhdr v s);
  endfor;

  % Mark the ar(1) archive magic bytes.
  z3 = armag c + (0,-1u);
  label bot ("AR magic", z3);
  drawarrow .1 [z3, armag s] -- .95 [z3, armag s];

endfig;

end
