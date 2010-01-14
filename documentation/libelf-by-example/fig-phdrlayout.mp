% Show how the Elf Program Header entry control layout in file and in memory.
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

markoff = 1.8pt;
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

def height suffix b = b ne - b se enddef;
def width suffix b = b ne - b nw enddef;

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
  elfsegment (bl0, .1 boxwd);
  elfsegment (phdr, boxwd) btex Phdr etex;
  elfsegment (bl1, .7 boxwd);
  elfsegment (scn, 1.5 boxwd) btex ELF Segment\raisebox{-1ex}{n} etex;
  elfsegment (bl2, boxwd);
  elfsegment (shdr, boxwd) btex Shdr etex;

  height ehdr = boxht;

  fill bpath ehdr withcolor .7 white;
  fill bpath phdr withcolor .8 white;
  fill bpath scn withcolor .8 white;
  fill bpath shdr withcolor .85 white;

  drawboxed (ehdr, bl0, bl1, scn, bl2, shdr);

  % Mark the 'Phdr' entry seperately.
  m1 = .8 [ phdr nw, phdr ne ];
  m2 = .8 [ phdr sw, phdr se ];
  fill (m1 - t) -- (m1 + t) -- (m2 + t) -- (m2 - t) -- cycle withcolor .6 white;

  drawboxed(phdr);

  % Draw the expanded Phdr entry.
  boxjoin(a.sw = b.nw; a.se = b.ne);
  boxit ph1 (btex p\_type etex);
  boxit ph2 (btex p\_f{}ilesz etex);
  boxit ph3 (btex p\_align etex);
  boxit ph4 (btex p\_memsz etex);
  boxit ph5 (btex ... etc ... etex);

  ph1 c = phdr c + (-.7u,2.2u);
  ph1 ne - ph1 nw = boxwd;

  forsuffixes v = 1,2,3,4,5:
    fill bpath ph.v withcolor .6 white;
  endfor;
  drawboxed(ph1, ph2, ph3, ph4, ph5);

  % Draw the 'expansion' lines.
  boxjoin();
  boxit phexp ();
  phexp nw = ph1 nw + (-markoff,markoff);
  phexp se = ph5 se + (markoff,-markoff);

  path cutline;
  cutline = (phdr nw -- phdr ne) shifted (0,markoff);

  m3 = (ph1 ne -- m1 + t) intersectionpoint cutline;
  draw ph1 ne -- m3 cutbefore bpath phexp;
  m4 = (ph5 sw -- m1 - t) intersectionpoint cutline;
  draw ph5 sw -- m4 cutbefore bpath phexp;

  % Draw the segment in 'memory'
  boxjoin();
  boxit mem (btex \small Segment in Memory etex);
  height mem = boxht;

  width mem = 1.1 (width scn);
  xpart mem sw = xpart scn nw;
  ypart mem w = ypart ph5 sw;

  fill bpath mem withcolor .8 white;
  drawboxed (mem);

  % Draw marks.
  pickup pencircle scaled .1 pt;

  draw (m1 - t) -- (m1 + t) -- (m2 + t) -- (m2 - t) -- cycle;

  mark (ehdr sw, down);
  mark (scn sw, down);
  mark (scn se, down);
  mark (scn nw, up);

  mark (mem sw, down);
  mark (mem se, down);
  mark (mem nw, up);
  % Labelled arrows.
  currentpen := p;

  elflabel (ehdr sw, scn sw, -markwd) btex \small p\_offset etex;
  elflabel (scn sw, scn se, -markwd) btex \small p\_f{}ilesz etex;

  elflabel (mem sw, mem se, -markwd) btex \small p\_memsz etex;

  % Other labels.
  label rt (btex \small \%p\_align etex, scn nw + (0,.7 markwd) - hfudge);
  label urt (btex \small p\_vaddr etex, mem nw + (0,.7 markwd) + hfudge);
  label top ("Program Header Table Entry", ph1 n);
  label lft ("ELF file:", ehdr w);
endfig;

end
