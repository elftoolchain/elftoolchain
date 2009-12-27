% Describe an ELF_Data descriptor.
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

input rboxes

markoff = 2pt;
markwd = markoff + 0.1u;

pair boxht, boxwd, hfudge, vfudge;
boxwd = (1u,0);
boxht = (0,.5u);
hfudge = (1.5pt,0);
vfudge = (0,1.5pt);

pen p;
p := currentpen;

verbatimtex
\documentclass{article}
\usepackage{times}
\begin{document}
etex;

def height suffix b = b ne - b se enddef;
def width suffix b = b ne - b nw enddef;

def elfsegment (suffix b)(expr ht) text $ =
  boxit b ($);
  height b = ht;
enddef;

def mark(expr p, d) =
  begingroup
  save moff, mwd;
  moff := markoff; mwd := markwd;
  if d = left:
    moff := -markoff; mwd := -markwd;
  fi;
  draw (p + (moff,0)) -- (p + (mwd,0));
  endgroup
enddef;

def elflabel(expr a, b, d) text $ =
  begingroup
  save p, q; pair p,q;
  p = a + (d,0) - vfudge; q = b + (d,0) + vfudge;
  drawdblarrow p -- q;
  label lft ($, .5[p,q]);
  endgroup
enddef;

beginfig(1);

  % Draw the Elf_Data descriptor
  boxjoin();

  rboxit data(btex \textbf{Elf\_Data} etex);
  width data = boxwd; height data = boxht;

  fill bpath data withcolor 0.85 white;
  drawboxed(data);
  label.rt (btex \texttt{Elf\_Data} Descriptor etex, data.e);

  % Draw the memory area.
  boxit mem();
  width mem = boxwd;
  height mem = 3 * boxht;

  mem n = data n + (1.5u,-1u);
  fill bpath mem withcolor 0.8 white;

  drawboxes(mem);

  % Draw the ELF file.
  boxjoin(a sw = b nw; a se = b ne);

  elfsegment (f1, 1.5 boxht);
  width f1 = boxwd;

  elfsegment (f2, .6 (height mem));
  f2 c = mem c + (1.5u,0) + .5 boxwd;
  fill bpath f2 withcolor 0.8 white;

  elfsegment (f3, boxht);

  drawboxes(f1,f2,f3);

  % place marks
  pickup pencircle scaled 0.1 pt;

  mark (f1 nw, left);
  mark (f2 nw, left);
  mark (f2 ne, right);
  mark (f2 sw, left);

  mark (mem nw, left);
  mark (mem sw, left);

  % Place size indicators.
  currentpen := p;

  elflabel (f1 nw, f1 sw, -.7 markwd) btex \small d\_off etex;
  elflabel (mem nw, mem sw, -.7 markwd) btex \small d\_size etex;

  % Draw connectors.
  drawarrow (mem ne + (markoff,0)){right} .. {right}(f2 nw - (markwd,0) - hfudge);
  drawarrow (mem se + (markoff,0)){right} .. {right}(f2 sw - (markwd,0) - hfudge);

  drawarrow data c .. (mem nw - (1u,0)) .. {right}(mem nw - (markwd,0) - hfudge)
    cutbefore bpath data;

  % Other labels
  label rt (btex \small \%d\_align etex, f2 ne + (markwd,0));
  label ulft (btex \small d\_buf etex, mem nw - (markoff,0));

  label bot (btex Memory Buffer etex, mem s);
  label top (btex ELF File Section etex, f1 n);

endfig;

end
