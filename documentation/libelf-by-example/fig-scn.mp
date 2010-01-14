% Show the relationship between data in an ELF file and the Elf_Scn
% descriptor with its set of associated Elf_Data descriptors.
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

pair boxwd, boxht, thinwd;
boxwd = (1u,0);
boxht = (0,.5u);
thinwd = (.2u,0);

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

def elfdata (suffix b) text $ =
  rboxit b ($);
  width b = 2* thinwd;
enddef;

beginfig(1);

  % Draw the sections in the file.
  boxjoin(a.ne=b.nw; a.se=b.sw);

  elfsegment (a0, boxwd) btex $\ldots$ etex;
  forsuffixes v = 1,2,3,4:
    elfsegment (a v, thinwd);
  endfor;
  elfsegment (a5, boxwd) btex $\ldots$ etex;
  height a0 = boxht;

  forsuffixes v = 1,2,3,4:
    fill bpath a v withcolor (.9 - .05*v) * white;
  endfor;

  drawboxes(a1,a2,a3,a4);
  drawboxed(a0,a5);

  label.rt("A section in an ELF file", a5.e);

  % Elf_Scn
  boxjoin();

  rboxit.s1(btex \textbf{Elf\_Scn} etex);
  height s1 = boxht;  width s1 = boxwd;

  s1.nw = a1.nw + (-1.5u,2u);
  fill bpath s1 withcolor 0.87 white;
  drawboxed(s1);

  label.rt(btex \small \texttt{Elf\_Scn} Descriptor etex, s1.e);

  % Draw the linked list of Elf_Data structures.
  boxjoin(a.w + (1.1u,0) = b.e; a.ne - a.se = b.ne - b.se);

  forsuffixes v = 1,2,3,4:
    elfdata (d v) "D" & str v;
  endfor;
  height d1 = boxht;
  d1.c = s1.e + (-.25u,-1u);

  forsuffixes v = 1,2,3,4:
    fill bpath d v withcolor (.9 - .05*v) * white;
  endfor;

  drawboxed(d1,d2,d3,d4);

  drawarrow s1.s{down} .. d1.w-(.3u,0){right} .. d1.w dashed evenly;
  forsuffixes v = 1,2,3:
    drawarrow d v e -- d [v+1] w dashed evenly;
  endfor;
  label.rt(btex \texttt{Elf\_Data} Descriptor List etex, d4.e);

  % Link up ELF_Data structures with their contents in the file.
  forsuffixes v = 1,2,3,4:
    drawdblarrow d v s {down} .. {down} a v n;
  endfor;

  % Highlight the 'section'
  pickup pencircle scaled 2pt;
  draw a1.nw -- a4.ne -- a4.se -- a1.sw -- cycle;

endfig;

end
