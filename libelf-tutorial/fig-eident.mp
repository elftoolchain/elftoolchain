% Show the layout of the e_ident[] byte array.
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

pair boxwd;

boxwd = (u,0);

arrowoff = markoff = 1.5pt;
arrowangle = 135;

def width suffix b = b ne - b nw enddef;

def makebox (suffix b) text $ =
  boxit b ($);
enddef;

def elflabel (expr p, off) (suffix pos, d) text $ =
  begingroup
    save n,q,wd; pair n; picture q;
    n = p + off; q = $ infont defaultfont;
    wd = xpart (lrcorner q - llcorner q) + labeloffset;	% width of underline
    label pos (q, n);
    if d = right:
      drawarrow (n - (wd,0)) -- n {right} .. {dir -45} (p + (0,arrowoff));
    else:
      drawarrow (n + (wd,0)) -- n {left} .. {dir -135} (p + (0,arrowoff));
    fi;
  endgroup
enddef;

verbatimtex
\documentclass{article}
\usepackage{times}
\begin{document}
etex;

beginfig(1);

  % Draw the e_ident[] array.
  boxjoin(a.ne = b.nw; a.se = b.sw);

  string t[];
  t[0] = "0x7F"; t[1] = "'E'"; t[2] = "'L'"; t[3] = "'F'";
  t[4] = t[5] = t[6] = t[7] = t[8] = " ";
  t[9] = "...";
  forsuffixes v = 0,1,2,3,4,5,6,7,8:
    boxit b.v(t[v]);
    width b.v = (u,0);
  endfor;

  boxit b9(t[9]);
  width b9 = (2u,0);

  path mark, t;
  picture p, d[];
  mark = (0,0) -- (.5u,0);

  % Descriptions
  d[0] = btex \small \texttt{EI\_MAG0} etex;
  d[1] = btex \small \texttt{EI\_MAG1} etex;
  d[2] = btex \small \texttt{EI\_MAG2} etex;
  d[3] = btex \small \texttt{EI\_MAG3} etex;
  d[4] = btex \small \texttt{EI\_CLASS} etex;
  d[5] = btex \small \texttt{EI\_DATA} etex;
  d[6] = btex \small \texttt{EI\_VERSION} etex;
  d[7] = btex \small \texttt{EI\_OSABI} etex;
  d[8] = btex \small \texttt{EI\_ABIVERSION} etex;
  forsuffixes v = 0,1,2,3,4,5,6,7,8:
    fill bpath b.v withcolor (if odd v: .8 white else: .9 white fi);
    drawboxed(b.v);
    t := (mark rotated -45) shifted (b.v.s + (markoff,-markoff));
    draw t;
    p := thelabel urt (d[v], (0,0));
    draw (p rotated -45) shifted (point infinity of t);;
  endfor;
  drawboxed(b9);

  label.lft (btex \texttt{e\_ident[]}: etex, b0.w);

  label.lft (btex offsets: etex, b0.sw shifted
    (0, ypart(point infinity of (mark rotated -45))));

  elflabel (b4.n, (-.5u,.5u), ulft, right) "class: 32 or 64 bit";
  elflabel (b5.n, (-1u,1u), ulft, right) "endianness";
  elflabel (b6.n, (.6u,1u), urt, left) "ELF version";
  elflabel (b7.n, (.6u,.5u), urt, left) "desired ABI";
endfig;

end
