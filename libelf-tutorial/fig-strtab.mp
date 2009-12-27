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

  % Draw the layout of string table.
  string t[];
  t[0] = "'\0'";
  t[1] = "'N'"; t[2] = "'a'"; t[3] = "'m'"; t[4] = "'e'";
  t[5] = "'1'"; t[6] = "'\0'";
  t[7] = "'N'"; t[8] = "'a'"; t[9] = "'m'"; t[10] = "'e'";
  t[11] = "'2'"; t[12] = "'\0'";
  t[13] = "...";

  boxjoin(a.ne = b.nw; a.se = b.sw);

  forsuffixes v = 0,1,2,3,4,5,6,7:
    boxit b.v(t[v]);
    width b.v = (u,0);
    fill bpath b.v withcolor (if odd v: .8 white else: .9 white fi);
    drawboxed(b.v);
  endfor;

  boxjoin();
  boxit b8(t[8]); width b8 = (u,0); b8.nw = b0.sw;
  fill bpath b8 withcolor .9 white;
  drawboxed(b8);

  boxjoin(a.ne = b.nw; a.se = b.sw);

  forsuffixes v = 9,10,11,12:
    boxit b.v(t[v]);
    width b.v = (u,0);
  endfor;

  b9.nw = b8.ne;

  forsuffixes v = 9,10,11,12:
    fill bpath b.v withcolor (if odd v: .8 white else: .9 white fi);
    drawboxed(b.v);
  endfor;

  boxit b13(t[13]);
  width b13 = (2u,0);

  fill bpath b13 withcolor .85 white;
  drawboxed(b13);

  boxit b14("'\0'");
  width b14 = (u,0);
  fill bpath b14 withcolor .9 white;
  drawboxed(b14);

  elflabel (b0.n, (-.5u,1u), ulft, right) "Initial NUL";
  elflabel (b6.n, (.6u,1u), urt, left) "NUL-separated strings";
  elflabel (b14.e, (.6u,.8u), urt, left) "Terminating NUL";
endfig;

end
