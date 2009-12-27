% Show how the Elf Section Header affects data layout.
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
u := 2cm;

input boxes

markoff = 2pt;
markwd = markoff + 0.2u;

pair boxht, boxwd, hfudge, m[], t;
boxwd = (1u,0);
boxht = (0,.4u);
hfudge = (1.5pt,0);
t = (.05u,0);

pen p;
p := currentpen;

verbatimtex
\documentclass{article}
\usepackage{times}
\begin{document}
etex;

def elfsegment (suffix b)(expr wd) text $ =
  boxit b ($);
  b.ne - b.nw = wd;
enddef;

def mark(expr p, d) =
  begingroup
  save moff, mwd;
  moff := markoff; mwd := markwd;
  if d = down:
    moff := -markoff; mwd := -markwd;
  fi;
  draw (p + (0,moff)) -- (p + (0,mwd));
  endgroup
enddef;

def elflabel(expr a, b, d) text $ =
  begingroup
  save p, q; pair p,q;
  p = a + (0,d) + hfudge; q = b + (0,d) - hfudge;
  drawdblarrow p -- q;
  label top ($, .5[p,q]);
  endgroup
enddef;


beginfig(1);

  % Draw the ELF file layout
  boxjoin(a.ne = b.nw; a.se = b.sw);

  elfsegment (ehdr, boxwd) btex Ehdr etex;
  elfsegment (phdr, boxwd) btex Phdr etex;
  elfsegment (bl1, .7 boxwd);
  elfsegment (scn, 1.5 boxwd) btex ELF Section\raisebox{-1ex}{n} etex;
  elfsegment (bl2, boxwd);
  elfsegment (shdr, boxwd) btex Shdr etex;

  ehdr.nw - ehdr.sw = boxht;

  fill bpath ehdr withcolor .7 white;
  fill bpath phdr withcolor .85 white;
  fill bpath scn withcolor .85 white;
  fill bpath shdr withcolor .8 white;

  drawboxed (ehdr, phdr, bl1, scn, bl2);

  % Mark the 'Shdr' entry seperately.
  m1 = .8 [ shdr nw, shdr ne ];
  m2 = .8 [ shdr sw, shdr se ];
  fill (m1 - t) -- (m1 + t) -- (m2 + t) -- (m2 - t) -- cycle withcolor .6 white;

  drawboxed(shdr);

  % Draw the expanded Shdr entry.
  boxjoin(a.sw = b.nw; a.se = b.ne);
  boxit sh1 (btex sh\_type etex);
  boxit sh2 (btex sh\_size etex);
  boxit sh3 (btex sh\_addralign etex);
  boxit sh4 (btex sh\_offset etex);
  boxit sh5 (btex ... etc ... etex);

  sh1 c = shdr c + (-u,-1.5u);
  sh1 ne - sh1 nw = boxwd;

  forsuffixes v = 1,2,3,4,5:
    fill bpath sh.v withcolor .6 white;
  endfor;
  drawboxed(sh1, sh2, sh3, sh4, sh5);

  % Draw the 'expansion' lines.
  boxjoin();

  boxit shexp ();
  shexp.nw = sh1.nw + (-markoff,markoff);
  shexp.se = sh5.se + (markoff,-markoff);

  path endline;
  endline = (shdr.sw -- shdr.se) shifted (0,-markoff);

  m3 = (sh1.nw -- m2 - t) intersectionpoint endline;
  draw sh1.nw -- m3 cutbefore bpath shexp;
  m4 = (sh5.se -- m2 + t) intersectionpoint endline;
  draw sh5.se -- m4 cutbefore bpath shexp;

  % Draw marks.
  pickup pencircle scaled .1 pt;

  draw (m1 - t) -- (m1 + t) -- (m2 + t) -- (m2 - t) -- cycle;

  mark (ehdr sw, down);
  mark (scn sw, down);
  mark (scn se, down);
  mark (scn nw, up);

  % Labelled arrows.
  currentpen := p;

  elflabel (ehdr sw, scn sw, -markwd) btex \small sh\_offset etex;
  elflabel (scn sw, scn se, -markwd) btex \small sh\_size etex;

  % Other labels.
  label rt (btex \small \%sh\_addralign etex, scn nw + (0,.7 markwd) - hfudge);

endfig;

end
