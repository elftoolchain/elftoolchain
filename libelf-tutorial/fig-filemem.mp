% Show the relationship between the file representation of an ELF data
% type and its memory representation.
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
u := 1cm;

input boxes

markoff = 2pt;

pair boxht, boxwd, hfudge;
boxht = (0,u);
boxwd = (u,0);
hfudge = (1.5pt,0);

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
  width b = wd;
enddef;

def mark (expr p, ht, d) =
  begingroup
  save moff, mht;
  moff := markoff; mht := ht;
  if d = down:
    moff := -markoff; mht := -ht;
  fi;
  draw (p + (0,moff)) -- (p + (0,mht));
  endgroup
enddef;

def elflabel(expr a, b, m) (suffix @) text $ =
  begingroup
  save p, q; pair p,q;
  p = a + (0,m) + hfudge; q = b + (0,m) - hfudge;
  drawdblarrow p -- q;
  label @ ($, .5[p,q] + .5 (0,m));
  endgroup
enddef;

beginfig(1);

  % layout boxes horizontally
  boxjoin(a.ne=b.nw; a.se=b.sw);

  elfsegment (a1, boxwd);
  height a1 = boxht;
  elfsegment (a2, 3 boxwd);
  elfsegment (a3, .75 boxwd);
  elfsegment (a4, 3 boxwd);
  elfsegment (a5, boxwd);
  drawboxes(a1,a2,a3,a4,a5);

  fill bpath a3 withcolor 0.7 white;

  label.lft("ELF File", a1.w);

  % Draw the "memory" blob.
  boxjoin();

  boxit.b1();
  width b1 = 1.5 boxwd;
  height b1 = boxht;
  b1.c = a3.c + (0,-3u);
  drawboxes(b1);

  fill bpath b1 withcolor 0.9 white;

  label.lft("Memory Representation", b1.w);

  % Place marks.
  pickup pencircle scaled .1 pt;

  mark (a3 sw, .6u, down);
  mark (a3 nw, .3u, up);
  mark (a3 ne, .3u, up);
  mark (b1 nw, .6u, up);
  mark (b1 sw, .3u, down);
  mark (b1 se, .3u, down);

  currentpen := p;

  elflabel (a3 nw, a3 ne, .2u, top) btex \small f{}ile size etex;
  label.rt(btex \small \%falign etex, a3.sw+(0,-.6u));
  label.rt(btex \small \%malign etex, b1.nw+(0,.5u));
  elflabel (b1 sw, b1 se, -.2u, bot) btex \small memory size etex;

  % Draw the curvy arrows and label them.

  z1 = b1.w + (-.5u,1.5u);
  z2 = b1.e + (.5u,1.5u);

  drawarrow b1.nw shifted (-markoff,markoff) .. z1 ..
    {dir45}a3.sw shifted (-markoff, -markoff);
  drawarrow a3.se shifted (markoff,-markoff) {dir-45} .. z2 ..
    b1.ne shifted (markoff,markoff);

  label.lft("xlatetof()", z1);
  label.rt("xlatetom()", z2);
  label.top ("File Representation", a3.n shifted (0,.7 u));
endfig;

end
