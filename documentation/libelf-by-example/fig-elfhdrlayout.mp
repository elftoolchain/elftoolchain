% Show how the Elf Executable Header controls the layout of the
% rest of the file.
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

input rboxes

% Offset and length of marks.
markoff = 2pt;
markwd = markoff + 0.1u;

pair boxht, boxwd, hfudge;
boxwd = (1u,0);
boxht = (0,.5u);
hfudge = (1.5pt,0);

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

def mark(expr p, wd, d) =
  begingroup
  save moff, mwd;
  moff := markoff; mwd := wd;
  if d = down:
    moff := -markoff; mwd := -wd;
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

  % Draw the layout of the ELF file.
  boxjoin(a.ne = b.nw; a.se = b.sw);

  elfsegment(ehdr, 1.2 boxwd) btex Ehdr etex;
  elfsegment(bl0, .3 boxwd);
  elfsegment(phdr, boxwd) btex Phdr etex;
  elfsegment(bl1, boxwd);
  elfsegment(shdr, 1.1 boxwd) btex Shdr etex;

  % Shade the areas of interest.
  fill bpath ehdr withcolor .7 white;
  fill bpath phdr withcolor .85 white;
  fill bpath shdr withcolor .8 white;

  drawboxed (ehdr, bl0, phdr, bl1, shdr);

  % Draw the marks.
  pickup pencircle scaled .1 pt;

  mark (ehdr sw, 3.5 markwd, down);
  mark (ehdr nw, markwd, up);
  mark (ehdr ne, markwd, up);
  mark (phdr sw, 1.9 markwd, down);
  mark (phdr nw, markwd, up);
  mark (phdr ne, markwd, up);
  mark (shdr sw, 3.5 markwd, down);
  mark (shdr nw, markwd, up);
  mark (shdr ne, markwd, up);

  % And the arrows.
  currentpen := p;

  elflabel (ehdr sw, phdr sw, -1.7 markwd) btex \small e\_phoff etex;
  elflabel (ehdr sw, shdr sw, -3.2 markwd) btex \small e\_shoff etex;
  elflabel (ehdr nw, ehdr ne, .7 markwd) btex \small e\_ehsize etex;
  elflabel (phdr nw, phdr ne, .7 markwd)
    btex \small N\raisebox{-1ex}{phdr} * e\_phentsize etex;
  elflabel (shdr nw, shdr ne, .7 markwd)
    btex N\raisebox{-1ex}{shdr} * e\_shentsize etex;

endfig;

end
